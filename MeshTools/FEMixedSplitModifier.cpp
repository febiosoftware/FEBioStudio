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
#include "FESplitModifier.h"
#include <MeshLib/MeshMetrics.h>
#include "FEMeshUntangler.h"
#include <unordered_map>
#include <array>
#include <vector>
#include <algorithm>
#include <functional>
#include <cstdint>
using namespace std;

//-----------------------------------------------------------------------------
// Local node numbering of a split element: first the nodes of the element, then
// the midpoints of its edges, then the centers of its quad faces and, for a hex8,
// its center. The CHILD tables list the local nodes of the children.
namespace {

	// hex8: edge midpoints 8-19, face centers 20-25, center 26
	const int HEX_EDGE[12][2] = { {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7} };
	const int HEX_FACE[6][4] = { {0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},{3,2,1,0},{4,5,6,7} };
	const int HEX_CHILD[8][8] = {
		{  0,  8, 24, 11, 16, 20, 26, 23 },
		{  8,  1,  9, 24, 20, 17, 21, 26 },
		{ 11, 24, 10,  3, 23, 26, 22, 19 },
		{ 24,  9,  2, 10, 26, 21, 18, 22 },
		{ 16, 20, 26, 23,  4, 12, 25, 15 },
		{ 20, 17, 21, 26, 12,  5, 13, 25 },
		{ 23, 26, 22, 19, 15, 25, 14,  7 },
		{ 26, 21, 18, 22, 25, 13,  6, 14 } };

	// penta6: edge midpoints 6-14, quad face centers 15-17. The triangles are split
	// in four, and the prism in two layers.
	const int PENTA_EDGE[9][2] = { {0,1},{1,2},{2,0},{3,4},{4,5},{5,3},{0,3},{1,4},{2,5} };
	const int PENTA_FACE[3][4] = { {0,1,4,3},{1,2,5,4},{2,0,3,5} };
	const int PENTA_CHILD[8][6] = {
		{  0,  6,  8, 12, 15, 17 },
		{  6,  1,  7, 15, 13, 16 },
		{  8,  7,  2, 17, 16, 14 },
		{  6,  7,  8, 15, 16, 17 },
		{ 12, 15, 17,  3,  9, 11 },
		{ 15, 13, 16,  9,  4, 10 },
		{ 17, 16, 14, 11, 10,  5 },
		{ 15, 16, 17,  9, 10, 11 } };

	// pyra5: edge midpoints 5-12, base center 13. Five children are half-size copies
	// (at the corners of the base and at the apex), one is upside down with its apex
	// at the center of the base, and four tets fill the gaps under the base edges.
	const int PYRA_EDGE[8][2] = { {0,1},{1,2},{2,3},{3,0},{0,4},{1,4},{2,4},{3,4} };
	const int PYRA_CHILD[6][5] = {
		{  0,  5, 13,  8,  9 },
		{  5,  1,  6, 13, 10 },
		{ 13,  6,  2,  7, 11 },
		{  8, 13,  7,  3, 12 },
		{  9, 10, 11, 12,  4 },
		{  9, 12, 11, 10, 13 } };
	const int PYRA_TET[4][4] = { {5,13,9,10},{6,13,10,11},{7,13,11,12},{8,13,12,9} };

	// tet4: edge midpoints 4-9. Four children are half-size copies at the corners;
	// the octahedron in the middle is split into four tets around one of its three
	// diagonals (the equator nodes are ordered so that the tets are not inverted).
	const int TET_EDGE[6][2] = { {0,1},{1,2},{2,0},{0,3},{1,3},{2,3} };
	const int TET_CHILD[4][4] = { {0,4,6,7},{4,1,5,8},{6,5,2,9},{7,8,9,3} };
	const int TET_DIAGONAL[3][2] = { {4,9},{5,7},{6,8} };
	const int TET_EQUATOR[3][4] = { {5,6,7,8},{4,8,9,6},{4,5,9,7} };

	// faces: tri3 (edge midpoints 3-5) and quad4 (edge midpoints 4-7, center 8).
	// The INNER tables list the new edges inside the split face.
	const int TRI_EDGE[3][2] = { {0,1},{1,2},{2,0} };
	const int TRI_CHILD[4][3] = { {0,3,5},{3,1,4},{5,4,2},{3,4,5} };
	const int TRI_INNER[3][2] = { {3,4},{4,5},{5,3} };
	const int QUAD_EDGE[4][2] = { {0,1},{1,2},{2,3},{3,0} };
	const int QUAD_CHILD[4][4] = { {0,4,8,7},{4,1,5,8},{8,5,2,6},{7,8,6,3} };
	const int QUAD_INNER[4][2] = { {4,8},{5,8},{6,8},{7,8} };

	uint64_t edgeKey(int a, int b)
	{
		if (a > b) swap(a, b);
		return ((uint64_t)(uint32_t)a << 32) | (uint64_t)(uint32_t)b;
	}

	array<int, 4> quadKey(int a, int b, int c, int d)
	{
		array<int, 4> k = { a, b, c, d };
		sort(k.begin(), k.end());
		return k;
	}

	struct QuadKeyHash
	{
		size_t operator () (const array<int, 4>& k) const
		{
			size_t h = 0;
			for (int i : k) h ^= hash<int>()(i) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};

	FEMeshUntangler::Element untanglerElement(const FSElement& el)
	{
		FEMeshUntangler::Element e;
		switch (el.Type())
		{
		case FE_TET4  : e.type = FEMeshUntangler::TET4; break;
		case FE_PYRA5 : e.type = FEMeshUntangler::PYRA5; break;
		case FE_PENTA6: e.type = FEMeshUntangler::PENTA6; break;
		default       : e.type = FEMeshUntangler::HEX8; break;
		}
		for (int j = 0; j < 8; ++j) e.n[j] = (j < el.Nodes() ? el.m_node[j] : el.m_node[0]);
		return e;
	}

	vector<vec3d> nodePositions(FSMesh& mesh)
	{
		vector<vec3d> r(mesh.Nodes());
		for (int i = 0; i < mesh.Nodes(); ++i) r[i] = mesh.Node(i).r;
		return r;
	}

	int elementNodes(const FEMeshUntangler::Element& e)
	{
		switch (e.type)
		{
		case FEMeshUntangler::TET4  : return 4;
		case FEMeshUntangler::PYRA5 : return 5;
		case FEMeshUntangler::PENTA6: return 6;
		default: return 8;
		}
	}

	// Smallest scaled Jacobian at the corners of a hex8, penta6 or pyra5 (not at the pyramid's
	// apex): the determinant of the unit vectors along the three edges of the corner (1 for a
	// cube corner, <= 0 for a folded corner). FEBio only checks the integration points, but a
	// folded corner gives an inverted element when the element is refined. (1 for a tet4.)
	double cornerQuality(const vector<vec3d>& r, const FEMeshUntangler::Element& e)
	{
		static const int HEX[8][3] = { {1,3,4},{2,0,5},{3,1,6},{0,2,7},{7,5,0},{4,6,1},{5,7,2},{6,4,3} };
		static const int PENTA[6][3] = { {1,2,3},{2,0,4},{0,1,5},{5,4,0},{3,5,1},{4,3,2} };
		static const int PYRA[4][3] = { {1,3,4},{2,0,4},{3,1,4},{0,2,4} };
		const int (*T)[3] = nullptr;
		int nc = 0;
		switch (e.type)
		{
		case FEMeshUntangler::HEX8  : T = HEX; nc = 8; break;
		case FEMeshUntangler::PENTA6: T = PENTA; nc = 6; break;
		case FEMeshUntangler::PYRA5 : T = PYRA; nc = 4; break;
		default: return 1.0;
		}
		double q = 1.0;
		for (int i = 0; i < nc; ++i)
		{
			const vec3d& x = r[e.n[i]];
			vec3d a = r[e.n[T[i][0]]] - x, b = r[e.n[T[i][1]]] - x, c = r[e.n[T[i][2]]] - x;
			double L = a.Length() * b.Length() * c.Length();
			q = min(q, (L > 0 ? ((a ^ b) * c) / L : -1.0));
		}
		return q;
	}

	// Raises the smallest scaled Jacobian of a set of elements: the nodes of the worst element
	// move, one at a time, along the gradient of its scaled Jacobian (within their constraint),
	// as long as the worst of the elements around the node improves and no corner of those
	// elements folds (drops below 0.1, or below its value if it is smaller). (The untangler
	// lowers the total distortion, which can leave one element worse than it needs to be.)
	void polishWorstElement(vector<vec3d>& r, const vector<FEMeshUntangler::Element>& el, const vector<FEMeshUntangler::NodeConstraint>& c)
	{
		const int NN = (int)r.size(), NE = (int)el.size();
		vector<vector<int> > star(NN);
		for (int k = 0; k < NE; ++k)
			for (int j = 0; j < elementNodes(el[k]); ++j) star[el[k].n[j]].push_back(k);
		auto starQuality = [&](int v) {
			double q = 1e300;
			for (int k : star[v]) q = min(q, FEMeshUntangler::ScaledJacobian(r, el[k]));
			return q;
		};
		auto starCorners = [&](int v) {
			double q = 1e300;
			for (int k : star[v]) q = min(q, cornerQuality(r, el[k]));
			return q;
		};

		for (int iter = 0; iter < 200; ++iter)
		{
			int w = -1;
			double qw = 1e300;
			for (int k = 0; k < NE; ++k)
			{
				double q = FEMeshUntangler::ScaledJacobian(r, el[k]);
				if (q < qw) { qw = q; w = k; }
			}
			if (w < 0) return;
			const FEMeshUntangler::Element& e = el[w];
			const int m = elementNodes(e);
			double L = 0;
			for (int j = 0; j < m; ++j) L += (r[e.n[j]] - r[e.n[(j + 1) % m]]).Length();
			L /= m;

			bool moved = false;
			for (int j = 0; (j < m) && !moved; ++j)
			{
				int v = e.n[j];
				if (c[v].type == FEMeshUntangler::FIXED) continue;
				const vec3d x0 = r[v];
				const double q0 = starQuality(v);
				const double cmin = min(starCorners(v), 0.1);

				// gradient of the worst element's scaled Jacobian with respect to the node
				const double h = 1e-6 * L;
				double g[3];
				for (int d = 0; d < 3; ++d)
				{
					vec3d dx(d == 0 ? h : 0, d == 1 ? h : 0, d == 2 ? h : 0);
					r[v] = x0 + dx; double qp = FEMeshUntangler::ScaledJacobian(r, e);
					r[v] = x0 - dx; double qm = FEMeshUntangler::ScaledJacobian(r, e);
					g[d] = (qp - qm) / (2 * h);
				}
				r[v] = x0;
				vec3d t(g[0], g[1], g[2]);
				if (c[v].type == FEMeshUntangler::PLANE) t -= c[v].dir * (t * c[v].dir);
				else if (c[v].type == FEMeshUntangler::LINE) t = c[v].dir * (t * c[v].dir);
				if (t.unit() == 0) continue;

				for (double s = 0.1 * L; s > 1e-4 * L; s *= 0.5)
				{
					r[v] = x0 + t * s;
					if ((starQuality(v) > q0) && (starCorners(v) >= cmin)) { moved = true; break; }
				}
				if (!moved) r[v] = x0;
			}
			if (!moved) return;
		}
	}

	// Optimizes the nodes around the elements whose scaled Jacobian is below qmin, with the
	// untangler of the hex-dominant remesher. Nodes on a surface (the outside, or between
	// parts) only move where they cannot leave it: in the plane of a flat neighborhood, or
	// along a straight crease; corner nodes do not move. The result is kept if it improves
	// the worst element around those elements. Returns the number of elements below qmin.
	int improveElements(FSMesh& mesh, double qmin)
	{
		const int NN = mesh.Nodes(), NE = mesh.Elements();
		vector<vec3d> r = nodePositions(mesh);
		vector<FEMeshUntangler::Element> E(NE);
		for (int i = 0; i < NE; ++i) E[i] = untanglerElement(mesh.Element(i));

		vector<int> bad;
		for (int i = 0; i < NE; ++i) if (FEMeshUntangler::ScaledJacobian(r, E[i]) < qmin) bad.push_back(i);
		if (bad.empty()) return 0;

		vector<vector<int> > NEL(NN);
		for (int i = 0; i < NE; ++i)
		{
			FSElement& el = mesh.Element(i);
			for (int k = 0; k < el.Nodes(); ++k) NEL[el.m_node[k]].push_back(i);
		}

		// the nodes that may move: those of the bad elements and of the elements around them
		// (RINGS layers of elements)
		const int RINGS = 2;
		vector<char> movable(NN, 0);
		for (int i : bad)
		{
			FSElement& el = mesh.Element(i);
			for (int k = 0; k < el.Nodes(); ++k) movable[el.m_node[k]] = 1;
		}
		for (int ring = 0; ring < RINGS; ++ring)
		{
			vector<char> next(movable);
			for (int i = 0; i < NE; ++i)
			{
				FSElement& el = mesh.Element(i);
				bool b = false;
				for (int k = 0; k < el.Nodes(); ++k) if (movable[el.m_node[k]]) { b = true; break; }
				if (b) for (int k = 0; k < el.Nodes(); ++k) next[el.m_node[k]] = 1;
			}
			movable.swap(next);
		}

		// the surfaces and creases around the nodes
		auto faceNormal = [&](FSFace& f) {
			vec3d n = (f.Nodes() == 4 ? (r[f.n[2]] - r[f.n[0]]) ^ (r[f.n[3]] - r[f.n[1]]) : (r[f.n[1]] - r[f.n[0]]) ^ (r[f.n[2]] - r[f.n[0]]));
			n.unit();
			return n;
		};
		vector<int> nface(NN, 0);
		vector<vec3d> normal(NN, vec3d(0, 0, 0));
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			FSFace& f = mesh.Face(i);
			vec3d n = faceNormal(f);
			for (int k = 0; k < f.Nodes(); ++k) { nface[f.n[k]]++; normal[f.n[k]] += n; }
		}
		for (vec3d& n : normal) n.unit();
		vector<double> flat(NN, 1.0);	// smallest cosine between the normal of a face around the node and their average
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			FSFace& f = mesh.Face(i);
			vec3d n = faceNormal(f);
			for (int k = 0; k < f.Nodes(); ++k) flat[f.n[k]] = min(flat[f.n[k]], n * normal[f.n[k]]);
		}
		vector<int> creases(NN, 0), creaseNbr(2 * NN, -1);
		for (int i = 0; i < mesh.Edges(); ++i)
		{
			FSEdge& e = mesh.Edge(i);
			if (e.m_gid < 0) continue;
			int a = e.n[0], b = e.n[1];
			if (creases[a] < 2) creaseNbr[2 * a + creases[a]] = b;
			if (creases[b] < 2) creaseNbr[2 * b + creases[b]] = a;
			creases[a]++;
			creases[b]++;
		}

		const double tol = 1e-9;
		vector<FEMeshUntangler::NodeConstraint> C(NN);
		for (int i = 0; i < NN; ++i)
		{
			FEMeshUntangler::NodeConstraint& c = C[i];
			c.type = FEMeshUntangler::FIXED;
			if (movable[i] == 0) continue;
			if (nface[i] == 0) c.type = FEMeshUntangler::FREE;
			else if (mesh.Node(i).m_gid >= 0) continue;	// corner
			else if (creases[i] == 2)
			{
				vec3d a = r[creaseNbr[2 * i]] - r[i], b = r[creaseNbr[2 * i + 1]] - r[i];
				a.unit(); b.unit();
				if (a * b < -1.0 + tol) { c.type = FEMeshUntangler::LINE; c.dir = a; }
			}
			else if ((creases[i] == 0) && (flat[i] > 1.0 - tol)) { c.type = FEMeshUntangler::PLANE; c.dir = normal[i]; }
		}

		// the elements that have a node that can move
		vector<int> patch;
		for (int i = 0; i < NE; ++i)
		{
			FSElement& el = mesh.Element(i);
			for (int k = 0; k < el.Nodes(); ++k)
				if (C[el.m_node[k]].type != FEMeshUntangler::FIXED) { patch.push_back(i); break; }
		}
		if (patch.empty()) return (int)bad.size();

		vector<int> local(NN, -1), global;
		for (int i : patch)
		{
			FSElement& el = mesh.Element(i);
			for (int k = 0; k < el.Nodes(); ++k)
				if (local[el.m_node[k]] < 0) { local[el.m_node[k]] = (int)global.size(); global.push_back(el.m_node[k]); }
		}
		vector<vec3d> rl(global.size());
		vector<FEMeshUntangler::NodeConstraint> cl(global.size());
		for (size_t k = 0; k < global.size(); ++k) { rl[k] = r[global[k]]; cl[k] = C[global[k]]; }
		vector<FEMeshUntangler::Element> el(patch.size());
		double before = 1e300;
		for (size_t k = 0; k < patch.size(); ++k)
		{
			before = min(before, FEMeshUntangler::ScaledJacobian(r, E[patch[k]]));
			el[k] = E[patch[k]];
			for (int j = 0; j < 8; ++j) el[k].n[j] = local[el[k].n[j]];
		}

		// keep a result if it improves the worst element without folding corners
		auto worst = [&](const vector<vec3d>& x) {
			double q = 1e300;
			for (auto& e : el) q = min(q, FEMeshUntangler::ScaledJacobian(x, e));
			return q;
		};
		auto worstCorner = [&](const vector<vec3d>& x) {
			double q = 1e300;
			for (auto& e : el) q = min(q, cornerQuality(x, e));
			return q;
		};
		const double cmin = min(worstCorner(rl), 0.1);
		auto better = [&](const vector<vec3d>& x) { return (worst(x) > before) && (worstCorner(x) >= cmin); };

		vector<vec3d> r0(rl);
		FEMeshUntangler untangler;
		untangler.SetThreads(1);
		untangler.Apply(rl, el, cl);
		if (!better(rl)) rl = r0;
		polishWorstElement(rl, el, cl);

		if (better(rl))
		{
			for (size_t k = 0; k < global.size(); ++k) mesh.Node(global[k]).r = rl[k];
		}
		return (int)bad.size();
	}
}

//-----------------------------------------------------------------------------
FEMixedSplitModifier::FEMixedSplitModifier() : FEModifier("Split")
{
	m_smoothSurface = false;
	m_repair = true;
	m_limitedNodes = 0;
	m_optimized = 0;
	m_worstBefore = 0;
}

//-----------------------------------------------------------------------------
double FEMixedSplitModifier::WorstElement(FSMesh& mesh)
{
	vector<vec3d> r = nodePositions(mesh);
	double q = 1.0;
	for (int i = 0; i < mesh.Elements(); ++i)
		q = min(q, FEMeshUntangler::ScaledJacobian(r, untanglerElement(mesh.Element(i))));
	return q;
}

//-----------------------------------------------------------------------------
void FEMixedSplitModifier::DoSurfaceSmoothing(bool b)
{
	m_smoothSurface = b;
}

//-----------------------------------------------------------------------------
bool FEMixedSplitModifier::IsSupported(FSMesh& mesh)
{
	int NE = mesh.Elements();
	if (NE == 0) return false;
	for (int i = 0; i < NE; ++i)
	{
		int ntype = mesh.Element(i).Type();
		if ((ntype != FE_HEX8) && (ntype != FE_PENTA6) && (ntype != FE_PYRA5) && (ntype != FE_TET4)) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
FSMesh* FEMixedSplitModifier::Apply(FSMesh* pm)
{
	m_limitedNodes = 0;
	m_optimized = 0;
	m_worstBefore = 0;
	if ((pm == nullptr) || !IsSupported(*pm))
	{
		error("Only meshes of hex8, penta6, pyra5 and tet4 elements can be split.");
		return nullptr;
	}

	const int NN0 = pm->Nodes();
	const int NE0 = pm->Elements();
	const int NF0 = pm->Faces();
	const int NC0 = pm->Edges();

	// positions of the nodes of the refined mesh (the nodes of the mesh come first)
	vector<vec3d> r(NN0);
	for (int i = 0; i < NN0; ++i) r[i] = pm->Node(i).r;

	// Every edge gets a node at its midpoint and every quad face a node at its center,
	// shared by all the elements around it.
	unordered_map<uint64_t, int> edgeNode;
	unordered_map<array<int, 4>, int, QuadKeyHash> faceNode;
	edgeNode.reserve(4 * (size_t)NN0);
	auto midNode = [&](int a, int b) {
		auto it = edgeNode.emplace(edgeKey(a, b), (int)r.size());
		if (it.second) r.push_back((r[a] + r[b]) * 0.5);
		return it.first->second;
	};
	auto centerNode = [&](int a, int b, int c, int d) {
		auto it = faceNode.emplace(quadKey(a, b, c, d), (int)r.size());
		if (it.second) r.push_back((r[a] + r[b] + r[c] + r[d]) * 0.25);
		return it.first->second;
	};

	// split the elements (a pyra5 has ten children, the others eight)
	int NE1 = 0;
	for (int i = 0; i < NE0; ++i) NE1 += (pm->Element(i).Type() == FE_PYRA5 ? 10 : 8);
	FSMesh* pmnew = new FSMesh;
	pmnew->Create(0, NE1);

	vector<pair<int, int> > hexCenter;	// (center node, hex8)
	int m[27];
	int ne = 0;
	auto addChild = [&](const FSElement& parent, int ntype, const int* loc) {
		// the children keep the part, material, fiber and material axes of the parent
		FSElement& el = pmnew->Element(ne++);
		el = parent;
		el.SetType(ntype);
		el.SetID(-1);
		for (int k = 0; k < el.Nodes(); ++k) el.m_node[k] = m[loc[k]];
	};

	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		const int* n = el.m_node;
		switch (el.Type())
		{
		case FE_HEX8:
		{
			for (int j = 0; j < 8; ++j) m[j] = n[j];
			for (int j = 0; j < 12; ++j) m[8 + j] = midNode(n[HEX_EDGE[j][0]], n[HEX_EDGE[j][1]]);
			for (int j = 0; j < 6; ++j)
			{
				const int* f = HEX_FACE[j];
				m[20 + j] = centerNode(n[f[0]], n[f[1]], n[f[2]], n[f[3]]);
			}
			vec3d c(0, 0, 0);
			for (int j = 0; j < 8; ++j) c += r[n[j]];
			m[26] = (int)r.size();
			r.push_back(c * 0.125);
			hexCenter.push_back({ m[26], i });
			for (int j = 0; j < 8; ++j) addChild(el, FE_HEX8, HEX_CHILD[j]);
		}
		break;
		case FE_PENTA6:
		{
			for (int j = 0; j < 6; ++j) m[j] = n[j];
			for (int j = 0; j < 9; ++j) m[6 + j] = midNode(n[PENTA_EDGE[j][0]], n[PENTA_EDGE[j][1]]);
			for (int j = 0; j < 3; ++j)
			{
				const int* f = PENTA_FACE[j];
				m[15 + j] = centerNode(n[f[0]], n[f[1]], n[f[2]], n[f[3]]);
			}
			for (int j = 0; j < 8; ++j) addChild(el, FE_PENTA6, PENTA_CHILD[j]);
		}
		break;
		case FE_PYRA5:
		{
			for (int j = 0; j < 5; ++j) m[j] = n[j];
			for (int j = 0; j < 8; ++j) m[5 + j] = midNode(n[PYRA_EDGE[j][0]], n[PYRA_EDGE[j][1]]);
			m[13] = centerNode(n[0], n[1], n[2], n[3]);
			for (int j = 0; j < 6; ++j) addChild(el, FE_PYRA5, PYRA_CHILD[j]);
			for (int j = 0; j < 4; ++j) addChild(el, FE_TET4, PYRA_TET[j]);
		}
		break;
		case FE_TET4:
		{
			for (int j = 0; j < 4; ++j) m[j] = n[j];
			for (int j = 0; j < 6; ++j) m[4 + j] = midNode(n[TET_EDGE[j][0]], n[TET_EDGE[j][1]]);
			for (int j = 0; j < 4; ++j) addChild(el, FE_TET4, TET_CHILD[j]);

			// split the octahedron along its shortest diagonal
			int d = 0;
			double L2min = 0;
			for (int k = 0; k < 3; ++k)
			{
				double L2 = (r[m[TET_DIAGONAL[k][0]]] - r[m[TET_DIAGONAL[k][1]]]).SqrLength();
				if ((k == 0) || (L2 < L2min)) { d = k; L2min = L2; }
			}
			for (int k = 0; k < 4; ++k)
			{
				const int loc[4] = { TET_DIAGONAL[d][0], TET_DIAGONAL[d][1], TET_EQUATOR[d][k], TET_EQUATOR[d][(k + 1) % 4] };
				addChild(el, FE_TET4, loc);
			}
		}
		break;
		}
	}
	assert(ne == NE1);

	// Split the edges in two and the faces in four. The children keep the partition
	// of their parent (surface or edge), so that the surfaces and edges of the object
	// are the same as before. The lines inside the split faces are new edges that are
	// not on a feature edge (gid = -1), as for all edges between faces of one surface.
	auto findMid = [&](int a, int b) {
		auto it = edgeNode.find(edgeKey(a, b));
		return (it == edgeNode.end() ? -1 : it->second);
	};
	auto findCenter = [&](int a, int b, int c, int d) {
		auto it = faceNode.find(quadKey(a, b, c, d));
		return (it == faceNode.end() ? -1 : it->second);
	};
	auto mismatch = [&]() -> FSMesh* {
		delete pmnew;
		error("The faces or edges of the mesh do not match its elements.");
		return nullptr;
	};

	vector<FSEdge> edges;
	edges.reserve(2 * (size_t)NC0 + 4 * (size_t)NF0);
	auto addEdge = [&](const FSEdge* parent, int a, int b) {
		FSEdge e;
		if (parent) e = *parent; else e.m_gid = -1;
		e.SetType(FE_EDGE2);
		e.n[0] = a;
		e.n[1] = b;
		e.m_face[0] = e.m_face[1] = -1;
		e.SetID((int)edges.size() + 1);
		edges.push_back(e);
	};
	for (int i = 0; i < NC0; ++i)
	{
		FSEdge& e = pm->Edge(i);
		int mid = (e.Type() == FE_EDGE2 ? findMid(e.n[0], e.n[1]) : -1);
		if (mid < 0) return mismatch();
		addEdge(&e, e.n[0], mid);
		addEdge(&e, mid, e.n[1]);
	}

	vector<FSFace> faces;
	vector<bool> exterior;	// faces on the outside (not between two parts)
	faces.reserve(4 * (size_t)NF0);
	exterior.reserve(4 * (size_t)NF0);
	auto addFace = [&](FSFace& parent, int ntype, const int* loc) {
		faces.push_back(parent);
		FSFace& face = faces.back();
		face.SetType((FEFaceType)ntype);
		face.SetID((int)faces.size());
		for (int k = 0; k < face.Nodes(); ++k) face.n[k] = m[loc[k]];
		for (int k = 0; k < 4; ++k) face.m_edge[k] = -1;	// (neighbors and elements are found when the mesh is built)
		exterior.push_back(parent.IsExternal());
	};
	for (int i = 0; i < NF0; ++i)
	{
		FSFace& f = pm->Face(i);
		const int* n = f.n;
		if (f.Type() == FE_FACE_TRI3)
		{
			for (int j = 0; j < 3; ++j) m[j] = n[j];
			for (int j = 0; j < 3; ++j)
			{
				m[3 + j] = findMid(n[TRI_EDGE[j][0]], n[TRI_EDGE[j][1]]);
				if (m[3 + j] < 0) return mismatch();
			}
			for (int j = 0; j < 4; ++j) addFace(f, FE_FACE_TRI3, TRI_CHILD[j]);
			for (int j = 0; j < 3; ++j) addEdge(nullptr, m[TRI_INNER[j][0]], m[TRI_INNER[j][1]]);
		}
		else if (f.Type() == FE_FACE_QUAD4)
		{
			for (int j = 0; j < 4; ++j) m[j] = n[j];
			for (int j = 0; j < 4; ++j)
			{
				m[4 + j] = findMid(n[QUAD_EDGE[j][0]], n[QUAD_EDGE[j][1]]);
				if (m[4 + j] < 0) return mismatch();
			}
			m[8] = findCenter(n[0], n[1], n[2], n[3]);
			if (m[8] < 0) return mismatch();
			for (int j = 0; j < 4; ++j) addFace(f, FE_FACE_QUAD4, QUAD_CHILD[j]);
			for (int j = 0; j < 4; ++j) addEdge(nullptr, m[QUAD_INNER[j][0]], m[QUAD_INNER[j][1]]);
		}
		else return mismatch();
	}

	const int NN1 = (int)r.size();
	vector<vec3d> rs;	// smoothed positions

	// Smooth the surface: after the split, every node on the outside moves to the
	// average, over the faces around it, of the centroid of each quad and of the
	// weighted average 1/4 (node) + 3/8 (its two neighbors in the face) of each
	// triangle. This is Catmull-Clark subdivision on regular quads and Loop
	// subdivision on regular triangles. The edges between surfaces are creases,
	// whose nodes move as 1/4, 1/2, 1/4 of their two neighbors along the crease (cubic
	// B-spline subdivision of the curve), and corner nodes (nodes of the object, or
	// nodes where other than two creases meet) do not move.
	// Only the part of the move normal to the surface (or to the crease) is kept: the
	// nodes are not shifted along the surface, which would distort the elements next
	// to it, and flat surfaces and straight edges keep their nodes where they are.
	if (m_smoothSurface)
	{
		vector<int> creases(NN1, 0);
		vector<int> creaseNbr(2 * NN1, -1);	// the two neighbors along the crease
		for (const FSEdge& e : edges)
		{
			if (e.m_gid < 0) continue;
			int a = e.n[0], b = e.n[1];
			if (creases[a] < 2) creaseNbr[2 * a + creases[a]] = b;
			if (creases[b] < 2) creaseNbr[2 * b + creases[b]] = a;
			creases[a]++;
			creases[b]++;
		}

		vector<int> nfaces(NN1, 0);
		vector<vec3d> faceSum(NN1, vec3d(0, 0, 0));
		vector<vec3d> normal(NN1, vec3d(0, 0, 0));	// area-weighted
		for (size_t i = 0; i < faces.size(); ++i)
		{
			if (exterior[i] == false) continue;
			const FSFace& f = faces[i];
			int nn = f.Nodes();
			vec3d A = (nn == 4 ? (r[f.n[2]] - r[f.n[0]]) ^ (r[f.n[3]] - r[f.n[1]]) : (r[f.n[1]] - r[f.n[0]]) ^ (r[f.n[2]] - r[f.n[0]]));
			for (int k = 0; k < nn; ++k)
			{
				int v = f.n[k];
				const vec3d& prev = r[f.n[(k + nn - 1) % nn]];
				const vec3d& next = r[f.n[(k + 1) % nn]];
				vec3d c;
				if (nn == 4) c = (r[v] + next + r[f.n[(k + 2) % 4]] + prev) * 0.25;
				else c = r[v] * 0.25 + (prev + next) * 0.375;
				faceSum[v] += c;
				nfaces[v]++;
				normal[v] += A;
			}
		}

		rs = r;
		for (int i = 0; i < NN1; ++i)
		{
			if (nfaces[i] == 0) continue;	// not on the outside
			bool corner = ((i < NN0) && (pm->Node(i).m_gid >= 0)) || ((creases[i] != 0) && (creases[i] != 2));
			if (corner) continue;
			vec3d d;
			if (creases[i] == 2)
			{
				const vec3d& a = r[creaseNbr[2 * i]];
				const vec3d& b = r[creaseNbr[2 * i + 1]];
				d = (a + b) * 0.25 - r[i] * 0.5;
				vec3d t = b - a;
				double L = t.Length();
				if (L > 0) { t /= L; d -= t * (d * t); }
			}
			else
			{
				d = faceSum[i] / (double)nfaces[i] - r[i];
				vec3d n = normal[i];
				double L = n.Length();
				if (L > 0) { n /= L; d = n * (d * n); }
				else d = vec3d(0, 0, 0);
			}
			rs[i] = r[i] + d;
		}

		// The new nodes inside the mesh stay at the midpoints and centers of the edges,
		// faces and hex8 they were created in, so that they follow the surface.
		for (auto& it : edgeNode)
		{
			int k = it.second;
			if (nfaces[k] > 0) continue;
			int a = (int)(it.first >> 32), b = (int)(it.first & 0xffffffffu);
			rs[k] = (rs[a] + rs[b]) * 0.5;
		}
		for (auto& it : faceNode)
		{
			int k = it.second;
			if (nfaces[k] > 0) continue;
			const array<int, 4>& q = it.first;
			rs[k] = (rs[q[0]] + rs[q[1]] + rs[q[2]] + rs[q[3]]) * 0.25;
		}
		for (auto& it : hexCenter)
		{
			const int* n = pm->Element(it.second).m_node;
			vec3d c(0, 0, 0);
			for (int j = 0; j < 8; ++j) c += rs[n[j]];
			rs[it.first] = c * 0.125;
		}
	}

	// create the nodes, faces and edges, and build the mesh
	pmnew->Create(NN1, 0, (int)faces.size(), (int)edges.size());
	for (int i = 0; i < NN0; ++i) pmnew->Node(i) = pm->Node(i);
	for (int i = 0; i < NN1; ++i) pmnew->Node(i).r = r[i];
	for (int i = 0; i < (int)faces.size(); ++i) pmnew->Face(i) = faces[i];
	for (int i = 0; i < (int)edges.size(); ++i) pmnew->Edge(i) = edges[i];

	// Move the nodes to their smoothed positions, but not so far that an element is
	// inverted: the displacements of the nodes of such an element are halved until it
	// is valid (and dropped as a last resort). Elements that are invalid without
	// smoothing are left alone.
	if (m_smoothSurface)
	{
		FSMesh& mesh = *pmnew;
		vector<char> moved(NN1, 0);
		for (int i = 0; i < NN1; ++i) moved[i] = ((rs[i] - r[i]).SqrLength() > 0 ? 1 : 0);
		vector<int> check;
		for (int i = 0; i < NE1; ++i)
		{
			FSElement& el = mesh.Element(i);
			bool b = false;
			for (int k = 0; k < el.Nodes(); ++k) if (moved[el.m_node[k]]) { b = true; break; }
			if (b && (FEMeshMetrics::SolidJacobian(mesh, el) > 0)) check.push_back(i);
		}

		vector<double> alpha(NN1, 1.0);
		for (int iter = 0; ; ++iter)
		{
			for (int i = 0; i < NN1; ++i) if (moved[i]) mesh.Node(i).r = r[i] + (rs[i] - r[i]) * alpha[i];

			vector<char> tag(NN1, 0);
			bool inverted = false;
			for (int i : check)
			{
				FSElement& el = mesh.Element(i);
				if (FEMeshMetrics::SolidJacobian(mesh, el) <= 0)
				{
					inverted = true;
					for (int k = 0; k < el.Nodes(); ++k) tag[el.m_node[k]] = 1;
				}
			}
			if (inverted == false) break;
			for (int i = 0; i < NN1; ++i) if (tag[i]) alpha[i] = (iter < 8 ? 0.5 * alpha[i] : 0.0);
		}
		for (int i = 0; i < NN1; ++i) if (moved[i] && (alpha[i] < 1.0)) m_limitedNodes++;
	}

	// Improve the elements that came out worse than the worst element before the refinement
	// (and below 0.2). This happens at a corner where an element's Jacobian is negative or
	// small although it is fine at the integration points that FEBio checks (for instance a
	// pyramid whose apex is below the plane of its base at a corner), since a child sits in
	// that corner.
	m_worstBefore = WorstElement(*pm);
	if (m_repair) m_optimized = improveElements(*pmnew, min(m_worstBefore, 0.2));

	pmnew->BuildMesh();

	return pmnew;
}
