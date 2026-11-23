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
#include "FEHexDominantRemesh.h"
#ifdef HAS_HEXDOM
#include "rhdm/rhdm_api.h"
#include "FESurfaceProjector.h"
#include "FEMeshUntangler.h"
#endif
#ifdef HAS_MMG
#include "FEMMGRemesh.h"
#endif
#include <vector>
#include <map>
#include <set>
#include <array>
#include <memory>
#include <functional>
#include <queue>
#include <unordered_map>
#include <algorithm>
using namespace std;

extern int EL_TET[6][2]; // in lut.cpp

//=========================================================================================
HexDominantRemesh::HexDominantRemesh() : FEModifier("Hex-dominant remesh")
{
	AddDoubleParam(0.0, "H", "Element size (0 = auto)");
	AddIntParam(200, "iterations", "Smoothing iterations per level");
	AddIntParam(0, "threads", "Threads (0 = all, 1 = deterministic)");
	AddDoubleParam(45.0, "angle", "Feature angle (degrees)");
	AddChoiceParam(0, "remesh_input", "Remesh input with MMG")->SetEnumNames("if too coarse\0always\0never\0");
}

FSMesh* HexDominantRemesh::Apply(FSGroup* pg)
{
	// remeshing a selection is not supported; always remesh the whole mesh
	if (pg == nullptr) return nullptr;
	return Apply(pg->GetMesh());
}

#ifdef HAS_HEXDOM
namespace {

	const double kPi = 3.14159265358979323846;

	// Newell normal of a polygon (not normalized)
	vec3d polygonNormal(const vector<vec3d>& r, const vector<int>& f)
	{
		vec3d n(0, 0, 0);
		int m = (int)f.size();
		for (int i = 0; i < m; ++i)
		{
			const vec3d& a = r[f[i]];
			const vec3d& b = r[f[(i + 1) % m]];
			n.x += (a.y - b.y) * (a.z + b.z);
			n.y += (a.z - b.z) * (a.x + b.x);
			n.z += (a.x - b.x) * (a.y + b.y);
		}
		return n;
	}

	vec3d polygonCenter(const vector<vec3d>& r, const vector<int>& f)
	{
		vec3d c(0, 0, 0);
		for (int n : f) c += r[n];
		return c / (double)f.size();
	}

	double tetVolume(const vec3d& a, const vec3d& b, const vec3d& c, const vec3d& d)
	{
		return ((b - a) ^ (c - a)) * (d - a) / 6.0;
	}

	// Orient the faces of a closed polyhedral cell consistently (outward up to a global sign).
	// Returns false if the faces do not form a closed, orientable surface.
	bool orientFaces(const vector<vector<int> >& faces, const vector<int>& cell, vector<vector<int> >& of)
	{
		int nf = (int)cell.size();
		of.resize(nf);
		for (int i = 0; i < nf; ++i) of[i] = faces[cell[i]];

		// map each undirected edge to the faces that use it
		map<pair<int, int>, vector<int> > edgeFaces;
		for (int i = 0; i < nf; ++i)
		{
			int m = (int)of[i].size();
			for (int j = 0; j < m; ++j)
			{
				int a = of[i][j], b = of[i][(j + 1) % m];
				edgeFaces[make_pair(min(a, b), max(a, b))].push_back(i);
			}
		}
		for (auto& it : edgeFaces) if (it.second.size() != 2) return false;

		// does face f traverse the directed edge a->b?
		auto hasDirected = [&](int f, int a, int b) {
			int m = (int)of[f].size();
			for (int j = 0; j < m; ++j) if ((of[f][j] == a) && (of[f][(j + 1) % m] == b)) return true;
			return false;
		};

		// breadth-first propagation: neighbors must traverse shared edges in opposite directions
		vector<bool> done(nf, false);
		vector<int> queue(1, 0); done[0] = true;
		for (size_t q = 0; q < queue.size(); ++q)
		{
			int f = queue[q];
			int m = (int)of[f].size();
			for (int j = 0; j < m; ++j)
			{
				int a = of[f][j], b = of[f][(j + 1) % m];
				for (int g : edgeFaces[make_pair(min(a, b), max(a, b))])
				{
					if ((g == f) || done[g]) continue;
					if (hasDirected(g, a, b)) reverse(of[g].begin(), of[g].end());
					done[g] = true;
					queue.push_back(g);
				}
			}
		}
		for (bool b : done) if (!b) return false;
		return true;
	}

	// volume enclosed by consistently oriented faces (positive if they point outward)
	double signedVolume(const vector<vec3d>& r, const vector<vector<int> >& of)
	{
		vec3d o = r[of[0][0]];
		double V = 0;
		for (auto& f : of)
			for (size_t j = 1; j + 1 < f.size(); ++j) V += tetVolume(o, r[f[0]], r[f[j]], r[f[j + 1]]);
		return V;
	}

	void flipFaces(vector<vector<int> >& of) { for (auto& f : of) reverse(f.begin(), f.end()); }

	// do oriented faces form a closed surface (each directed edge matched by its reverse)?
	bool isClosed(const vector<vector<int> >& of)
	{
		map<pair<int, int>, int> e;
		for (auto& f : of)
			for (size_t j = 0; j < f.size(); ++j) e[make_pair(f[j], f[(j + 1) % f.size()])]++;
		for (auto& it : e)
		{
			if (it.second != 1) return false;
			auto rev = e.find(make_pair(it.first.second, it.first.first));
			if ((rev == e.end()) || (rev->second != 1)) return false;
		}
		return true;
	}

	// Merge vertices that are closer than tol, then remove the faces that degenerate, glue
	// faces that became identical, and drop cells that collapse completely (their faces
	// cancel in pairs). Returns the number of merged vertices, or -1 if the result is not a
	// valid cell complex (the inputs are then not modified).
	int weldVertices(const vector<vec3d>& r, double tol, vector<vector<int> >& faces, vector<vector<int> >& cells, vector<bool>& isHex)
	{
		int NV = (int)r.size();
		if ((NV == 0) || (tol <= 0)) return 0;

		vector<int> parent(NV);
		for (int i = 0; i < NV; ++i) parent[i] = i;
		std::function<int(int)> root = [&](int i) { while (parent[i] != i) { parent[i] = parent[parent[i]]; i = parent[i]; } return i; };

		// spatial hash with cells of size tol
		auto cellKey = [](long long i, long long j, long long k) { return ((i & 0x1FFFFF) << 42) | ((j & 0x1FFFFF) << 21) | (k & 0x1FFFFF); };
		unordered_map<long long, vector<int> > grid;
		int nmerged = 0;
		for (int i = 0; i < NV; ++i)
		{
			long long ix = (long long)floor(r[i].x / tol), iy = (long long)floor(r[i].y / tol), iz = (long long)floor(r[i].z / tol);
			for (int dz = -1; dz <= 1; ++dz)
				for (int dy = -1; dy <= 1; ++dy)
					for (int dx = -1; dx <= 1; ++dx)
					{
						auto it = grid.find(cellKey(ix + dx, iy + dy, iz + dz));
						if (it == grid.end()) continue;
						for (int j : it->second)
						{
							if ((r[i] - r[j]).Length() >= tol) continue;
							int a = root(i), b = root(j);
							if (a != b) { parent[max(a, b)] = min(a, b); nmerged++; }
						}
					}
			grid[cellKey(ix, iy, iz)].push_back(i);
		}
		if (nmerged == 0) return 0;

		// faces: replace merged vertices, drop repeated consecutive vertices and spikes (an edge
		// a-b-a traversed back and forth, of zero area), glue identical faces
		vector<int> canon(faces.size(), -1);
		map<vector<int>, int> keyToId;
		vector<vector<int> > newFaces;
		for (size_t f = 0; f < faces.size(); ++f)
		{
			vector<int> p;
			for (int v : faces[f]) p.push_back(root(v));
			for (bool changed = true; changed && (p.size() >= 3); )
			{
				changed = false;
				int m = (int)p.size();
				for (int i = 0; i < m; ++i)
				{
					int i1 = (i + 1) % m, i2 = (i + 2) % m;
					if (p[i] == p[i1]) { p.erase(p.begin() + i1); changed = true; break; }
					if (p[i] == p[i2])
					{
						p.erase(p.begin() + max(i1, i2));
						p.erase(p.begin() + min(i1, i2));
						changed = true;
						break;
					}
				}
			}
			set<int> u(p.begin(), p.end());
			if (u.size() < 3) continue;				// degenerate face
			if (u.size() != p.size()) return -1;	// pinched face
			vector<int> key = p;
			sort(key.begin(), key.end());
			auto it = keyToId.find(key);
			if (it == keyToId.end()) { it = keyToId.insert(make_pair(key, (int)newFaces.size())).first; newFaces.push_back(p); }
			canon[f] = it->second;
		}

		// cells: faces that appear twice in a cell cancel; cells without faces are removed
		vector<vector<int> > newCells;
		vector<bool> newHex;
		for (size_t c = 0; c < cells.size(); ++c)
		{
			map<int, int> cnt;
			for (int f : cells[c]) if (canon[f] >= 0) cnt[canon[f]]++;
			vector<int> cf;
			for (auto& it : cnt) if (it.second % 2 == 1) cf.push_back(it.first);
			if (cf.empty()) continue;
			bool hex = isHex[c] && (cf.size() == 6);
			for (int f : cf) if (newFaces[f].size() != 4) hex = false;
			newCells.push_back(cf);
			newHex.push_back(hex);
		}

		// check the result: faces shared by at most two cells, closed cells
		vector<int> use(newFaces.size(), 0);
		for (auto& c : newCells) for (int f : c) if (++use[f] > 2) return -1;
		for (auto& c : newCells)
		{
			vector<vector<int> > of;
			if (!orientFaces(newFaces, c, of)) return -1;
		}

		faces.swap(newFaces);
		cells.swap(newCells);
		isHex.swap(newHex);
		return nmerged;
	}

	// Convert a hexahedral cell (6 quad faces) to HEX8 connectivity.
	// Returns false if the faces do not form a topological hexahedron.
	bool hexConnectivity(const vector<vec3d>& r, const vector<vector<int> >& faces, const vector<int>& cell, int* hn)
	{
		const vector<int>& bottom = faces[cell[0]];
		if (bottom.size() != 4) return false;

		// edge adjacency of the cell
		map<int, set<int> > adj;
		for (int fid : cell)
		{
			const vector<int>& f = faces[fid];
			if (f.size() != 4) return false;
			for (int i = 0; i < 4; ++i)
			{
				int a = f[i], b = f[(i + 1) % 4];
				adj[a].insert(b);
				adj[b].insert(a);
			}
		}
		if (adj.size() != 8) return false;

		int top[4];
		for (int i = 0; i < 4; ++i)
		{
			int a = bottom[i];
			int t = -1, nt = 0;
			for (int b : adj[a])
			{
				if (find(bottom.begin(), bottom.end(), b) == bottom.end()) { t = b; nt++; }
			}
			if (nt != 1) return false;
			top[i] = t;
		}

		// orient so that the bottom-face normal points toward the top face
		vector<int> bot(bottom.begin(), bottom.end());
		vector<int> tp(top, top + 4);
		vec3d nb = polygonNormal(r, bot);
		vec3d d = polygonCenter(r, tp) - polygonCenter(r, bot);
		if (nb * d < 0)
		{
			swap(bot[1], bot[3]);
			swap(tp[1], tp[3]);
		}
		for (int i = 0; i < 4; ++i) { hn[i] = bot[i]; hn[i + 4] = tp[i]; }
		return true;
	}

	// Maximize t subject to n_k.x + d_k >= t (with |n_k| = 1) and lo <= x <= hi, with a dense
	// simplex method and Bland's rule (the problem has only four unknowns). Returns t.
	double deepestPoint(const vector<array<double, 4> >& P, const vec3d& lo, const vec3d& hi, vec3d& x)
	{
		int np = (int)P.size();
		int m = np + 3;		// constraints
		int nc = 4 + m;		// variables: y (3), t' and one slack per constraint
		vector<double> A((size_t)m * (nc + 1), 0.0);
		auto at = [&](int i, int j) -> double& { return A[(size_t)i * (nc + 1) + j]; };

		// shift the unknowns: x = lo + y (y >= 0), t = t' - T (t' >= 0)
		double T = 0;
		for (auto& p : P) T = max(T, -(p[0] * lo.x + p[1] * lo.y + p[2] * lo.z + p[3]));
		T += 1.0;
		for (int k = 0; k < np; ++k)
		{
			const array<double, 4>& p = P[k];
			at(k, 0) = -p[0]; at(k, 1) = -p[1]; at(k, 2) = -p[2]; at(k, 3) = 1.0;
			at(k, 4 + k) = 1.0;
			at(k, nc) = p[0] * lo.x + p[1] * lo.y + p[2] * lo.z + p[3] + T;
		}
		const double w[3] = { hi.x - lo.x, hi.y - lo.y, hi.z - lo.z };
		for (int j = 0; j < 3; ++j)
		{
			int k = np + j;
			at(k, j) = 1.0; at(k, 4 + k) = 1.0; at(k, nc) = w[j];
		}
		vector<int> basis(m);
		for (int i = 0; i < m; ++i) basis[i] = 4 + i;
		vector<double> obj(nc + 1, 0.0);
		obj[3] = -1.0;	// maximize t'

		const double tol = 1e-12;
		for (int iter = 0; iter < 50 * m; ++iter)
		{
			// entering variable: smallest index with a negative reduced cost
			int e = -1;
			for (int j = 0; j < nc; ++j) if (obj[j] < -tol) { e = j; break; }
			if (e < 0) break;

			// leaving variable: minimum ratio, ties broken by the smallest basic index
			int l = -1;
			double best = 0;
			for (int i = 0; i < m; ++i)
			{
				double a = at(i, e);
				if (a > tol)
				{
					double ratio = at(i, nc) / a;
					if ((l < 0) || (ratio < best - tol) || ((ratio <= best + tol) && (basis[i] < basis[l]))) { l = i; best = ratio; }
				}
			}
			if (l < 0) break;	// unbounded (not possible, x is boxed)

			double piv = at(l, e);
			for (int j = 0; j <= nc; ++j) at(l, j) /= piv;
			for (int i = 0; i < m; ++i)
			{
				if (i == l) continue;
				double f = at(i, e);
				if (f != 0) for (int j = 0; j <= nc; ++j) at(i, j) -= f * at(l, j);
			}
			double f = obj[e];
			if (f != 0) for (int j = 0; j <= nc; ++j) obj[j] -= f * at(l, j);
			basis[l] = e;
		}

		double z[4] = { 0, 0, 0, 0 };
		for (int i = 0; i < m; ++i) if (basis[i] < 4) z[basis[i]] = at(i, nc);
		x = vec3d(lo.x + z[0], lo.y + z[1], lo.z + z[2]);
		return z[3] - T;
	}

	// Find the split point x of a polyhedral cell with outward faces 'of'. The cell is split
	// into a TET4 on each triangle, a PYRA5 on each quad, and TET4s around the center fc[k]
	// of each larger face. The Jacobian of every sub-element at FEBio's integration
	// points is affine in x (the PYRA5 apex shape function is (1+t)/2), so the valid split
	// points form a convex polytope. Linear programming finds the point farthest from its
	// bounding planes. That point is then moved toward the vertex average for as long as
	// the margin stays above half of its optimum.
	// With corners = true, the corners of the PYRA5 bases are valid as well: x is above the
	// plane of the two base edges at each corner (also affine in x). FEBio only checks the
	// integration points, but a folded corner gives inverted elements when the mesh is refined.
	// Returns the margin relative to the cell size; it is positive if all sub-elements are valid.
	double splitPoint(const vector<vec3d>& r, const vector<vector<int> >& of, const vector<vec3d>& fc, vec3d& x, bool corners = false)
	{
		set<int> vs;
		for (auto& f : of) for (int n : f) vs.insert(n);
		vec3d c0(0, 0, 0);
		for (int n : vs) c0 += r[n];
		c0 /= (double)vs.size();
		double hc = 0;
		for (int n : vs) hc = max(hc, (r[n] - c0).Length());
		x = c0;
		if (hc <= 0) return 0;

		// constraints a.x + b >= 0, stored as n.u + d >= 0 with |n| = 1 and u = (x - c0)/hc
		vector<array<double, 4> > P;
		auto addPlane = [&](const vec3d& a, double b) {
			double L = a.Length();
			if (L <= 1e-14 * hc * hc) return;	// does not depend on x
			P.push_back({ a.x / L, a.y / L, a.z / L, (a * c0 + b) / (L * hc) });
		};

		const double ga = 1.0 / sqrt(3.0);
		static const double hr[8] = { -1, 1, 1, -1, -1, 1, 1, -1 };
		static const double hs[8] = { -1, -1, 1, 1, -1, -1, 1, 1 };
		static const double ht[8] = { -1, -1, -1, -1, 1, 1, 1, 1 };
		for (size_t k = 0; k < of.size(); ++k)
		{
			const vector<int>& f = of[k];
			int m = (int)f.size();
			if (m == 3)
			{
				// tet (f0, f2, f1, x)
				vec3d a = (r[f[2]] - r[f[0]]) ^ (r[f[1]] - r[f[0]]);
				addPlane(a, -(a * r[f[0]]));
			}
			else if (m == 4)
			{
				// pyramid with base (f3, f2, f1, f0) and apex x: det J = (gr x gs).(gt0 + x/2)
				vec3d b[4] = { r[f[3]], r[f[2]], r[f[1]], r[f[0]] };
				for (int g = 0; g < 8; ++g)
				{
					double R = ga * hr[g], S = ga * hs[g], T = ga * ht[g];
					double Hr[4] = { -0.125 * (1 - S) * (1 - T), 0.125 * (1 - S) * (1 - T), 0.125 * (1 + S) * (1 - T), -0.125 * (1 + S) * (1 - T) };
					double Hs[4] = { -0.125 * (1 - R) * (1 - T), -0.125 * (1 + R) * (1 - T), 0.125 * (1 + R) * (1 - T), 0.125 * (1 - R) * (1 - T) };
					double Ht[4] = { -0.125 * (1 - R) * (1 - S), -0.125 * (1 + R) * (1 - S), -0.125 * (1 + R) * (1 + S), -0.125 * (1 - R) * (1 + S) };
					vec3d gr(0, 0, 0), gs(0, 0, 0), gt(0, 0, 0);
					for (int i = 0; i < 4; ++i) { gr += b[i] * Hr[i]; gs += b[i] * Hs[i]; gt += b[i] * Ht[i]; }
					vec3d n = gr ^ gs;
					addPlane(n * 0.5, n * gt);
				}
				if (corners)
				{
					// det of the two base edges at corner i and the edge to the apex
					for (int i = 0; i < 4; ++i)
					{
						vec3d a = (b[(i + 1) % 4] - b[i]) ^ (b[(i + 3) % 4] - b[i]);
						addPlane(a, -(a * b[i]));
					}
				}
			}
			else
			{
				// tets (f[i+1], f[i], fc, x)
				const vec3d& c = fc[k];
				for (int i = 0; i < m; ++i)
				{
					const vec3d& p0 = r[f[(i + 1) % m]];
					vec3d a = (r[f[i]] - p0) ^ (c - p0);
					addPlane(a, -(a * p0));
				}
			}
		}
		if (P.empty()) return 0;

		vec3d u;
		double t = deepestPoint(P, vec3d(-1, -1, -1), vec3d(1, 1, 1), u);
		if (t > 0)
		{
			// move toward the vertex average (u = 0) while all margins stay >= t/2
			double alpha = 1.0;
			for (auto& p : P)
			{
				double g0 = p[0] * u.x + p[1] * u.y + p[2] * u.z + p[3];
				double g1 = p[3];
				if (g1 < 0.5 * t) alpha = min(alpha, (g0 - 0.5 * t) / (g0 - g1));
			}
			u = u * (1.0 - max(0.0, alpha));
			t = 1e300;
			for (auto& p : P) t = min(t, p[0] * u.x + p[1] * u.y + p[2] * u.z + p[3]);
		}
		x = c0 + u * hc;
		return t;
	}

	// Quality of the sub-elements that splitPoint creates on face f (oriented outward) with the
	// split point p: the smallest scaled Jacobian of the TET4 on a triangle, the PYRA5 on a quad,
	// or the TET4s (f[i+1], f[i], fc, p) on a larger face with center fc. It is <= 0 if one of
	// them is inverted or degenerate.
	double faceQuality(const vector<vec3d>& r, const vector<int>& f, const vec3d& fc, const vec3d& p)
	{
		int m = (int)f.size();
		vector<vec3d> x(m + 2);
		for (int i = 0; i < m; ++i) x[i] = r[f[i]];
		x[m] = fc;
		x[m + 1] = p;
		FEMeshUntangler::Element e;
		for (int j = 0; j < 8; ++j) e.n[j] = 0;
		if (m == 3)
		{
			e.type = FEMeshUntangler::TET4;
			e.n[0] = 0; e.n[1] = 2; e.n[2] = 1; e.n[3] = m + 1;
			return FEMeshUntangler::ScaledJacobian(x, e);
		}
		if (m == 4)
		{
			e.type = FEMeshUntangler::PYRA5;
			e.n[0] = 3; e.n[1] = 2; e.n[2] = 1; e.n[3] = 0; e.n[4] = m + 1;
			return FEMeshUntangler::ScaledJacobian(x, e);
		}
		double q = 1e300;
		e.type = FEMeshUntangler::TET4;
		for (int i = 0; i < m; ++i)
		{
			e.n[0] = (i + 1) % m; e.n[1] = i; e.n[2] = m; e.n[3] = m + 1;
			q = min(q, FEMeshUntangler::ScaledJacobian(x, e));
		}
		return q;
	}

	// If a cell with outward faces 'of' is a tetrahedron, pyramid or triangular prism, return
	// its element type (FE_TET4, FE_PYRA5, FE_PENTA6) and connectivity; otherwise FE_INVALID_ELEMENT_TYPE.
	FSElementType primitiveElement(const vector<vector<int> >& of, int* en)
	{
		int ntri = 0, nquad = 0;
		set<int> vs;
		for (auto& f : of)
		{
			if (f.size() == 3) ntri++; else if (f.size() == 4) nquad++; else return FE_INVALID_ELEMENT_TYPE;
			for (int n : f) vs.insert(n);
		}
		int nf = (int)of.size();
		if ((nf == 4) && (ntri == 4) && (vs.size() == 4))
		{
			const vector<int>& f = of[0];
			for (int n : vs) if ((n != f[0]) && (n != f[1]) && (n != f[2])) en[3] = n;
			en[0] = f[0]; en[1] = f[2]; en[2] = f[1];
			return FE_TET4;
		}
		if ((nf == 5) && (ntri == 4) && (nquad == 1) && (vs.size() == 5))
		{
			int kq = 0;
			while (of[kq].size() != 4) kq++;
			const vector<int>& q = of[kq];
			int apex = -1;
			for (int n : vs) if (find(q.begin(), q.end(), n) == q.end()) apex = n;
			for (auto& f : of) if ((f.size() == 3) && (find(f.begin(), f.end(), apex) == f.end())) return FE_INVALID_ELEMENT_TYPE;
			en[0] = q[3]; en[1] = q[2]; en[2] = q[1]; en[3] = q[0]; en[4] = apex;
			return FE_PYRA5;
		}
		if ((nf == 5) && (ntri == 2) && (nquad == 3) && (vs.size() == 6))
		{
			const vector<int>* A = nullptr, * B = nullptr;
			for (auto& f : of) if (f.size() == 3) { if (A == nullptr) A = &f; else B = &f; }
			// bottom = reversed A (normal toward B); top = the vertices of B joined to them by quad edges
			int bot[3] = { (*A)[0], (*A)[2], (*A)[1] };
			for (int i = 0; i < 3; ++i)
			{
				int top = -1, ntop = 0;
				for (auto& f : of)
				{
					if (f.size() != 4) continue;
					for (int j = 0; j < 4; ++j)
					{
						int a = f[j], b = f[(j + 1) % 4];
						int other = (a == bot[i] ? b : (b == bot[i] ? a : -1));
						if ((other >= 0) && (find(B->begin(), B->end(), other) != B->end()))
						{
							if (other != top) { top = other; ntop++; }
						}
					}
				}
				if (ntop != 1) return FE_INVALID_ELEMENT_TYPE;
				en[i] = bot[i]; en[i + 3] = top;
			}
			return FE_PENTA6;
		}
		return FE_INVALID_ELEMENT_TYPE;
	}

	FEMeshUntangler::ElemType untanglerType(FSElementType t)
	{
		switch (t)
		{
		case FE_TET4: return FEMeshUntangler::TET4;
		case FE_PYRA5: return FEMeshUntangler::PYRA5;
		case FE_PENTA6: return FEMeshUntangler::PENTA6;
		default: return FEMeshUntangler::HEX8;
		}
	}

	int elementNodes(FSElementType t) { return (t == FE_TET4 ? 4 : (t == FE_PYRA5 ? 5 : (t == FE_PENTA6 ? 6 : 8))); }

	// is the primitive element (if any) of this cell valid?
	FSElementType validPrimitive(const vector<vec3d>& r, const vector<vector<int> >& of, int* en)
	{
		FSElementType t = primitiveElement(of, en);
		if (t == FE_INVALID_ELEMENT_TYPE) return t;
		FEMeshUntangler::Element e;
		e.type = untanglerType(t);
		for (int j = 0; j < 8; ++j) e.n[j] = (j < elementNodes(t) ? en[j] : 0);
		return (FEMeshUntangler::MinJacobian(r, e) > 0 ? t : FE_INVALID_ELEMENT_TYPE);
	}

	// boundary triangles of a TET4 mesh, oriented outward
	vector<array<int, 3> > tetBoundary(FSMesh* pm)
	{
		const int FT[4][3] = { {0,2,1}, {0,1,3}, {1,2,3}, {0,3,2} };
		map<array<int, 3>, pair<int, array<int, 3> > > faceMap;
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FSElement& el = pm->Element(i);
			for (int j = 0; j < 4; ++j)
			{
				array<int, 3> f = { el.m_node[FT[j][0]], el.m_node[FT[j][1]], el.m_node[FT[j][2]] };
				array<int, 3> key = f;
				sort(key.begin(), key.end());
				auto& v = faceMap[key];
				v.first++;
				v.second = f;
			}
		}
		vector<array<int, 3> > tris;
		for (auto& it : faceMap) if (it.second.first == 1) tris.push_back(it.second.second);
		return tris;
	}

	// forwards the log of a sub-task to this task's logger
	class ForwardLogger : public TaskLogger
	{
	public:
		ForwardLogger(FSThreadedTask* task) : m_task(task) {}
		void Log(const std::string& msg) override { m_task->Log("%s", msg.c_str()); }
	private:
		FSThreadedTask* m_task;
	};
}
#endif

FSMesh* HexDominantRemesh::Apply(FSMesh* pm)
{
#ifdef HAS_HEXDOM
	// The GUI applies the same modifier object repeatedly: clear the errors and the cancel
	// flag of a previous run.
	resetProgress();
	ClearErrors();

	auto canceled = [this]() -> FSMesh* {
		Log("Hex-dominant remesh canceled.\n");
		error("Canceled.");
		return nullptr;
	};

	if (pm == nullptr) { error("This object has no mesh."); return nullptr; }
	if (pm->IsType(FE_TET4) == false) { error("This modifier requires a TET4 mesh."); return nullptr; }

	const double featureAngle = GetFloatValue(FEATURE_ANGLE);
	int NN = pm->Nodes();
	int NE = pm->Elements();

	// --- the input surface; the boundary nodes of the new mesh are put back on it ---
	vector<vec3d> inputNodes(NN);
	for (int i = 0; i < NN; ++i) inputNodes[i] = pm->Node(i).pos();
	vector<array<int, 3> > inputTris;
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& f = pm->Face(i);
		if ((f.Nodes() == 3) && f.IsExternal()) inputTris.push_back({ f.n[0], f.n[1], f.n[2] });
	}
	if (inputTris.empty()) inputTris = tetBoundary(pm);
	if (inputTris.empty()) { error("The tetrahedral mesh has no boundary."); return nullptr; }

	double boundaryEdgeSum = 0;
	for (auto& t : inputTris)
		for (int j = 0; j < 3; ++j) boundaryEdgeSum += (inputNodes[t[j]] - inputNodes[t[(j + 1) % 3]]).Length();
	double avgBoundaryEdge = boundaryEdgeSum / (3.0 * inputTris.size());

	FESurfaceProjector proj;
	proj.Build(inputNodes, inputTris, featureAngle);

	// --- input statistics ---
	set<int> parts;
	double edgeSum = 0.0;
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = pm->Element(i);
		parts.insert(el.m_gid);
		for (int j = 0; j < 6; ++j)
		{
			vec3d a = pm->Node(el.m_node[EL_TET[j][0]]).pos();
			vec3d b = pm->Node(el.m_node[EL_TET[j][1]]).pos();
			edgeSum += (a - b).Length();
		}
	}
	double avgTetEdge = edgeSum / (6.0 * NE);
	if (parts.size() > 1)
		Log("Warning: the mesh has %d parts; the hex-dominant mesh will be a single part.\n", (int)parts.size());

	// target edge length; the automatic value is the one the mesher would choose itself:
	// 3.5 x the average edge length of the boundary triangles
	double Huser = GetFloatValue(ELEM_SIZE);
	double Htarget = (Huser > 0 ? Huser : 3.5 * avgBoundaryEdge);
	if (Huser > 0)
		Log("Element size: H = %lg\n", Htarget);
	else
		Log("Element size: H = %lg (automatic: 3.5 x the average boundary edge length, %lg)\n", Htarget, avgBoundaryEdge);
	Log("Input mesh: %d tets, average edge length = %lg\n", NE, avgTetEdge);

	// --- remesh the input with MMG if it is too coarse (or if requested) ---
	// Input tets up to about H work well; larger ones are remeshed to MMG_SIZE x H.
	FSMesh* tetMesh = pm;
	unique_ptr<FSMesh> remeshed;
	bool wasRemeshed = false;
	int remeshMode = GetIntValue(REMESH_INPUT);	// 0 = if too coarse, 1 = always, 2 = never
	const double MMG_SIZE = 0.5;
	bool coarse = (avgTetEdge > Htarget);
	if ((remeshMode == 1) || ((remeshMode == 0) && coarse))
	{
#ifdef HAS_MMG
		setCurrentTask("Remeshing input with MMG", 0.0);
		MMGRemesh mmg;
		Param* p = nullptr;
		if ((p = mmg.GetParam("H"))) p->SetFloatValue(MMG_SIZE * Htarget);
		if ((p = mmg.GetParam("hv"))) p->SetFloatValue(0.1 * Htarget);
		if ((p = mmg.GetParam("angle"))) p->SetFloatValue(featureAngle);
		ForwardLogger logger(this);
		mmg.SetTaskLogger(&logger);
		FSMesh* pmr = mmg.Apply(pm);
		if (pmr && pmr->IsType(FE_TET4) && (pmr->Elements() > 0))
		{
			remeshed.reset(pmr);
			tetMesh = pmr;
			wasRemeshed = true;
			Log("Remeshed the input with MMG (element size %lg): %d tets.\n", MMG_SIZE * Htarget, pmr->Elements());
		}
		else
		{
			delete pmr;
			Log("Warning: MMG could not remesh the input (%s); using the input mesh.\n", mmg.GetErrorString().c_str());
		}
#else
		Log("Warning: MMG is not available in this build; the input mesh is used as is.\n");
#endif
	}

	if (IsCanceled()) return canceled();

	// collect the mesher's input
	int TN = tetMesh->Nodes();
	int TE = tetMesh->Elements();
	vector<double> nodes(3 * TN);
	for (int i = 0; i < TN; ++i)
	{
		vec3d r = tetMesh->Node(i).pos();
		nodes[3 * i] = r.x; nodes[3 * i + 1] = r.y; nodes[3 * i + 2] = r.z;
	}
	vector<int> tets(4 * TE);
	for (int i = 0; i < TE; ++i)
	{
		FSElement& el = tetMesh->Element(i);
		for (int j = 0; j < 4; ++j) tets[4 * i + j] = el.m_node[j];
	}

	rhdm::HexDomParams params;
	params.edgeLength = Htarget;
	params.iterationsPerLevel = GetIntValue(ITERATIONS);
	params.threads = GetIntValue(THREADS);

	// sharp features of the mesher's input: the fields are aligned with them, so that the
	// lattice runs along the sharp edges instead of cutting them
	{
		FESurfaceProjector inputFeatures;
		const FESurfaceProjector* fp = &proj;
		if (wasRemeshed)
		{
			vector<vec3d> rn(TN);
			for (int i = 0; i < TN; ++i) rn[i] = tetMesh->Node(i).pos();
			inputFeatures.Build(rn, tetBoundary(tetMesh), featureAngle);
			fp = &inputFeatures;
		}
		params.featureType.assign(TN, 0);
		params.featureDir.assign(3 * TN, 0.0);
		for (int c = 0; c < fp->Curves(); ++c)
		{
			const FESurfaceProjector::Curve& C = fp->GetCurve(c);
			int m = (int)C.nodes.size();
			for (int k = 0; k < m; ++k)
			{
				if (!C.closed && ((k == 0) || (k == m - 1))) continue;
				vec3d t = fp->NodePosition(C.nodes[(k + 1) % m]) - fp->NodePosition(C.nodes[(k + m - 1) % m]);
				t.unit();
				int n = C.nodes[k];
				params.featureType[n] = 1;
				params.featureDir[3 * n] = t.x; params.featureDir[3 * n + 1] = t.y; params.featureDir[3 * n + 2] = t.z;
			}
		}
		for (int n : fp->CornerNodes()) params.featureType[n] = 2;
	}

	// --- run the mesher ---
	setCurrentTask("Hex-dominant meshing", 0.0);
	rhdm::HexDomResult res;
	string err;
	string stage;
	bool ok = rhdm::HexDominantMesh(nodes, tets, params, res, err,
		[this, &stage](double f, const char* sz) {
			// setCurrentTask writes to the log, so only call it when the stage changes
			if (stage != sz) { stage = sz; setCurrentTask(sz, 100.0 * f); }
			else setProgress(100.0 * f);
			return !IsCanceled();
		});
	if (IsCanceled()) return canceled();
	if (!ok) { error(err); return nullptr; }
	remeshed.reset();

	const double H = res.edgeLength;
	if (!wasRemeshed && (avgTetEdge > H))
		Log("Warning: the input tets are larger than the element size H on average. For best results,\n"
			"         use smaller tets, or let this modifier remesh the input with MMG.\n");

	// --- convert the polyhedral result into a conforming HEX8/PYRA5/TET4 mesh ---
	int NV = (int)res.nodes.size() / 3;
	vector<vec3d> r(NV);
	for (int i = 0; i < NV; ++i) r[i] = vec3d(res.nodes[3 * i], res.nodes[3 * i + 1], res.nodes[3 * i + 2]);

	// merge coincident vertices (the extraction can leave clusters of them, joined by cells
	// of zero volume that no node movement can repair)
	vector<vector<int> > faces = res.faces;
	vector<vector<int> > cells = res.cells;
	vector<bool> isHex(res.isHex.begin(), res.isHex.end());
	int nwelded = weldVertices(r, 1e-4 * H, faces, cells, isHex);
	if (nwelded > 0) Log("Merged %d coincident %s of the hex-dominant mesh.\n", nwelded, (nwelded == 1 ? "vertex" : "vertices"));
	else if (nwelded < 0) Log("Warning: the hex-dominant mesh has coincident vertices that could not be merged.\n");

	int NF = (int)faces.size();
	int NC = (int)cells.size();

	// orient the faces of every cell consistently
	vector<vector<vector<int> > > cellFaces(NC);
	for (int c = 0; c < NC; ++c)
	{
		if (!orientFaces(faces, cells[c], cellFaces[c]))
		{
			error("Hex-dominant mesher returned a polyhedron that is not closed.");
			return nullptr;
		}
	}

	// cells of each face (boundary faces have one)
	vector<int> faceCells(NF, 0);
	vector<array<int, 2> > faceCellPair(NF, { -1, -1 });
	for (int c = 0; c < NC; ++c)
		for (int k = 0; k < (int)cells[c].size(); ++k)
		{
			int fid = cells[c][k];
			faceCells[fid]++;
			if (faceCellPair[fid][0] < 0) faceCellPair[fid][0] = c; else faceCellPair[fid][1] = c;
		}

	// Make the faces point outward. The sign of a cell's volume is unreliable if the cell is
	// nearly flat, so the orientation is propagated across shared faces (which must be
	// traversed in opposite directions), starting in each connected part from the cell with
	// the largest volume.
	{
		vector<pair<double, int> > order(NC);
		for (int c = 0; c < NC; ++c) order[c] = make_pair(-fabs(signedVolume(r, cellFaces[c])), c);
		sort(order.begin(), order.end());
		vector<bool> done(NC, false);
		for (auto& oc : order)
		{
			int s0 = oc.second;
			if (done[s0]) continue;
			if (signedVolume(r, cellFaces[s0]) < 0) flipFaces(cellFaces[s0]);
			done[s0] = true;
			vector<int> queue(1, s0);
			for (size_t q = 0; q < queue.size(); ++q)
			{
				int a = queue[q];
				for (int k = 0; k < (int)cells[a].size(); ++k)
				{
					int fid = cells[a][k];
					int b = (faceCellPair[fid][0] == a ? faceCellPair[fid][1] : faceCellPair[fid][0]);
					if ((b < 0) || done[b]) continue;
					int kb = (int)(find(cells[b].begin(), cells[b].end(), fid) - cells[b].begin());
					const vector<int>& fa = cellFaces[a][k];
					const vector<int>& fb = cellFaces[b][kb];
					int m = (int)fb.size();
					bool same = false;
					for (int j = 0; j < m; ++j) if ((fb[j] == fa[0]) && (fb[(j + 1) % m] == fa[1])) { same = true; break; }
					if (same) flipFaces(cellFaces[b]);
					done[b] = true;
					queue.push_back(b);
				}
			}
		}
	}

	// Remove flat cells on the boundary. The extraction can leave a thin flap of a cell lying on
	// the surface, sometimes folded over a neighboring boundary face; no split point or node
	// movement can make it valid. A boundary cell whose vertices are all on the boundary and
	// whose thickness (2 x volume / area) is below 0.1 H is removed, and its other faces become
	// boundary faces (which are then projected onto the input surface like all others).
	int nflatRemoved = 0;
	{
		vector<bool> alive(NC, true);
		for (bool changed = true; changed; )
		{
			changed = false;
			vector<bool> bnode(NV, false);
			for (int f = 0; f < NF; ++f) if (faceCells[f] == 1) for (int v : faces[f]) bnode[v] = true;
			for (int c = 0; c < NC; ++c)
			{
				if (!alive[c]) continue;
				bool onB = false, allB = true;
				for (int fid : cells[c])
				{
					if (faceCells[fid] == 1) onB = true;
					for (int v : faces[fid]) if (!bnode[v]) allB = false;
				}
				if (!onB || !allB) continue;
				double area = 0;
				for (auto& f : cellFaces[c]) area += 0.5 * polygonNormal(r, f).Length();
				if (2.0 * fabs(signedVolume(r, cellFaces[c])) >= 0.1 * H * area) continue;

				for (int fid : cells[c])
				{
					faceCells[fid]--;
					if (faceCellPair[fid][0] == c) faceCellPair[fid][0] = faceCellPair[fid][1];
					faceCellPair[fid][1] = -1;
				}
				alive[c] = false;
				nflatRemoved++;
				changed = true;
				for (int fid : cells[c]) if (faceCells[fid] == 1) for (int v : faces[fid]) bnode[v] = true;
			}
		}
		if (nflatRemoved > 0)
		{
			vector<int> newId(NC, -1);
			int nc = 0;
			for (int c = 0; c < NC; ++c)
				if (alive[c])
				{
					newId[c] = nc;
					cells[nc] = cells[c];
					cellFaces[nc] = cellFaces[c];
					isHex[nc] = isHex[c];
					nc++;
				}
			NC = nc;
			cells.resize(NC);
			cellFaces.resize(NC);
			isHex.resize(NC);
			for (int f = 0; f < NF; ++f)
				for (int k = 0; k < 2; ++k)
					if (faceCellPair[f][k] >= 0) faceCellPair[f][k] = newId[faceCellPair[f][k]];
			Log("Removed %d flat %s on the boundary.\n", nflatRemoved, (nflatRemoved == 1 ? "cell" : "cells"));
		}
	}

	// --- put the boundary nodes on the input surface and recover its sharp edges ---
	// boundary edges of the polyhedral mesh and their (boundary) faces
	vector<bool> onBoundary(NV, false);
	map<pair<int, int>, vector<int> > bedgeFaces;
	for (int f = 0; f < NF; ++f)
	{
		if (faceCells[f] != 1) continue;
		const vector<int>& of = faces[f];
		int m = (int)of.size();
		for (int j = 0; j < m; ++j)
		{
			int a = of[j], b = of[(j + 1) % m];
			bedgeFaces[make_pair(min(a, b), max(a, b))].push_back(f);
			onBoundary[a] = true;
		}
	}
	vector<vector<int> > bnbr(NV);
	for (auto& it : bedgeFaces) { bnbr[it.first.first].push_back(it.first.second); bnbr[it.first.second].push_back(it.first.first); }
	auto edgeKey = [](int a, int b) { return make_pair(min(a, b), max(a, b)); };
	vector<vector<int> > bnodeFaces(NV);	// boundary faces of each node
	for (int f = 0; f < NF; ++f) if (faceCells[f] == 1) for (int v : faces[f]) bnodeFaces[v].push_back(f);

	// Replace face f by smaller faces (given in the orientation of faces[f]) in the cells that use
	// it, and update the boundary data. Split faces inherit the region of f (once regions exist).
	vector<int> faceRegion;
	auto splitFace = [&](int f, const vector<vector<int> >& pieces) {
		const vector<int> poly = faces[f];
		vector<int> ids;
		for (auto& p : pieces)
		{
			ids.push_back((int)faces.size());
			faces.push_back(p);
			faceCells.push_back(faceCells[f]);
			faceCellPair.push_back(faceCellPair[f]);
			if (!faceRegion.empty()) faceRegion.push_back(faceRegion[f]);
		}
		for (int k = 0; k < 2; ++k)
		{
			int c = faceCellPair[f][k];
			if (c < 0) continue;
			int kf = (int)(find(cells[c].begin(), cells[c].end(), f) - cells[c].begin());
			const vector<int> of = cellFaces[c][kf];
			int i0 = (int)(find(of.begin(), of.end(), poly[0]) - of.begin());
			bool same = (of[(i0 + 1) % of.size()] == poly[1]);
			cells[c].erase(cells[c].begin() + kf);
			cellFaces[c].erase(cellFaces[c].begin() + kf);
			for (int id : ids)
			{
				vector<int> t = faces[id];
				if (!same) reverse(t.begin(), t.end());
				cells[c].push_back(id);
				cellFaces[c].push_back(t);
			}
			isHex[c] = false;
		}
		if (faceCells[f] == 1)
		{
			int m = (int)poly.size();
			for (int j = 0; j < m; ++j)
			{
				vector<int>& v = bedgeFaces[edgeKey(poly[j], poly[(j + 1) % m])];
				v.erase(remove(v.begin(), v.end(), f), v.end());
			}
			for (int v : poly) { vector<int>& bf = bnodeFaces[v]; bf.erase(remove(bf.begin(), bf.end(), f), bf.end()); }
			for (int id : ids)
			{
				const vector<int> p = faces[id];
				int mp = (int)p.size();
				for (int j = 0; j < mp; ++j)
				{
					int a = p[j], b = p[(j + 1) % mp];
					bedgeFaces[edgeKey(a, b)].push_back(id);
					if (find(bnbr[a].begin(), bnbr[a].end(), b) == bnbr[a].end()) { bnbr[a].push_back(b); bnbr[b].push_back(a); }
				}
				for (int v : p) bnodeFaces[v].push_back(id);
			}
		}
		faceCells[f] = 0;
		faceCellPair[f] = { -1, -1 };
	};

	// The node constraints are used by the untangler: corners are fixed, nodes on feature edges
	// slide along their feature curve, and the other boundary nodes slide on their patch of the
	// input surface (NodeConstraint::id is the curve or the patch).
	typedef FEMeshUntangler::NodeConstraint NodeConstraint;
	vector<NodeConstraint> cons(NV);
	const vector<vec3d> rinit = r;
	vector<bool> taken(NV, false);	// nodes on recovered corners and feature curves
	set<pair<int, int> > recoveredEdges;

	// 1) corners: the nearest boundary node (each node serves one corner). A node needs at least
	//    as many boundary edges as there are feature curves meeting at the corner, so nodes with
	//    fewer edges are only used if there is no other node near the corner.
	const vector<int>& inputCorners = proj.CornerNodes();
	map<int, int> cornerNode;	// input corner -> output node
	{
		map<int, int> curvesAtCorner;
		for (int c = 0; c < proj.Curves(); ++c)
		{
			const FESurfaceProjector::Curve& C = proj.GetCurve(c);
			if (!C.closed) { curvesAtCorner[C.nodes.front()]++; curvesAtCorner[C.nodes.back()]++; }
		}
		// widen the search radius for the corners that have no node yet
		for (double radius : { 0.75 * H, 1.0 * H })
		{
			vector<pair<double, pair<int, int> > > cand;
			for (int ic : inputCorners)
			{
				if (cornerNode.count(ic)) continue;
				const vec3d& P = proj.NodePosition(ic);
				int needed = curvesAtCorner[ic];
				for (int v = 0; v < NV; ++v)
				{
					if (!onBoundary[v] || taken[v]) continue;
					double d = (r[v] - P).Length();
					if (d < radius) cand.push_back(make_pair(d + ((int)bnbr[v].size() < needed ? radius : 0.0), make_pair(ic, v)));
				}
			}
			sort(cand.begin(), cand.end());
			for (auto& c : cand)
			{
				int ic = c.second.first, v = c.second.second;
				if (cornerNode.count(ic) || taken[v]) continue;
				cornerNode[ic] = v;
				taken[v] = true;
				r[v] = proj.NodePosition(ic);
				cons[v].type = FEMeshUntangler::FIXED;
			}
		}
	}

	// 2) feature curves: the shortest path along boundary edges between the curve's corner nodes
	//    (or between nodes near three points of a closed curve) that stays within a band around
	//    the curve. The path's nodes are placed on the curve in order, spaced like the path.
	//    The lattice may cut a sharp edge by a fraction of H, so the band is widened for the
	//    curves that were not recovered yet (all curves get a chance with the narrowest band
	//    first, so that paths do not take nodes that other curves need).
	int ncurves = proj.Curves(), nrecovered = 0, ndiagonals = 0;
	{
		vector<int> order(ncurves);
		for (int c = 0; c < ncurves; ++c) order[c] = c;
		sort(order.begin(), order.end(), [&](int a, int b) { return proj.GetCurve(a).length > proj.GetCurve(b).length; });
		vector<bool> recovered(ncurves, false);

		// try to recover curve c with paths in a band of width rband; returns true on success
		auto recoverCurve = [&](int c, double rband) -> bool {
			const FESurfaceProjector::Curve& C = proj.GetCurve(c);

			// the free boundary node nearest to the curve point at arc length s
			auto nearestFree = [&](double s) {
				vec3d P = proj.CurvePoint(c, s);
				int best = -1;
				double dbest = rband;
				for (int v = 0; v < NV; ++v)
				{
					if (!onBoundary[v] || taken[v]) continue;
					double d = (r[v] - P).Length();
					if (d < dbest) { dbest = d; best = v; }
				}
				return best;
			};

			// anchors (node, arc length) that the path must visit in order
			vector<pair<int, double> > anchors;
			vector<int> newAnchors;	// anchors taken here (released on failure)
			auto release = [&]() { for (int v : newAnchors) taken[v] = false; return false; };
			if (!C.closed)
			{
				auto ia = cornerNode.find(C.nodes.front()), ib = cornerNode.find(C.nodes.back());
				if ((ia == cornerNode.end()) || (ib == cornerNode.end())) return false;
				anchors.push_back(make_pair(ia->second, 0.0));
				if (ia->second == ib->second)	// a loop through one corner
					for (int k = 1; k <= 2; ++k)
					{
						int v = nearestFree(k * C.length / 3.0);
						if (v < 0) return release();
						anchors.push_back(make_pair(v, k * C.length / 3.0));
						taken[v] = true; newAnchors.push_back(v);
					}
				anchors.push_back(make_pair(ib->second, C.length));
			}
			else
			{
				for (int k = 0; k < 3; ++k)
				{
					int v = nearestFree(k * C.length / 3.0);
					if (v < 0) return release();
					anchors.push_back(make_pair(v, k * C.length / 3.0));
					taken[v] = true; newAnchors.push_back(v);
				}
				anchors.push_back(make_pair(anchors[0].first, C.length));
			}

			// distance of a node from the curve
			map<int, double> dcache;
			auto dcurve = [&](int v) {
				auto it = dcache.find(v);
				if (it != dcache.end()) return it->second;
				vec3d q, t;
				double d = (proj.ClosestFeaturePoint(r[v], q, t, c) ? (q - r[v]).Length() : 1e300);
				dcache[v] = d;
				return d;
			};

			vector<int> pathNodes;
			vector<double> pathS;
			set<int> inPath;
			for (size_t k = 0; k + 1 < anchors.size(); ++k)
			{
				int a = anchors[k].first, b = anchors[k + 1].first;
				if (a == b) return release();

				// Dijkstra over steps (u -> w) between nodes in the band, along boundary edges or
				// across a boundary face along a diagonal (the face is then split in two); the cost
				// grows with the distance from the curve. A diagonal costs extra, more so in a quad,
				// which may belong to a hexahedron. Three consecutive path nodes p, u, w in one face
				// would be collinear once snapped onto the curve: this is penalized if the face can
				// be split into triangles around u below, and not allowed if p and w are joined by
				// an edge of the face (the triangle p, u, w would be degenerate).
				auto allowed = [&](int w) { return (w == b) || (!taken[w] && !inPath.count(w) && (dcurve(w) <= rband)); };
				auto cost = [&](int u, int w) {
					double dm = 0.5 * (dcurve(u) + dcurve(w)) / H;
					return (r[w] - r[u]).Length() * (1.0 + 10.0 * dm * dm);
				};
				// 0: no face has p, u and w; 1: a face has them; 2: ... and p and w are adjacent in it
				auto sameFace = [&](int p, int u, int w) {
					int res = 0;
					for (int g : bnodeFaces[u])
					{
						const vector<int>& fg = faces[g];
						int m = (int)fg.size();
						int ip = (int)(find(fg.begin(), fg.end(), p) - fg.begin());
						int iw = (int)(find(fg.begin(), fg.end(), w) - fg.begin());
						if ((ip == m) || (iw == m)) continue;
						if (((ip + 1) % m == iw) || ((iw + 1) % m == ip)) return 2;
						res = 1;
					}
					return res;
				};
				auto forSteps = [&](int u, const std::function<void(int, double)>& fn) {
					for (int g : bnodeFaces[u])
					{
						const vector<int>& fg = faces[g];
						int m = (int)fg.size();
						int iu = (int)(find(fg.begin(), fg.end(), u) - fg.begin());
						for (int j = 1; j < m; ++j)
						{
							bool edge = (j == 1) || (j == m - 1);
							fn(fg[(iu + j) % m], edge ? 0.0 : (m == 4 ? 1.0 : 0.25) * H);
						}
					}
				};
				typedef pair<int, int> State;	// (previous node, node)
				typedef pair<double, State> QItem;
				priority_queue<QItem, vector<QItem>, greater<QItem> > pq;
				map<State, double> dist;
				map<State, State> prev;
				forSteps(a, [&](int w, double extra) {
					if (!allowed(w)) return;
					State st(a, w);
					double d = cost(a, w) + extra;
					auto it = dist.find(st);
					if ((it == dist.end()) || (d < it->second)) { dist[st] = d; prev[st] = State(-1, a); pq.push(QItem(d, st)); }
				});
				bool found = false;
				State last(-1, -1);
				while (!pq.empty())
				{
					QItem top = pq.top(); pq.pop();
					State st = top.second;
					if (top.first > dist[st]) continue;
					int p = st.first, u = st.second;
					if (u == b) { found = true; last = st; break; }
					forSteps(u, [&](int w, double extra) {
						if ((w == p) || !allowed(w)) return;
						int sf = sameFace(p, u, w);
						if (sf == 2) return;
						State nst(u, w);
						double nd = top.first + cost(u, w) + extra + (sf ? 0.5 * H : 0.0);
						auto it = dist.find(nst);
						if ((it == dist.end()) || (nd < it->second)) { dist[nst] = nd; prev[nst] = st; pq.push(QItem(nd, nst)); }
					});
				}
				if (!found) return release();

				vector<int> seg;
				for (State st = last; st.first >= 0; st = prev[st]) seg.push_back(st.second);
				seg.push_back(a);
				reverse(seg.begin(), seg.end());
				set<int> unique(seg.begin(), seg.end());
				if (unique.size() != seg.size()) return release();	// the path visits a node twice
				vector<double> cum(seg.size(), 0.0);
				for (size_t i = 1; i < seg.size(); ++i) cum[i] = cum[i - 1] + (r[seg[i]] - r[seg[i - 1]]).Length();
				double sa = anchors[k].second, sb = anchors[k + 1].second;
				for (size_t i = (k == 0 ? 0 : 1); i < seg.size(); ++i)
				{
					pathNodes.push_back(seg[i]);
					pathS.push_back(sa + (sb - sa) * (cum.back() > 0 ? cum[i] / cum.back() : 0.0));
					inPath.insert(seg[i]);
				}
			}

			// split the faces that the path crosses along a diagonal
			for (size_t i = 0; i + 1 < pathNodes.size(); ++i)
			{
				int a = pathNodes[i], b = pathNodes[i + 1];
				auto ie = bedgeFaces.find(edgeKey(a, b));
				if ((ie != bedgeFaces.end()) && !ie->second.empty()) continue;
				vector<int> fa = bnodeFaces[a];
				for (int g : fa)
				{
					const vector<int> fg = faces[g];
					int m = (int)fg.size();
					int ia = (int)(find(fg.begin(), fg.end(), a) - fg.begin());
					int ib = (int)(find(fg.begin(), fg.end(), b) - fg.begin());
					if (ib == m) continue;
					vector<int> p1, p2;
					for (int k = ia; ; k = (k + 1) % m) { p1.push_back(fg[k]); if (k == ib) break; }
					for (int k = ib; ; k = (k + 1) % m) { p2.push_back(fg[k]); if (k == ia) break; }
					splitFace(g, { p1, p2 });
					ndiagonals++;
					break;
				}
			}

			for (size_t i = 0; i < pathNodes.size(); ++i)
			{
				int v = pathNodes[i];
				taken[v] = true;
				if (cons[v].type == FEMeshUntangler::FIXED) continue;	// a corner
				vec3d t;
				r[v] = proj.CurvePoint(c, pathS[i], &t);
				cons[v].type = FEMeshUntangler::LINE;
				cons[v].dir = t;
				cons[v].id = c;
			}
			for (size_t i = 0; i + 1 < pathNodes.size(); ++i) recoveredEdges.insert(edgeKey(pathNodes[i], pathNodes[i + 1]));
			return true;
		};

		for (double band : { 0.75, 1.0 })
			for (int c : order)
				if (!recovered[c] && recoverCurve(c, band * H)) { recovered[c] = true; nrecovered++; }
	}

	// 3) patches: the recovered edges split the boundary faces into regions; a region is assigned
	//    the input patch that is nearest to most of its other nodes
	NF = (int)faces.size();
	faceRegion.assign(NF, -1);
	int nregions = 0;
	for (int f0 = 0; f0 < NF; ++f0)
	{
		if ((faceCells[f0] != 1) || (faceRegion[f0] >= 0)) continue;
		vector<int> stack(1, f0);
		faceRegion[f0] = nregions;
		while (!stack.empty())
		{
			int f = stack.back(); stack.pop_back();
			int m = (int)faces[f].size();
			for (int j = 0; j < m; ++j)
			{
				pair<int, int> e = edgeKey(faces[f][j], faces[f][(j + 1) % m]);
				if (recoveredEdges.count(e)) continue;
				for (int g : bedgeFaces[e]) if (faceRegion[g] < 0) { faceRegion[g] = nregions; stack.push_back(g); }
			}
		}
		nregions++;
	}
	vector<int> nodeRegion(NV, -1);
	for (int f = 0; f < NF; ++f)
		if (faceCells[f] == 1) for (int v : faces[f]) nodeRegion[v] = faceRegion[f];
	vector<int> regionPatch(nregions, -1);
	{
		vector<map<int, int> > votes(nregions);
		for (int v = 0; v < NV; ++v)
		{
			if (!onBoundary[v] || taken[v] || (nodeRegion[v] < 0)) continue;
			vec3d q, n;
			int patch = -1;
			if (proj.ClosestPoint(r[v], q, n, -1, &patch)) votes[nodeRegion[v]][patch]++;
		}
		for (int k = 0; k < nregions; ++k)
		{
			int total = 0, best = 0, bestPatch = -1;
			for (auto& it : votes[k]) { total += it.second; if (it.second > best) { best = it.second; bestPatch = it.first; } }
			if (total > 0) regionPatch[k] = bestPatch;
		}
	}

	// 4) All other boundary nodes go to the closest point of their region's patch, which keeps
	//    them on their side of the recovered edges. If that patch is much farther away than the
	//    nearest one (e.g. a region that spans several patches because a curve was not
	//    recovered), the node goes to the nearest patch instead.
	auto projectToPatch = [&](const vec3d& p, int regionP, vec3d& q, vec3d& n) {
		int nearest = -1;
		proj.ClosestPoint(p, q, n, -1, &nearest);
		if ((regionP < 0) || (regionP == nearest)) return nearest;
		vec3d q2, n2;
		if (proj.ClosestPoint(p, q2, n2, regionP) && ((q2 - p).Length() <= (q - p).Length() + 0.25 * H)) { q = q2; n = n2; return regionP; }
		return nearest;
	};
	int nproj = 0;
	double maxMove = 0.0;
	for (int v = 0; v < NV; ++v)
	{
		if (!onBoundary[v]) continue;
		if (!taken[v])
		{
			vec3d q, n;
			int patch = projectToPatch(r[v], (nodeRegion[v] >= 0 ? regionPatch[nodeRegion[v]] : -1), q, n);
			r[v] = q;
			cons[v].type = FEMeshUntangler::PLANE;
			cons[v].dir = n;
			cons[v].id = patch;
		}
		maxMove = max(maxMove, (r[v] - rinit[v]).Length());
		nproj++;
	}
	Log("Projected %d boundary nodes onto the input surface (largest move = %lg); recovered %d of %d corners and %d of %d feature curves.\n",
		nproj, maxMove, (int)cornerNode.size(), (int)inputCorners.size(), nrecovered, ncurves);
	if (ndiagonals > 0) Log("Split %d boundary faces along a diagonal to recover feature curves.\n", ndiagonals);

	// 5) A face with three consecutive nodes on a straight line (from a path along two of its
	//    edges) has a zero angle there; split it into triangles around the middle node, in every
	//    cell that uses it. (A hex that loses such a face is split like any other polyhedron.)
	int nflat = 0;
	{
		auto straightAt = [&](const vector<int>& f, int j) {
			int m = (int)f.size();
			const vec3d& a = r[f[(j + m - 1) % m]];
			const vec3d& u = r[f[j]];
			const vec3d& b = r[f[(j + 1) % m]];
			vec3d d1 = u - a, d2 = b - u;
			double L2 = (b - a).SqrLength();
			return (L2 > 0) && ((d1 ^ d2).SqrLength() <= 1e-12 * L2 * L2) && (d1 * d2 > 0);
		};
		int NF0 = (int)faces.size();
		for (int f = 0; f < NF0; ++f)
		{
			const vector<int> poly = faces[f];
			int m = (int)poly.size();
			if ((m < 4) || (faceCells[f] == 0)) continue;
			int j0 = -1;
			for (int j = 0; j < m; ++j) if (taken[poly[j]] && straightAt(poly, j)) { j0 = j; break; }
			if (j0 < 0) continue;

			// fan of triangles around the middle node, in the orientation of faces[f]
			vector<vector<int> > fan;
			for (int k = 1; k + 1 < m; ++k) fan.push_back({ poly[j0], poly[(j0 + k) % m], poly[(j0 + k + 1) % m] });
			splitFace(f, fan);
			nflat++;
		}
		NF = (int)faces.size();
		if (nflat > 0) Log("Split %d faces with three nodes on a feature edge into triangles.\n", nflat);
	}

	// centers of faces with more than four edges (on the input surface for boundary faces);
	// their nodes are created after merging
	vector<vec3d> fcPos(NF), fcNormal(NF);
	vector<bool> fcOnSurface(NF, false);
	vector<int> fcPatch(NF, -1);
	for (int f = 0; f < NF; ++f)
	{
		if (faces[f].size() <= 4) continue;
		vec3d fc = polygonCenter(r, faces[f]);
		if (faceCells[f] == 1)
		{
			vec3d q, n;
			int patch = projectToPatch(fc, regionPatch[faceRegion[f]], q, n);
			fc = q; fcNormal[f] = n; fcOnSurface[f] = true; fcPatch[f] = patch;
		}
		fcPos[f] = fc;
	}

	// relative split margin of a cell, given its oriented faces and their ids
	auto cellMargin = [&](const vector<vector<int> >& of, const vector<int>& fids, vec3d& x) {
		vector<vec3d> fp(fids.size());
		for (size_t k = 0; k < fids.size(); ++k) fp[k] = fcPos[fids[k]];
		return splitPoint(r, of, fp, x);
	};

	// --- merge cells that have no good split point (e.g. nearly flat cells) ---
	// Such a cell absorbs up to three neighbors, chosen greedily by the split margin of the
	// merged cell (non-hex neighbors are tried first).
	const double minMargin = 0.02;
	vector<vector<int> > cellIds = cells;
	vector<vector<vector<int> > > cellOF = cellFaces;
	vector<bool> hexCell(NC), alive(NC, true);
	for (int c = 0; c < NC; ++c) hexCell[c] = isHex[c];
	int nmerged = 0;
	for (int pass = 0; pass < 3; ++pass)
	{
		int nmergedPass = 0;
		for (int c = 0; c < NC; ++c)
		{
			if (!alive[c] || hexCell[c]) continue;
			int en[8];
			if (validPrimitive(r, cellOF[c], en) != FE_INVALID_ELEMENT_TYPE) continue;
			vec3d x;
			double m0 = cellMargin(cellOF[c], cellIds[c], x);
			if (m0 >= minMargin) continue;

			// Greedily absorb neighbors (non-hex neighbors first), up to three, until the merged
			// cell has a good split point; accept a merge that makes an invalid split valid, or a
			// poor one good.
			double target = (m0 <= 0 ? 0.0 : minMargin);
			vector<int> ids = cellIds[c];
			vector<vector<int> > of = cellOF[c];
			vector<int> absorbed;
			double mcur = m0;
			bool success = false;
			for (int step = 0; (step < 3) && !success; ++step)
			{
				set<int> nbrs;
				for (int fid : ids)
					for (int k = 0; k < 2; ++k)
					{
						int nb = faceCellPair[fid][k];
						if ((nb >= 0) && (nb != c) && alive[nb] && (find(absorbed.begin(), absorbed.end(), nb) == absorbed.end())) nbrs.insert(nb);
					}

				double mstep = -1e300;
				int best = -1;
				vector<int> bestIds;
				vector<vector<int> > bestOF;
				for (int hexPass = 0; hexPass < 2; ++hexPass)
				{
					for (int nb : nbrs)
					{
						if (hexCell[nb] != (hexPass == 1)) continue;
						set<int> fa(ids.begin(), ids.end());
						set<int> fb(cellIds[nb].begin(), cellIds[nb].end());
						vector<int> mids;
						vector<vector<int> > mof;
						for (size_t k = 0; k < ids.size(); ++k)
							if (fb.count(ids[k]) == 0) { mids.push_back(ids[k]); mof.push_back(of[k]); }
						for (size_t k = 0; k < cellIds[nb].size(); ++k)
							if (fa.count(cellIds[nb][k]) == 0) { mids.push_back(cellIds[nb][k]); mof.push_back(cellOF[nb][k]); }
						if (mids.empty() || !isClosed(mof)) continue;
						double m = cellMargin(mof, mids, x);
						if (m > mstep) { mstep = m; best = nb; bestIds = mids; bestOF = mof; }
					}
					if ((best >= 0) && (mstep > target)) break;	// prefer a good non-hex merge
				}
				if ((best < 0) || (mstep <= mcur)) break;	// no improvement
				ids = bestIds;
				of = bestOF;
				absorbed.push_back(best);
				mcur = mstep;
				success = (mcur > target);
			}
			if (!success) continue;

			for (int nb : absorbed)
			{
				for (int fid : cellIds[nb])
					for (int k = 0; k < 2; ++k) if (faceCellPair[fid][k] == nb) faceCellPair[fid][k] = c;
				alive[nb] = false;
			}
			vector<int> bestIds = ids;
			vector<vector<int> > bestOF = of;
			cellIds[c] = bestIds;
			cellOF[c] = bestOF;
			hexCell[c] = false;
			nmergedPass += (int)absorbed.size();
		}
		nmerged += nmergedPass;
		if (nmergedPass == 0) break;
	}

	// --- merge regions around the cells that still have no valid split point ---
	// A folded cell (e.g. one pinched flat against the boundary) or a cluster of cells of zero
	// volume can need more neighbors than the greedy merge tries. From a center point p, a
	// region grows by absorbing the cell behind every face that p does not see (the sub-element
	// on the face with apex p would be inverted or degenerate) until p sees all of its faces;
	// it fails if p does not see one of its boundary faces, or if it gets too large (the cell is
	// then left to the optimization). A few center points are tried; the region with the best
	// split margin wins, with a penalty for every hexahedron (and, less, every cell) it absorbs.
	int nrepaired = 0, nrepairedCells = 0;
	{
		const double badMargin = 1e-3;		// cells with a smaller split margin are repaired
		const double minQuality = 0.02;		// a face is seen from p if its sub-elements are at least this good
		const int maxRegion = 20;			// cells per region

		auto centroid = [&](const vector<vector<int> >& of) {
			set<int> vs;
			for (auto& f : of) vs.insert(f.begin(), f.end());
			vec3d s(0, 0, 0);
			for (int v : vs) s += r[v];
			return s / (double)vs.size();
		};
		struct Region { vector<int> cells, ids; vector<vector<int> > of; int nhex = 0; double margin = 0; };
		// the split margin, less a penalty for every hexahedron and (less) for every other cell absorbed
		auto score = [](const Region& R) { return R.margin - 0.1 * R.nhex - 0.01 * ((int)R.cells.size() - R.nhex); };
		auto absorbCell = [&](Region& R, int nb) {
			set<int> fr(R.ids.begin(), R.ids.end());
			set<int> fn(cellIds[nb].begin(), cellIds[nb].end());
			vector<int> ids;
			vector<vector<int> > of;
			for (size_t k = 0; k < R.ids.size(); ++k)
				if (fn.count(R.ids[k]) == 0) { ids.push_back(R.ids[k]); of.push_back(R.of[k]); }
			for (size_t k = 0; k < cellIds[nb].size(); ++k)
				if (fr.count(cellIds[nb][k]) == 0) { ids.push_back(cellIds[nb][k]); of.push_back(cellOF[nb][k]); }
			R.ids.swap(ids);
			R.of.swap(of);
			R.cells.push_back(nb);
			if (hexCell[nb]) R.nhex++;
		};

		for (int c = 0; c < NC; ++c)
		{
			if (!alive[c] || hexCell[c]) continue;
			int en[8];
			if (validPrimitive(r, cellOF[c], en) != FE_INVALID_ELEMENT_TYPE) continue;
			vec3d x0;
			double m0 = cellMargin(cellOF[c], cellIds[c], x0);
			if (m0 >= badMargin) continue;
			if (IsCanceled()) return canceled();

			// center points: the cell's best split point and centroid, and the centroids of its
			// neighbors and of the cell with each neighbor
			vec3d cc = centroid(cellOF[c]);
			vector<vec3d> centers = { x0, cc };
			set<int> nbrs;
			for (int fid : cellIds[c])
				for (int k = 0; k < 2; ++k)
				{
					int nb = faceCellPair[fid][k];
					if ((nb >= 0) && (nb != c) && alive[nb]) nbrs.insert(nb);
				}
			for (int nb : nbrs)
			{
				vec3d cn = centroid(cellOF[nb]);
				centers.push_back(cn);
				centers.push_back((cc + cn) * 0.5);
			}

			Region best;
			bool found = false;
			for (const vec3d& p : centers)
			{
				Region R;
				R.cells.push_back(c);
				R.ids = cellIds[c];
				R.of = cellOF[c];
				bool ok = false;
				set<int> seen;	// faces that p sees (they stay seen as long as they bound the region)
				while ((int)R.cells.size() <= maxRegion)
				{
					// the cells behind the faces that p does not see
					set<int> absorb;
					bool fail = false;
					for (size_t k = 0; k < R.ids.size(); ++k)
					{
						int fid = R.ids[k];
						if (seen.count(fid)) continue;
						if (faceQuality(r, R.of[k], fcPos[fid], p) > minQuality) { seen.insert(fid); continue; }
						int other = -1;
						for (int j = 0; j < 2; ++j)
						{
							int nb = faceCellPair[fid][j];
							if ((nb >= 0) && (find(R.cells.begin(), R.cells.end(), nb) == R.cells.end())) other = nb;
						}
						if (other < 0) { fail = true; break; }	// a boundary face
						absorb.insert(other);
					}
					if (fail) break;
					if (absorb.empty()) { ok = isClosed(R.of); break; }
					for (int nb : absorb) absorbCell(R, nb);
				}
				if (!ok) continue;
				vec3d x;
				R.margin = cellMargin(R.of, R.ids, x);
				if (R.margin < badMargin) continue;
				if (!found || (score(R) > score(best))) { best = R; found = true; }
			}
			if (!found) continue;

			for (int nb : best.cells)
			{
				if (nb == c) continue;
				for (int fid : cellIds[nb])
					for (int k = 0; k < 2; ++k) if (faceCellPair[fid][k] == nb) faceCellPair[fid][k] = c;
				alive[nb] = false;
			}
			cellIds[c] = best.ids;
			cellOF[c] = best.of;
			hexCell[c] = false;
			nrepaired++;
			nrepairedCells += (int)best.cells.size();
		}
	}

	// face-center nodes for faces with more than four edges (shared between cells)
	vector<int> faceCenter(NF, -1);
	for (int c = 0; c < NC; ++c)
	{
		if (!alive[c] || hexCell[c]) continue;
		int en[8];
		if (validPrimitive(r, cellOF[c], en) != FE_INVALID_ELEMENT_TYPE) continue;
		for (int fid : cellIds[c])
		{
			if ((faces[fid].size() > 4) && (faceCenter[fid] < 0))
			{
				NodeConstraint nc;
				if (fcOnSurface[fid]) { nc.type = FEMeshUntangler::PLANE; nc.dir = fcNormal[fid]; nc.id = fcPatch[fid]; }
				faceCenter[fid] = (int)r.size();
				r.push_back(fcPos[fid]);
				cons.push_back(nc);
			}
		}
	}

	struct Elem { FSElementType type; int n[8]; bool cell; };	// cell: the element is a whole cell (hex, or a cell that is a tet, pyramid or prism)
	vector<Elem> elems;
	elems.reserve(NC * 6);
	int ncells = 0, nhex = 0, npoly = 0, nbadhex = 0, nnosplit = 0;
	int ndtet = 0, ndpyra = 0, ndpenta = 0;	// cells that are a single element
	for (int c = 0; c < NC; ++c)
	{
		if (!alive[c]) continue;
		ncells++;
		const vector<int>& cell = cellIds[c];
		if (hexCell[c])
		{
			Elem e = {}; e.type = FE_HEX8;
			if (hexConnectivity(r, faces, cell, e.n))
			{
				e.cell = true;
				elems.push_back(e);
				nhex++;
				continue;
			}
			nbadhex++; // fall through and split it like a general polyhedron
		}

		const vector<vector<int> >& of = cellOF[c];

		// tetrahedra, pyramids and prisms become one element (if it is valid)
		{
			Elem e = {};
			e.type = validPrimitive(r, of, e.n);
			if (e.type != FE_INVALID_ELEMENT_TYPE)
			{
				e.cell = true;
				elems.push_back(e);
				if (e.type == FE_TET4) ndtet++; else if (e.type == FE_PYRA5) ndpyra++; else ndpenta++;
				continue;
			}
		}

		vec3d x;
		if (cellMargin(of, cell, x) <= 0) nnosplit++;

		int ic = (int)r.size();
		r.push_back(x);
		cons.push_back(NodeConstraint());
		npoly++;

		for (size_t k = 0; k < of.size(); ++k)
		{
			const vector<int>& f = of[k];
			int m = (int)f.size();
			if (m == 3)
			{
				Elem e = {}; e.type = FE_TET4;
				e.n[0] = f[0]; e.n[1] = f[2]; e.n[2] = f[1]; e.n[3] = ic;
				elems.push_back(e);
			}
			else if (m == 4)
			{
				// base normal must point toward the apex, i.e. reverse the outward face
				Elem e = {}; e.type = FE_PYRA5;
				e.n[0] = f[3]; e.n[1] = f[2]; e.n[2] = f[1]; e.n[3] = f[0]; e.n[4] = ic;
				elems.push_back(e);
			}
			else
			{
				int fc = faceCenter[cell[k]];
				for (int i = 0; i < m; ++i)
				{
					Elem e = {}; e.type = FE_TET4;
					e.n[0] = f[(i + 1) % m]; e.n[1] = f[i]; e.n[2] = fc; e.n[3] = ic;
					elems.push_back(e);
				}
			}
		}
	}

	// remove nodes that no element uses (e.g. vertices of faces removed by merging)
	{
		auto nodeCount = [](FSElementType t) { return elementNodes(t); };
		vector<int> newId(r.size(), -1);
		for (const Elem& e : elems) for (int j = 0; j < nodeCount(e.type); ++j) newId[e.n[j]] = 0;
		int nn = 0;
		for (size_t i = 0; i < r.size(); ++i) if (newId[i] == 0) newId[i] = nn++;
		if (nn < (int)r.size())
		{
			vector<vec3d> r2(nn);
			vector<NodeConstraint> c2(nn);
			for (size_t i = 0; i < r.size(); ++i) if (newId[i] >= 0) { r2[newId[i]] = r[i]; c2[newId[i]] = cons[i]; }
			r.swap(r2);
			cons.swap(c2);
			for (Elem& e : elems) for (int j = 0; j < nodeCount(e.type); ++j) e.n[j] = newId[e.n[j]];
		}
	}

	// --- untangle and optimize all nodes ---
	vector<FEMeshUntangler::Element> uel(elems.size());
	for (size_t i = 0; i < elems.size(); ++i)
	{
		const Elem& e = elems[i];
		uel[i].type = untanglerType(e.type);
		for (int j = 0; j < 8; ++j) uel[i].n[j] = e.n[j];
	}

	const int maxOuter = 40;
	FEMeshUntangler untangler;
	untangler.SetThreads(GetIntValue(THREADS));
	untangler.SetMaxIterations(maxOuter, 100);
	auto project = [&](vector<vec3d>& x, vector<NodeConstraint>& c) {
		for (size_t i = 0; i < x.size(); ++i)
		{
			vec3d q, d;
			if ((c[i].type == FEMeshUntangler::PLANE) && proj.ClosestPoint(x[i], q, d, c[i].id)) { x[i] = q; c[i].dir = d; }
			else if ((c[i].type == FEMeshUntangler::LINE) && proj.ClosestFeaturePoint(x[i], q, d, c[i].id)) { x[i] = q; c[i].dir = d; }
		}
	};
	auto progress = [&](int iter, double /*minJ*/) {
		setProgress(min(99.0, 100.0 * (iter + 1) / (2.0 * maxOuter)));
		return !IsCanceled();
	};
	setCurrentTask("Optimizing element quality", 0.0);
	if (!untangler.Apply(r, uel, cons, project, progress)) return canceled();
	int ninverted = untangler.InvertedElements();
	Log("Element quality optimization: %d iterations, smallest scaled Jacobian = %lg\n",
		untangler.Iterations(), untangler.MinScaledJacobian());

	// --- pyramids with a folded corner ---
	// FEBio only checks the integration points, but a PYRA5 whose apex is below the plane of
	// the two base edges at a corner gives inverted elements when the mesh is refined. This
	// happens on warped bases, usually next to the surface. The optimization only penalizes
	// such corners, since unfolding them by moving nodes makes the elements around them much
	// worse, and no other apex inside the pyramid's sides can unfold them either (the fold is
	// in the edges at the corner). So the pyramid is merged with the element on the other side
	// of its base, and their union is split around a new point (see splitPoint, here with the
	// corners of the new pyramids as constraints as well). A pyramid whose base is on the
	// surface becomes two tets, split along the diagonal through the folded corner.
	auto untanglerElem = [](const Elem& e) {
		FEMeshUntangler::Element ue;
		ue.type = untanglerType(e.type);
		int nn = elementNodes(e.type);
		for (int j = 0; j < 8; ++j) ue.n[j] = e.n[j < nn ? j : 0];
		return ue;
	};
	auto cornerQuality = [&](const Elem& e) { return FEMeshUntangler::CornerScaledJacobian(r, untanglerElem(e)); };
	auto quality = [&](const Elem& e) { return FEMeshUntangler::ScaledJacobian(r, untanglerElem(e)); };
	// outward faces of an element
	auto elemFaces = [](const Elem& e) {
		static const int HEX[6][4] = { {0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},{3,2,1,0},{4,5,6,7} };
		static const int PENTA[5][4] = { {0,1,4,3},{1,2,5,4},{0,3,5,2},{0,2,1,-1},{3,4,5,-1} };
		static const int PYRA[5][4] = { {0,1,4,-1},{1,2,4,-1},{2,3,4,-1},{3,0,4,-1},{3,2,1,0} };
		static const int TET[4][4] = { {0,1,3,-1},{1,2,3,-1},{2,0,3,-1},{2,1,0,-1} };
		const int (*F)[4] = TET;
		int nf = 4;
		switch (e.type)
		{
		case FE_HEX8: F = HEX; nf = 6; break;
		case FE_PENTA6: F = PENTA; nf = 5; break;
		case FE_PYRA5: F = PYRA; nf = 5; break;
		default: break;
		}
		vector<vector<int> > of(nf);
		for (int k = 0; k < nf; ++k)
			for (int j = 0; j < 4; ++j) if (F[k][j] >= 0) of[k].push_back(e.n[F[k][j]]);
		return of;
	};

	// Nearly flat corners (below CORNER_MIN) are treated the same way, since the children in
	// them get worse with each refinement, except that a hex is only given up for a folded
	// corner. The new elements are kept if their corners are better and their worst element
	// is not worse than before (or than 0.1).
	const double CORNER_MIN = 0.05;
	int nfolded = 0, nunfolded = 0, nfoldHex = 0, nreplaced = 0;
	{
		vector<int> folded;
		for (int i = 0; i < (int)elems.size(); ++i)
		{
			if (elems[i].type != FE_PYRA5) continue;
			double q = cornerQuality(elems[i]);
			if (q < CORNER_MIN) folded.push_back(i);
			if (q <= 0) nfolded++;
		}
		if (!folded.empty())
		{
			vector<vector<int> > nodeElems(r.size());
			auto addElem = [&](const Elem& e) {
				int k = (int)elems.size();
				elems.push_back(e);
				for (int j = 0; j < elementNodes(e.type); ++j)
				{
					if (e.n[j] >= (int)nodeElems.size()) nodeElems.resize(e.n[j] + 1);
					nodeElems[e.n[j]].push_back(k);
				}
			};
			for (int i = 0; i < (int)elems.size(); ++i)
				for (int j = 0; j < elementNodes(elems[i].type); ++j) nodeElems[elems[i].n[j]].push_back(i);
			vector<char> dead(elems.size(), 0);

			for (int i : folded)
			{
				if (dead[i] || (cornerQuality(elems[i]) >= CORNER_MIN)) continue;
				const Elem P = elems[i];
				const int* b = P.n;	// base b[0..3], apex b[4]

				// the element on the other side of the base
				int j = -1;
				for (int k : nodeElems[b[0]])
				{
					if ((k == i) || dead[k]) continue;
					const Elem& e = elems[k];
					int m = 0;
					for (int l = 0; l < elementNodes(e.type); ++l)
						for (int q = 0; q < 4; ++q) if (e.n[l] == b[q]) m++;
					if (m == 4) { j = k; break; }
				}
				// a hex is only given up for a folded corner, not for a nearly flat one
				if ((j >= 0) && (elems[j].type == FE_HEX8) && (cornerQuality(P) > 0)) continue;

				vector<Elem> newElems;
				if (j < 0)
				{
					// the base is on the surface: two tets along the diagonal through the folded corner
					int k0 = 0;
					double qmin = 1e300;
					for (int k = 0; k < 4; ++k)
					{
						const vec3d& x = r[b[k]];
						vec3d e1 = r[b[(k + 1) % 4]] - x, e2 = r[b[(k + 3) % 4]] - x, e3 = r[b[4]] - x;
						double q = (e1 ^ e2) * e3;
						if (q < qmin) { qmin = q; k0 = k; }
					}
					Elem t1 = {}, t2 = {};
					t1.type = t2.type = FE_TET4;
					t1.n[0] = b[k0]; t1.n[1] = b[(k0 + 1) % 4]; t1.n[2] = b[(k0 + 2) % 4]; t1.n[3] = b[4];
					t2.n[0] = b[k0]; t2.n[1] = b[(k0 + 2) % 4]; t2.n[2] = b[(k0 + 3) % 4]; t2.n[3] = b[4];
					if ((quality(t1) <= 0) || (quality(t2) <= 0)) continue;
					newElems = { t1, t2 };
				}
				else
				{
					// merge with the element across the base, and split the union around a new point
					vector<int> base(b, b + 4);
					sort(base.begin(), base.end());
					vector<vector<int> > of;
					const Elem* both[2] = { &P, &elems[j] };
					for (const Elem* e : both)
						for (const vector<int>& f : elemFaces(*e))
						{
							vector<int> s(f);
							sort(s.begin(), s.end());
							if (s != base) of.push_back(f);
						}
					vec3d x;
					if (splitPoint(r, of, vector<vec3d>(of.size(), vec3d(0, 0, 0)), x, true) <= 0) continue;
					const int m = (int)r.size();
					r.push_back(x);
					cons.push_back(NodeConstraint());
					for (const vector<int>& f : of)
					{
						Elem e = {};
						if (f.size() == 3) { e.type = FE_TET4; e.n[0] = f[0]; e.n[1] = f[2]; e.n[2] = f[1]; e.n[3] = m; }
						else { e.type = FE_PYRA5; e.n[0] = f[3]; e.n[1] = f[2]; e.n[2] = f[1]; e.n[3] = f[0]; e.n[4] = m; }
						newElems.push_back(e);
					}

					// The split point maximizes a distance to the planes of the constraints; move it
					// to raise the worst scaled Jacobian of the new elements at the integration
					// points, keeping their corners above what they are (or above CORNER_MIN), with
					// a compass search (m is the only free node).
					auto worstNew = [&](bool corners) {
						double q = 1e300;
						for (const Elem& e : newElems) q = min(q, corners ? cornerQuality(e) : quality(e));
						return q;
					};
					double L = 0;
					for (const vector<int>& f : of) for (int n : f) L = max(L, (r[n] - x).Length());
					const double cfloor = min(worstNew(true), CORNER_MIN);
					double best = worstNew(false);
					const vec3d dirs[6] = { vec3d(1,0,0), vec3d(-1,0,0), vec3d(0,1,0), vec3d(0,-1,0), vec3d(0,0,1), vec3d(0,0,-1) };
					int iter = 0;
					for (double h = 0.1 * L; (h > 1e-3 * L) && (iter < 200); ++iter)
					{
						bool improved = false;
						for (const vec3d& d : dirs)
						{
							const vec3d x0 = r[m];
							r[m] = x0 + d * h;
							double q = worstNew(false);
							if ((q > best) && (worstNew(true) >= cfloor)) { best = q; improved = true; break; }
							r[m] = x0;
						}
						if (!improved) h *= 0.5;
					}
				}

				// Keep the new elements if their corners are better and their worst element is not
				// worse than before (or 0.1).
				{
					double q0 = 1e300, c0 = 1e300, q1 = 1e300, c1 = 1e300;
					for (int k : { i, j })
						if (k >= 0) { q0 = min(q0, quality(elems[k])); c0 = min(c0, cornerQuality(elems[k])); }
					for (const Elem& e : newElems) { q1 = min(q1, quality(e)); c1 = min(c1, cornerQuality(e)); }
					if ((c1 <= c0) || (q1 < min(q0, 0.1)))
					{
						if (j >= 0) { r.pop_back(); cons.pop_back(); }	// the new point
						continue;
					}
				}

				// replace the pyramid (and its neighbor, which can be one too); a whole cell becomes a
				// split polyhedron
				nreplaced++;
				if ((j >= 0) && (elems[j].type == FE_PYRA5) && (cornerQuality(elems[j]) < CORNER_MIN)) nreplaced++;
				int ncellsMerged = 0;
				for (int k : { i, j })
				{
					if (k < 0) continue;
					dead[k] = 1;
					const Elem& e = elems[k];
					if (!e.cell) continue;
					ncellsMerged++;
					switch (e.type)
					{
					case FE_HEX8: nhex--; nfoldHex++; break;
					case FE_PENTA6: ndpenta--; break;
					case FE_PYRA5: ndpyra--; break;
					default: ndtet--; break;
					}
				}
				if (ncellsMerged > 0) npoly++;
				if (ncellsMerged > 1) ncells--;
				for (const Elem& e : newElems) { addElem(e); dead.push_back(0); }
			}

			int nleft = 0;
			vector<Elem> live;
			live.reserve(elems.size());
			for (int i = 0; i < (int)elems.size(); ++i)
			{
				if (dead[i]) continue;
				live.push_back(elems[i]);
				if ((elems[i].type == FE_PYRA5) && (cornerQuality(elems[i]) <= 0)) nleft++;
			}
			elems.swap(live);
			nunfolded = nfolded - nleft;	// (folded pyramids that are gone)
		}
	}

	// build the new mesh
	FSMesh* newMesh = new FSMesh();
	newMesh->Create((int)r.size(), (int)elems.size());
	for (int i = 0; i < (int)r.size(); ++i)
	{
		FSNode& nd = newMesh->Node(i);
		nd.r = r[i];
		nd.m_gid = -1;
	}
	for (int i = 0; i < (int)elems.size(); ++i)
	{
		FSElement& el = newMesh->Element(i);
		el.SetType(elems[i].type);
		el.m_gid = 0;
		int ne = el.Nodes();
		for (int j = 0; j < ne; ++j) el.m_node[j] = elems[i].n[j];
	}
	newMesh->RebuildMesh(featureAngle);

	int ntet = 0, npyra = 0;
	for (const Elem& e : elems) { if (e.type == FE_TET4) ntet++; else if (e.type == FE_PYRA5) npyra++; }
	ntet -= ndtet; npyra -= ndpyra;
	Log("Hex-dominant remesh (H = %lg): %d cells: %d hexahedra (%.1f%%), %d tet4, %d pyra5 and %d penta6 cells, and %d polyhedra split into %d pyra5 + %d tet4\n",
		H, ncells, nhex, (ncells > 0 ? 100.0 * nhex / ncells : 0.0), ndtet, ndpyra, ndpenta, npoly, npyra, ntet);
	if (nmerged > 0) Log("Merged %d cells into neighboring polyhedra that had no good split point.\n", nmerged);
	if (nrepaired > 0) Log("Merged %d cells into %d %s around folded or flat cells.\n", nrepairedCells, nrepaired, (nrepaired == 1 ? "polyhedron" : "polyhedra"));
	if (nreplaced > 0)
	{
		Log("Replaced %d %s with a folded or nearly flat corner (a negative or small Jacobian at a corner of the base, which FEBio does not check but which gives inverted or poor elements when the mesh is refined): merged with the element across the base and split again, or split into two tets on the surface.\n",
			nreplaced, (nreplaced == 1 ? "pyramid" : "pyramids"));
		if (nfoldHex > 0) Log("(This split %d %s.)\n", nfoldHex, (nfoldHex == 1 ? "hexahedron" : "hexahedra"));
	}
	if (nfolded > nunfolded) Log("Warning: %d %s a folded corner. The mesh is valid for FEBio, but refining it can give inverted elements.\n",
		nfolded - nunfolded, (nfolded - nunfolded == 1 ? "pyramid has" : "pyramids have"));
	if (nnosplit == 1) Log("1 polyhedron had no split point that makes all of its elements valid; the optimization moved its vertices.\n");
	else if (nnosplit > 1) Log("%d polyhedra had no split point that makes all of their elements valid; the optimization moved their vertices.\n", nnosplit);
	if (nbadhex > 0) Log("Warning: %d hex cells had invalid topology and were split.\n", nbadhex);
	if (ninverted > 0) Log("Warning: %d elements have a non-positive Jacobian at an integration point. Inspect and fix them before running FEBio.\n", ninverted);

	return newMesh;
#else
	error("This version does not have hex-dominant meshing support");
	return nullptr;
#endif
}
