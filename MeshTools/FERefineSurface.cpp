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
#include "FERefineSurface.h"
#include <MeshLib/FESurfaceMesh.h>
using namespace std;

FERefineSurface::FERefineSurface() : FESurfaceModifier("Refine Mesh")
{
	AddIntParam(1, "iterations", "iterations");
}

void FERefineSurface::SetIterations(int n)
{
	SetIntValue(0, n);
}

FSSurfaceMesh* FERefineSurface::Apply(FSSurfaceMesh* pm)
{
	int niter = GetIntValue(0);

	if (niter <= 0)
	{
		FESurfaceModifier::SetError("Invalid number of iterations");
		return 0;
	}

	if (pm->IsType(FE_FACE_TRI3) == false)
	{
		FESurfaceModifier::SetError("This only works on triangle meshes");
		return 0;
	}

	FSSurfaceMesh* newMesh = 0;
	setProgress(0.0);
	for (int i = 0; i<niter; ++i)
	{
		newMesh = Split(pm);

		if (i < niter - 1)
		{
			if (i != 0) delete pm;
			pm = newMesh;
		}

		setProgress(100.0*(i + 1.0) / niter);
	}
	return newMesh;
}

FSSurfaceMesh* FERefineSurface::Split(FSSurfaceMesh* pm)
{
	int NN0 = pm->Nodes();
	int NF0 = pm->Faces();

	// tag all faces
	for (int i=0; i<NF0; ++i) 
	{
		FSFace* face = &pm->Face(i);
		face->m_ntag = i;
	}

	// count edges
	int nen = 0;
	for (int i=0; i<NF0; ++i)
	{
		FSFace& face = pm->Face(i);
		for (int j=0; j<3; ++j)
		{
			int nj = face.m_nbr[j];
			FSFace* pj = (nj != -1 ? &pm->Face(nj) : 0);
			if ((pj == 0) || (face.m_ntag < pj->m_ntag)) nen++;
		}
	}

	// create the new mesh
	int NN1 = NN0 + nen;
	int NF1 = 4*NF0;
	FSSurfaceMesh* pnew = new FSSurfaceMesh;
	pnew->Create(NN1, 0, NF1);

	// copy old nodes
	for (int i=0; i<NN0; ++i) pnew->Node(i) = pm->Node(i);

	// add the new edge nodes
	vector<int> ELT(3*NF0, -1);
	nen = 0;
	for (int i=0; i<NF0; ++i)
	{
		FSFace& face = pm->Face(i);
		for (int j=0; j<3; ++j)
		{
			int nj = face.m_nbr[j];
			FSFace* pj = (nj != -1 ? &pm->Face(nj) : 0);
			if ((pj == 0) || (face.m_ntag < pj->m_ntag))
			{
				FSNode& n0 = pm->Node(face.n[j      ]);
				FSNode& n1 = pm->Node(face.n[(j+1)%3]);
				vec3d r = (n0.r + n1.r)*0.5;

				pnew->Node(NN0 + nen).r = r;

				ELT[3*i + j] = NN0 + nen;

				nen++;
			}
		}
	}

	for (int i = 0; i<NF0; ++i)
	{
		FSFace& face = pm->Face(i);
		for (int j = 0; j<3; ++j)
		{
			int nj = face.m_nbr[j];
			FSFace* pj = (nj != -1 ? &pm->Face(nj) : 0);
			if (pj && (face.m_ntag > pj->m_ntag))
			{
				int nk = pj->FindEdge(face.GetEdge(j)); assert(nk != -1);

				ELT[3*i + j] = ELT[3*nj + nk];
			}
		}
	}

	// split the faces
	int nf = 0;
	for (int i=0; i<NF0; ++i)
	{
		FSFace& face = pm->Face(i);

		FSFace& f0 = pnew->Face(nf++); f0 = face;
		FSFace& f1 = pnew->Face(nf++); f1 = face;
		FSFace& f2 = pnew->Face(nf++); f2 = face;
		FSFace& f3 = pnew->Face(nf++); f3 = face;

		int* en = &ELT[3*i];

		f0.n[0] = face.n[0]; f1.n[0] = en[0]    ; f2.n[0] = en[2]    ; f3.n[0] = en[0];
		f0.n[1] = en[0]    ; f1.n[1] = face.n[1]; f2.n[1] = en[1]    ; f3.n[1] = en[1];
		f0.n[2] = en[2]    ; f1.n[2] = en[1]    ; f2.n[2] = face.n[2]; f3.n[2] = en[2];
	}

	// update the new mesh
	pnew->RebuildMesh();

	// All done
	return pnew;
}
