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
#include "ShellLayers.h"
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
	m_collapseTol = 0.0;
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
	if (!m_scale.empty() && (m_scale.size() != m_path.size())) { m_err = "The scale list does not match the path."; return nullptr; }
	if (!m_orient.empty() && (m_orient.size() != m_path.size())) { m_err = "The orientation list does not match the path."; return nullptr; }

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

	// analyze the section
	ShellSection sec;
	if (!sec.Init(shell, m_err)) return nullptr;
	const int nn = sec.nn;
	const int step = (sec.quadratic ? 2 : 1);	// quadratic elements need stations half way between the layers

	// Stations along the path (positions and tangents). Hermite spline through the original points with the
	// tangents above (interpolating linearly would cut the corners of a curved path).
	std::vector<std::pair<int, double>> st;		// (segment, parameter)
	if (m_layers <= 0)
	{
		for (int i = 0; i < np; ++i)
		{
			st.push_back({ i, 0.0 });
			if (step == 2) st.push_back({ i, 0.5 });
		}
		st.push_back({ np - 1, 1.0 });
	}
	else
	{
		double total = 0;
		for (int i = 0; i < np; ++i) total += clen[i];
		const int nst = step * m_layers;
		int seg = 0;
		double s0 = 0;
		for (int k = 0; k <= nst; ++k)
		{
			double t = total * (double)k / (double)nst;
			while ((seg < np - 1) && (s0 + clen[seg] < t)) { s0 += clen[seg]; ++seg; }
			double u = (t - s0) / clen[seg];
			u = std::min(1.0, std::max(0.0, u));
			st.push_back({ seg, u });
		}
	}

	std::vector<vec3d> P, tang;
	std::vector<double> sc;
	std::vector<vec3d> orient;
	for (size_t k = 0; k < st.size(); ++k)
	{
		int seg = st[k].first;
		double u = st[k].second;
		if (!m_orient.empty())
		{
			vec3d o = m_orient[seg] * (1.0 - u) + m_orient[seg + 1] * u;
			double L = o.norm();
			orient.push_back(L > 1e-12 ? o / L : m_orient[seg]);
		}
		sc.push_back(m_scale.empty() ? 1.0 : (m_scale[seg] * (1.0 - u) + m_scale[seg + 1] * u));
		double u2 = u * u, u3 = u2 * u;
		double h00 = 2 * u3 - 3 * u2 + 1, h10 = u3 - 2 * u2 + u, h01 = -2 * u3 + 3 * u2, h11 = u3 - u2;
		double d00 = 6 * u2 - 6 * u, d10 = 3 * u2 - 4 * u + 1, d01 = -6 * u2 + 6 * u, d11 = 3 * u2 - 2 * u;
		vec3d m0 = tv[seg] * clen[seg], m1 = tv[seg + 1] * clen[seg];
		vec3d pt = m_path[seg] * h00 + m0 * h10 + m_path[seg + 1] * h01 + m1 * h11;
		vec3d dt = m_path[seg] * d00 + m0 * d10 + m_path[seg + 1] * d01 + m1 * d11;
		if (k == 0) { pt = m_path[0]; dt = tv[0]; }
		if (k == st.size() - 1) { pt = m_path[np]; dt = tv[np]; }
		if (dt.norm() < 1e-12) dt = chord[seg];
		P.push_back(pt);
		tang.push_back(dt / dt.norm());
	}
	const int nst = (int)P.size() - 1;
	const int nl = nst / step;
	if (nl < 1) { m_err = "The path is too short."; return nullptr; }

	// section node positions (global)
	std::vector<vec3d> r0(nn);
	for (int i = 0; i < nn; ++i) r0[i] = T.LocalToGlobal(shell.Node(sec.base[i]).r);

	// the section is attached to the first path point; each station applies the accumulated transport rotation
	std::vector<vec3d> layerPos((nl + 1) * nn);
	std::vector<vec3d> midPos(sec.quadratic ? nl * sec.ncorner : 0);
	Mat3 R;
	// with prescribed orientations, the section's own normal at the start is orient[0]; first rotate it onto the path tangent
	if (m_rotate && !orient.empty()) R = RotateTo(orient[0], tang[0]);
	for (int k = 0; k <= nst; ++k)
	{
		if ((k > 0) && m_rotate) R = RotateTo(tang[k - 1], tang[k]) * R;
		Mat3 Rk = R;
		if (m_rotate && !orient.empty()) Rk = RotateTo(tang[k], orient[k]) * R;
		if (k % step == 0)
		{
			int l = k / step;
			for (int i = 0; i < nn; ++i)
			{
				vec3d d = (r0[i] - P[0]) * sc[k];
				layerPos[l * nn + i] = P[k] + (m_rotate ? (Rk * d) : d);
			}
		}
		else
		{
			int l = k / step;
			for (int c = 0; c < sec.ncorner; ++c)
			{
				vec3d d = (r0[sec.cornerNode[c]] - P[0]) * sc[k];
				midPos[l * sec.ncorner + c] = P[k] + (m_rotate ? (Rk * d) : d);
			}
		}
	}

	FSMesh* mesh = sec.BuildSolid(r0, nl, layerPos, midPos);
	if (!FinishSolidMesh(*mesh, m_collapseTol, m_autoPartition, m_creaseAngle, m_err))
	{
		delete mesh;
		return nullptr;
	}
	return mesh;
}
