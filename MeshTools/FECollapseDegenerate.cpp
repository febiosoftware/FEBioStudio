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
#include "FECollapseDegenerate.h"
#include <MeshLib/FSMeshBuilder.h>
#include <map>
#include <set>
#include <vector>
#include <cmath>
#include <algorithm>

namespace {

	struct UnionFind {
		std::vector<int> p;
		UnionFind(int n) : p(n) { for (int i = 0; i < n; ++i) p[i] = i; }
		int find(int x) { while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; } return x; }
		void join(int a, int b) { a = find(a); b = find(b); if (a != b) { if (a < b) p[b] = a; else p[a] = b; } }
	};

	// the element types we can handle: returns the number of corner nodes (0 if unsupported)
	int CornerNodes(int type, bool& quadratic)
	{
		quadratic = false;
		switch (type) {
		case FE_HEX8:    return 8;
		case FE_HEX20:   quadratic = true; return 8;
		case FE_HEX27:   quadratic = true; return 8;
		case FE_PENTA6:  return 6;
		case FE_PENTA15: quadratic = true; return 6;
		case FE_PYRA5:   return 5;
		case FE_PYRA13:  quadratic = true; return 5;
		case FE_TET4:    return 4;
		case FE_TET10:   quadratic = true; return 4;
		}
		return 0;
	}

	typedef std::pair<int, int> Edge;
	inline Edge MakeEdge(int a, int b) { return Edge(std::min(a, b), std::max(a, b)); }

	struct Face {
		std::vector<int> loop;		// corner nodes (consecutive duplicates removed)
	};

	// Converts a (remapped) element in place. Returns 0 on success, 1 if the element should be deleted, -1 on failure.
	int ConvertElement(FSElement& el, FSMesh& mesh, std::string& err)
	{
		bool quadratic = false;
		const int nc = CornerNodes(el.Type(), quadratic);

		// unique corner nodes
		std::vector<int> U;
		for (int i = 0; i < nc; ++i)
			if (std::find(U.begin(), U.end(), el.m_node[i]) == U.end()) U.push_back(el.m_node[i]);
		const int nu = (int)U.size();
		if (nu < 4) return 1;
		if (nu == nc) return 0;	// all corner nodes are still distinct

		// gather the faces (with the mid-edge nodes for quadratic elements)
		std::map<Edge, int> midNode;
		std::vector<Face> faces;
		for (int i = 0; i < el.Faces(); ++i)
		{
			FSFace f;
			el.GetFace(i, f);
			int kf = 0;
			switch (f.Type()) {
			case FE_FACE_TRI3: case FE_FACE_TRI6: kf = 3; break;
			case FE_FACE_QUAD4: case FE_FACE_QUAD8: case FE_FACE_QUAD9: kf = 4; break;
			default: err = "Unsupported element face type."; return -1;
			}
			if (quadratic)
			{
				int off = kf;	// mid-edge nodes follow the corners
				for (int k = 0; k < kf; ++k)
				{
					int a = f.n[k], b = f.n[(k + 1) % kf];
					if (a != b) midNode[MakeEdge(a, b)] = f.n[off + k];
				}
			}
			// remove consecutive duplicates (cyclic)
			Face fc;
			for (int k = 0; k < kf; ++k)
			{
				int a = f.n[k];
				if (fc.loop.empty() || (fc.loop.back() != a)) fc.loop.push_back(a);
			}
			while ((fc.loop.size() > 1) && (fc.loop.front() == fc.loop.back())) fc.loop.pop_back();
			if (fc.loop.size() < 3) continue;	// collapsed to an edge or a point
			// any remaining repeated node means a bow-tie face that we can't handle
			std::set<int> s(fc.loop.begin(), fc.loop.end());
			if (s.size() != fc.loop.size()) { err = "Unsupported degenerate element (twisted face)."; return -1; }
			faces.push_back(fc);
		}

		std::vector<Face> tris, quads;
		for (const Face& f : faces) (f.loop.size() == 3 ? tris : quads).push_back(f);

		auto R = [&](int n) -> vec3d { return mesh.Node(n).r; };

		std::vector<int> corner;	// ordered corner nodes of the new element
		int newType = 0;

		if (nu == 4 && tris.size() == 4 && quads.empty())
		{
			corner = U;
			if (((R(corner[1]) - R(corner[0])) ^ (R(corner[2]) - R(corner[0]))) * (R(corner[3]) - R(corner[0])) < 0) std::swap(corner[1], corner[2]);
			newType = quadratic ? FE_TET10 : FE_TET4;
		}
		else if (nu == 5 && tris.size() == 4 && quads.size() == 1)
		{
			const std::vector<int>& q = quads[0].loop;
			int apex = -1;
			for (int n : U) if (std::find(q.begin(), q.end(), n) == q.end()) apex = n;
			if (apex < 0) { err = "Unsupported degenerate element."; return -1; }
			corner = { q[0], q[1], q[2], q[3], apex };
			vec3d c = (R(q[0]) + R(q[1]) + R(q[2]) + R(q[3])) * 0.25;
			vec3d N = (R(q[2]) - R(q[0])) ^ (R(q[3]) - R(q[1]));
			if (N * (R(apex) - c) < 0) std::swap(corner[1], corner[3]);
			newType = quadratic ? FE_PYRA13 : FE_PYRA5;
		}
		else if (nu == 6 && tris.size() == 2 && quads.size() == 3)
		{
			const std::vector<int>& A = tris[0].loop;
			const std::vector<int>& B = tris[1].loop;
			for (int a : A) if (std::find(B.begin(), B.end(), a) != B.end()) { err = "Unsupported degenerate element."; return -1; }

			// match each node of A with the node of B it is connected to by an edge of a quad face
			int match[3];
			for (int i = 0; i < 3; ++i)
			{
				match[i] = -1;
				for (const Face& q : quads)
				{
					for (int k = 0; k < 4; ++k)
					{
						int a = q.loop[k], b = q.loop[(k + 1) % 4];
						if ((a == A[i]) && (std::find(B.begin(), B.end(), b) != B.end())) match[i] = b;
						if ((b == A[i]) && (std::find(B.begin(), B.end(), a) != B.end())) match[i] = a;
					}
				}
				if (match[i] < 0) { err = "Unsupported degenerate element."; return -1; }
			}
			corner = { A[0], A[1], A[2], match[0], match[1], match[2] };
			vec3d cA = (R(corner[0]) + R(corner[1]) + R(corner[2])) / 3.0;
			vec3d cB = (R(corner[3]) + R(corner[4]) + R(corner[5])) / 3.0;
			if (((R(corner[1]) - R(corner[0])) ^ (R(corner[2]) - R(corner[0]))) * (cB - cA) < 0)
			{
				std::swap(corner[1], corner[2]);
				std::swap(corner[4], corner[5]);
			}
			newType = quadratic ? FE_PENTA15 : FE_PENTA6;
		}
		else
		{
			err = "Unsupported degenerate element configuration.";
			return -1;
		}

		// mid-edge nodes (FEBio ordering of the quadratic elements)
		std::vector<int> nodes = corner;
		if (quadratic)
		{
			static const int pe[9][2] = { {0,1},{1,2},{2,0},{3,4},{4,5},{5,3},{0,3},{1,4},{2,5} };
			static const int ye[8][2] = { {0,1},{1,2},{2,3},{3,0},{0,4},{1,4},{2,4},{3,4} };
			static const int te[6][2] = { {0,1},{1,2},{2,0},{0,3},{1,3},{2,3} };
			const int (*tab)[2] = (newType == FE_PENTA15 ? pe : (newType == FE_PYRA13 ? ye : te));
			int ne = (newType == FE_PENTA15 ? 9 : (newType == FE_PYRA13 ? 8 : 6));
			for (int i = 0; i < ne; ++i)
			{
				auto it = midNode.find(MakeEdge(corner[tab[i][0]], corner[tab[i][1]]));
				if (it == midNode.end()) { err = "Could not find a mid-edge node of a degenerate element."; return -1; }
				nodes.push_back(it->second);
			}
		}

		el.SetType(newType);
		for (size_t i = 0; i < nodes.size(); ++i) el.m_node[i] = nodes[i];
		return 0;
	}
}

int CollapseDegenerateElements(FSMesh& mesh, double tol, std::string& err)
{
	err.clear();
	const int NN = mesh.Nodes();
	const int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i) mesh.Element(i).m_ntag = 0;

	// automatic tolerance: 1e-3 x average element size
	if (tol <= 0.0)
	{
		double sum = 0; int cnt = 0;
		for (int i = 0; i < NE; ++i)
		{
			FSElement& el = mesh.Element(i);
			bool q;
			int nc = CornerNodes(el.Type(), q);
			if (nc == 0) continue;
			double s = 0;
			for (int j = 1; j < nc; ++j) s += (mesh.Node(el.m_node[j]).r - mesh.Node(el.m_node[0]).r).norm();
			sum += s / (nc - 1); cnt++;
		}
		if (cnt == 0) return 0;
		tol = 1e-3 * sum / cnt;
	}
	const double tol2 = tol * tol;

	// weld nodes of the same element that are within the tolerance
	UnionFind uf(NN);
	bool any = false;
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		bool q;
		if (CornerNodes(el.Type(), q) == 0) continue;
		int n = el.Nodes();
		for (int a = 0; a < n; ++a)
			for (int b = a + 1; b < n; ++b)
			{
				int na = el.m_node[a], nb = el.m_node[b];
				if (na == nb) continue;
				if ((mesh.Node(na).r - mesh.Node(nb).r).norm2() <= tol2) { uf.join(na, nb); any = true; }
			}
	}
	if (!any) return 0;

	// move the representative node to the average position of its cluster
	std::vector<vec3d> sum(NN, vec3d(0, 0, 0));
	std::vector<int> cnt(NN, 0);
	for (int i = 0; i < NN; ++i) { int r = uf.find(i); sum[r] += mesh.Node(i).r; cnt[r]++; }
	std::vector<int> rep(NN);
	for (int i = 0; i < NN; ++i)
	{
		rep[i] = uf.find(i);
		if (rep[i] != i) mesh.Node(i).m_gid = -1;	// so that the unused node can be removed
	}
	for (int i = 0; i < NN; ++i)
		if (rep[i] == i && cnt[i] > 1) mesh.Node(i).r = sum[i] / (double)cnt[i];

	// remap the elements and convert the degenerate ones
	int modified = 0;
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		bool quadratic;
		if (CornerNodes(el.Type(), quadratic) == 0) continue;
		int n = el.Nodes();
		bool changed = false;
		for (int j = 0; j < n; ++j)
		{
			int r = rep[el.m_node[j]];
			if (r != el.m_node[j]) { el.m_node[j] = r; changed = true; }
		}
		if (!changed) continue;

		// is the element degenerate (repeated corner nodes or repeated nodes)?
		bool degenerate = false;
		for (int a = 0; a < n && !degenerate; ++a)
			for (int b = a + 1; b < n; ++b)
				if (el.m_node[a] == el.m_node[b]) { degenerate = true; break; }

		if (degenerate)
		{
			int ret = ConvertElement(el, mesh, err);
			if (ret < 0) return -1;
			if (ret == 1) el.m_ntag = 1;
		}
		modified++;
	}
	return modified;
}

//-----------------------------------------------------------------------------
FECollapseDegenerateElements::FECollapseDegenerateElements() : FEModifier("Collapse degenerate elements")
{
	AddDoubleParam(0.0, "tolerance", "Nodes of an element closer than this are merged (0 = automatic, 1e-3 x average element size)");
}

FSMesh* FECollapseDegenerateElements::Apply(FSMesh* pm)
{
	FSMesh* pnm = new FSMesh(*pm);

	std::string err;
	int n = CollapseDegenerateElements(*pnm, GetFloatValue(0), err);
	if (n < 0)
	{
		delete pnm;
		error(err);
		return nullptr;
	}
	if (n == 0)
	{
		// nothing to do
		return pnm;
	}

	// delete elements that collapsed completely, remove unused nodes and rebuild the mesh
	FSMeshBuilder meshBuilder(*pnm);
	if (meshBuilder.DeleteTaggedElements(1) == 0)
	{
		pnm->RebuildMesh();
		meshBuilder.RemoveIsolatedNodes();
	}
	return pnm;
}
