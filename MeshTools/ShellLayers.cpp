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
#include "ShellLayers.h"
#include "FECollapseDegenerate.h"
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSMeshBuilder.h>
#include <algorithm>
#include <map>

bool ShellSection::Init(const FSMesh& shell, std::string& err)
{
	const int N0 = shell.Nodes();
	std::vector<int> nodeMap(N0, -1);
	bool haveLinear = false, haveQuadratic = false;
	std::vector<bool> isCorner;

	for (int i = 0; i < shell.Elements(); ++i)
	{
		const FSElement& el = shell.Element(i);
		int nc = 0, ne = 0;
		switch (el.Type()) {
		case FE_TRI3:  nc = 3; ne = 3; haveLinear = true; break;
		case FE_QUAD4: nc = 4; ne = 4; haveLinear = true; break;
		case FE_TRI6:  nc = 3; ne = 6; haveQuadratic = true; break;
		case FE_QUAD8: nc = 4; ne = 8; haveQuadratic = true; break;
		default:
			err = "Only TRI3, QUAD4, TRI6 and QUAD8 shell elements are supported.";
			return false;
		}

		elems.push_back(i);
		ecorners.push_back(nc);
		std::vector<int> en(ne);
		for (int k = 0; k < ne; ++k)
		{
			int n = el.m_node[k];
			if (nodeMap[n] == -1)
			{
				nodeMap[n] = nn++;
				base.push_back(n);
				isCorner.push_back(false);
			}
			en[k] = nodeMap[n];
			if (k < nc) isCorner[en[k]] = true;
		}
		enodes.push_back(en);
	}
	if (elems.empty()) { err = "The shell mesh has no elements."; return false; }
	if (haveLinear && haveQuadratic) { err = "The shell mesh mixes linear and quadratic elements."; return false; }
	quadratic = haveQuadratic;

	cornerIndex.assign(nn, -1);
	for (int i = 0; i < nn; ++i)
		if (isCorner[i]) { cornerIndex[i] = ncorner++; cornerNode.push_back(i); }
	return true;
}

vec3d ShellSection::AreaNormal(int e, const std::vector<vec3d>& r0) const
{
	const std::vector<int>& v = enodes[e];
	if (ecorners[e] == 3) return ((r0[v[1]] - r0[v[0]]) ^ (r0[v[2]] - r0[v[0]])) * 0.5;
	return ((r0[v[2]] - r0[v[0]]) ^ (r0[v[3]] - r0[v[1]])) * 0.5;
}

FSMesh* ShellSection::BuildSolid(const std::vector<vec3d>& r0, int nl, const std::vector<vec3d>& layerPos, const std::vector<vec3d>& midPos) const
{
	const int nmid = (quadratic ? nl * ncorner : 0);
	FSMesh* mesh = new FSMesh();
	mesh->Create(nn * (nl + 1) + nmid, (int)elems.size() * nl);
	for (int i = 0; i < (int)layerPos.size(); ++i) mesh->Node(i).r = layerPos[i];
	for (int i = 0; i < nmid; ++i) mesh->Node(nn * (nl + 1) + i).r = midPos[i];

	// orientation of each element: the base face normal must point towards the top face for a positive Jacobian
	std::vector<bool> flip(elems.size(), false);
	for (size_t e = 0; e < elems.size(); ++e)
	{
		const std::vector<int>& v = enodes[e];
		vec3d dm(0, 0, 0);
		for (int k = 0; k < ecorners[e]; ++k) dm += layerPos[nn + v[k]] - layerPos[v[k]];
		if (AreaNormal((int)e, r0) * dm < 0) flip[e] = true;
	}

	int ie = 0;
	for (int l = 0; l < nl; ++l)
	{
		auto L = [&](int layer, int i) { return layer * nn + i; };
		auto M = [&](int i) { return (nl + 1) * nn + l * ncorner + cornerIndex[i]; };

		for (size_t e = 0; e < elems.size(); ++e)
		{
			FSElement& el = mesh->Element(ie++);
			el.m_gid = 0;

			const int nc = ecorners[e];
			int v[4], m[4] = { -1, -1, -1, -1 };
			for (int k = 0; k < nc; ++k) v[k] = enodes[e][k];
			if (quadratic) for (int k = 0; k < nc; ++k) m[k] = enodes[e][nc + k];

			if (flip[e])
			{
				if (nc == 3)
				{
					std::swap(v[1], v[2]);
					std::swap(m[0], m[2]);	// new mid-edge nodes: (v0,v2) (v2,v1) (v1,v0) = old (m2, m1, m0)
				}
				else
				{
					std::swap(v[1], v[3]);
					std::swap(m[0], m[3]);	// new mid-edge nodes = old (m3, m2, m1, m0)
					std::swap(m[1], m[2]);
				}
			}

			if (nc == 3)
			{
				if (!quadratic)
				{
					el.SetType(FE_PENTA6);
					for (int k = 0; k < 3; ++k) { el.m_node[k] = L(l, v[k]); el.m_node[k + 3] = L(l + 1, v[k]); }
				}
				else
				{
					el.SetType(FE_PENTA15);
					for (int k = 0; k < 3; ++k)
					{
						el.m_node[k] = L(l, v[k]);
						el.m_node[k + 3] = L(l + 1, v[k]);
						el.m_node[6 + k] = L(l, m[k]);		// 6:(0,1) 7:(1,2) 8:(2,0)
						el.m_node[9 + k] = L(l + 1, m[k]);	// 9:(3,4) 10:(4,5) 11:(5,3)
						el.m_node[12 + k] = M(v[k]);		// 12:(0,3) 13:(1,4) 14:(2,5)
					}
				}
			}
			else
			{
				if (!quadratic)
				{
					el.SetType(FE_HEX8);
					for (int k = 0; k < 4; ++k) { el.m_node[k] = L(l, v[k]); el.m_node[k + 4] = L(l + 1, v[k]); }
				}
				else
				{
					el.SetType(FE_HEX20);
					for (int k = 0; k < 4; ++k)
					{
						el.m_node[k] = L(l, v[k]);
						el.m_node[k + 4] = L(l + 1, v[k]);
						el.m_node[8 + k] = L(l, m[k]);		// 8:(0,1) 9:(1,2) 10:(2,3) 11:(3,0)
						el.m_node[12 + k] = L(l + 1, m[k]);	// 12:(4,5) 13:(5,6) 14:(6,7) 15:(7,4)
						el.m_node[16 + k] = M(v[k]);		// 16:(0,4) 17:(1,5) 18:(2,6) 19:(3,7)
					}
				}
			}
		}
	}
	return mesh;
}

bool FinishSolidMesh(FSMesh& mesh, double collapseTol, bool autoPartition, double creaseAngle, std::string& err)
{
	// eliminate degenerate elements (e.g. where the section touches an axis of rotation)
	std::string cerr;
	int nc = CollapseDegenerateElements(mesh, collapseTol, cerr);
	if (nc < 0)
	{
		err = "Could not eliminate degenerate elements: " + cerr;
		return false;
	}
	if (nc > 0)
	{
		FSMeshBuilder cleaner(mesh);
		if (cleaner.DeleteTaggedElements(1) == 0) cleaner.RemoveIsolatedNodes();
	}

	mesh.RebuildMesh();
	if (autoPartition)
	{
		// same as the Auto Partition modifier in the Mesh panel
		FSMeshBuilder meshBuilder(mesh);
		meshBuilder.AutoPartition(creaseAngle);
	}
	return true;
}

FSMesh* ConvertShellToQuadratic(const FSMesh& shell)
{
	const int N0 = shell.Nodes();
	const int NE = shell.Elements();
	std::map<std::pair<int, int>, int> edgeNode;
	std::vector<vec3d> pos;
	for (int i = 0; i < N0; ++i) pos.push_back(shell.Node(i).r);

	auto mid = [&](int a, int b) {
		std::pair<int, int> key(std::min(a, b), std::max(a, b));
		auto it = edgeNode.find(key);
		if (it != edgeNode.end()) return it->second;
		int n = (int)pos.size();
		pos.push_back((shell.Node(a).r + shell.Node(b).r) * 0.5);
		edgeNode[key] = n;
		return n;
	};

	std::vector<std::vector<int>> conn(NE);
	for (int i = 0; i < NE; ++i)
	{
		const FSElement& el = shell.Element(i);
		int nc = 0;
		if (el.Type() == FE_TRI3) nc = 3;
		else if (el.Type() == FE_QUAD4) nc = 4;
		else return nullptr;
		std::vector<int>& c = conn[i];
		for (int k = 0; k < nc; ++k) c.push_back(el.m_node[k]);
		for (int k = 0; k < nc; ++k) c.push_back(mid(el.m_node[k], el.m_node[(k + 1) % nc]));
	}

	FSMesh* mesh = new FSMesh();
	mesh->Create((int)pos.size(), NE);
	for (int i = 0; i < (int)pos.size(); ++i) mesh->Node(i).r = pos[i];
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh->Element(i);
		el.m_gid = 0;
		el.SetType(conn[i].size() == 6 ? FE_TRI6 : FE_QUAD8);
		for (size_t k = 0; k < conn[i].size(); ++k) el.m_node[k] = conn[i][k];
	}
	mesh->RebuildMesh();
	return mesh;
}
