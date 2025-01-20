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
#include <MeshLib/FEMesh.h>
#include <MeshLib/FEFaceEdgeList.h>
#include "FEModifier.h"
//using namespace std;

//-----------------------------------------------------------------------------
FETet4ToTet20::FETet4ToTet20() : FEModifier("Tet4-to-Tet20")
{
	m_bsmooth = false;
}

//-----------------------------------------------------------------------------
FSMesh* FETet4ToTet20::Apply(FSMesh* pm)
{
	// before we get started, let's make sure this is a tet4 mesh
	if (pm->IsType(FE_TET4) == false) return 0;

	// build the edge table
	FSEdgeList ET(*pm);

	// build the element-edge table
	FSElementEdgeList EET(*pm, ET);

	// build the face table
	FSFaceTable FT(*pm);

	// build the element face table
	FSElementFaceList EFT(*pm, FT);

	// build the face-edge table
	FSFaceEdgeList FET(*pm, ET);

	// build the face-face table
	FSFaceFaceList FFT(*pm, FT);

	// build the edge-edge table
	FSEdgeIndexList CET(*pm, ET);

	// the new number of nodes is given by the number of nodes, 2*edges, faces
	int NN = pm->Nodes();
	int NC = ET.size();
	int NF = FT.size();
	int nodes = NN + 2*NC + NF;

	// number of elements stays the same
	int elems = pm->Elements();

	// create a new mesh
	FSMesh* pnew = new FSMesh;
	pnew->Create(nodes, elems, pm->Faces(), pm->Edges());

	// copy the old nodes
	for (int i = 0; i<NN; ++i)
	{
		FSNode& n0 = pnew->Node(i);
		FSNode& n1 = pm->Node(i);
		n0.r = n1.r;
		n0.m_gid = n1.m_gid;
	}

	// create the edge nodes
	for (int i = 0; i<NC; i++)
	{
		std::pair<int, int>& edge = ET[i];
		FSNode& n0 = pnew->Node(NN + 2*i);
		FSNode& n1 = pnew->Node(NN + 2*i+1);
		vec3d& ra = pm->Node(edge.first).r;
		vec3d& rb = pm->Node(edge.second).r;
		n0.r = ra + (rb - ra) / 3.0;
		n1.r = ra + (rb - ra)*(2.0 / 3.0);
	}

	// create the face nodes
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = FT[i];
		FSNode& n0 = pnew->Node(i + NN + 2*NC);
		vec3d& ra = pm->Node(face.n[0]).r;
		vec3d& rb = pm->Node(face.n[1]).r;
		vec3d& rc = pm->Node(face.n[2]).r;
		n0.r = (ra + rb + rc) / 3.0;
	}

	// create the new elements
	const int TET[6][2] = {{0,1},{1,2},{0,2},{0,3},{1,3},{2,3}};
	for (int i = 0; i<elems; ++i)
	{
		FSElement& e0 = pm->Element(i);
		FSElement& e1 = pnew->Element(i);
		e1 = e0;

		e1.SetType(FE_TET20);
		e1.m_gid = e0.m_gid;

		e1.m_node[0] = e0.m_node[0];
		e1.m_node[1] = e0.m_node[1];
		e1.m_node[2] = e0.m_node[2];
		e1.m_node[3] = e0.m_node[3];

		for (int j=0; j<6; ++j)
		{
			int ne = EET[i][j];
			if (ET[ne].first == e1.m_node[TET[j][0]])
			{
				e1.m_node[4 + 2*j    ] = 2 * ne + NN;
				e1.m_node[4 + 2*j + 1] = 2 * ne + NN + 1;
			}
			else if (ET[ne].first == e1.m_node[TET[j][1]])
			{
				e1.m_node[4 + 2 * j    ] = 2 * ne + NN + 1;
				e1.m_node[4 + 2 * j + 1] = 2 * ne + NN;
			}
			else assert(false);
		}

		e1.m_node[16] = EFT[i][0] + NN + 2 * NC;
		e1.m_node[17] = EFT[i][1] + NN + 2 * NC;
		e1.m_node[18] = EFT[i][2] + NN + 2 * NC;
		e1.m_node[19] = EFT[i][3] + NN + 2 * NC;
	}

	// create the new faces
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& f0 = pm->Face(i);
		FSFace& f1 = pnew->Face(i);

		f1.SetType(FE_FACE_TRI10);
		f1.m_gid = f0.m_gid;
		f1.m_sid = f0.m_sid;
		f1.n[0] = f0.n[0];
		f1.n[1] = f0.n[1];
		f1.n[2] = f0.n[2];

		const int FNT[3] = {0, 1, 0};
		for (int j=0; j<3; ++j)
		{
			int ne = FET[i][j];
			if (ET[ne].first == f1.n[FNT[j]])
			{
				f1.n[3 + 2*j    ] = 2 * ne + NN;
				f1.n[3 + 2*j + 1] = 2 * ne + NN + 1;
			}
			else if (ET[ne].second == f1.n[FNT[j]])
			{
				f1.n[3 + 2*j    ] = 2 * ne + NN + 1;
				f1.n[3 + 2*j + 1] = 2 * ne + NN;
			}
			else assert(false);
		}

		f1.n[9] = FFT[i] + NN + 2*NC;
		f1.m_elem[0] = f0.m_elem[0];
		f1.m_elem[1] = f0.m_elem[1];
		f1.m_elem[2] = f0.m_elem[2];
		f1.m_nbr[0] = f0.m_nbr[0];
		f1.m_nbr[1] = f0.m_nbr[1];
		f1.m_nbr[2] = f0.m_nbr[2];
	}

	// create the new edges
	for (int i = 0; i<pm->Edges(); ++i)
	{
		FSEdge& e0 = pm->Edge(i);
		FSEdge& e1 = pnew->Edge(i);

		e1.SetType(FE_EDGE4);
		e1.n[0] = e0.n[0];
		e1.n[1] = e0.n[1];
		e1.n[2] = 2*CET[i] + NN;
		e1.n[3] = 2*CET[i] + NN + 1;
		e1.m_nid = e0.m_nid;
		e1.m_gid = e0.m_gid;
		e1.m_nbr[0] = e0.m_nbr[0];
		e1.m_nbr[1] = e0.m_nbr[1];
		e1.m_elem = e0.m_elem;
		e1.SetExterior(e0.IsExterior());
	}

	pnew->UpdateMesh();

	return pnew;
}
