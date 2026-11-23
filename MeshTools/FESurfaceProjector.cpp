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
#include "FESurfaceProjector.h"
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
using namespace std;

namespace {

	// closest point on triangle (a,b,c) to p (Ericson, Real-Time Collision Detection, 5.1.5)
	vec3d closestPointTriangle(const vec3d& p, const vec3d& a, const vec3d& b, const vec3d& c)
	{
		vec3d ab = b - a, ac = c - a, ap = p - a;
		double d1 = ab * ap, d2 = ac * ap;
		if ((d1 <= 0) && (d2 <= 0)) return a;

		vec3d bp = p - b;
		double d3 = ab * bp, d4 = ac * bp;
		if ((d3 >= 0) && (d4 <= d3)) return b;

		double vc = d1 * d4 - d3 * d2;
		if ((vc <= 0) && (d1 >= 0) && (d3 <= 0)) return a + ab * (d1 / (d1 - d3));

		vec3d cp = p - c;
		double d5 = ab * cp, d6 = ac * cp;
		if ((d6 >= 0) && (d5 <= d6)) return c;

		double vb = d5 * d2 - d1 * d6;
		if ((vb <= 0) && (d2 >= 0) && (d6 <= 0)) return a + ac * (d2 / (d2 - d6));

		double va = d3 * d6 - d5 * d4;
		if ((va <= 0) && (d4 - d3 >= 0) && (d5 - d6 >= 0)) return b + (c - b) * ((d4 - d3) / ((d4 - d3) + (d5 - d6)));

		double den = 1.0 / (va + vb + vc);
		return a + ab * (vb * den) + ac * (vc * den);
	}

	// closest point on segment (a,b) to p
	vec3d closestPointSegment(const vec3d& p, const vec3d& a, const vec3d& b)
	{
		vec3d ab = b - a;
		double L2 = ab * ab;
		if (L2 <= 0) return a;
		double t = ((p - a) * ab) / L2;
		t = max(0.0, min(1.0, t));
		return a + ab * t;
	}
}

//-----------------------------------------------------------------------------
void FESurfaceProjector::Grid::insert(int item, const vec3d& bmin, const vec3d& bmax)
{
	int i0 = max(0, (int)floor((bmin.x - box0.x) / h)), i1 = min(nx - 1, (int)floor((bmax.x - box0.x) / h));
	int j0 = max(0, (int)floor((bmin.y - box0.y) / h)), j1 = min(ny - 1, (int)floor((bmax.y - box0.y) / h));
	int k0 = max(0, (int)floor((bmin.z - box0.z) / h)), k1 = min(nz - 1, (int)floor((bmax.z - box0.z) / h));
	for (int k = k0; k <= k1; ++k)
		for (int j = j0; j <= j1; ++j)
			for (int i = i0; i <= i1; ++i) cell[key(i, j, k)].push_back(item);
}

//-----------------------------------------------------------------------------
FESurfaceProjector::FESurfaceProjector() {}

//-----------------------------------------------------------------------------
void FESurfaceProjector::Build(const vector<vec3d>& nodes, const vector<array<int, 3> >& tris, double featureAngle)
{
	m_node = nodes;
	m_tri = tris;
	m_triNormal.clear();
	m_seg.clear();
	m_segCurve.clear();
	m_corner.clear();
	m_triPatch.clear();
	m_patches = 0;
	m_curve.clear();
	m_triGrid = Grid();
	m_segGrid = Grid();
	if (m_tri.empty()) return;

	// bounding box and average edge length
	vec3d b0 = m_node[m_tri[0][0]], b1 = b0;
	double Lsum = 0;
	for (auto& t : m_tri)
		for (int j = 0; j < 3; ++j)
		{
			const vec3d& r = m_node[t[j]];
			b0.x = min(b0.x, r.x); b0.y = min(b0.y, r.y); b0.z = min(b0.z, r.z);
			b1.x = max(b1.x, r.x); b1.y = max(b1.y, r.y); b1.z = max(b1.z, r.z);
			Lsum += (m_node[t[(j + 1) % 3]] - r).Length();
		}
	double diag = (b1 - b0).Length();
	if (diag <= 0) diag = 1.0;
	double h = 2.0 * Lsum / (3.0 * m_tri.size());
	if (h <= 0) h = diag;

	// grid dimensions (limit the number of cells)
	vec3d pad(1e-6 * diag, 1e-6 * diag, 1e-6 * diag);
	b0 -= pad; b1 += pad;
	int nx, ny, nz;
	while (true)
	{
		nx = max(1, (int)ceil((b1.x - b0.x) / h));
		ny = max(1, (int)ceil((b1.y - b0.y) / h));
		nz = max(1, (int)ceil((b1.z - b0.z) / h));
		if ((double)nx * ny * nz <= 8e6) break;
		h *= 1.5;
	}
	m_triGrid.box0 = b0; m_triGrid.h = h; m_triGrid.nx = nx; m_triGrid.ny = ny; m_triGrid.nz = nz;
	m_segGrid.box0 = b0; m_segGrid.h = h; m_segGrid.nx = nx; m_segGrid.ny = ny; m_segGrid.nz = nz;

	int NT = (int)m_tri.size();
	m_triNormal.resize(NT);
	for (int i = 0; i < NT; ++i)
	{
		const vec3d& a = m_node[m_tri[i][0]];
		const vec3d& b = m_node[m_tri[i][1]];
		const vec3d& c = m_node[m_tri[i][2]];
		vec3d n = (b - a) ^ (c - a); n.unit();
		m_triNormal[i] = n;

		vec3d tmin = a, tmax = a;
		for (const vec3d* r : { &b, &c })
		{
			tmin.x = min(tmin.x, r->x); tmin.y = min(tmin.y, r->y); tmin.z = min(tmin.z, r->z);
			tmax.x = max(tmax.x, r->x); tmax.y = max(tmax.y, r->y); tmax.z = max(tmax.z, r->z);
		}
		m_triGrid.insert(i, tmin, tmax);
	}

	// feature edges: non-manifold edges and edges with a large dihedral angle
	const double kPi = 3.14159265358979323846;
	double cosFeature = cos(featureAngle * kPi / 180.0);
	map<pair<int, int>, vector<int> > edgeTris;
	for (int i = 0; i < NT; ++i)
		for (int j = 0; j < 3; ++j)
		{
			int a = m_tri[i][j], b = m_tri[i][(j + 1) % 3];
			edgeTris[make_pair(min(a, b), max(a, b))].push_back(i);
		}
	for (auto& it : edgeTris)
	{
		const vector<int>& et = it.second;
		bool feature = (et.size() != 2) || (m_triNormal[et[0]] * m_triNormal[et[1]] < cosFeature);
		if (feature) m_seg.push_back({ it.first.first, it.first.second });
	}
	for (int i = 0; i < (int)m_seg.size(); ++i)
	{
		const vec3d& a = m_node[m_seg[i][0]];
		const vec3d& b = m_node[m_seg[i][1]];
		vec3d smin(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
		vec3d smax(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
		m_segGrid.insert(i, smin, smax);
	}

	// corners: ends of feature curves, junctions of three or more feature edges, and
	// sharp turns of a feature curve
	map<int, vector<int> > nodeSegs;
	for (auto& s : m_seg) { nodeSegs[s[0]].push_back(s[1]); nodeSegs[s[1]].push_back(s[0]); }
	for (auto& it : nodeSegs)
	{
		const vector<int>& nb = it.second;
		bool corner = (nb.size() != 2);
		if (!corner)
		{
			const vec3d& r = m_node[it.first];
			vec3d u1 = m_node[nb[0]] - r; u1.unit();
			vec3d u2 = m_node[nb[1]] - r; u2.unit();
			corner = (u1 * u2 > -cosFeature);
		}
		if (corner) m_corner.push_back(it.first);
	}

	// patches: triangles connected across edges that are not feature edges
	set<pair<int, int> > featureSet;
	for (auto& sg : m_seg) featureSet.insert(make_pair(min(sg[0], sg[1]), max(sg[0], sg[1])));
	m_triPatch.assign(NT, -1);
	for (int t0 = 0; t0 < NT; ++t0)
	{
		if (m_triPatch[t0] >= 0) continue;
		vector<int> stack(1, t0);
		m_triPatch[t0] = m_patches;
		while (!stack.empty())
		{
			int t = stack.back(); stack.pop_back();
			for (int j = 0; j < 3; ++j)
			{
				int a = m_tri[t][j], b = m_tri[t][(j + 1) % 3];
				pair<int, int> e(min(a, b), max(a, b));
				if (featureSet.count(e)) continue;
				for (int u : edgeTris[e]) if (m_triPatch[u] < 0) { m_triPatch[u] = m_patches; stack.push_back(u); }
			}
		}
		m_patches++;
	}

	// curves: walk along the feature edges from corner to corner; what is left are closed loops
	map<int, vector<int> > nodeSegIds;
	for (int i = 0; i < (int)m_seg.size(); ++i) { nodeSegIds[m_seg[i][0]].push_back(i); nodeSegIds[m_seg[i][1]].push_back(i); }
	set<int> cornerSet(m_corner.begin(), m_corner.end());
	m_segCurve.assign(m_seg.size(), -1);
	auto walk = [&](int start, int seg0, bool closed) {
		Curve C;
		C.closed = closed;
		C.nodes.push_back(start);
		int node = start, seg = seg0;
		while (seg >= 0)
		{
			m_segCurve[seg] = (int)m_curve.size();
			int next = (m_seg[seg][0] == node ? m_seg[seg][1] : m_seg[seg][0]);
			if (closed && (next == start)) break;
			C.nodes.push_back(next);
			node = next;
			if (!closed && cornerSet.count(node)) break;
			seg = -1;
			for (int k : nodeSegIds[node]) if (m_segCurve[k] < 0) { seg = k; break; }
		}
		C.s.assign(C.nodes.size(), 0.0);
		for (size_t k = 1; k < C.nodes.size(); ++k) C.s[k] = C.s[k - 1] + (m_node[C.nodes[k]] - m_node[C.nodes[k - 1]]).Length();
		C.length = C.s.back() + (closed ? (m_node[C.nodes[0]] - m_node[C.nodes.back()]).Length() : 0.0);
		m_curve.push_back(C);
	};
	for (int c : m_corner)
		for (int k : nodeSegIds[c]) if (m_segCurve[k] < 0) walk(c, k, false);
	for (int k = 0; k < (int)m_seg.size(); ++k)
		if (m_segCurve[k] < 0) walk(m_seg[k][0], k, true);
}

//-----------------------------------------------------------------------------
vec3d FESurfaceProjector::CurvePoint(int curve, double s, vec3d* tangent) const
{
	const Curve& C = m_curve[curve];
	int n = (int)C.nodes.size();
	if (n == 1) { if (tangent) *tangent = vec3d(1, 0, 0); return m_node[C.nodes[0]]; }
	if (C.closed) { s = fmod(s, C.length); if (s < 0) s += C.length; }
	else s = max(0.0, min(C.length, s));

	// find the edge (k, k+1) that contains s (for a closed curve, the last edge closes the loop)
	int nedges = (C.closed ? n : n - 1);
	int k = (int)(upper_bound(C.s.begin(), C.s.end(), s) - C.s.begin()) - 1;
	k = max(0, min(nedges - 1, k));
	const vec3d& a = m_node[C.nodes[k]];
	const vec3d& b = m_node[C.nodes[(k + 1) % n]];
	double s0 = C.s[k];
	double s1 = (k + 1 < n ? C.s[k + 1] : C.length);
	double w = (s1 > s0 ? (s - s0) / (s1 - s0) : 0.0);
	w = max(0.0, min(1.0, w));
	if (tangent) { *tangent = b - a; tangent->unit(); }
	return a + (b - a) * w;
}

//-----------------------------------------------------------------------------
template <class F> void FESurfaceProjector::search(const Grid& g, const vec3d& p, F dist) const
{
	if (g.cell.empty()) return;

	// cell of p (not clamped: p may lie outside the grid)
	double fi = floor((p.x - g.box0.x) / g.h);
	double fj = floor((p.y - g.box0.y) / g.h);
	double fk = floor((p.z - g.box0.z) / g.h);
	const double big = 1e8;
	int ci = (int)max(-big, min(big, fi));
	int cj = (int)max(-big, min(big, fj));
	int ck = (int)max(-big, min(big, fk));

	double best = 1e300;
	auto visit = [&](int i, int j, int k) {
		auto it = g.cell.find(g.key(i, j, k));
		if (it == g.cell.end()) return;
		for (int item : it->second) best = min(best, dist(item));
	};

	for (int r = 0; ; ++r)
	{
		int i0 = ci - r, i1 = ci + r;
		int j0 = cj - r, j1 = cj + r;
		int k0 = ck - r, k1 = ck + r;
		for (int k = max(k0, 0); k <= min(k1, g.nz - 1); ++k)
			for (int j = max(j0, 0); j <= min(j1, g.ny - 1); ++j)
			{
				if ((k == k0) || (k == k1) || (j == j0) || (j == j1))
				{
					for (int i = max(i0, 0); i <= min(i1, g.nx - 1); ++i) visit(i, j, k);
				}
				else
				{
					if ((i0 >= 0) && (i0 < g.nx)) visit(i0, j, k);
					if ((i1 >= 0) && (i1 < g.nx) && (i1 != i0)) visit(i1, j, k);
				}
			}

		// all cells outside shell r are at least r*h away from p
		if (best <= r * g.h) break;

		// stop when the shell covers the whole grid
		if ((i0 <= 0) && (j0 <= 0) && (k0 <= 0) && (i1 >= g.nx - 1) && (j1 >= g.ny - 1) && (k1 >= g.nz - 1)) break;
	}
}

//-----------------------------------------------------------------------------
bool FESurfaceProjector::ClosestPoint(const vec3d& p, vec3d& q, vec3d& n, int patch, int* patchOut) const
{
	int imin = -1;
	double dmin = 1e300;
	search(m_triGrid, p, [&](int i) {
		if ((patch >= 0) && (m_triPatch[i] != patch)) return 1e300;
		const array<int, 3>& t = m_tri[i];
		vec3d c = closestPointTriangle(p, m_node[t[0]], m_node[t[1]], m_node[t[2]]);
		double d = (c - p).Length();
		if (d < dmin) { dmin = d; imin = i; q = c; }
		return d;
	});
	if (imin < 0) return false;
	n = m_triNormal[imin];
	if (patchOut) *patchOut = m_triPatch[imin];
	return true;
}

//-----------------------------------------------------------------------------
bool FESurfaceProjector::ClosestFeaturePoint(const vec3d& p, vec3d& q, vec3d& t, int curve) const
{
	int imin = -1;
	double dmin = 1e300;
	search(m_segGrid, p, [&](int i) {
		if ((curve >= 0) && (m_segCurve[i] != curve)) return 1e300;
		const vec3d& a = m_node[m_seg[i][0]];
		const vec3d& b = m_node[m_seg[i][1]];
		vec3d c = closestPointSegment(p, a, b);
		double d = (c - p).Length();
		if (d < dmin) { dmin = d; imin = i; q = c; }
		return d;
	});
	if (imin < 0) return false;
	t = m_node[m_seg[imin][1]] - m_node[m_seg[imin][0]];
	t.unit();
	return true;
}

//-----------------------------------------------------------------------------
bool FESurfaceProjector::ClosestCorner(const vec3d& p, vec3d& q) const
{
	double dmin = 1e300;
	for (int n : m_corner)
	{
		double d = (m_node[n] - p).SqrLength();
		if (d < dmin) { dmin = d; q = m_node[n]; }
	}
	return !m_corner.empty();
}
