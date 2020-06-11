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
#ifdef HAS_MMG
#include "mmg/mmg3d/libmmg3d.h"
#endif

extern int ET_TET[6][2]; // in lut.cpp

FEMMGRemesh::FEMMGRemesh() : FEModifier("Tet Refine")
{
	AddDoubleParam(0.0, "H", "Element size");

	AddDoubleParam( 0.0, "hmin", "Min element size");
	AddDoubleParam(0.05, "hv", "Global Hausdorff value");
	AddDoubleParam(1.3, "grad", "Gradation");
}

FEMesh* FEMMGRemesh::Apply(FEMesh* pm)
{
#ifdef HAS_MMG
	if (pm == nullptr) { SetError("This object has no mesh."); return 0; }
	if (pm->IsType(FE_TET4) == false) { SetError("This is not a TET4 mesh"); return 0; }

	int NE = pm->Elements();
	int NN = pm->Nodes();
	int NF = pm->Faces();

	// build the MMG mesh
	MMG5_pMesh mmgMesh;
	MMG5_pSol  mmgSol;
	mmgMesh = NULL;
	mmgSol = NULL;

	MMG3D_Init_mesh(MMG5_ARG_start,
		MMG5_ARG_ppMesh, &mmgMesh, MMG5_ARG_ppMet, &mmgSol,
		MMG5_ARG_end);

	// allocate mesh size
	if (MMG3D_Set_meshSize(mmgMesh, NN, NE, 0, NF, 0, 0) != 1)
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
		MMG3D_Set_vertex(mmgMesh, r.x, r.y, r.z, 0, i + 1);
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
	MMG3D_Get_meshSize(mmgMesh, &NN, &NE, NULL, &NF, NULL, NULL);
	newMesh->Create(NN, NE, NF);

	// get the vertex coordinates
	for (int i = 0; i < NN; ++i)
	{
		FENode& vi = newMesh->Node(i);
		vec3d& ri = vi.r;
		MMG3D_Get_vertex(mmgMesh, &ri.x, &ri.y, &ri.z, NULL, NULL, NULL);
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
	newMesh->BuildMesh();

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
