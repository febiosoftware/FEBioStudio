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
#include <cstdlib>
#include <algorithm>

ShellSweeper::ShellSweeper()
{
	m_layers = 0;
	m_rotate = true;
	m_autoPartition = false;
	m_creaseAngle = 30.0;
	m_collapseTol = 0.0;
	m_startNormal = vec3d(0, 0, 0);
	for (int i = 0; i < 6; ++i) m_startCov[i] = 0.0;
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
	if (!m_covariance.empty() && (m_covariance.size() != 6 * m_path.size())) { m_err = "The covariance list does not match the path."; return nullptr; }
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
	std::vector<double> covk;
	for (size_t k = 0; k < st.size(); ++k)
	{
		int seg = st[k].first;
		double u = st[k].second;
		if (!m_covariance.empty())
			for (int j = 0; j < 6; ++j) covk.push_back(m_covariance[6 * seg + j] * (1.0 - u) + m_covariance[6 * (seg + 1) + j] * u);
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
	// in-plane deformation (size and shape matching) --------------------------------------------------------
	vec3d e1, e2;
	{
		vec3d n0 = (m_startNormal.norm() > 0.5 ? m_startNormal : (orient.empty() ? tang[0] : orient[0]));
		vec3d a = (fabs(n0.x) < 0.9) ? vec3d(1, 0, 0) : vec3d(0, 1, 0);
		e1 = n0 ^ a; e1 /= e1.norm();
		e2 = n0 ^ e1; e2 /= e2.norm();
	}
	auto quad = [](const double* C, const vec3d& u, const vec3d& v) {
		// u^T C v for the symmetric matrix C = (xx,xy,xz,yy,yz,zz)
		return u.x * (C[0] * v.x + C[1] * v.y + C[2] * v.z) + u.y * (C[1] * v.x + C[3] * v.y + C[4] * v.z) + u.z * (C[2] * v.x + C[4] * v.y + C[5] * v.z);
	};
	// square root of a symmetric positive definite 2x2 matrix [a b; b c]
	auto sqrt2 = [](double a, double b, double c, double out[4]) {
		double det = a * c - b * b;
		if (det <= 0) return false;
		double s = sqrt(det), t = sqrt(a + c + 2 * s);
		out[0] = (a + s) / t; out[1] = b / t; out[2] = b / t; out[3] = (c + s) / t;
		return true;
	};
	bool deform = !m_covariance.empty();
	double IS[4] = { 1, 0, 0, 1 };		// inverse square root of the section's own 2x2 covariance
	if (deform)
	{
		double S[4];
		if (sqrt2(quad(m_startCov, e1, e1), quad(m_startCov, e1, e2), quad(m_startCov, e2, e2), S))
		{
			double det = S[0] * S[3] - S[1] * S[2];
			IS[0] = S[3] / det; IS[1] = -S[1] / det; IS[2] = -S[2] / det; IS[3] = S[0] / det;
		}
		else deform = false;
	}

	// rotations of all stations
	std::vector<Mat3> Rks(nst + 1);
	{
		Mat3 R;
		// with prescribed orientations, the section's own normal at the start is orient[0]; first rotate it onto the path tangent
		if (m_rotate && !orient.empty()) R = RotateTo(orient[0], tang[0]);
		for (int k = 0; k <= nst; ++k)
		{
			if ((k > 0) && m_rotate) R = RotateTo(tang[k - 1], tang[k]) * R;
			Rks[k] = R;
			if (m_rotate && !orient.empty()) Rks[k] = RotateTo(tang[k], orient[k]) * R;
		}
	}

	// outline matching -----------------------------------------------------------------------------------------
	const int M = 720;
	const double PI2 = 6.283185307179586;
	bool outlineMode = false;
	std::vector<std::vector<double>> prof;		// radial profile of each outline, in the frame of its station
	std::vector<double> alpha;					// in-plane rotation of each target
	std::vector<double> prof0orig;				// (smooth start) the original profile of the start outline
	bool smoothOn = false;
	std::vector<int> stT(nst + 1, 0);			// station --> interval of targets
	std::vector<double> stU(nst + 1, 0.0), stAlpha(nst + 1, 0.0), stFbar(nst + 1, 1.0);

	auto evalProfile = [&](const std::vector<double>& R, double ang) {
		double t = ang / PI2;
		t -= floor(t);
		double x = t * M;
		int i = (int)x;
		double w = x - i;
		if (i >= M) { i = 0; w = 0; }
		return R[i] * (1.0 - w) + R[(i + 1) % M] * w;
	};

	// corner anchoring: sharp corners of the target outlines (e.g. a half-lumen section) are passed through explicitly. A boundary node of
	// the start section is assigned to each corner (nuAnc); the polar angles of the nodes are warped (piecewise linearly) so that these nodes
	// end up exactly at the corners of every station (stKap), and the radial profiles are registered the same way.
	std::vector<double> nuAnc;							// start-section angle of the anchor nodes
	std::vector<std::vector<double>> Aang;				// corner angles of each target (in the frame of the start section, i.e. without alpha)
	std::vector<std::vector<double>> stKap(nst + 1);	// anchor angles at each station
	auto wrap2 = [&](double a) { a = fmod(a, PI2); if (a < 0) a += PI2; return a; };
	auto wrapPM = [&](double a) { a = wrap2(a); if (a > 0.5 * PI2) a -= PI2; return a; };
	auto warp = [&](const std::vector<double>& src, const std::vector<double>& dst, double x) {
		int N = (int)src.size();
		for (int i = 0; i < N; ++i)
		{
			int j = (i + 1) % N;
			double d = wrap2(x - src[i]);
			double L = wrap2(src[j] - src[i]); if (N == 1 || L < 1e-12) L = PI2;
			if (d < L)
			{
				double Lm = wrap2(dst[j] - dst[i]); if (N == 1 || Lm < 1e-12) Lm = PI2;
				return dst[i] + d / L * Lm;
			}
		}
		return x;
	};
	auto findCorners = [&](const std::vector<double>& R) {
		std::vector<double> px(M), py(M), tn(M), sm(M), out;
		for (int j = 0; j < M; ++j) { px[j] = R[j] * cos(PI2 * j / M); py[j] = R[j] * sin(PI2 * j / M); }
		for (int j = 0; j < M; ++j)
		{
			int a = (j + M - 1) % M, b = (j + 1) % M;
			double d0x = px[j] - px[a], d0y = py[j] - py[a], d1x = px[b] - px[j], d1y = py[b] - py[j];
			tn[j] = fabs(atan2(d0x * d1y - d0y * d1x, d0x * d1x + d0y * d1y));
		}
		for (int j = 0; j < M; ++j) sm[j] = tn[(j + M - 1) % M] + tn[j] + tn[(j + 1) % M];
		const double thr = 45.0 * PI2 / 360.0;
		for (int j = 0; j < M; ++j)
		{
			if (sm[j] < thr) continue;
			bool mx = true;
			for (int d = 1; d <= 4 && mx; ++d)
			{
				if (sm[(j + M - d) % M] > sm[j]) mx = false;
				if (sm[(j + d) % M] >= sm[j]) mx = false;
			}
			if (mx) out.push_back(PI2 * j / M);
		}
		return out;
	};
	std::vector<double> tS;								// path arclength at each target
	// radius of the outline at angle th (frame of the start section) at station k. The profiles of the targets are interpolated with a
	// cubic Hermite spline in the path parameter (finite-difference slopes, limited so that there is no overshoot), so that the surface
	// does not get a crease at every section.
	auto profAt = [&](int k, double th) {
		const int nt2 = (int)prof.size();
		int t = stT[k]; double u = stU[k];
		auto val = [&](int q) {
			q = std::min(std::max(q, 0), nt2 - 1);
			double g = th;
			if (!nuAnc.empty()) g = warp(stKap[k], Aang[q], th);
			return evalProfile(prof[q], g + alpha[q]);
		};
		double v0 = val(t), v1 = val(t + 1);
		double h = tS[t + 1] - tS[t];
		double m0 = v1 - v0, m1 = v1 - v0;
		if ((t > 0) && (tS[t + 1] > tS[t - 1]))
		{
			double vm = val(t - 1);
			m0 = ((v0 - vm) * (v1 - v0) > 0) ? h * (v1 - vm) / (tS[t + 1] - tS[t - 1]) : 0.0;
		}
		if ((t + 2 < nt2) && (tS[t + 2] > tS[t]))
		{
			double vp = val(t + 2);
			m1 = ((v1 - v0) * (vp - v1) > 0) ? h * (vp - v0) / (tS[t + 2] - tS[t]) : 0.0;
		}
		double u2 = u * u, u3 = u2 * u;
		return (2 * u3 - 3 * u2 + 1) * v0 + (u3 - 2 * u2 + u) * m0 + (-2 * u3 + 3 * u2) * v1 + (u3 - u2) * m1;
	};

	if (!m_outlines.empty() && (m_layers <= 0) && m_rotate && !m_covariance.empty() && (m_outlines.size() == m_outlineIndex.size()) && (m_outlines.size() >= 2)
		&& (m_outlineIndex[0] == 0) && (m_outlineIndex.back() * step == nst))
	{
		const int nt = (int)m_outlines.size();
		bool ok = true;
		prof.assign(nt, std::vector<double>(M, 0.0));
		for (int t = 0; t < nt && ok; ++t)
		{
			if (t > 0 && m_outlineIndex[t] <= m_outlineIndex[t - 1]) { ok = false; break; }
			int k = m_outlineIndex[t] * step;
			vec3d b1 = Rks[k] * e1, b2 = Rks[k] * e2;
			std::vector<double>& R = prof[t];

			// project the outline into the plane of the section
			struct S2 { double ax, ay, ex, ey; };
			std::vector<S2> segs;
			for (const auto& sg : m_outlines[t])
			{
				vec3d a = sg.first - P[k], b = sg.second - P[k];
				S2 q = { a * b1, a * b2, (b - a) * b1, (b - a) * b2 };
				segs.push_back(q);
			}
			int nmiss = 0;
			for (int j = 0; j < M; ++j)
			{
				double dx = cos(PI2 * j / M), dy = sin(PI2 * j / M);
				double best = 0;
				for (const S2& q : segs)
				{
					double den = dx * q.ey - dy * q.ex;
					if (fabs(den) < 1e-14) continue;
					double tt = (q.ax * q.ey - q.ay * q.ex) / den;
					double uu = (q.ax * dy - q.ay * dx) / den;
					if ((tt > 1e-12) && (uu >= -1e-9) && (uu <= 1.0 + 1e-9) && (tt > best)) best = tt;
				}
				R[j] = best;
				if (best <= 0) nmiss++;
			}
			if (nmiss == M) { ok = false; break; }
			if (nmiss > 0)
			{
				// fill gaps by interpolating between the neighbouring hits
				for (int j = 0; j < M; ++j)
				{
					if (R[j] > 0) continue;
					int jl = j, jr = j, dl = 0, dr = 0;
					while (R[(jl + M) % M] <= 0) { jl--; dl++; }
					while (R[jr % M] <= 0) { jr++; dr++; }
					double vl = R[(jl + M) % M], vr = R[jr % M];
					R[j] = (vl * dr + vr * dl) / (double)(dl + dr);
				}
			}
		}

		if (ok && m_smoothStart)
		{
			prof0orig = prof[0];
			std::vector<double> S(M, 0.0);
			for (int h = 0; h <= 4; ++h)
			{
				double a = 0, b = 0;
				for (int j = 0; j < M; ++j) { double th = PI2 * j / M; a += prof0orig[j] * cos(h * th); b += prof0orig[j] * sin(h * th); }
				double f = (h == 0 ? 1.0 : 2.0) / M;
				a *= f; b *= f;
				for (int j = 0; j < M; ++j) { double th = PI2 * j / M; S[j] += a * cos(h * th) + b * sin(h * th); }
			}
			bool pos = true;
			for (int j = 0; j < M; ++j) if (S[j] <= 0.2 * prof0orig[j]) pos = false;
			if (pos) { prof[0] = S; smoothOn = true; }
		}

		if (ok)
		{
			// Optional in-plane rotation of the targets so that the radial outline profiles agree. Each target is aligned with the
			// previous one, within a small range, and only if the shapes are clearly non-circular and the rotation clearly helps;
			// otherwise the sections keep the (minimal-twist) transported orientation.
			alpha.assign(nt, 0.0);
			if (m_alignTwist)
			{
				auto stats = [&](const std::vector<double>& R, double& mean, double& cv) {
					mean = 0; for (double v : R) mean += v; mean /= M;
					double var = 0; for (double v : R) var += (v - mean) * (v - mean);
					cv = sqrt(var / M) / mean;
				};
				const int maxShift = M * 15 / 360;
				std::vector<int> shiftS(nt, 0);
				auto wrap = [&](int x) { return ((x % M) + M) % M; };
				for (int t = 1; t < nt; ++t)
				{
					double mp, cp, mt, ct;
					stats(prof[t - 1], mp, cp);
					stats(prof[t], mt, ct);
					int base = shiftS[t - 1], bestShift = 0;
					if (std::min(cp, ct) > 0.05)
					{
						std::vector<double> err(2 * maxShift + 1);
						for (int d = -maxShift; d <= maxShift; ++d)
						{
							double e = 0;
							for (int j = 0; j < M; ++j)
							{
								double v = prof[t][wrap(j + base + d)] / mt - prof[t - 1][wrap(j + base)] / mp;
								e += v * v;
							}
							err[d + maxShift] = e;
						}
						double emin = *std::min_element(err.begin(), err.end());
						int bd = 0, bestAbs = M;
						for (int d = -maxShift; d <= maxShift; ++d)
							if ((err[d + maxShift] <= emin * (1.0 + 1e-6) + 1e-12) && (abs(d) < bestAbs)) { bestAbs = abs(d); bd = d; }
						if (emin < 0.8 * err[maxShift]) bestShift = bd;
					}
					shiftS[t] = base + bestShift;
					alpha[t] = PI2 * shiftS[t] / (double)M;
				}
			}

			// corner anchoring
			{
				std::vector<std::vector<double>> cor(nt);
				int hist[16] = { 0 };
				for (int t = 0; t < nt; ++t)
				{
					if (t > 0) cor[t] = findCorners(prof[t]);
					for (double& c : cor[t]) c = wrap2(c - alpha[t]);
					std::sort(cor[t].begin(), cor[t].end());
					if (cor[t].size() >= 2 && cor[t].size() < 16) hist[cor[t].size()]++;
				}
				int N = 0, nbest = 0;
				for (int c = 2; c < 16; ++c) if (hist[c] > nbest) { nbest = hist[c]; N = c; }
				if (N >= 2 && N <= 8)
				{
					Aang.assign(nt, std::vector<double>());
					int pc = -1, first = -1;
					for (int t = 0; t < nt; ++t)
					{
						if ((int)cor[t].size() != N) continue;
						if (pc < 0) { Aang[t] = cor[t]; first = t; }
						else
						{
							int bs = 0; double be = 1e30;
							for (int sft = 0; sft < N; ++sft)
							{
								double e = 0;
								for (int i = 0; i < N; ++i) { double d = wrapPM(cor[t][(i + sft) % N] - Aang[pc][i]); e += d * d; }
								if (e < be) { be = e; bs = sft; }
							}
							Aang[t].resize(N);
							for (int i = 0; i < N; ++i) Aang[t][i] = cor[t][(i + bs) % N];
						}
						pc = t;
					}
					// anchor nodes on the boundary of the start section
					std::map<std::pair<int, int>, int> ecount;
					for (int e = 0; e < sec.Elements(); ++e)
					{
						int nc = sec.ecorners[e];
						for (int c = 0; c < nc; ++c)
						{
							int a = sec.enodes[e][c], b = sec.enodes[e][(c + 1) % nc];
							ecount[{ std::min(a, b), std::max(a, b) }]++;
						}
					}
					std::vector<char> bnd(nn, 0);
					for (auto& ec : ecount) if (ec.second == 1) { bnd[ec.first.first] = 1; bnd[ec.first.second] = 1; }
					int last = -1;
					for (int t = 0; t < nt; ++t) if (!Aang[t].empty()) last = t;
					// Targets whose corners are still far from those of the finished shape (a section that the divider has only just started to
					// cut) are not anchored: squeezing all the nodes of the later chord onto their tiny chord would collapse the elements into a fan.
					for (int t = 0; t < nt && last >= 0; ++t)
					{
						if (Aang[t].empty() || t == last) continue;
						double dmax = 0;
						for (int i = 0; i < N; ++i) dmax = std::max(dmax, fabs(wrapPM(Aang[t][i] - Aang[last][i])));
						if (dmax > 30.0 * PI2 / 360.0) Aang[t].clear();
					}
					first = -1;
					for (int t = 0; t < nt; ++t) if (!Aang[t].empty()) { first = t; break; }
					std::vector<double> nu(N);
					std::vector<int> nuNode(N, -1);
					bool good = (first >= 0) && (last >= 0);
					for (int i = 0; i < N && good; ++i)
					{
						double bd = 1e30;
						for (int q = 0; q < nn; ++q)
						{
							if (!bnd[q] || (sec.cornerIndex[q] < 0)) continue;
							vec3d d = r0[q] - P[0];
							double ph = atan2(d * e2, d * e1);
							double da = fabs(wrapPM(ph - Aang[last][i]));
							if (da < bd) { bd = da; nu[i] = wrap2(ph); nuNode[i] = q; }
						}
						if (nuNode[i] < 0) good = false;
					}
					if (good)
					{
						double sum = 0;
						for (int i = 0; i < N; ++i)
						{
							if (nuNode[i] == nuNode[(i + 1) % N]) good = false;
							sum += wrap2(nu[(i + 1) % N] - nu[i]);
						}
						if (fabs(sum - PI2) > 1e-6 && !(N == 1)) good = false;
					}
					if (good)
					{
						// the anchors are the corners of the last cornered target; targets before the first cornered one start at the anchor nodes
						// (no corner yet), and uncornered targets in between hold the previous corners
						nuAnc = nu;
						for (int t = 0; t < nt; ++t)
						{
							if (!Aang[t].empty()) continue;
							if (t < first) Aang[t] = nu; else Aang[t] = Aang[t - 1];
						}
					}
				}
			}

			// arclength of the targets along the path
			{
				std::vector<double> cum(nst + 1, 0.0);
				for (int k = 1; k <= nst; ++k) cum[k] = cum[k - 1] + (P[k] - P[k - 1]).norm();
				tS.assign(nt, 0.0);
				for (int t = 0; t < nt; ++t) tS[t] = cum[m_outlineIndex[t] * step];
			}

			// station data
			for (int t = 0; t + 1 < nt; ++t)
			{
				int k0 = m_outlineIndex[t] * step, k1 = m_outlineIndex[t + 1] * step;
				for (int k = k0; k <= k1; ++k)
				{
					stT[k] = t;
					double u = (double)(k - k0) / (double)(k1 - k0);
					stU[k] = u;
					stAlpha[k] = alpha[t] * (1.0 - u) + alpha[t + 1] * u;
					if (!nuAnc.empty())
					{
						stKap[k].resize(nuAnc.size());
						for (size_t i = 0; i < nuAnc.size(); ++i) stKap[k][i] = Aang[t][i] + u * wrapPM(Aang[t + 1][i] - Aang[t][i]);
					}
					double fb = 0;
					for (int j = 0; j < M; ++j)
					{
						double phi = PI2 * j / M;
						double th = nuAnc.empty() ? phi : warp(nuAnc, stKap[k], phi);
						fb += profAt(k, th) / prof[0][j];
					}
					stFbar[k] = fb / M;
				}
			}
			outlineMode = true;
		}
	}

	m_outlineUsed = outlineMode;

	// In outline mode the sections end up exactly in the planes of the targets: a slightly non-planar start section (a dished or tilted
	// cap) would otherwise carry its out-of-plane offsets along the whole sweep and leave gaps/overlaps where the solid meets another one.
	bool flatten = false;
	if (outlineMode)
	{
		double maxPerp = 0, maxR = 0;
		for (int i = 0; i < nn; ++i)
		{
			vec3d d = r0[i] - P[0];
			double x = d * e1, y = d * e2;
			vec3d dp = d - e1 * x - e2 * y;
			maxPerp = std::max(maxPerp, dp.norm());
			maxR = std::max(maxR, sqrt(x * x + y * y));
		}
		flatten = (maxPerp < 0.1 * maxR);
	}

	// position of the section point r (global) at path station k with rotation Rk
	auto place = [&](const vec3d& r, int k, const Mat3& Rk) {
		vec3d d = r - P[0];
		if (outlineMode)
		{
			double x = d * e1, y = d * e2;
			vec3d dperp = d - e1 * x - e2 * y;
			double rr = sqrt(x * x + y * y);
			double a = 0, b = 0;
			if (rr > 1e-14)
			{
				double phi = atan2(y, x);
				int t = stT[k];
				double u = stU[k];
				double Rs = evalProfile(prof[0], phi);
				if (smoothOn)
				{
					// move the start section onto its smooth outline (boundary nodes fully, interior nodes in proportion to rho^2)
					double Ro = evalProfile(prof0orig, phi);
					double c = Rs / Ro;
					double rho0 = std::min(rr / Ro, 1.0);
					rr *= 1.0 + (c - 1.0) * rho0 * rho0;
				}
				double rho = std::min(rr / Rs, 1.0);
				double th = phi;
				if (!nuAnc.empty()) th = phi + rho * rho * wrapPM(warp(nuAnc, stKap[k], phi) - phi);
				double Rt = profAt(k, th);
				double f = Rt / Rs;
				double sfac = stFbar[k] + (f - stFbar[k]) * rho * rho;
				a = rr * sfac * cos(th + stAlpha[k]);
				b = rr * sfac * sin(th + stAlpha[k]);
			}
			d = e1 * a + e2 * b + (flatten ? vec3d(0, 0, 0) : dperp);
		}
		else if (deform)
		{
			vec3d b1 = m_rotate ? (Rk * e1) : e1, b2 = m_rotate ? (Rk * e2) : e2;
			const double* C = &covk[6 * k];
			double T[4];
			if (sqrt2(quad(C, b1, b1), quad(C, b1, b2), quad(C, b2, b2), T))
			{
				// S2 = T * IS
				double S2[4] = { T[0] * IS[0] + T[1] * IS[2], T[0] * IS[1] + T[1] * IS[3], T[2] * IS[0] + T[3] * IS[2], T[2] * IS[1] + T[3] * IS[3] };
				double x = d * e1, y = d * e2;
				vec3d dperp = d - e1 * x - e2 * y;
				d = e1 * (S2[0] * x + S2[1] * y) + e2 * (S2[2] * x + S2[3] * y) + dperp;
			}
		}
		else d = d * sc[k];
		return P[k] + (m_rotate ? (Rk * d) : d);
	};

	for (int k = 0; k <= nst; ++k)
	{
		const Mat3& Rk = Rks[k];
		if (k % step == 0)
		{
			int l = k / step;
			for (int i = 0; i < nn; ++i) layerPos[l * nn + i] = place(r0[i], k, Rk);
		}
		else
		{
			int l = k / step;
			for (int c = 0; c < sec.ncorner; ++c) midPos[l * sec.ncorner + c] = place(r0[sec.cornerNode[c]], k, Rk);
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
