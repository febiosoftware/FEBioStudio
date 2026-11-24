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
bool VesselSections::RemoveCaps(const std::vector<vec3d>& pts, std::vector<int>& tri, const std::vector<int>& startTris, vec3d& startCentroid, std::vector<int>& startCapTris, std::string& err)
{
	const int NT = (int)tri.size() / 3;
	if ((NT == 0) || startTris.empty()) { err = "No start cap faces."; return false; }
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

	const double cosTol = cos(5.0 * 3.141592653589793 / 180.0);
	auto grow = [&](std::vector<int>& patch, std::vector<char>& used, const vec3d& n0, const vec3d& p0) {
		std::vector<int> stack(patch.begin(), patch.end());
		while (!stack.empty())
		{
			int t = stack.back(); stack.pop_back();
			for (int q : nb[t])
			{
				if (used[q]) continue;
				// a thin sliver triangle has an unreliable normal; otherwise the normal must agree with the plane's
				if ((nrm[q] * n0 < cosTol) && (area[q] > 0.02 * area[t])) continue;
				bool flat = true;
				for (int k = 0; k < 3; ++k) if (fabs((pts[tri[3 * q + k]] - p0) * n0) > 3e-3 * diag) flat = false;
				if (!flat) continue;
				used[q] = 1; patch.push_back(q); stack.push_back(q);
			}
		}
	};

	std::vector<char> used(NT, 0);
	std::vector<int> start;
	vec3d sn(0, 0, 0), sc(0, 0, 0);
	double sa = 0;
	for (int t : startTris)
	{
		if ((t < 0) || (t >= NT) || used[t]) continue;
		used[t] = 1; start.push_back(t);
		sn += nrm[t] * area[t]; sa += area[t];
		sc += (pts[tri[3 * t]] + pts[tri[3 * t + 1]] + pts[tri[3 * t + 2]]) * (area[t] / 3.0);
	}
	if (sa <= 0) { err = "The selected start faces have no area."; return false; }
	double sl = sn.norm();
	if (sl < 0.9 * sa) { err = "The selected start faces are not planar. Select only the faces of the flat end cap of the vessel."; return false; }
	sn = sn / sl; sc = sc / sa;
	startCentroid = sc;
	grow(start, used, sn, sc);

	// the end cap: the largest other planar patch
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
	if ((bestPatch.size() < 4)) { err = "Could not find the flat end cap at the other end of the vessel."; return false; }

	startCapTris.clear();
	for (int q : start) { startCapTris.push_back(tri[3 * q]); startCapTris.push_back(tri[3 * q + 1]); startCapTris.push_back(tri[3 * q + 2]); }
	std::vector<char> remove(NT, 0);
	for (int q : start) remove[q] = 1;
	for (int q : bestPatch) remove[q] = 1;
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
	if (loops.size() != 2)
	{
		err = "The surface has " + std::to_string(loops.size()) + " open boundary loops; exactly two (the two ends of the vessel) are needed. Branching vessels are not supported.";
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

	const vec3d nBout = -m_nIn[1];
	auto signedDistB = [&](const vec3d& x) { return (x - m_end[1].c) * nBout; };

	vec3d c = m_end[0].c;
	vec3d t = m_nIn[0];
	double req = sqrt(m_end[0].area / 3.141592653589793);
	m_center.clear();
	m_center.push_back(c);
	const double hmin = 1e-3 * m_diag;
	for (int iter = 0; iter < 50000; ++iter)
	{
		double h = std::max(0.5 * req, hmin);
		vec3d cp = c + t * h;
		if (signedDistB(cp) >= 0) break;

		std::vector<std::vector<vec3d>> loops;
		Slice(cp, t, loops);
		std::vector<vec3d> loop;
		if (!PickLoop(loops, cp, t, cp, loop)) { err = "Lost the vessel while tracing the centerline (no cross-section found)."; return false; }
		Poly info;
		PolygonInfo(loop, info);
		vec3d cn = info.c;
		double step = (cn - c).norm();
		if (step < 1e-9 * m_diag) { err = "The centerline tracing stalled."; return false; }
		vec3d tn = (cn - c) / step;
		t = t * 0.5 + tn * 0.5;
		t /= t.norm();
		c = cn;
		req = sqrt(info.area / 3.141592653589793);
		m_center.push_back(c);
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
			std::vector<std::vector<vec3d>> loops;
			std::vector<vec3d> loop;
			Slice(p, T[i], loops);
			Poly info;
			if (PickLoop(loops, p, T[i], p, loop)) { PolygonInfo(loop, info); A = info.area; }
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
		std::vector<std::vector<vec3d>> loops;
		Slice(p, t, loops);
		std::vector<vec3d> loop;
		if (!PickLoop(loops, p, t, p, loop)) return false;
		PolygonInfo(loop, out);
		if (out.n * t < 0)
		{
			std::reverse(out.pts.begin(), out.pts.end());
			out.n = -out.n;
		}
		return true;
	};

	// importance function for the spacing
	std::vector<double> sp;		// section positions along the centerline
	if (adaptivity <= 0.0 || N == 1)
	{
		for (int j = 0; j <= N; ++j) sp.push_back(Ltot * j / (double)N);
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
			double target = M[nf] * j / (double)N;
			int i = (int)(std::upper_bound(M.begin(), M.end(), target) - M.begin()) - 1;
			i = std::min(std::max(i, 0), nf - 1);
			double w = (M[i + 1] > M[i] ? (target - M[i]) / (M[i + 1] - M[i]) : 0.0);
			sp.push_back(sf[i] + w * ds);
		}
		sp.front() = 0.0; sp.back() = Ltot;
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
