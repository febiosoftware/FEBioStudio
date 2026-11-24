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
#include "FEQuadraticRefine.h"
#include <MeshLib/FSMesh.h>
#include <MeshLib/hex.h>
#include <MeshLib/penta.h>
#include <MeshLib/quad8.h>
#include <map>
#include <tuple>
#include <vector>
#include <cmath>
#include <algorithm>

namespace {

	struct P3 { double r, s, t; };

	// quadratic triangle shape functions (area coordinates r, s)
	void tri6_shape(double* H, double r, double s)
	{
		double u = 1.0 - r - s;
		H[0] = u * (2 * u - 1);
		H[1] = r * (2 * r - 1);
		H[2] = s * (2 * s - 1);
		H[3] = 4 * u * r;
		H[4] = 4 * r * s;
		H[5] = 4 * s * u;
	}

	// description of an element type
	struct TypeInfo {
		int nc, nn;										// corner and total node count
		std::vector<std::pair<int, int>> edges;			// mid-edge nodes: pairs of corners
		std::vector<std::vector<P3>> subs;				// corner coordinates (parametric) of each sub-element
		int sub_type;
	};

	typedef std::pair<int, int> E;

	const TypeInfo& GetTypeInfo(int type)
	{
		static bool init = false;
		static TypeInfo quad, tri, hex, penta;
		if (!init)
		{
			init = true;

			quad.nc = 4; quad.nn = 8; quad.sub_type = FE_QUAD8;
			quad.edges = { E(0,1), E(1,2), E(2,3), E(3,0) };
			for (int j = 0; j < 2; ++j) for (int i = 0; i < 2; ++i)
			{
				double a = -1.0 + i, b = -1.0 + j;
				quad.subs.push_back({ {a,b,0}, {a + 1,b,0}, {a + 1,b + 1,0}, {a,b + 1,0} });
			}

			tri.nc = 3; tri.nn = 6; tri.sub_type = FE_TRI6;
			tri.edges = { E(0,1), E(1,2), E(2,0) };

			// triangle split into four triangles (corner coordinates in the (r,s) plane)
			P3 p0 = { 0,0,0 }, p1 = { 1,0,0 }, p2 = { 0,1,0 }, m01 = { 0.5,0,0 }, m12 = { 0.5,0.5,0 }, m20 = { 0,0.5,0 };
			std::vector<std::vector<P3>> T = { {p0,m01,m20}, {m01,p1,m12}, {m20,m12,p2}, {m01,m12,m20} };
			tri.subs = T;

			hex.nc = 8; hex.nn = 20; hex.sub_type = FE_HEX20;
			hex.edges = { E(0,1), E(1,2), E(2,3), E(3,0), E(4,5), E(5,6), E(6,7), E(7,4), E(0,4), E(1,5), E(2,6), E(3,7) };
			for (int k = 0; k < 2; ++k) for (int j = 0; j < 2; ++j) for (int i = 0; i < 2; ++i)
			{
				double a = -1.0 + i, b = -1.0 + j, c = -1.0 + k;
				hex.subs.push_back({ {a,b,c}, {a + 1,b,c}, {a + 1,b + 1,c}, {a,b + 1,c},
					{a,b,c + 1}, {a + 1,b,c + 1}, {a + 1,b + 1,c + 1}, {a,b + 1,c + 1} });
			}

			penta.nc = 6; penta.nn = 15; penta.sub_type = FE_PENTA15;
			penta.edges = { E(0,1), E(1,2), E(2,0), E(3,4), E(4,5), E(5,3), E(0,3), E(1,4), E(2,5) };
			for (int k = 0; k < 2; ++k)
			{
				double t0 = -1.0 + k, t1 = t0 + 1.0;
				for (const auto& tr : T)
				{
					std::vector<P3> c(6);
					for (int m = 0; m < 3; ++m) { c[m] = { tr[m].r, tr[m].s, t0 }; c[m + 3] = { tr[m].r, tr[m].s, t1 }; }
					penta.subs.push_back(c);
				}
			}
		}
		switch (type) {
		case FE_QUAD8: return quad;
		case FE_TRI6: return tri;
		case FE_HEX20: return hex;
		default: return penta;
		}
	}

	bool Supported(int type) { return (type == FE_QUAD8) || (type == FE_TRI6) || (type == FE_HEX20) || (type == FE_PENTA15); }

	void Shape(int type, double* H, const P3& p)
	{
		switch (type) {
		case FE_QUAD8: QUAD8::shape(H, p.r, p.s); break;
		case FE_TRI6: tri6_shape(H, p.r, p.s); break;
		case FE_HEX20: HEX20::shape(H, p.r, p.s, p.t); break;
		default: PENTA15::shape(H, p.r, p.s, p.t); break;
		}
	}

	// spatial hash that merges (almost) coincident points
	class PointMerger
	{
	public:
		PointMerger(double tol) : m_tol(tol), m_cell(4.0 * tol) {}

		typedef std::tuple<long long, long long, long long> Key;
		Key key(const vec3d& r, int di = 0, int dj = 0, int dk = 0) const {
			return Key((long long)std::floor(r.x / m_cell) + di, (long long)std::floor(r.y / m_cell) + dj, (long long)std::floor(r.z / m_cell) + dk);
		}

		// returns the index of a point close to r, or -1
		int find(const vec3d& r) const {
			for (int i = -1; i <= 1; ++i) for (int j = -1; j <= 1; ++j) for (int k = -1; k <= 1; ++k)
			{
				auto it = m_map.find(key(r, i, j, k));
				if (it == m_map.end()) continue;
				for (int n : it->second) if ((m_pts[n] - r).norm() < m_tol) return n;
			}
			return -1;
		}

		int add(const vec3d& r) {
			int n = (int)m_pts.size();
			m_pts.push_back(r);
			m_map[key(r)].push_back(n);
			return n;
		}

		int count() const { return (int)m_pts.size(); }
		const vec3d& point(int i) const { return m_pts[i]; }

	private:
		double m_tol, m_cell;
		std::vector<vec3d> m_pts;
		std::map<Key, std::vector<int>> m_map;
	};
}

FEQuadraticRefineModifier::FEQuadraticRefineModifier() : FEModifier("Refine quadratic mesh")
{
}

bool FEQuadraticRefineModifier::CanRefine(const FSMesh& mesh)
{
	if (mesh.Elements() == 0) return false;
	for (int i = 0; i < mesh.Elements(); ++i)
		if (!Supported(mesh.Element(i).Type())) return false;
	return true;
}

FSMesh* FEQuadraticRefineModifier::Apply(FSMesh* pm)
{
	if (!CanRefine(*pm)) return nullptr;

	const int N0 = pm->Nodes();
	const int NE0 = pm->Elements();

	// merge tolerance relative to the mesh size
	vec3d rmin = pm->Node(0).r, rmax = rmin;
	for (int i = 1; i < N0; ++i)
	{
		const vec3d& r = pm->Node(i).r;
		rmin.x = std::min(rmin.x, r.x); rmin.y = std::min(rmin.y, r.y); rmin.z = std::min(rmin.z, r.z);
		rmax.x = std::max(rmax.x, r.x); rmax.y = std::max(rmax.y, r.y); rmax.z = std::max(rmax.z, r.z);
	}
	double diag = (rmax - rmin).norm();
	if (diag <= 0) diag = 1.0;
	PointMerger merger(1e-7 * diag);

	// existing nodes keep their indices
	for (int i = 0; i < N0; ++i) merger.add(pm->Node(i).r);

	struct NewElem { int type; int gid; int n[20]; };
	std::vector<NewElem> elems;

	for (int e = 0; e < NE0; ++e)
	{
		const FSElement& el = pm->Element(e);
		const TypeInfo& info = GetTypeInfo(el.Type());
		const int type = el.Type();

		vec3d x[20];
		for (int k = 0; k < info.nn; ++k) x[k] = pm->Node(el.m_node[k]).r;

		for (const auto& sub : info.subs)
		{
			// parametric coordinates of the sub-element's nodes
			P3 q[20];
			for (int k = 0; k < info.nc; ++k) q[k] = sub[k];
			for (size_t m = 0; m < info.edges.size(); ++m)
			{
				const P3& a = sub[info.edges[m].first];
				const P3& b = sub[info.edges[m].second];
				q[info.nc + m] = { 0.5 * (a.r + b.r), 0.5 * (a.s + b.s), 0.5 * (a.t + b.t) };
			}

			NewElem ne;
			ne.type = info.sub_type;
			ne.gid = el.m_gid;
			for (int k = 0; k < info.nn; ++k)
			{
				double H[20];
				Shape(type, H, q[k]);
				vec3d r(0, 0, 0);
				for (int i = 0; i < info.nn; ++i) r += x[i] * H[i];

				// reuse an existing node if the sub-element node coincides with one
				int n = merger.find(r);
				if (n < 0) n = merger.add(r);
				ne.n[k] = n;
			}
			elems.push_back(ne);
		}
	}

	FSMesh* pnew = new FSMesh;
	pnew->Create(merger.count(), (int)elems.size());
	for (int i = 0; i < merger.count(); ++i) pnew->Node(i).r = merger.point(i);
	for (size_t i = 0; i < elems.size(); ++i)
	{
		FSElement& el = pnew->Element((int)i);
		el.SetType(elems[i].type);
		el.m_gid = elems[i].gid;
		int nn = el.Nodes();
		for (int k = 0; k < nn; ++k) el.m_node[k] = elems[i].n[k];
	}
	pnew->RebuildMesh();
	return pnew;
}
