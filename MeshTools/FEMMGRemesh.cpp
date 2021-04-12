/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "FEMMGRemesh.h"
#include <MeshLib/FESurfaceMesh.h>
#ifdef HAS_MMG
#include "mmg/mmg3d/libmmg3d.h"
#include <mmg/mmgs/libmmgs.h>
#endif
#include <MeshLib/FEMeshBuilder.h>

extern int ET_TET[6][2]; // in lut.cpp
extern int ET_TRI[3][2]; // in lut.cpp

//=========================================================================================
FEMMGRemesh::FEMMGRemesh() : FEModifier("MMG Remesh")
{
	AddDoubleParam(0.0, "H", "Element size");

	AddDoubleParam( 0.0, "hmin", "Min element size");
	AddDoubleParam(0.05, "hv", "Global Hausdorff value");
	AddDoubleParam(1.3, "grad", "Gradation");
	AddBoolParam(true, "Only remesh selection");
}

FEMesh* FEMMGRemesh::Apply(FEMesh* pm)
{
	if (pm == nullptr) { SetError("This object has no mesh."); return 0; }
	if (pm->IsType(FE_TET4)) return RemeshTET4(pm);
	else if (pm->IsType(FE_TRI3)) return RemeshTRI3(pm);
	else { SetError("This is not a TET4 mesh"); return 0; }
}

FEMesh* FEMMGRemesh::RemeshTET4(FEMesh* pm)
{
#ifdef HAS_MMG
	int NE = pm->Elements();
	int NN = pm->Nodes();
	int NF = pm->Faces();

	int NC = 0;
	for (int i = 0; i < pm->Edges(); ++i)
	{
		if (pm->Edge(i).m_gid >= 0) NC++;
	}

	// build the MMG mesh
	MMG5_pMesh mmgMesh;
	MMG5_pSol  mmgSol;
	mmgMesh = NULL;
	mmgSol = NULL;

	MMG3D_Init_mesh(MMG5_ARG_start,
		MMG5_ARG_ppMesh, &mmgMesh, MMG5_ARG_ppMet, &mmgSol,
		MMG5_ARG_end);

	// allocate mesh size
	if (MMG3D_Set_meshSize(mmgMesh, NN, NE, 0, NF, 0, NC) != 1)
	{
		assert(false);
		SetError("Error in MMG3D_Set_meshSize");
		return nullptr;
	}

	// build the GID to SID map. We will use this later to reassign the SID's to the faces
	int NS = 0;
	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.m_gid > NS) NS = f.m_gid;
	}
	vector<int> ST(NS+1, 0);
	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = pm->Face(i);
		ST[f.m_gid] = f.m_sid;
	}

	// build the MMG mesh
	for (int i = 0; i < NN; ++i)
	{
		FENode& vi = pm->Node(i);
		vec3d r = vi.pos();
		MMG3D_Set_vertex(mmgMesh, r.x, r.y, r.z, vi.m_gid, i + 1);
	}

	for (int i = 0; i < NE; ++i)
	{
		FEElement& e = pm->Element(i);
		int* n = e.m_node;
		MMG3D_Set_tetrahedron(mmgMesh, n[0] + 1, n[1] + 1, n[2] + 1, n[3] + 1, e.m_gid, i + 1);
	}

	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = pm->Face(i);
		int* n = f.n;
		MMG3D_Set_triangle(mmgMesh, n[0] + 1, n[1] + 1, n[2] + 1, f.m_gid, i + 1);
	}
	for (int i = 0; i < NC; ++i)
	{
		FEEdge& e = pm->Edge(i);
		if (e.m_gid >= 0)
		{
			int* n = e.n;
			MMG3D_Set_edge(mmgMesh, n[0] + 1, n[1] + 1, e.m_gid, i + 1);
		}
	}

	// Now, we build the "solution", i.e. the target element size.
	// If no elements are selected, we set a homogenous remeshing using the element size parameter.
	// set the "solution", i.e. desired element size
	if (MMG3D_Set_solSize(mmgMesh, mmgSol, MMG5_Vertex, NN, MMG5_Scalar) != 1)
	{
		assert(false);
		SetError("Error in MMG3D_Set_solSize");
		return nullptr;
	}

	double h = GetFloatValue(ELEM_SIZE);

	vector<pair<double, int> > edgeLength(NN, pair<double, int>(0.0, 0));
	int nsel = 0;
	for (int i = 0; i < NE; ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.IsSelected()) nsel++;
	}

	if (nsel == 0)
	{
		for (int k = 1; k <= NN; k++) {
			MMG3D_Set_scalarSol(mmgSol, h, k);
		}
	}
	else
	{
		bool remeshSelectionOnly = GetBoolValue(4);
		if (remeshSelectionOnly)
		{
			for (int i = 0; i < NE; ++i)
			{
				FEElement& e = pm->Element(i);
				if (e.IsSelected() == false) MMG3D_Set_requiredTetrahedron(mmgMesh, i + 1);
			}
		}

		// build the edge length table
		for (int i = 0; i < NE; ++i)
		{
			FEElement& el = pm->Element(i);
			for (int j = 0; j < 6; ++j)
			{
				int a = el.m_node[ET_TET[j][0]];
				int b = el.m_node[ET_TET[j][1]];

				vec3d ra = pm->Node(a).pos();
				vec3d rb = pm->Node(b).pos();

				double L2 = (ra - rb).SqrLength();

				edgeLength[a].first += L2; edgeLength[a].second++;
				edgeLength[b].first += L2; edgeLength[b].second++;
			}
		}
		for (int i = 0; i < NN; ++i)
		{
			if (edgeLength[i].second != 0)
			{
				edgeLength[i].first /= (double)edgeLength[i].second;
				edgeLength[i].first = sqrt(edgeLength[i].first);
			}
		}

		for (int i = 0; i < NE; ++i)
		{
			FEElement& el = pm->Element(i);
			if (el.IsSelected())
			{
				for (int j = 0; j < el.Nodes(); ++j)
				{
					edgeLength[el.m_node[j]].first = h;
				}
			}
		}

		for (int k = 0; k < NN; k++) {
			MMG3D_Set_scalarSol(mmgSol, edgeLength[k].first, k + 1);
		}
	}

	// set the control parameters
	MMG3D_Set_dparameter(mmgMesh, mmgSol, MMG3D_DPARAM_hmin, GetFloatValue(HMIN));
	MMG3D_Set_dparameter(mmgMesh, mmgSol, MMG3D_DPARAM_hausd, GetFloatValue(HAUSDORFF));
	MMG3D_Set_dparameter(mmgMesh, mmgSol, MMG3D_DPARAM_hgrad, GetFloatValue(HGRAD));

	// run the mesher
	int ier = MMG3D_mmg3dlib(mmgMesh, mmgSol);

	if (ier == MMG5_STRONGFAILURE) {
		if (h == 0.0) SetError("Element size cannot be zero.");
		else SetError("MMG was not able to remesh the mesh.");
		return nullptr;
	}
	else if (ier == MMG5_LOWFAILURE)
	{
		SetError("MMG return low failure error");
	}

	// convert back to prv mesh
	FEMesh* newMesh = new FEMesh();

	// get the new mesh sizes
	MMG3D_Get_meshSize(mmgMesh, &NN, &NE, NULL, &NF, NULL, &NC);
	newMesh->Create(NN, NE, NF, NC);

	// get the vertex coordinates
	for (int i = 0; i < NN; ++i)
	{
		FENode& vi = newMesh->Node(i);
		vec3d& ri = vi.r;
		int isCorner = 0;
		MMG3D_Get_vertex(mmgMesh, &ri.x, &ri.y, &ri.z, &vi.m_gid, &isCorner, NULL);
		if (isCorner == 0) vi.m_gid = -1;
	}

	// get the tetra
	for (int i = 0; i < NE; ++i)
	{
		FEElement& el = newMesh->Element(i);
		el.SetType(FE_TET4);
		int* n = el.m_node;
		MMG3D_Get_tetrahedron(mmgMesh, n, n + 1, n + 2, n + 3, &el.m_gid, NULL);
		el.m_node[0]--;
		el.m_node[1]--;
		el.m_node[2]--;
		el.m_node[3]--;
	}

	// get the triangles
	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = newMesh->Face(i);
		f.SetType(FE_FACE_TRI3);
		int* n = f.n;
		MMG3D_Get_triangle(mmgMesh, n, n + 1, n + 2, &f.m_gid, NULL);
		f.m_sid = ST[f.m_gid];
		f.n[0]--;
		f.n[1]--;
		f.n[2]--;
	}
	// get the edges
	for (int i = 0; i < NC; ++i)
	{
		FEEdge& e = newMesh->Edge(i);
		e.SetType(FE_EDGE2);
		int* n = e.n;
		int isRidge;
		int ret = MMG3D_Get_edge(mmgMesh, n, n + 1, &e.m_gid, &isRidge, NULL);
		assert(ret != 0);
		e.n[0]--;
		e.n[1]--;
		assert(e.m_gid >= 0);
	}

	newMesh->BuildMesh();

	// NOTE: Not sure why, but it appears the edge data returned by MMG is not always correct
	//       See github issue #26. For now, I'm forcing rebuild of edge data
	FEMeshBuilder meshBuilder(*newMesh);
	meshBuilder.RepairEdges();

	// Clean up
	MMG3D_Free_all(MMG5_ARG_start,
		MMG5_ARG_ppMesh, &mmgMesh, MMG5_ARG_ppMet, &mmgSol,
		MMG5_ARG_end);

	return newMesh;

#else
	SetError("This version does not have MMG support");
	return nullptr;
#endif
}

FEMesh* FEMMGRemesh::RemeshTRI3(FEMesh* pm)
{
	assert(pm->IsType(FE_TRI3));

#ifdef HAS_MMG
	int NE = pm->Elements();
	int NN = pm->Nodes();
	int NF = pm->Faces();

	int NC = 0;
	for (int i = 0; i < pm->Edges(); ++i)
	{
		if (pm->Edge(i).m_gid >= 0) NC++;
	}

	// build the MMG mesh
	MMG5_pMesh mmgMesh;
	MMG5_pSol  mmgSol;
	mmgMesh = NULL;
	mmgSol = NULL;
	MMGS_Init_mesh(MMG5_ARG_start,
		MMG5_ARG_ppMesh, &mmgMesh, 
		MMG5_ARG_ppMet, &mmgSol,
		MMG5_ARG_end);

	// allocate mesh size
	if (MMGS_Set_meshSize(mmgMesh, NN, NE, NC) != 1)
	{
		assert(false);
		SetError("Error in MMGS_Set_meshSize");
		return nullptr;
	}

	// build the GID to SID map. We will use this later to reassign the SID's to the faces
	int NS = 0;
	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.m_gid > NS) NS = f.m_gid;
	}
	vector<int> ST(NS + 1, 0);
	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = pm->Face(i);
		ST[f.m_gid] = f.m_sid;
	}

	// build the MMG mesh
	for (int i = 0; i < NN; ++i)
	{
		FENode& vi = pm->Node(i);
		vec3d r = vi.pos();
		MMGS_Set_vertex(mmgMesh, r.x, r.y, r.z, vi.m_gid, i + 1);
	}

	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = pm->Face(i);
		int* n = f.n;
		MMGS_Set_triangle(mmgMesh, n[0] + 1, n[1] + 1, n[2] + 1, f.m_gid, i + 1);
	}
	for (int i = 0; i < NC; ++i)
	{
		FEEdge& e = pm->Edge(i);
		if (e.m_gid >= 0)
		{
			int* n = e.n;
			MMGS_Set_edge(mmgMesh, n[0] + 1, n[1] + 1, e.m_gid, i + 1);
		}
	}

	// Now, we build the "solution", i.e. the target element size.
	// If no elements are selected, we set a homogenous remeshing using the element size parameter.
	// set the "solution", i.e. desired element size
	if (MMGS_Set_solSize(mmgMesh, mmgSol, MMG5_Vertex, NN, MMG5_Scalar) != 1)
	{
		assert(false);
		SetError("Error in MMG3D_Set_solSize");
		return nullptr;
	}

	double h = GetFloatValue(ELEM_SIZE);

	vector<pair<double, int> > edgeLength(NN, pair<double, int>(0.0, 0));
	int nsel = 0;
	for (int i = 0; i < NE; ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.IsSelected()) nsel++;
	}

	if (nsel == 0)
	{
		for (int k = 1; k <= NN; k++) {
			MMGS_Set_scalarSol(mmgSol, h, k);
		}
	}
	else
	{
		bool remeshSelectionOnly = GetBoolValue(4);
		if (remeshSelectionOnly)
		{
			for (int i = 0; i < NE; ++i)
			{
				FEElement& e = pm->Element(i);
				if (e.IsSelected() == false) MMGS_Set_requiredTriangle(mmgMesh, i + 1);
			}
		}

		// build the edge length table
		for (int i = 0; i < NE; ++i)
		{
			FEElement& el = pm->Element(i);
			for (int j = 0; j < 3; ++j)
			{
				int a = el.m_node[ET_TRI[j][0]];
				int b = el.m_node[ET_TRI[j][1]];

				vec3d ra = pm->Node(a).pos();
				vec3d rb = pm->Node(b).pos();

				double L2 = (ra - rb).SqrLength();

				edgeLength[a].first += L2; edgeLength[a].second++;
				edgeLength[b].first += L2; edgeLength[b].second++;
			}
		}
		for (int i = 0; i < NN; ++i)
		{
			if (edgeLength[i].second != 0)
			{
				edgeLength[i].first /= (double)edgeLength[i].second;
				edgeLength[i].first = sqrt(edgeLength[i].first);
			}
		}

		for (int i = 0; i < NE; ++i)
		{
			FEElement& el = pm->Element(i);
			if (el.IsSelected())
			{
				for (int j = 0; j < el.Nodes(); ++j)
				{
					edgeLength[el.m_node[j]].first = h;
				}
			}
		}

		for (int k = 0; k < NN; k++) {
			MMGS_Set_scalarSol(mmgSol, edgeLength[k].first, k + 1);
		}
	}

	// set the control parameters
	MMGS_Set_dparameter(mmgMesh, mmgSol, MMGS_DPARAM_hmin, GetFloatValue(HMIN));
	MMGS_Set_dparameter(mmgMesh, mmgSol, MMGS_DPARAM_hausd, GetFloatValue(HAUSDORFF));
	MMGS_Set_dparameter(mmgMesh, mmgSol, MMGS_DPARAM_hgrad, GetFloatValue(HGRAD));

	// run the mesher
	int ier = MMGS_mmgslib(mmgMesh, mmgSol);

	if (ier == MMG5_STRONGFAILURE) {
		if (h == 0.0) SetError("Element size cannot be zero.");
		else SetError("MMG was not able to remesh the mesh.");
		return nullptr;
	}
	else if (ier == MMG5_LOWFAILURE)
	{
		SetError("MMG return low failure error");
	}

	// convert back to prv mesh
	FEMesh* newMesh = new FEMesh();

	// get the new mesh sizes
	MMGS_Get_meshSize(mmgMesh, &NN, &NF, &NE);
	newMesh->Create(NN, NF, NF, NC);

	// get the vertex coordinates
	for (int i = 0; i < NN; ++i)
	{
		FENode& vi = newMesh->Node(i);
		vec3d& ri = vi.r;
		int isCorner = 0;
		MMGS_Get_vertex(mmgMesh, &ri.x, &ri.y, &ri.z, &vi.m_gid, &isCorner, NULL);
		if (isCorner == 0) vi.m_gid = -1;
	}

	// get the triangles
	for (int i = 0; i < NF; ++i)
	{
		FEElement& el = newMesh->Element(i);
		el.SetType(FE_TRI3);
		int* n = el.m_node;
		MMGS_Get_triangle(mmgMesh, n, n + 1, n + 2, &el.m_gid, NULL);
		el.m_node[0]--;
		el.m_node[1]--;
		el.m_node[2]--;

		FEFace& f = newMesh->Face(i);
		f.SetType(FE_FACE_TRI3);
		f.m_gid = el.m_gid;
		f.n[0] = el.m_node[0];
		f.n[1] = el.m_node[1];
		f.n[2] = el.m_node[2];
//		f.m_sid = ST[f.m_gid];
	}
	// get the edges
	for (int i = 0; i < NC; ++i)
	{
		FEEdge& e = newMesh->Edge(i);
		e.SetType(FE_EDGE2);
		int* n = e.n;
		int isRidge;
		int ret = MMG3D_Get_edge(mmgMesh, n, n + 1, &e.m_gid, &isRidge, NULL);
		assert(ret != 0);
		e.n[0]--;
		e.n[1]--;
		assert(e.m_gid >= 0);
	}

	newMesh->BuildMesh();

	// NOTE: Not sure why, but it appears the edge data returned by MMG is not always correct
	//       See github issue #26. For now, I'm forcing rebuild of edge data
	FEMeshBuilder meshBuilder(*newMesh);
	meshBuilder.RepairEdges();

	// Clean up
	MMGS_Free_all(MMG5_ARG_start,
		MMG5_ARG_ppMesh, &mmgMesh, MMG5_ARG_ppMet, &mmgSol,
		MMG5_ARG_end);

	return newMesh;

#else
	SetError("This version does not have MMG support");
	return nullptr;
#endif
}

//================================================================================================
FEMMGSurfaceRemesh::FEMMGSurfaceRemesh() : FESurfaceModifier("MMG Remesh")
{
	AddDoubleParam(0.0, "H", "Element size");

	AddDoubleParam(0.0, "hmin", "Min element size");
	AddDoubleParam(0.05, "hv", "Global Hausdorff value");
	AddDoubleParam(1.3, "grad", "Gradation");
	AddBoolParam(true, "Only remesh selection");
}

FESurfaceMesh* FEMMGSurfaceRemesh::Apply(FESurfaceMesh* pm)
{
	if (pm == nullptr) { SetError("This object has no mesh."); return 0; }
	assert(pm->IsType(FE_FACE_TRI3));

#ifdef HAS_MMG
	int NN = pm->Nodes();
	int NF = pm->Faces();

	int NC = 0;
	for (int i = 0; i < pm->Edges(); ++i)
	{
		if (pm->Edge(i).m_gid >= 0) NC++;
	}

	// build the MMG mesh
	MMG5_pMesh mmgMesh;
	MMG5_pSol  mmgSol;
	mmgMesh = NULL;
	mmgSol = NULL;
	MMGS_Init_mesh(MMG5_ARG_start,
		MMG5_ARG_ppMesh, &mmgMesh,
		MMG5_ARG_ppMet, &mmgSol,
		MMG5_ARG_end);

	// allocate mesh size
	if (MMGS_Set_meshSize(mmgMesh, NN, NF, 0) != 1)
	{
		assert(false);
		SetError("Error in MMGS_Set_meshSize");
		return nullptr;
	}

	// build the GID to SID map. We will use this later to reassign the SID's to the faces
	int NS = 0;
	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.m_gid > NS) NS = f.m_gid;
	}
	vector<int> ST(NS + 1, 0);
	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = pm->Face(i);
		ST[f.m_gid] = f.m_sid;
	}

	// build the MMG mesh
	for (int i = 0; i < NN; ++i)
	{
		FENode& vi = pm->Node(i);
		vec3d r = vi.pos();
		MMGS_Set_vertex(mmgMesh, r.x, r.y, r.z, vi.m_gid, i + 1);
	}

	for (int i = 0; i < NF; ++i)
	{
		FEFace& f = pm->Face(i);
		int* n = f.n;
		MMGS_Set_triangle(mmgMesh, n[0] + 1, n[1] + 1, n[2] + 1, f.m_gid, i + 1);
	}
/*	for (int i = 0; i < NC; ++i)
	{
		FEEdge& e = pm->Edge(i);
		if (e.m_gid >= 0)
		{
			int* n = e.n;
			MMGS_Set_edge(mmgMesh, n[0] + 1, n[1] + 1, e.m_gid, i + 1);
		}
	}
*/
	// Now, we build the "solution", i.e. the target element size.
	// If no elements are selected, we set a homogenous remeshing using the element size parameter.
	// set the "solution", i.e. desired element size
	if (MMGS_Set_solSize(mmgMesh, mmgSol, MMG5_Vertex, NN, MMG5_Scalar) != 1)
	{
		assert(false);
		SetError("Error in MMG3D_Set_solSize");
		return nullptr;
	}

	double h = GetFloatValue(ELEM_SIZE);

	vector<pair<double, int> > edgeLength(NN, pair<double, int>(0.0, 0));
	int nsel = 0;
	for (int i = 0; i < NF; ++i)
	{
		FEFace& face = pm->Face(i);
		if (face.IsSelected()) nsel++;
	}

	if (nsel == 0)
	{
		for (int k = 1; k <= NN; k++) {
			MMGS_Set_scalarSol(mmgSol, h, k);
		}
	}
	else
	{
		bool remeshSelectionOnly = GetBoolValue(4);
		if (remeshSelectionOnly)
		{
			for (int i = 0; i < NF; ++i)
			{
				FEFace& face = pm->Face(i);
				if (face.IsSelected() == false) MMGS_Set_requiredTriangle(mmgMesh, i + 1);
			}
		}

		// build the edge length table
		for (int i = 0; i < NF; ++i)
		{
			FEFace& face = pm->Face(i);
			for (int j = 0; j < 3; ++j)
			{
				int a = face.n[ET_TRI[j][0]];
				int b = face.n[ET_TRI[j][1]];

				vec3d ra = pm->Node(a).pos();
				vec3d rb = pm->Node(b).pos();

				double L2 = (ra - rb).SqrLength();

				edgeLength[a].first += L2; edgeLength[a].second++;
				edgeLength[b].first += L2; edgeLength[b].second++;
			}
		}
		for (int i = 0; i < NN; ++i)
		{
			if (edgeLength[i].second != 0)
			{
				edgeLength[i].first /= (double)edgeLength[i].second;
				edgeLength[i].first = sqrt(edgeLength[i].first);
			}
		}

		for (int i = 0; i < NF; ++i)
		{
			FEFace& face = pm->Face(i);
			if (face.IsSelected())
			{
				for (int j = 0; j < face.Nodes(); ++j)
				{
					edgeLength[face.n[j]].first = h;
				}
			}
		}

		for (int k = 0; k < NN; k++) {
			MMGS_Set_scalarSol(mmgSol, edgeLength[k].first, k + 1);
		}
	}

	// set the control parameters
	MMGS_Set_dparameter(mmgMesh, mmgSol, MMGS_DPARAM_hmin, GetFloatValue(HMIN));
	MMGS_Set_dparameter(mmgMesh, mmgSol, MMGS_DPARAM_hausd, GetFloatValue(HAUSDORFF));
	MMGS_Set_dparameter(mmgMesh, mmgSol, MMGS_DPARAM_hgrad, GetFloatValue(HGRAD));

	// run the mesher
	int ier = MMGS_mmgslib(mmgMesh, mmgSol);

	if (ier == MMG5_STRONGFAILURE) {
		if (h == 0.0) SetError("Element size cannot be zero.");
		else SetError("MMG was not able to remesh the mesh.");
		return nullptr;
	}
	else if (ier == MMG5_LOWFAILURE)
	{
		SetError("MMG return low failure error");
	}

	// convert back to prv mesh
	FESurfaceMesh* newMesh = new FESurfaceMesh();

	// get the new mesh sizes
	MMGS_Get_meshSize(mmgMesh, &NN, &NF, &NC);
	newMesh->Create(NN, NC, NF);

	// get the vertex coordinates
	for (int i = 0; i < NN; ++i)
	{
		FENode& vi = newMesh->Node(i);
		vec3d& ri = vi.r;
		int isCorner = 0;
		MMGS_Get_vertex(mmgMesh, &ri.x, &ri.y, &ri.z, &vi.m_gid, &isCorner, NULL);
		if (isCorner == 0) vi.m_gid = -1;
	}

	// get the triangles
	for (int i = 0; i < NF; ++i)
	{
		FEFace& face = newMesh->Face(i);
		face.SetType(FE_FACE_TRI3);
		int* n = face.n;
		MMGS_Get_triangle(mmgMesh, n, n + 1, n + 2, &face.m_gid, NULL);
		face.n[0]--;
		face.n[1]--;
		face.n[2]--;
		if (face.m_gid <= NS) face.m_sid = ST[face.m_gid];
	}
	// get the edges
	for (int i = 0; i < NC; ++i)
	{
		FEEdge& e = newMesh->Edge(i);
		e.SetType(FE_EDGE2);
		int* n = e.n;
		int isRidge;
		int ret = MMG3D_Get_edge(mmgMesh, n, n + 1, &e.m_gid, &isRidge, NULL);
		assert(ret != 0);
		e.n[0]--;
		e.n[1]--;
		assert(e.m_gid >= 0);
	}

	newMesh->BuildMesh();
//	newMesh->Update();

	// Clean up
	MMGS_Free_all(MMG5_ARG_start,
		MMG5_ARG_ppMesh, &mmgMesh, MMG5_ARG_ppMet, &mmgSol,
		MMG5_ARG_end);

	return newMesh;

#else
	SetError("This version does not have MMG support");
	return nullptr;
#endif
}
