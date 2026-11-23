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
#include "FacePatch.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GOCCObject.h>
#include <GLLib/GLMesh.h>
#include <MeshLib/FSMeshBase.h>
#include <MeshLib/FSFace.h>
#include <map>
#include <tuple>
#include <set>
#include <cmath>
#include <algorithm>

#ifdef HAS_OCC
#include <gp_Pnt.hxx>
#include <gp_Pln.hxx>
#include <gp_Dir.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#endif

bool ExtractFacePatches(GObject* po, const std::vector<int>& faceIds, std::vector<FacePatch>& patches)
{
	patches.clear();
	if (po == nullptr) return false;
	GLMesh* gm = po->GetRenderMesh();
	if (gm == nullptr) return false;

	std::map<int, int> faceToPatch;
	for (int id : faceIds)
	{
		if (faceToPatch.find(id) == faceToPatch.end())
		{
			faceToPatch[id] = (int)patches.size();
			patches.push_back(FacePatch());
		}
	}

	// per patch: render node index --> patch point index
	std::vector<std::map<int, int>> nodeMap(patches.size());
	for (int i = 0; i < gm->Faces(); ++i)
	{
		const GLMesh::FACE& f = gm->Face(i);
		auto it = faceToPatch.find(f.pid);
		if (it == faceToPatch.end()) continue;
		int ip = it->second;
		FacePatch& P = patches[ip];
		for (int k = 0; k < 3; ++k)
		{
			int n = f.n[k];
			auto jt = nodeMap[ip].find(n);
			int m;
			if (jt == nodeMap[ip].end())
			{
				vec3f r = gm->Node(n).r;
				m = (int)P.pts.size();
				P.pts.push_back(vec3d(r.x, r.y, r.z));
				nodeMap[ip][n] = m;
			}
			else m = jt->second;
			P.tri.push_back(m);
		}
	}

	// the render mesh may duplicate nodes (e.g. for smoothing), so merge coincident points
	for (FacePatch& P : patches)
	{
		if (P.empty()) continue;
		vec3d rmin = P.pts[0], rmax = P.pts[0];
		for (const vec3d& r : P.pts)
		{
			rmin.x = std::min(rmin.x, r.x); rmin.y = std::min(rmin.y, r.y); rmin.z = std::min(rmin.z, r.z);
			rmax.x = std::max(rmax.x, r.x); rmax.y = std::max(rmax.y, r.y); rmax.z = std::max(rmax.z, r.z);
		}
		double h = 1e-6 * (rmax - rmin).norm();
		if (h <= 0) h = 1e-12;
		typedef std::tuple<long long, long long, long long> Key;
		std::map<Key, int> uniq;
		std::vector<int> remap(P.pts.size());
		std::vector<vec3d> newPts;
		for (size_t i = 0; i < P.pts.size(); ++i)
		{
			const vec3d& r = P.pts[i];
			Key k((long long)std::llround(r.x / h), (long long)std::llround(r.y / h), (long long)std::llround(r.z / h));
			auto it = uniq.find(k);
			if (it == uniq.end()) { it = uniq.insert({ k, (int)newPts.size() }).first; newPts.push_back(r); }
			remap[i] = it->second;
		}
		for (int& t : P.tri) t = remap[t];
		P.pts = newPts;

		// remove degenerate triangles
		std::vector<int> tri;
		for (size_t i = 0; i + 2 < P.tri.size(); i += 3)
		{
			int a = P.tri[i], b = P.tri[i + 1], c = P.tri[i + 2];
			if ((a != b) && (b != c) && (a != c)) { tri.push_back(a); tri.push_back(b); tri.push_back(c); }
		}
		P.tri = tri;
	}

	// remove empty patches
	patches.erase(std::remove_if(patches.begin(), patches.end(), [](const FacePatch& p) { return p.empty(); }), patches.end());
	return !patches.empty();
}

bool ExtractMeshFacePatch(FSMeshBase* mesh, const std::vector<int>& faceIds, FacePatch& patch)
{
	patch = FacePatch();
	if (mesh == nullptr) return false;

	std::map<int, int> nodeMap;
	auto addPoint = [&](int n) {
		auto it = nodeMap.find(n);
		if (it != nodeMap.end()) return it->second;
		int m = (int)patch.pts.size();
		patch.pts.push_back(mesh->Node(n).r);
		nodeMap[n] = m;
		return m;
	};

	for (int id : faceIds)
	{
		if ((id < 0) || (id >= mesh->Faces())) continue;
		const FSFace& f = mesh->Face(id);
		int nc = 3;
		switch (f.Type()) {
		case FE_FACE_QUAD4:
		case FE_FACE_QUAD8:
		case FE_FACE_QUAD9: nc = 4; break;
		default: nc = 3;
		}
		int m[4];
		for (int k = 0; k < nc; ++k) m[k] = addPoint(f.n[k]);
		patch.tri.push_back(m[0]); patch.tri.push_back(m[1]); patch.tri.push_back(m[2]);
		if (nc == 4) { patch.tri.push_back(m[0]); patch.tri.push_back(m[2]); patch.tri.push_back(m[3]); }
	}
	return !patch.empty();
}

double PatchesDiagonal(const std::vector<FacePatch>& patches)
{
	bool first = true;
	vec3d rmin, rmax;
	for (const FacePatch& P : patches)
		for (const vec3d& r : P.pts)
		{
			if (first) { rmin = rmax = r; first = false; }
			rmin.x = std::min(rmin.x, r.x); rmin.y = std::min(rmin.y, r.y); rmin.z = std::min(rmin.z, r.z);
			rmax.x = std::max(rmax.x, r.x); rmax.y = std::max(rmax.y, r.y); rmax.z = std::max(rmax.z, r.z);
		}
	return first ? 0.0 : (rmax - rmin).norm();
}

FacePatch MergePatches(const std::vector<FacePatch>& patches)
{
	FacePatch out;
	for (const FacePatch& P : patches)
	{
		int off = (int)out.pts.size();
		out.pts.insert(out.pts.end(), P.pts.begin(), P.pts.end());
		for (int t : P.tri) out.tri.push_back(t + off);
	}
	return out;
}

bool IsPlanarPatch(const FacePatch& P, vec3d& center, vec3d& normal, double relTol)
{
	if (P.empty()) return false;

	// area-weighted normal and centroid
	vec3d N(0, 0, 0), c(0, 0, 0);
	for (const vec3d& r : P.pts) c += r;
	c /= (double)P.pts.size();
	for (int i = 0; i < P.Triangles(); ++i)
	{
		const vec3d& a = P.pts[P.tri[3 * i]];
		const vec3d& b = P.pts[P.tri[3 * i + 1]];
		const vec3d& d = P.pts[P.tri[3 * i + 2]];
		N += (b - a) ^ (d - a);
	}
	double L = N.norm();
	if (L == 0) return false;
	N /= L;

	std::vector<FacePatch> tmp(1, P);
	double diag = PatchesDiagonal(tmp);
	double dmax = 0;
	for (const vec3d& r : P.pts) dmax = std::max(dmax, std::fabs((r - c) * N));
	if (dmax > relTol * diag) return false;

	center = c;
	normal = N;
	return true;
}

#ifdef HAS_OCC
namespace {

	// Extract the closed boundary loops of a patch (as lists of point indices).
	bool BoundaryLoops(const FacePatch& P, std::vector<std::vector<int>>& loops)
	{
		std::map<std::pair<int, int>, int> ecount;
		for (int i = 0; i < P.Triangles(); ++i)
		{
			for (int k = 0; k < 3; ++k)
			{
				int a = P.tri[3 * i + k];
				int b = P.tri[3 * i + (k + 1) % 3];
				ecount[{ std::min(a, b), std::max(a, b) }]++;
			}
		}
		std::map<int, std::vector<int>> adj;
		for (auto& e : ecount)
		{
			if (e.second == 1)
			{
				adj[e.first.first].push_back(e.first.second);
				adj[e.first.second].push_back(e.first.first);
			}
		}
		if (adj.empty()) return false;
		for (auto& a : adj) if (a.second.size() != 2) return false; // pinched or open boundary

		std::set<int> visited;
		for (auto& a : adj)
		{
			if (visited.count(a.first)) continue;
			std::vector<int> loop;
			int prev = -1, cur = a.first;
			while (true)
			{
				visited.insert(cur);
				loop.push_back(cur);
				const std::vector<int>& nb = adj[cur];
				int next = (nb[0] != prev) ? nb[0] : nb[1];
				if ((prev == -1) && (nb[0] == nb[1])) return false;
				prev = cur;
				cur = next;
				if (cur == a.first) break;
				if (visited.count(cur)) return false;
			}
			if (loop.size() < 3) return false;
			loops.push_back(loop);
		}
		return true;
	}

	// drop vertices that lie on a straight line between their neighbours
	std::vector<int> SimplifyLoop(const FacePatch& P, const std::vector<int>& loop)
	{
		std::vector<int> out = loop;
		bool changed = true;
		while (changed && (out.size() > 3))
		{
			changed = false;
			for (size_t i = 0; i < out.size(); ++i)
			{
				const vec3d& a = P.pts[out[(i + out.size() - 1) % out.size()]];
				const vec3d& b = P.pts[out[i]];
				const vec3d& c = P.pts[out[(i + 1) % out.size()]];
				vec3d u = b - a, v = c - b;
				double lu = u.norm(), lv = v.norm();
				if ((lu > 0) && (lv > 0) && (((u ^ v).norm() <= 1e-6 * lu * lv) && (u * v > 0)))
				{
					out.erase(out.begin() + i);
					changed = true;
					break;
				}
			}
		}
		return out;
	}

	// signed area of the loop with respect to the normal n
	double LoopArea(const FacePatch& P, const std::vector<int>& loop, const vec3d& n)
	{
		vec3d s(0, 0, 0);
		for (size_t i = 0; i < loop.size(); ++i)
		{
			const vec3d& a = P.pts[loop[i]];
			const vec3d& b = P.pts[loop[(i + 1) % loop.size()]];
			s += a ^ b;
		}
		return 0.5 * (s * n);
	}

	TopoDS_Wire MakeWire(const FacePatch& P, const std::vector<int>& loop)
	{
		BRepBuilderAPI_MakePolygon mk;
		for (int id : loop) mk.Add(gp_Pnt(P.pts[id].x, P.pts[id].y, P.pts[id].z));
		mk.Close();
		return mk.Wire();
	}
}
#endif

GOCCObject* BuildPlanarOCCShell(const std::vector<FacePatch>& patches, std::string* errMsg)
{
	auto fail = [&](const char* msg) -> GOCCObject* { if (errMsg) *errMsg = msg; return nullptr; };
#ifdef HAS_OCC
	if (patches.empty()) return fail("No face data.");

	double diag = PatchesDiagonal(patches);
	BRepBuilderAPI_Sewing sewing(1e-5 * diag);

	for (const FacePatch& P : patches)
	{
		vec3d c, n;
		if (!IsPlanarPatch(P, c, n)) return fail("A selected face is not planar.");

		std::vector<std::vector<int>> loops;
		if (!BoundaryLoops(P, loops)) return fail("Could not determine the boundary of a selected face.");

		// find the outer loop (largest area) and orient: outer CCW, holes CW (wrt n)
		int outer = 0;
		double amax = 0;
		for (size_t i = 0; i < loops.size(); ++i)
		{
			loops[i] = SimplifyLoop(P, loops[i]);
			double a = std::fabs(LoopArea(P, loops[i], n));
			if (a > amax) { amax = a; outer = (int)i; }
		}
		for (size_t i = 0; i < loops.size(); ++i)
		{
			double a = LoopArea(P, loops[i], n);
			bool wantPositive = ((int)i == outer);
			if ((a > 0) != wantPositive) std::reverse(loops[i].begin(), loops[i].end());
		}

		gp_Pln plane(gp_Pnt(c.x, c.y, c.z), gp_Dir(n.x, n.y, n.z));
		BRepBuilderAPI_MakeFace mf(plane, MakeWire(P, loops[outer]));
		if (!mf.IsDone()) return fail("Failed to create an OpenCascade face.");
		for (size_t i = 0; i < loops.size(); ++i)
		{
			if ((int)i != outer) mf.Add(MakeWire(P, loops[i]));
		}
		sewing.Add(mf.Face());
	}

	sewing.Perform();
	TopoDS_Shape sewed = sewing.SewedShape();

	BRep_Builder builder;
	TopoDS_Shell shell;
	builder.MakeShell(shell);
	int nf = 0;
	for (TopExp_Explorer ex(sewed, TopAbs_FACE); ex.More(); ex.Next(), ++nf) builder.Add(shell, ex.Current());
	if (nf == 0) return fail("Failed to sew the faces.");

	GOCCObject* po = new GOCCObject;
	TopoDS_Shape shape = shell;
	po->SetShape(shape);
	return po;
#else
	return fail("OpenCascade support is not available in this build.");
#endif
}
