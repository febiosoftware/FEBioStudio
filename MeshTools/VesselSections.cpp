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
#include "VesselSections.h"
#include <map>
#include <set>
#include <tuple>
#include <cmath>
#include <algorithm>

namespace {
	struct Basis { vec3d e1, e2; };
	Basis MakeBasis(const vec3d& n)
	{
		vec3d a = (fabs(n.x) < 0.9) ? vec3d(1, 0, 0) : vec3d(0, 1, 0);
		Basis b;
		b.e1 = n ^ a; b.e1 /= b.e1.norm();
		b.e2 = n ^ b.e1; b.e2 /= b.e2.norm();
		return b;
	}

	bool InsidePolygon2D(const std::vector<double>& x, const std::vector<double>& y, double px, double py)
	{
		bool in = false;
		int n = (int)x.size();
		for (int i = 0, j = n - 1; i < n; j = i++)
		{
			if (((y[i] > py) != (y[j] > py)) && (px < (x[j] - x[i]) * (py - y[i]) / (y[j] - y[i]) + x[i])) in = !in;
		}
		return in;
	}
}

//-----------------------------------------------------------------------------
void VesselSections::PolygonInfo(const std::vector<vec3d>& p, Poly& out)
{
	out.pts = p;
	int n = (int)p.size();
	vec3d N(0, 0, 0);
	for (int i = 0; i < n; ++i) N += p[i] ^ p[(i + 1) % n];
	N *= 0.5;
	double A = N.norm();
	out.area = A;
	out.n = (A > 0 ? N / A : vec3d(0, 0, 1));
	// centroid by a triangle fan
	vec3d c(0, 0, 0);
	double wsum = 0;
	for (int k = 1; k + 1 < n; ++k)
	{
		double w = (((p[k] - p[0]) ^ (p[k + 1] - p[0])) * 0.5) * out.n;
		c += (p[0] + p[k] + p[k + 1]) * (w / 3.0);
		wsum += w;
	}
	out.c = (fabs(wsum) > 0 ? c / wsum : p[0]);
}

//-----------------------------------------------------------------------------
bool VesselSections::TriangulatePolygon(const std::vector<vec3d>& poly, std::vector<int>& tri)
{
	tri.clear();
	int n = (int)poly.size();
	if (n < 3) return false;
	Poly info;
	PolygonInfo(poly, info);
	Basis B = MakeBasis(info.n);
	std::vector<double> x(n), y(n);
	for (int i = 0; i < n; ++i) { x[i] = (poly[i] - info.c) * B.e1; y[i] = (poly[i] - info.c) * B.e2; }
	// counter-clockwise in the (e1,e2) basis since n = e1 x e2 and the polygon's area vector is along n
	std::vector<int> idx(n);
	for (int i = 0; i < n; ++i) idx[i] = i;

	auto cross = [&](int a, int b, int c) { return (x[b] - x[a]) * (y[c] - y[a]) - (y[b] - y[a]) * (x[c] - x[a]); };
	int guard = 0;
	while ((int)idx.size() > 3 && guard++ < 100000)
	{
		bool clipped = false;
		int m = (int)idx.size();
		for (int i = 0; i < m; ++i)
		{
			int a = idx[(i + m - 1) % m], b = idx[i], c = idx[(i + 1) % m];
			if (cross(a, b, c) <= 1e-14) continue;	// reflex or degenerate
			bool ear = true;
			for (int j = 0; j < m && ear; ++j)
			{
				int q = idx[j];
				if ((q == a) || (q == b) || (q == c)) continue;
				if ((cross(a, b, q) >= 0) && (cross(b, c, q) >= 0) && (cross(c, a, q) >= 0)) ear = false;
			}
			if (!ear) continue;
			tri.push_back(a); tri.push_back(b); tri.push_back(c);
			idx.erase(idx.begin() + i);
			clipped = true;
			break;
		}
		if (!clipped)
		{
			// no ear found (degenerate polygon): clip the vertex with the smallest |cross| to make progress
			int m2 = (int)idx.size();
			int best = 0; double bv = 1e300;
			for (int i = 0; i < m2; ++i)
			{
				double v = fabs(cross(idx[(i + m2 - 1) % m2], idx[i], idx[(i + 1) % m2]));
				if (v < bv) { bv = v; best = i; }
			}
			idx.erase(idx.begin() + best);
		}
	}
	if (idx.size() == 3 && cross(idx[0], idx[1], idx[2]) > 0) { tri.push_back(idx[0]); tri.push_back(idx[1]); tri.push_back(idx[2]); }
	return !tri.empty();
}

//-----------------------------------------------------------------------------
bool VesselSections::RemoveCaps(const std::vector<vec3d>& pts, std::vector<int>& tri, const std::vector<int>& startTris, vec3d& startCentroid, std::vector<int>& startCapTris, std::string& err, std::vector<std::vector<int>>* allCapTris, std::vector<vec3d>* allCapCentroids)
{
	const int NT = (int)tri.size() / 3;
	if ((NT == 0) || startTris.empty()) { err = "No cap faces."; return false; }
	vec3d lo = pts[0], hi = pts[0];
	for (const vec3d& r : pts)
	{
		lo.x = std::min(lo.x, r.x); lo.y = std::min(lo.y, r.y); lo.z = std::min(lo.z, r.z);
		hi.x = std::max(hi.x, r.x); hi.y = std::max(hi.y, r.y); hi.z = std::max(hi.z, r.z);
	}
	double diag = (hi - lo).norm();
	double h = 1e-6 * diag;
	typedef std::tuple<long long, long long, long long> Key;
	std::map<Key, int> uniq;
	std::vector<int> W(pts.size());
	for (size_t i = 0; i < pts.size(); ++i)
	{
		const vec3d& r = pts[i];
		Key k((long long)std::llround(r.x / h), (long long)std::llround(r.y / h), (long long)std::llround(r.z / h));
		auto it = uniq.find(k);
		if (it == uniq.end()) it = uniq.insert({ k, (int)uniq.size() }).first;
		W[i] = it->second;
	}
	std::vector<vec3d> nrm(NT);
	std::vector<double> area(NT);
	std::map<std::pair<int, int>, std::vector<int>> edgeTris;
	for (int t = 0; t < NT; ++t)
	{
		const vec3d &a = pts[tri[3 * t]], &b = pts[tri[3 * t + 1]], &c = pts[tri[3 * t + 2]];
		vec3d an = ((b - a) ^ (c - a)) * 0.5;
		area[t] = an.norm();
		nrm[t] = (area[t] > 0 ? an / area[t] : vec3d(0, 0, 0));
		for (int k = 0; k < 3; ++k)
		{
			int u = W[tri[3 * t + k]], v = W[tri[3 * t + (k + 1) % 3]];
			edgeTris[{ std::min(u, v), std::max(u, v) }].push_back(t);
		}
	}
	// neighbours of each triangle
	std::vector<std::vector<int>> nb(NT);
	for (auto& e : edgeTris)
		for (int a : e.second) for (int b : e.second) if (a != b) nb[a].push_back(b);

	auto grow = [&](std::vector<int>& patch, std::vector<char>& used, const vec3d& n0, const vec3d& p0) {
		std::vector<int> stack(patch.begin(), patch.end());
		while (!stack.empty())
		{
			int t = stack.back(); stack.pop_back();
			for (int q : nb[t])
			{
				if (used[q]) continue;
				// the vertices must lie on the plane within a small tolerance (the cap of an STL is only approximately flat, and may
				// have small steps); the wall of the vessel rises steeply off the plane and is not accepted
				bool flat = true;
				for (int k = 0; k < 3; ++k) if (fabs((pts[tri[3 * q + k]] - p0) * n0) > 3e-3 * diag) flat = false;
				if (!flat) continue;
				used[q] = 1; patch.push_back(q); stack.push_back(q);
			}
		}
	};

	// the selected triangles, grouped into connected pieces (one piece per cap)
	std::vector<char> sel(NT, 0);
	for (int t : startTris) if ((t >= 0) && (t < NT)) sel[t] = 1;
	std::vector<char> seen(NT, 0);
	std::vector<std::vector<int>> pieces;
	for (int t0 = 0; t0 < NT; ++t0)
	{
		if (!sel[t0] || seen[t0]) continue;
		std::vector<int> piece(1, t0), stack(1, t0);
		seen[t0] = 1;
		while (!stack.empty())
		{
			int t = stack.back(); stack.pop_back();
			for (int q : nb[t]) if (sel[q] && !seen[q]) { seen[q] = 1; piece.push_back(q); stack.push_back(q); }
		}
		pieces.push_back(piece);
	}

	std::vector<char> used(NT, 0);
	std::vector<std::vector<int>> caps;
	std::vector<double> capArea;
	std::vector<vec3d> capCentroid;
	for (const std::vector<int>& piece : pieces)
	{
		std::vector<int> cap;
		vec3d sn(0, 0, 0), sc(0, 0, 0);
		double sa = 0;
		for (int t : piece)
		{
			if (used[t]) continue;
			used[t] = 1; cap.push_back(t);
			sn += nrm[t] * area[t]; sa += area[t];
			sc += (pts[tri[3 * t]] + pts[tri[3 * t + 1]] + pts[tri[3 * t + 2]]) * (area[t] / 3.0);
		}
		if (sa <= 0) continue;
		double sl = sn.norm();
		if (sl < 0.9 * sa) { err = "The selected faces of a cap are not planar. Select only the faces of the flat end caps of the vessel."; return false; }
		sn = sn / sl; sc = sc / sa;
		if (pieces.size() == 1) grow(cap, used, sn, sc);	// one selected cap: complete it; several: the selection is used as it is
		double A = 0;
		vec3d cc(0, 0, 0);
		for (int t : cap) { A += area[t]; cc += (pts[tri[3 * t]] + pts[tri[3 * t + 1]] + pts[tri[3 * t + 2]]) * (area[t] / 3.0); }
		caps.push_back(cap); capArea.push_back(A); capCentroid.push_back(cc / A);
	}
	if (caps.empty()) { err = "The selected faces have no area."; return false; }

	if (caps.size() == 1)
	{
		// one cap selected: the end cap is the largest other planar patch
		std::vector<int> bestPatch;
		double bestArea = 0;
		std::vector<int> order(NT);
		for (int t = 0; t < NT; ++t) order[t] = t;
		std::sort(order.begin(), order.end(), [&](int a, int b) { return area[a] > area[b]; });
		for (int oi = 0; oi < NT; ++oi)
		{
			int t = order[oi];
			if (used[t] || (area[t] <= 0)) continue;
			std::vector<int> patch(1, t);
			used[t] = 1;
			vec3d p0 = (pts[tri[3 * t]] + pts[tri[3 * t + 1]] + pts[tri[3 * t + 2]]) * (1.0 / 3.0);
			grow(patch, used, nrm[t], p0);
			double A = 0;
			for (int q : patch) A += area[q];
			if (A > bestArea) { bestArea = A; bestPatch = patch; }
		}
		if (bestPatch.size() < 4) { err = "Could not find the flat end cap at the other end of the vessel."; return false; }
		caps.push_back(bestPatch);
		capArea.push_back(bestArea);
		{
			vec3d cc(0, 0, 0);
			double A = 0;
			for (int q : bestPatch) { cc += (pts[tri[3 * q]] + pts[tri[3 * q + 1]] + pts[tri[3 * q + 2]]) * (area[q] / 3.0); A += area[q]; }
			capCentroid.push_back(A > 0 ? cc / A : cc);
		}
		// the start is the selected cap (index 0)
		startCentroid = capCentroid[0];
		startCapTris.clear();
		for (int q : caps[0]) { startCapTris.push_back(tri[3 * q]); startCapTris.push_back(tri[3 * q + 1]); startCapTris.push_back(tri[3 * q + 2]); }
	}
	else
	{
		// several caps selected (a bifurcation): the largest one is the inlet
		int inlet = 0;
		for (size_t i = 1; i < caps.size(); ++i) if (capArea[i] > capArea[inlet]) inlet = (int)i;
		startCentroid = capCentroid[inlet];
		startCapTris.clear();
		for (int q : caps[inlet]) { startCapTris.push_back(tri[3 * q]); startCapTris.push_back(tri[3 * q + 1]); startCapTris.push_back(tri[3 * q + 2]); }
	}

	if (allCapTris)
	{
		allCapTris->clear();
		for (const auto& cap : caps)
		{
			std::vector<int> v;
			for (int q : cap) { v.push_back(tri[3 * q]); v.push_back(tri[3 * q + 1]); v.push_back(tri[3 * q + 2]); }
			allCapTris->push_back(v);
		}
	}
	if (allCapCentroids) *allCapCentroids = capCentroid;
	std::vector<char> remove(NT, 0);
	for (const auto& cap : caps) for (int q : cap) remove[q] = 1;
	std::vector<int> out;
	for (int t = 0; t < NT; ++t)
		if (!remove[t]) { out.push_back(tri[3 * t]); out.push_back(tri[3 * t + 1]); out.push_back(tri[3 * t + 2]); }
	tri = out;
	return true;
}

//-----------------------------------------------------------------------------
bool VesselSections::Init(const std::vector<vec3d>& pts, const std::vector<int>& tri, std::string& err)
{
	if (tri.size() < 3) { err = "The surface has no triangles."; return false; }

	// bounding box
	vec3d lo = pts[0], hi = pts[0];
	for (const vec3d& r : pts)
	{
		lo.x = std::min(lo.x, r.x); lo.y = std::min(lo.y, r.y); lo.z = std::min(lo.z, r.z);
		hi.x = std::max(hi.x, r.x); hi.y = std::max(hi.y, r.y); hi.z = std::max(hi.z, r.z);
	}
	m_diag = (hi - lo).norm();
	if (m_diag <= 0) { err = "The surface has no extent."; return false; }

	// weld coincident vertices
	double h = 1e-6 * m_diag;
	typedef std::tuple<long long, long long, long long> Key;
	std::map<Key, int> uniq;
	std::vector<int> remap(pts.size());
	m_pts.clear();
	for (size_t i = 0; i < pts.size(); ++i)
	{
		const vec3d& r = pts[i];
		Key k((long long)std::llround(r.x / h), (long long)std::llround(r.y / h), (long long)std::llround(r.z / h));
		auto it = uniq.find(k);
		if (it == uniq.end()) { it = uniq.insert({ k, (int)m_pts.size() }).first; m_pts.push_back(r); }
		remap[i] = it->second;
	}
	m_tri.clear();
	for (size_t i = 0; i + 2 < tri.size(); i += 3)
	{
		int a = remap[tri[i]], b = remap[tri[i + 1]], c = remap[tri[i + 2]];
		if ((a == b) || (b == c) || (a == c)) continue;
		m_tri.push_back(a); m_tri.push_back(b); m_tri.push_back(c);
	}

	// boundary edges (used by exactly one triangle)
	std::map<std::pair<int, int>, int> count;
	for (size_t i = 0; i + 2 < m_tri.size(); i += 3)
		for (int k = 0; k < 3; ++k)
		{
			int a = m_tri[i + k], b = m_tri[i + (k + 1) % 3];
			count[{ std::min(a, b), std::max(a, b) }]++;
		}
	std::map<int, std::vector<int>> adj;
	for (auto& e : count)
		if (e.second == 1) { adj[e.first.first].push_back(e.first.second); adj[e.first.second].push_back(e.first.first); }
	if (adj.empty()) { err = "The surface is closed. Select the faces of the start cap, or use a surface with two open ends."; return false; }
	for (auto& a : adj)
		if (a.second.size() != 2) { err = "The open boundary of the surface is not made of simple loops (a vertex touches more than two boundary edges)."; return false; }

	std::vector<std::vector<vec3d>> loops;
	std::set<int> seen;
	for (auto& a : adj)
	{
		if (seen.count(a.first)) continue;
		std::vector<vec3d> loop;
		int prev = -1, cur = a.first;
		while (!seen.count(cur))
		{
			seen.insert(cur);
			loop.push_back(m_pts[cur]);
			int next = adj[cur][0];
			if (next == prev) next = adj[cur][1];
			prev = cur; cur = next;
		}
		loops.push_back(loop);
	}
	m_branched = false;
	if (loops.size() == 3)
	{
		m_bl.assign(3, Poly());
		for (int i = 0; i < 3; ++i)
		{
			PolygonInfo(loops[i], m_bl[i]);
			if (m_bl[i].area <= 0) { err = "An open end of the vessel has zero area."; return false; }
		}
		m_branched = true;
		return true;
	}
	if (loops.size() != 2)
	{
		err = "The surface has " + std::to_string(loops.size()) + " open boundary loops; two (the ends of a vessel) or three (a bifurcation) are needed.";
		for (auto& L : loops) { Poly pi; PolygonInfo(L, pi); err += "\n  loop: " + std::to_string(L.size()) + " points, area " + std::to_string(pi.area) + ", centroid (" + std::to_string(pi.c.x) + ", " + std::to_string(pi.c.y) + ", " + std::to_string(pi.c.z) + ")"; }
		return false;
	}
	for (int i = 0; i < 2; ++i) PolygonInfo(loops[i], m_end[i]);
	if (m_hasHint && ((m_end[1].c - m_hint).norm() < (m_end[0].c - m_hint).norm())) std::swap(m_end[0], m_end[1]);
	if ((m_end[0].area <= 0) || (m_end[1].area <= 0)) { err = "An open end of the vessel has zero area."; return false; }
	return true;
}

//-----------------------------------------------------------------------------
void VesselSections::Slice(const vec3d& o, const vec3d& n, std::vector<std::vector<vec3d>>& loops) const
{
	loops.clear();
	const int NV = (int)m_pts.size();
	std::vector<double> d(NV);
	for (int i = 0; i < NV; ++i) d[i] = (m_pts[i] - o) * n;

	// edge key --> intersection point; edge key --> the (at most two) neighbouring edge keys
	typedef std::pair<int, int> EK;
	std::map<EK, vec3d> point;
	std::map<EK, std::vector<EK>> link;
	for (size_t i = 0; i + 2 < m_tri.size(); i += 3)
	{
		int v[3] = { m_tri[i], m_tri[i + 1], m_tri[i + 2] };
		bool pos[3] = { d[v[0]] >= 0, d[v[1]] >= 0, d[v[2]] >= 0 };
		if ((pos[0] == pos[1]) && (pos[1] == pos[2])) continue;
		EK keys[2];
		int nk = 0;
		for (int k = 0; k < 3; ++k)
		{
			int a = v[k], b = v[(k + 1) % 3];
			if (pos[k] == pos[(k + 1) % 3]) continue;
			EK key(std::min(a, b), std::max(a, b));
			if (point.find(key) == point.end())
			{
				double da = d[a], db = d[b];
				double w = da / (da - db);
				point[key] = m_pts[a] + (m_pts[b] - m_pts[a]) * w;
			}
			keys[nk++] = key;
		}
		if (nk == 2)
		{
			link[keys[0]].push_back(keys[1]);
			link[keys[1]].push_back(keys[0]);
		}
	}

	std::set<EK> seen;
	for (auto& l : link)
	{
		if (seen.count(l.first)) continue;
		std::vector<vec3d> loop;
		EK prev(-1, -1), cur = l.first;
		bool closed = false;
		while (true)
		{
			if (seen.count(cur)) { closed = (cur == l.first); break; }
			seen.insert(cur);
			loop.push_back(point[cur]);
			const std::vector<EK>& nb = link[cur];
			EK next = nb[0];
			if ((next == prev) && (nb.size() > 1)) next = nb[1];
			prev = cur; cur = next;
		}
		if (loop.size() >= 3) loops.push_back(loop);
		(void)closed;
	}
}

//-----------------------------------------------------------------------------
bool VesselSections::PickLoop(const std::vector<std::vector<vec3d>>& loops, const vec3d& o, const vec3d& n, const vec3d& q, std::vector<vec3d>& loop) const
{
	if (loops.empty()) return false;
	Basis B = MakeBasis(n);
	double qx = (q - o) * B.e1, qy = (q - o) * B.e2;
	int best = -1;
	double bestArea = 1e300;
	double nearest = 1e300;
	int nearestIdx = 0;
	for (size_t i = 0; i < loops.size(); ++i)
	{
		const std::vector<vec3d>& L = loops[i];
		std::vector<double> x(L.size()), y(L.size());
		for (size_t k = 0; k < L.size(); ++k) { x[k] = (L[k] - o) * B.e1; y[k] = (L[k] - o) * B.e2; }
		Poly info;
		PolygonInfo(L, info);
		if (InsidePolygon2D(x, y, qx, qy) && (info.area < bestArea)) { best = (int)i; bestArea = info.area; }
		double dist = (info.c - q).norm();
		if (dist < nearest) { nearest = dist; nearestIdx = (int)i; }
	}
	loop = loops[best >= 0 ? best : nearestIdx];
	return true;
}

//-----------------------------------------------------------------------------
namespace {
	// keep the part of the planar polygon with n.(x-p) >= 0 (Sutherland-Hodgman)
	std::vector<vec3d> ClipPolygon(const std::vector<vec3d>& L, const vec3d& p, const vec3d& n)
	{
		std::vector<vec3d> out;
		int m = (int)L.size();
		for (int i = 0; i < m; ++i)
		{
			const vec3d& a = L[i];
			const vec3d& b = L[(i + 1) % m];
			double da = (a - p) * n, db = (b - p) * n;
			if (da >= 0) out.push_back(a);
			if ((da >= 0) != (db >= 0))
			{
				double w = da / (da - db);
				out.push_back(a + (b - a) * w);
			}
		}
		return out;
	}
}

//-----------------------------------------------------------------------------
bool VesselSections::CutSection(const vec3d& p, const vec3d& n, Poly& out, bool allowClip) const
{
	std::vector<std::vector<vec3d>> loops;
	Slice(p, n, loops);
	std::vector<vec3d> loop;
	if (!PickLoop(loops, p, n, p, loop)) return false;
	if (m_clip && allowClip)
	{
		loop = ClipPolygon(loop, m_clipP, m_clipN);
		if (loop.size() < 3) return false;
	}
	PolygonInfo(loop, out);
	return out.area > 0;
}

//-----------------------------------------------------------------------------
bool VesselSections::InsideNormal(const Poly& e, const vec3d& toward, vec3d& nIn) const
{
	double req = sqrt(e.area / 3.141592653589793);
	vec3d n = e.n;
	int found = 0;
	for (int sgn = 1; sgn >= -1; sgn -= 2)
	{
		vec3d o = e.c + n * (sgn * 0.1 * req);
		std::vector<std::vector<vec3d>> loops;
		Slice(o, n, loops);
		Basis B = MakeBasis(n);
		bool inside = false;
		for (const auto& L : loops)
		{
			std::vector<double> xs(L.size()), ys(L.size());
			for (size_t k = 0; k < L.size(); ++k) { xs[k] = (L[k] - o) * B.e1; ys[k] = (L[k] - o) * B.e2; }
			if (InsidePolygon2D(xs, ys, 0.0, 0.0)) { inside = true; break; }
		}
		if (inside)
		{
			if (found == 0) nIn = n * (double)sgn;
			found++;
		}
	}
	if (found == 0) return false;
	if (found == 2) nIn = (n * toward > 0 ? n : -n);
	return true;
}

//-----------------------------------------------------------------------------
bool VesselSections::TraceCenterline(std::string& err)
{
	// Which side of each end loop is the inside of the vessel?
	for (int e = 0; e < 2; ++e)
	{
		double req = sqrt(m_end[e].area / 3.141592653589793);
		vec3d n = m_end[e].n;
		int found = 0;
		vec3d other = m_end[1 - e].c - m_end[e].c;
		for (int sgn = 1; sgn >= -1; sgn -= 2)
		{
			vec3d o = m_end[e].c + n * (sgn * 0.1 * req);
			std::vector<std::vector<vec3d>> loops;
			Slice(o, n, loops);
			Basis B = MakeBasis(n);
			bool inside = false;
			for (const auto& L : loops)
			{
				// is the point o (the origin of the in-plane coordinates) inside this loop?
				std::vector<double> xs(L.size()), ys(L.size());
				for (size_t k = 0; k < L.size(); ++k) { xs[k] = (L[k] - o) * B.e1; ys[k] = (L[k] - o) * B.e2; }
				if (InsidePolygon2D(xs, ys, 0.0, 0.0)) { inside = true; break; }
			}
			if (inside)
			{
				if (found == 0) m_nIn[e] = n * (double)sgn;
				found++;
			}
		}
		if (found == 0) { err = "Could not determine the inside of the vessel at one of its ends."; return false; }
		if (found == 2) m_nIn[e] = (n * other > 0 ? n : -n);
	}

	return MarchCenterline(err);
}

//-----------------------------------------------------------------------------
bool VesselSections::MarchCenterline(std::string& err)
{
	const vec3d nBout = -m_nIn[1];
	auto signedDistB = [&](const vec3d& x) { return (x - m_end[1].c) * nBout; };

	vec3d c = m_end[0].c;
	vec3d t = m_nIn[0];
	double req = sqrt(m_end[0].area / 3.141592653589793);
	m_center.clear();
	m_center.push_back(c);
	m_clipS = 1e300;
	bool clipOn = m_clip;
	size_t clipOffIndex = 0;
	double clipOffR = 0;
	const double hmin = 1e-3 * m_diag;
	for (int iter = 0; iter < 50000; ++iter)
	{
		double h = std::max(0.5 * req, hmin);
		vec3d cp = c + t * h;
		if (signedDistB(cp) >= 0) break;

		Poly info;
		if (!CutSection(cp, t, info, clipOn)) { err = "Lost the vessel while tracing the centerline (no cross-section found)."; return false; }
		vec3d cn = info.c;
		double step = (cn - c).norm();
		if (step < 1e-9 * m_diag) { err = "The centerline tracing stalled."; return false; }
		vec3d tn = (cn - c) / step;
		t = t * 0.5 + tn * 0.5;
		t /= t.norm();
		c = cn;
		req = sqrt(info.area / 3.141592653589793);
		m_center.push_back(c);
		// the clip plane (divider of a bifurcation) is only meant for the part of the branch that shares its lumen with the other branch; once
		// the lumen has left the plane for good, a branch that curves back across it must not be cut
		if (clipOn && ((c - m_clipP) * m_clipN > 1.5 * req)) { clipOn = false; clipOffIndex = m_center.size() - 1; clipOffR = req; }
		if (iter == 49999) { err = "The centerline tracing did not reach the other end."; return false; }
	}
	// finish at the center of the other end (drop the last point if it is very close)
	if (m_center.size() > 1 && (m_center.back() - m_end[1].c).norm() < 0.25 * std::max(0.5 * req, hmin)) m_center.pop_back();
	m_center.push_back(m_end[1].c);
	if (m_center.size() < 2) { err = "The vessel is too short."; return false; }

	// light smoothing (the ends stay fixed)
	for (int pass = 0; pass < 3; ++pass)
	{
		std::vector<vec3d> q = m_center;
		for (size_t i = 1; i + 1 < m_center.size(); ++i) q[i] = m_center[i - 1] * 0.25 + m_center[i] * 0.5 + m_center[i + 1] * 0.25;
		m_center = q;
	}
	PrepareCurve();
	if (m_clip && !clipOn) m_clipS = m_s[std::min(clipOffIndex, m_s.size() - 1)] + clipOffR;
	return true;
}

//-----------------------------------------------------------------------------
void VesselSections::PrepareCurve()
{
	int n = (int)m_center.size();
	m_s.assign(n, 0.0);
	for (int i = 1; i < n; ++i) m_s[i] = m_s[i - 1] + (m_center[i] - m_center[i - 1]).norm();
	m_tv.assign(n, vec3d(0, 0, 1));
	for (int i = 0; i < n; ++i)
	{
		vec3d d;
		if (i == 0) d = m_center[1] - m_center[0];
		else if (i == n - 1) d = m_center[n - 1] - m_center[n - 2];
		else d = m_center[i + 1] - m_center[i - 1];
		double L = d.norm();
		m_tv[i] = (L > 0 ? d / L : vec3d(0, 0, 1));
	}
	// the end tangents are along the end normals
	m_tv[0] = m_nIn[0];
	m_tv[n - 1] = -m_nIn[1];
}

//-----------------------------------------------------------------------------
void VesselSections::CurveAt(double s, vec3d& p, vec3d& t) const
{
	int n = (int)m_center.size();
	s = std::min(std::max(s, 0.0), m_s.back());
	int seg = (int)(std::upper_bound(m_s.begin(), m_s.end(), s) - m_s.begin()) - 1;
	seg = std::min(std::max(seg, 0), n - 2);
	double L = m_s[seg + 1] - m_s[seg];
	double u = (L > 0 ? (s - m_s[seg]) / L : 0.0);
	double u2 = u * u, u3 = u2 * u;
	double h00 = 2 * u3 - 3 * u2 + 1, h10 = u3 - 2 * u2 + u, h01 = -2 * u3 + 3 * u2, h11 = u3 - u2;
	double d00 = 6 * u2 - 6 * u, d10 = 3 * u2 - 4 * u + 1, d01 = -6 * u2 + 6 * u, d11 = 3 * u2 - 2 * u;
	vec3d m0 = m_tv[seg] * L, m1 = m_tv[seg + 1] * L;
	p = m_center[seg] * h00 + m0 * h10 + m_center[seg + 1] * h01 + m1 * h11;
	vec3d dt = m_center[seg] * d00 + m0 * d10 + m_center[seg + 1] * d01 + m1 * d11;
	double dl = dt.norm();
	t = (dl > 1e-12 ? dt / dl : m_tv[seg]);
}

//-----------------------------------------------------------------------------
// The cutting planes are normally perpendicular to the centerline, but near a flared or obliquely cut
// end the centerline turns so fast that neighbouring planes would cross inside the vessel (which cuts
// the section short and folds the mesh). So limit how fast the plane normal may rotate to a fraction
// of (1/local radius) per unit length.
void VesselSections::BuildNormalField()
{
	const int nf = 400;
	const double RATE = 0.3;
	const double Lt = m_s.back();
	const double ds = Lt / nf;
	std::vector<vec3d> T(nf + 1);
	std::vector<double> rl(nf + 1);
	for (int i = 0; i <= nf; ++i)
	{
		vec3d p;
		CurveAt(ds * i, p, T[i]);
		double A;
		if (i == 0) A = m_end[0].area;
		else if (i == nf) A = m_end[1].area;
		else
		{
			Poly info;
			if (CutSection(p, T[i], info, ds * i <= m_clipS)) A = info.area;
			else A = (i > 0 ? 3.141592653589793 * rl[i - 1] * rl[i - 1] : m_end[0].area);
		}
		rl[i] = sqrt(std::max(A, 1e-30) / 3.141592653589793);
	}
	std::vector<vec3d> N = T;
	auto toward = [](const vec3d& a, const vec3d& b, double m) {
		double th = acos(std::min(1.0, std::max(-1.0, a * b)));
		if (th <= m) return a;
		vec3d v = a - b * (a * b);
		double l = v.norm();
		if (l < 1e-14) return a;
		v = v / l;
		return b * cos(m) + v * sin(m);
	};
	for (int it = 0; it < 6; ++it)
	{
		for (int i = 1; i < nf; ++i) N[i] = toward(N[i], N[i - 1], RATE * ds / rl[i]);
		for (int i = nf - 1; i > 0; --i) N[i] = toward(N[i], N[i + 1], RATE * ds / rl[i]);
	}
	// At the ends the planes must agree with the end sections (e.g. a cap that is slightly tilted with respect to the vessel). Let the
	// plane normal change smoothly from the end normal to the interior one over a few radii, instead of within a fraction of a radius.
	for (int e = 0; e < 2; ++e)
	{
		const int ie = (e == 0 ? 0 : nf);
		const int dir = (e == 0 ? 1 : -1);
		int iL = (int)ceil(3.0 * rl[ie] / ds);
		iL = std::min(iL, nf / 4);
		if (iL < 2) continue;
		vec3d Ne = N[ie], Nr = N[ie + dir * iL];
		for (int q = 0; q <= iL; ++q)
		{
			double x = (double)q / (double)iL;
			double w = x * x * (3.0 - 2.0 * x);
			vec3d v = Ne * (1.0 - w) + Nr * w;
			double l = v.norm();
			if (l > 1e-12) N[ie + dir * q] = v / l;
		}
	}
	m_nf = N;
}

//-----------------------------------------------------------------------------
vec3d VesselSections::PlaneNormal(double s) const
{
	if (m_nf.size() < 2)
	{
		vec3d p, t;
		CurveAt(s, p, t);
		return t;
	}
	int nf = (int)m_nf.size() - 1;
	double x = std::min(std::max(s / m_s.back(), 0.0), 1.0) * nf;
	int i = std::min((int)x, nf - 1);
	double w = x - i;
	vec3d v = m_nf[i] * (1 - w) + m_nf[i + 1] * w;
	double l = v.norm();
	return (l > 0 ? v / l : m_nf[i]);
}

//-----------------------------------------------------------------------------
bool VesselSections::BuildSections(int N, double adaptivity, std::vector<Poly>& sections, std::string& err)
{
	sections.clear();
	if (N < 1) { err = "At least one interval (two sections) is needed."; return false; }
	if (m_center.size() < 2 && !TraceCenterline(err)) return false;
	const double Ltot = m_s.back();
	BuildNormalField();

	// the cross-section at arclength s of the centerline
	auto cut = [&](double s, Poly& out) {
		vec3d p, t;
		CurveAt(s, p, t);
		t = PlaneNormal(s);
		if (!CutSection(p, t, out, s <= m_clipS)) return false;
		if (out.n * t < 0)
		{
			std::reverse(out.pts.begin(), out.pts.end());
			out.n = -out.n;
		}
		return true;
	};

	// spacing grading: x in [0,1] -> fraction of the length (m_grade > 0: denser sections at the start, coarser at the end)
	// (the spacing is reduced by the factor 1 - m_grade at the start and grows linearly back to the nominal one over the first 35% of the length)
	const double gx0 = 0.35;
	auto gfun = [&](double x) {
		if (x <= gx0) return (1.0 - m_grade) * x + m_grade * x * x / (2.0 * gx0);
		return (1.0 - m_grade) * gx0 + m_grade * gx0 / 2.0 + (x - gx0);
	};
	const double gnorm = gfun(1.0);
	auto grade = [&](double x) { return gfun(x) / gnorm; };

	// importance function for the spacing
	std::vector<double> sp;		// section positions along the centerline
	if (adaptivity <= 0.0 || N == 1)
	{
		for (int j = 0; j <= N; ++j) sp.push_back(Ltot * grade(j / (double)N));
	}
	else
	{
		const int nf = 400;
		std::vector<double> r(nf + 1, 0.0), kap(nf + 1, 0.0), g(nf + 1, 0.0), sf(nf + 1);
		std::vector<vec3d> tf(nf + 1);
		for (int i = 0; i <= nf; ++i)
		{
			sf[i] = Ltot * i / (double)nf;
			vec3d p;
			CurveAt(sf[i], p, tf[i]);
			Poly info;
			double area = 0;
			if (i == 0) area = m_end[0].area;
			else if (i == nf) area = m_end[1].area;
			else if (cut(sf[i], info)) area = info.area;
			else area = (i > 0 ? 3.141592653589793 * r[i - 1] * r[i - 1] : m_end[0].area);
			r[i] = sqrt(area / 3.141592653589793);
		}
		double ds = Ltot / nf;
		for (int i = 1; i < nf; ++i)
		{
			kap[i] = (tf[i + 1] - tf[i - 1]).norm() / (2 * ds) * r[i];	// turning per radius
			g[i] = fabs(r[i + 1] - r[i - 1]) / (2 * ds);					// change of radius along the vessel
		}
		kap[0] = kap[1]; kap[nf] = kap[nf - 1]; g[0] = g[1]; g[nf] = g[nf - 1];
		auto robustMax = [&](std::vector<double> v) {
			std::sort(v.begin(), v.end());
			return std::max(v[(size_t)(0.95 * (v.size() - 1))], 1e-12);
		};
		double km = robustMax(kap), gm = robustMax(g);
		std::vector<double> M(nf + 1, 0.0);
		for (int i = 1; i <= nf; ++i)
		{
			auto imp = [&](int k) { return 1.0 + adaptivity * 0.5 * (std::min(kap[k] / km, 1.0) + std::min(g[k] / gm, 1.0)); };
			M[i] = M[i - 1] + 0.5 * (imp(i - 1) + imp(i)) * ds;
		}
		for (int j = 0; j <= N; ++j)
		{
			double target = M[nf] * grade(j / (double)N);
			int i = (int)(std::upper_bound(M.begin(), M.end(), target) - M.begin()) - 1;
			i = std::min(std::max(i, 0), nf - 1);
			double w = (M[i + 1] > M[i] ? (target - M[i]) / (M[i + 1] - M[i]) : 0.0);
			sp.push_back(sf[i] + w * ds);
		}
		sp.front() = 0.0; sp.back() = Ltot;
	}

	// Rim sections: a cap often has a lip, i.e. the radius of the vessel changes by several percent within a fraction of a radius of the
	// end (the rim of the cap is smaller than the vessel just inside). Sections that are further apart than that smear the step over the
	// first layer of elements, so add a section where the radius has settled.
	if (sp.size() >= 3)
	{
		const double kPi2 = 3.141592653589793;
		for (int e = 0; e < 2; ++e)
		{
			const double rEnd = sqrt(std::max(m_end[e].area, 1e-30) / kPi2);
			auto radiusAt = [&](double d, double& r) {
				Poly info;
				if (!cut(e == 0 ? d : Ltot - d, info)) return false;
				r = sqrt(info.area / kPi2);
				return true;
			};
			double dfound = -1;
			const double dd = 0.03 * rEnd;
			double rprev = rEnd, rr = 0;
			bool jump = false;
			for (double d = 0.06 * rEnd; d < 0.8 * rEnd; d += dd)
			{
				if (!radiusAt(d, rr)) break;
				if (fabs(rr - rEnd) > 0.06 * rEnd) jump = true;
				if (jump && (fabs(rr - rprev) < 0.12 * dd)) { dfound = d; break; }
				rprev = rr;
			}
			if (dfound < 0) continue;
			double sNew = (e == 0 ? dfound : Ltot - dfound);
			bool close = false;
			for (double q : sp) if (fabs(q - sNew) < 0.5 * dfound) close = true;
			if (!close) sp.push_back(sNew);
		}
		std::sort(sp.begin(), sp.end());
	}

	// cut the sections
	for (size_t j = 0; j < sp.size(); ++j)
	{
		Poly P;
		if (j == 0)
		{
			P = m_end[0];
			if (P.n * m_nIn[0] < 0) { std::reverse(P.pts.begin(), P.pts.end()); P.n = -P.n; }
		}
		else if (j + 1 == sp.size())
		{
			P = m_end[1];
			if (P.n * (-m_nIn[1]) < 0) { std::reverse(P.pts.begin(), P.pts.end()); P.n = -P.n; }
		}
		else if (!cut(sp[j], P))
		{
			err = "Could not cut a cross-section at " + std::to_string(sp[j]) + " along the vessel.";
			return false;
		}
		sections.push_back(P);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool VesselSections::BuildBifurcation(int nTrunk, int nDaughter, double adaptivity, Bifurcation& out, std::string& err)
{
	const double kPi = 3.141592653589793;
	if (!m_branched) { err = "The surface does not have three open ends."; return false; }
	if ((nTrunk < 1) || (nDaughter < 1)) { err = "At least one interval is needed in each branch."; return false; }

	// the inlet: the loop nearest the start point (or the largest one)
	int inlet = 0;
	for (int i = 1; i < 3; ++i)
	{
		if (m_hasHint) { if ((m_bl[i].c - m_hint).norm() < (m_bl[inlet].c - m_hint).norm()) inlet = i; }
		else if (m_bl[i].area > m_bl[inlet].area) inlet = i;
	}
	int o0 = (inlet + 1) % 3, o1 = (inlet + 2) % 3;
	Poly in = m_bl[inlet];
	Poly outl[2] = { m_bl[o0], m_bl[o1] };
	vec3d towardOut = (outl[0].c + outl[1].c) * 0.5;

	vec3d nIn0;
	if (!InsideNormal(in, towardOut - in.c, nIn0)) { err = "Could not determine the inside of the vessel at the inlet."; return false; }

	// ---- the trunk: march from the inlet until the cross-section starts to split into the two daughters
	m_clip = false;
	std::vector<vec3d> cl;
	std::vector<double> areaHist;
	vec3d c = in.c, t = nIn0;
	double req = sqrt(in.area / kPi);
	const double hmin = 1e-3 * m_diag;
	cl.push_back(c);
	areaHist.push_back(in.area);
	Poly Floop;
	vec3d Ft = t;
	bool haveF = false;
	for (int iter = 0; iter < 20000; ++iter)
	{
		double h = std::max(0.5 * req, hmin);
		vec3d cp = c + t * h;
		std::vector<std::vector<vec3d>> loops;
		Slice(cp, t, loops);
		Poly info;
		if (loops.empty()) break;
		bool split = false;
		{
			// loops of significant size
			int big = 0;
			for (const auto& L : loops) { Poly pi; PolygonInfo(L, pi); if (pi.area > 0.1 * areaHist.back()) big++; }
			if (big >= 2) split = true;
		}
		std::vector<vec3d> loop;
		if (!split)
		{
			if (!PickLoop(loops, cp, t, cp, loop)) break;
			PolygonInfo(loop, info);
			double aref = 0;
			int na = 0;
			for (int k = (int)areaHist.size() - 1; (k >= 0) && (na < 6); --k, ++na) aref += areaHist[k];
			aref /= na;
			if ((cl.size() >= 3) && (info.area > 1.3 * aref)) split = true;
		}
		if (split) { haveF = (cl.size() >= 2); break; }
		vec3d cn = info.c;
		double step = (cn - c).norm();
		if (step < 1e-9 * m_diag) break;
		vec3d tn = (cn - c) / step;
		t = t * 0.5 + tn * 0.5;
		t /= t.norm();
		c = cn;
		req = sqrt(info.area / kPi);
		cl.push_back(c);
		areaHist.push_back(info.area);
		Floop = info;
		Ft = t;
		haveF = true;
	}
	if (!haveF || Floop.pts.empty()) { err = "Could not trace the trunk of the bifurcation."; return false; }
	if (cl.size() < 3) { err = "The trunk of the bifurcation is too short."; return false; }
	// the trunk ends on the plane through the last centerline point (the cross-section Floop was cut there with the tangent Ft)
	{
		Poly chk;
		if (!CutSection(cl.back(), Ft, chk)) { err = "Could not cut the trunk end section."; return false; }
		Floop = chk;
	}
	if (Floop.n * Ft < 0) { std::reverse(Floop.pts.begin(), Floop.pts.end()); Floop.n = -Floop.n; }
	cl.back() = Floop.c;
	out.fp = cl.back();
	out.ft = Ft;

	// ---- the divider plane: at the narrowest gap between the two lobes where the cross-section has just split
	{
		bool found = false;
		std::vector<vec3d> loopA, loopB;
		vec3d cp = cl.back();
		double Af = Floop.area;
		double step = std::max(0.25 * req, hmin);
		for (int iter = 0; iter < 4000 && !found; ++iter)
		{
			cp = cp + Ft * step;
			std::vector<std::vector<vec3d>> loops;
			Slice(cp, Ft, loops);
			std::vector<int> big;
			for (size_t i = 0; i < loops.size(); ++i) { Poly pi; PolygonInfo(loops[i], pi); if (pi.area > 0.02 * Af) big.push_back((int)i); }
			if (big.size() >= 2)
			{
				const std::vector<vec3d>& A = loops[big[0]];
				const std::vector<vec3d>& B = loops[big[1]];
				double dmin = 1e300;
				vec3d pa, pb;
				for (const vec3d& a : A) for (const vec3d& b : B) { double d = (a - b).norm(); if (d < dmin) { dmin = d; pa = a; pb = b; } }
				vec3d n = pb - pa;
				n = n - Ft * (n * Ft);
				if (n.norm() < 1e-12) continue;
				out.dn = n / n.norm();
				out.dp = (pa + pb) * 0.5;
				loopA = A; loopB = B;
				found = true;
			}
			if ((cp - out.fp).norm() > 3.0 * m_diag) break;
		}
		if (!found) { err = "Could not find where the vessel divides into two branches."; return false; }

		// Tilt the divider (about the carina point) so that it passes through the centre of the trunk end section: its two halves are
		// then of similar size, and have blunt corners, instead of one half being a thin circular segment. Only if both lobes stay on
		// their own side of the tilted plane.
		vec3d e = out.ft ^ out.dn;
		vec3d v = out.dp - out.fp;
		vec3d n2 = e ^ v;
		if (n2.norm() > 1e-9 * m_diag)
		{
			n2 = n2 / n2.norm();
			if (n2 * out.dn < 0) n2 = -n2;
			const double tol = 1e-6 * m_diag;
			bool ok = true;
			for (const vec3d& a : loopA) if ((a - out.dp) * n2 > tol) ok = false;
			for (const vec3d& b : loopB) if ((b - out.dp) * n2 < -tol) ok = false;
			if (ok) out.dn = n2;
		}
	}
	// which outlet is on which side
	double s0 = (outl[0].c - out.dp) * out.dn, s1 = (outl[1].c - out.dp) * out.dn;
	if (s0 * s1 >= 0) { err = "The two outlets are on the same side of the divider plane."; return false; }
	if (s0 < 0) { std::swap(outl[0], outl[1]); }
	// outl[0] is now on the positive side of the divider

	// ---- the trunk tube
	{
		VesselSections tr;
		tr.m_pts = m_pts; tr.m_tri = m_tri; tr.m_diag = m_diag;
		tr.m_end[0] = in; tr.m_end[1] = Floop;
		tr.m_nIn[0] = nIn0; tr.m_nIn[1] = -Ft;
		tr.m_center = cl;
		for (int pass = 0; pass < 3; ++pass)
		{
			std::vector<vec3d> q = tr.m_center;
			for (size_t i = 1; i + 1 < tr.m_center.size(); ++i) q[i] = tr.m_center[i - 1] * 0.25 + tr.m_center[i] * 0.5 + tr.m_center[i + 1] * 0.25;
			tr.m_center = q;
		}
		tr.PrepareCurve();
		if (!tr.BuildSections(nTrunk, adaptivity, out.trunk, err)) return false;
	}

	// ---- the daughters: half-lumen sections
	for (int j = 0; j < 2; ++j)
	{
		VesselSections d;
		d.m_pts = m_pts; d.m_tri = m_tri; d.m_diag = m_diag;
		d.m_clip = true;
		d.m_grade = 0.5;		// denser sections at the junction, where the chord of the half sections develops
		d.m_clipP = out.dp;
		d.m_clipN = (j == 0 ? out.dn : -out.dn);
		std::vector<vec3d> half = ClipPolygon(Floop.pts, d.m_clipP, d.m_clipN);
		if (half.size() < 3) { err = "The divider plane does not cut the end of the trunk."; return false; }
		Poly Fh;
		PolygonInfo(half, Fh);
		if (Fh.n * Ft < 0) { std::reverse(Fh.pts.begin(), Fh.pts.end()); Fh.n = -Fh.n; }
		d.m_end[0] = Fh;
		d.m_end[1] = outl[j];
		d.m_nIn[0] = Ft;
		vec3d nIn1;
		if (!d.InsideNormal(outl[j], Fh.c - outl[j].c, nIn1)) { err = "Could not determine the inside of the vessel at an outlet."; return false; }
		d.m_nIn[1] = nIn1;
		if (!d.MarchCenterline(err)) return false;
		// (at least some adaptivity in the branches, which puts sections where the radius changes quickly, e.g. at a tapering outlet)
		if (!d.BuildSections(nDaughter, std::max(adaptivity, 1.0), out.daughter[j], err)) return false;
	}
	return true;
}
