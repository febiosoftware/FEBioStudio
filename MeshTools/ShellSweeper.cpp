/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "stdafx.h"
#include "ShellSweeper.h"
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSMeshBuilder.h>
#include <FECore/FETransform.h>
#include <map>
#include <set>
#include <tuple>
#include <cmath>
#include <algorithm>

ShellSweeper::ShellSweeper()
{
	m_layers = 0;
	m_rotate = true;
	m_autoPartition = false;
	m_creaseAngle = 30.0;
}

//-----------------------------------------------------------------------------
bool ShellSweeper::ChainSegments(const std::vector<PathSegment>& segs, const vec3d& nearPoint, std::vector<vec3d>& path, std::string& err)
{
	path.clear();
	if (segs.empty()) { err = "The path has no segments."; return false; }

	// weld the end points
	double L = 0;
	for (const PathSegment& s : segs) L = std::max(L, (s.b - s.a).norm());
	double h = 1e-6 * std::max(L, 1e-12);
	typedef std::tuple<long long, long long, long long> Key;
	std::map<Key, int> uniq;
	std::vector<vec3d> pts;
	auto getPoint = [&](const vec3d& r) {
		Key k((long long)std::llround(r.x / h), (long long)std::llround(r.y / h), (long long)std::llround(r.z / h));
		auto it = uniq.find(k);
		if (it == uniq.end()) { it = uniq.insert({ k, (int)pts.size() }).first; pts.push_back(r); }
		return it->second;
	};

	// Note that the same segment can appear more than once (e.g. an edge shared by two faces is
	// often listed once per face), so duplicates are ignored.
	std::set<std::pair<int, int>> used;
	std::vector<std::vector<int>> adj;
	for (const PathSegment& s : segs)
	{
		int a = getPoint(s.a), b = getPoint(s.b);
		if (a == b) continue;
		if (!used.insert({ std::min(a, b), std::max(a, b) }).second) continue;
		if ((int)adj.size() < (int)pts.size()) adj.resize(pts.size());
		adj[a].push_back(b);
		adj[b].push_back(a);
	}
	adj.resize(pts.size());

	std::vector<int> ends;
	for (int i = 0; i < (int)pts.size(); ++i)
	{
		if (adj[i].size() > 2) { err = "The path branches. Select edges that form a single, unbranched curve."; return false; }
		if (adj[i].size() == 1) ends.push_back(i);
	}
	if (ends.size() != 2) { err = "The path must be a single open curve (a closed loop or several disconnected pieces were found)."; return false; }

	// start at the end closest to the given point
	int start = ends[0];
	if ((pts[ends[1]] - nearPoint).norm2() < (pts[ends[0]] - nearPoint).norm2()) start = ends[1];

	int prev = -1, cur = start;
	while (true)
	{
		path.push_back(pts[cur]);
		int next = -1;
		for (int n : adj[cur]) if (n != prev) next = n;
		if (next == -1) break;
		prev = cur;
		cur = next;
	}
	if (path.size() < 2) { err = "The path is degenerate."; return false; }
	return true;
}

//-----------------------------------------------------------------------------
std::vector<vec3d> ShellSweeper::Resample(const std::vector<vec3d>& path, int n)
{
	std::vector<double> s(path.size(), 0.0);
	for (size_t i = 1; i < path.size(); ++i) s[i] = s[i - 1] + (path[i] - path[i - 1]).norm();
	double total = s.back();

	std::vector<vec3d> out;
	size_t seg = 1;
	for (int k = 0; k <= n; ++k)
	{
		double t = total * (double)k / (double)n;
		while ((seg < path.size() - 1) && (s[seg] < t)) ++seg;
		double ds = s[seg] - s[seg - 1];
		double w = (ds > 0) ? (t - s[seg - 1]) / ds : 0.0;
		w = std::min(1.0, std::max(0.0, w));
		out.push_back(path[seg - 1] * (1.0 - w) + path[seg] * w);
	}
	return out;
}

//-----------------------------------------------------------------------------
namespace {

	struct Mat3 {
		double m[3][3];
		Mat3() { for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) m[i][j] = (i == j) ? 1.0 : 0.0; }
		vec3d operator * (const vec3d& v) const {
			return vec3d(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
				m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
				m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
		}
		Mat3 operator * (const Mat3& b) const {
			Mat3 c;
			for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) {
				c.m[i][j] = 0;
				for (int k = 0; k < 3; ++k) c.m[i][j] += m[i][k] * b.m[k][j];
			}
			return c;
		}
	};

	// minimal rotation that takes unit vector a to unit vector b
	Mat3 RotateTo(const vec3d& a, const vec3d& b)
	{
		Mat3 R;
		vec3d v = a ^ b;
		double c = a * b;
		double s = v.norm();
		if (s < 1e-12)
		{
			if (c > 0) return R;
			// anti-parallel: rotate 180 degrees about any axis perpendicular to a
			vec3d axis = (fabs(a.x) < 0.9) ? (a ^ vec3d(1, 0, 0)) : (a ^ vec3d(0, 1, 0));
			axis /= axis.norm();
			for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) R.m[i][j] = 0;
			double u[3] = { axis.x, axis.y, axis.z };
			for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) R.m[i][j] = 2.0 * u[i] * u[j] - (i == j ? 1.0 : 0.0);
			return R;
		}
		// Rodrigues' formula
		vec3d k = v / s;
		double K[3][3] = { { 0, -k.z, k.y }, { k.z, 0, -k.x }, { -k.y, k.x, 0 } };
		double K2[3][3];
		for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) {
			K2[i][j] = 0;
			for (int l = 0; l < 3; ++l) K2[i][j] += K[i][l] * K[l][j];
		}
		for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j)
			R.m[i][j] = (i == j ? 1.0 : 0.0) + s * K[i][j] + (1.0 - c) * K2[i][j];
		return R;
	}
}

//-----------------------------------------------------------------------------
FSMesh* ShellSweeper::Apply(const FSMesh& shell, const Transform& T)
{
	m_err.clear();
	if (m_path.size() < 2) { m_err = "No sweep path has been set."; return nullptr; }

	// chords of the original path
	const int np = (int)m_path.size() - 1;		// number of original segments
	std::vector<vec3d> chord(np);
	std::vector<double> clen(np);
	for (int i = 0; i < np; ++i)
	{
		vec3d c = m_path[i + 1] - m_path[i];
		double L = c.norm();
		if (L == 0) { m_err = "The path has zero-length segments."; return nullptr; }
		chord[i] = c / L;
		clen[i] = L;
	}

	// Tangents at the original path points: at interior points the bisector of the two adjacent chords (exact
	// for points on a circular arc). At the end points there is only one chord, which is off the true tangent by
	// about half a segment's turning angle, so extrapolate from the neighbouring chord.
	std::vector<vec3d> tv(np + 1);
	for (int i = 1; i < np; ++i)
	{
		vec3d t = chord[i - 1] + chord[i];
		if (t.norm() < 1e-12) t = chord[i];
		tv[i] = t / t.norm();
	}
	tv[0] = chord[0];
	tv[np] = chord[np - 1];
	if (np >= 2)
	{
		vec3d t0 = chord[0] + (chord[0] - chord[1]) * 0.5;
		vec3d tn = chord[np - 1] + (chord[np - 1] - chord[np - 2]) * 0.5;
		if (t0.norm() > 1e-12) tv[0] = t0 / t0.norm();
		if (tn.norm() > 1e-12) tv[np] = tn / tn.norm();
	}

	// the path points and tangents used for the layers
	std::vector<vec3d> P, tang;
	if (m_layers <= 0)
	{
		P = m_path;
		tang = tv;
	}
	else
	{
		// Sample the path at equal arclength using a cubic Hermite spline through the original points with the
		// tangents above. (Interpolating linearly would cut the corners of a curved path, and would lose the
		// curvature near the ends if the path has fewer segments than layers.)
		double total = 0;
		for (int i = 0; i < np; ++i) total += clen[i];
		int seg = 0;
		double s0 = 0;
		for (int k = 0; k <= m_layers; ++k)
		{
			double t = total * (double)k / (double)m_layers;
			while ((seg < np - 1) && (s0 + clen[seg] < t)) { s0 += clen[seg]; ++seg; }
			double u = (t - s0) / clen[seg];
			u = std::min(1.0, std::max(0.0, u));

			double u2 = u * u, u3 = u2 * u;
			double h00 = 2 * u3 - 3 * u2 + 1, h10 = u3 - 2 * u2 + u, h01 = -2 * u3 + 3 * u2, h11 = u3 - u2;
			double d00 = 6 * u2 - 6 * u, d10 = 3 * u2 - 4 * u + 1, d01 = -6 * u2 + 6 * u, d11 = 3 * u2 - 2 * u;
			vec3d m0 = tv[seg] * clen[seg], m1 = tv[seg + 1] * clen[seg];
			vec3d pt = m_path[seg] * h00 + m0 * h10 + m_path[seg + 1] * h01 + m1 * h11;
			vec3d dt = m_path[seg] * d00 + m0 * d10 + m_path[seg + 1] * d01 + m1 * d11;
			if (k == 0) { pt = m_path[0]; dt = tv[0]; }
			if (k == m_layers) { pt = m_path[np]; dt = tv[np]; }
			if (dt.norm() < 1e-12) dt = chord[seg];
			P.push_back(pt);
			tang.push_back(dt / dt.norm());
		}
	}
	const int nl = (int)P.size() - 1;
	if (nl < 1) { m_err = "The path is too short."; return nullptr; }

	// collect the shell elements and nodes
	const int N0 = shell.Nodes();
	std::vector<int> nodeMap(N0, -1);
	std::vector<int> elems;
	int nn = 0;
	for (int i = 0; i < shell.Elements(); ++i)
	{
		const FSElement& el = shell.Element(i);
		int ne = 0;
		if (el.Type() == FE_TRI3) ne = 3;
		else if (el.Type() == FE_QUAD4) ne = 4;
		else { m_err = "The section must be a linear shell mesh of triangles and quads (second-order shells are not supported)."; return nullptr; }
		elems.push_back(i);
		for (int k = 0; k < ne; ++k)
			if (nodeMap[el.m_node[k]] == -1) nodeMap[el.m_node[k]] = nn++;
	}
	if (elems.empty()) { m_err = "The section has no shell elements."; return nullptr; }

	// section node positions (global)
	std::vector<vec3d> r0(nn);
	for (int i = 0; i < N0; ++i) if (nodeMap[i] >= 0) r0[nodeMap[i]] = T.LocalToGlobal(shell.Node(i).r);

	// the section is attached to the first path point; each layer applies the accumulated transport rotation
	std::vector<vec3d> pos((nl + 1) * nn);
	Mat3 R;
	for (int l = 0; l <= nl; ++l)
	{
		if ((l > 0) && m_rotate) R = RotateTo(tang[l - 1], tang[l]) * R;
		for (int i = 0; i < nn; ++i)
		{
			vec3d d = r0[i] - P[0];
			pos[l * nn + i] = P[l] + (m_rotate ? (R * d) : d);
		}
	}

	// orientation of each element: the base face normal must point towards the top face
	std::vector<bool> flip(elems.size(), false);
	for (size_t e = 0; e < elems.size(); ++e)
	{
		const FSElement& el = shell.Element(elems[e]);
		int ne = (el.Type() == FE_TRI3 ? 3 : 4);
		int v[4];
		for (int k = 0; k < ne; ++k) v[k] = nodeMap[el.m_node[k]];
		vec3d N;
		if (ne == 3) N = (r0[v[1]] - r0[v[0]]) ^ (r0[v[2]] - r0[v[0]]);
		else N = (r0[v[2]] - r0[v[0]]) ^ (r0[v[3]] - r0[v[1]]);
		vec3d dm(0, 0, 0);
		for (int k = 0; k < ne; ++k) dm += pos[nn + v[k]] - pos[v[k]];
		if (N * dm < 0) flip[e] = true;
	}

	FSMesh* mesh = new FSMesh();
	mesh->Create(nn * (nl + 1), (int)elems.size() * nl);
	for (int i = 0; i < (int)pos.size(); ++i) mesh->Node(i).r = pos[i];

	int ie = 0;
	for (int l = 0; l < nl; ++l)
	{
		for (size_t e = 0; e < elems.size(); ++e)
		{
			const FSElement& sel = shell.Element(elems[e]);
			FSElement& el = mesh->Element(ie++);
			el.m_gid = 0;
			int ne = (sel.Type() == FE_TRI3 ? 3 : 4);
			int v[4];
			for (int k = 0; k < ne; ++k) v[k] = nodeMap[sel.m_node[k]];
			if (flip[e])
			{
				if (ne == 3) std::swap(v[1], v[2]);
				else std::swap(v[1], v[3]);
			}
			int b = l * nn, t = (l + 1) * nn;
			if (ne == 3)
			{
				el.SetType(FE_PENTA6);
				for (int k = 0; k < 3; ++k) { el.m_node[k] = b + v[k]; el.m_node[k + 3] = t + v[k]; }
			}
			else
			{
				el.SetType(FE_HEX8);
				for (int k = 0; k < 4; ++k) { el.m_node[k] = b + v[k]; el.m_node[k + 4] = t + v[k]; }
			}
		}
	}

	mesh->RebuildMesh();
	if (m_autoPartition)
	{
		FSMeshBuilder meshBuilder(*mesh);
		meshBuilder.AutoPartition(m_creaseAngle);
	}
	return mesh;
}
