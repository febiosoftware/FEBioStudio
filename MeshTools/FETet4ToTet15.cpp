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
#include "FEModifier.h"
#include <MeshLib/FEFaceEdgeList.h>
//using namespace std;

//-----------------------------------------------------------------------------
FETet4ToTet15::FETet4ToTet15(bool bsmooth) : FEModifier("Tet4-to-Tet15")
{
	m_bsmooth = bsmooth;
}

//-----------------------------------------------------------------------------
FSMesh* FETet4ToTet15::Apply(FSMesh* pm)
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

	// the new number of nodes is given by the number of nodes, edges, faces and elements
	int NN = pm->Nodes();
	int NC = ET.size();
	int NF = FT.size();
	int NT = pm->Elements();
	int nodes = NN + NC + NF + NT;

	// number of elements stays the same
	int elems = NT;

	// create a new mesh
	FSMesh* pnew = new FSMesh;
	pnew->Create(nodes, elems, pm->Faces(), pm->Edges());

	// copy the old nodes
	for (int i=0; i<NN; ++i)
	{
		FSNode& n0 = pnew->Node(i);
		FSNode& n1 = pm->Node(i);
		n0.r = n1.r;
		n0.m_gid = n1.m_gid;
	}

	// create the edge nodes
	for (int i=0; i<NC; ++i)
	{
		std::pair<int,int>& edge = ET[i];
		FSNode& n0 = pnew->Node(i + NN);
		vec3d& ra = pm->Node(edge.first ).r;
		vec3d& rb = pm->Node(edge.second).r;
		n0.r = (ra + rb)*0.5;
	}

	// create the face nodes
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = FT[i];
		FSNode& n0 = pnew->Node(i + NN + NC);
		vec3d& ra = pm->Node(face.n[0]).r;
		vec3d& rb = pm->Node(face.n[1]).r;
		vec3d& rc = pm->Node(face.n[2]).r;
		n0.r = (ra + rb + rc)/3.0;
	}

	// create the element nodes
	for (int i=0; i<NT; ++i)
	{
		FSElement& el = pm->Element(i);
		FSNode& n0 = pnew->Node(i + NN + NC + NF);
		vec3d& ra = pm->Node(el.m_node[0]).r;
		vec3d& rb = pm->Node(el.m_node[1]).r;
		vec3d& rc = pm->Node(el.m_node[2]).r;
		vec3d& rd = pm->Node(el.m_node[3]).r;
		n0.r = (ra + rb + rc + rd)*0.25;
	}

	// create the new elements
	for (int i=0; i<NT; ++i)
	{
		FSElement& e0 = pm->Element(i);
		FSElement& e1 = pnew->Element(i);
		e1 = e0;

		e1.SetType(FE_TET15);
		e1.m_gid = e0.m_gid;

		e1.m_node[0] = e0.m_node[0];
		e1.m_node[1] = e0.m_node[1];
		e1.m_node[2] = e0.m_node[2];
		e1.m_node[3] = e0.m_node[3];

		e1.m_node[4] = EET[i][0] + NN;
		e1.m_node[5] = EET[i][1] + NN;
		e1.m_node[6] = EET[i][2] + NN;
		e1.m_node[7] = EET[i][3] + NN;
		e1.m_node[8] = EET[i][4] + NN;
		e1.m_node[9] = EET[i][5] + NN;

		e1.m_node[10] = EFT[i][3] + NN + NC;
		e1.m_node[11] = EFT[i][0] + NN + NC;
		e1.m_node[12] = EFT[i][1] + NN + NC;
		e1.m_node[13] = EFT[i][2] + NN + NC;

		e1.m_node[14] = i + NN + NC + NF;
	}

	// create the new faces
	for (int i=0; i<pm->Faces(); ++i)
	{
		FSFace& f0 = pm->Face(i);
		FSFace& f1 = pnew->Face(i);

		f1.SetType(FE_FACE_TRI7);
		f1.m_gid = f0.m_gid;
		f1.m_sid = f0.m_sid;
		f1.n[0] = f0.n[0];
		f1.n[1] = f0.n[1];
		f1.n[2] = f0.n[2];
		f1.n[3] = FET[i][0] + NN;
		f1.n[4] = FET[i][1] + NN;
		f1.n[5] = FET[i][2] + NN;
		f1.n[6] = FFT[i] + NN + NC;
		f1.m_elem[0] = f0.m_elem[0];
		f1.m_elem[1] = f0.m_elem[1];
		f1.m_elem[2] = f0.m_elem[2];
		f1.m_nbr[0] = f0.m_nbr[0];
		f1.m_nbr[1] = f0.m_nbr[1];
		f1.m_nbr[2] = f0.m_nbr[2];
	}

	// create the new edges
	for (int i=0; i<pm->Edges(); ++i)
	{
		FSEdge& e0 = pm->Edge(i);
		FSEdge& e1 = pnew->Edge(i);

		e1.SetType(FE_EDGE3);
		e1.n[0] = e0.n[0];
		e1.n[1] = e0.n[1];
		e1.n[2] = CET[i] + NN;
		e1.m_nid = e0.m_nid;
		e1.m_gid = e0.m_gid;
		e1.m_nbr[0] = e0.m_nbr[0];
		e1.m_nbr[1] = e0.m_nbr[1];
		e1.m_elem = e0.m_elem;
		e1.SetExterior(e0.IsExterior());
	}

	// apply surface smoothing
	if (m_bsmooth)
	{
		FETet10Smooth mod;
		mod.Apply(pnew);
	}

	pnew->UpdateMesh();

	return pnew;
}
