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
#include "FEFixSurfaceMesh.h"
#include "FEFillHole.h"
#include "FEFillQuadHole.h"
#include <MeshLib/MeshMetrics.h>
#include <MeshLib/FSSurfaceMesh.h>
#include <MeshLib/FSNodeFaceList.h>

//-----------------------------------------------------------------------------
FEFixSurfaceMesh::FEFixSurfaceMesh() : FESurfaceModifier("Fix mesh")
{
	m_mod = nullptr;
	AddIntParam(0, "Task:", "Task:")->SetEnumNames("Remove duplicate faces\0Remove non-manifold faces\0Fix winding\0Invert mesh\0Fill all holes\0Removed duplicate edges\0");
}

//-----------------------------------------------------------------------------
FSTaskProgress FEFixSurfaceMesh::GetProgress()
{
	if (m_mod) return m_mod->GetProgress();
	else return FESurfaceModifier::GetProgress();
}

//-----------------------------------------------------------------------------
FSSurfaceMesh* FEFixSurfaceMesh::Apply(FSSurfaceMesh* pm)
{
	m_mod = nullptr;
	ClearError();

	// create a copy of the mesh
	FSSurfaceMesh* pnew = new FSSurfaceMesh(*pm);

	// apply the task on this mesh
	int task = GetIntValue(0);
	bool ret = false;
	try {
		switch (task)
		{
		case 0: ret = RemoveDuplicateFaces(pnew); break;
		case 1: ret = RemoveNonManifoldFaces(pnew); break;
		case 2: ret = FixElementWinding(pnew); break;
		case 3: ret = InvertMesh(pnew); break;
		case 4: ret = FillAllHoles(pnew); break;
		case 5: ret = RemoveDuplicateEdges(pnew); break;
		}
	}
	catch (...)
	{
		SetError("*** The operation aborted due to an exception. ***");
		ret = false;
	}

	if (ret == false)
	{
		delete pnew;
		pnew = 0;
	}

	return pnew;
}

//-----------------------------------------------------------------------------
bool FEFixSurfaceMesh::RemoveDuplicateFaces(FSSurfaceMesh* pm)
{
	// clear all tags
	pm->TagAllFaces(0);

	// build the node-face table
	FSNodeFaceList NFT;
	NFT.Build(pm);

	// loop over all elements
	setProgress(0.0);
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		int n = NFT.Valence(i);
		for (int j = 0; j < n - 1; ++j)
		{
			FSFace& fj = *NFT.Face(i, j);
			if (fj.m_ntag == 0)
			{
				for (int k = j + 1; k < n; ++k)
				{
					FSFace& fk = *NFT.Face(i, k);
					if (fj == fk)
					{
						fk.m_ntag = 1;
					}
				}
			}
		}
		setProgress(100.0*(i + 1.0) / pm->Nodes());
	}
	int taggedFaces = 0;
	for (int i = 0; i < pm->Faces(); ++i)
	{
		if (pm->Face(i).m_ntag == 1) taggedFaces++;
	}
	SetError("Found %d duplicate faces", taggedFaces);

	// remove tagged faces
	pm->DeleteTaggedFaces(1);

	// rebuild the mesh
	pm->RebuildMesh();

	// done
	return true;
}

//-----------------------------------------------------------------------------
bool FEFixSurfaceMesh::RemoveNonManifoldFaces(FSSurfaceMesh* pm)
{
	// clear all tags
	pm->TagAllFaces(0);

	// loop over all face
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		int n = face.Edges();
		for (int j = 0; j < n; ++j) if (face.m_nbr[j] == -1) { face.m_ntag = 1; break; }
	}

	// remove tagged faces
	pm->DeleteTaggedFaces(1);

	// rebuild the mesh
	pm->RebuildMesh();

	// done
	return true;
}

//-----------------------------------------------------------------------------
void flipTri3(FSFace& f)
{
	// flip nodes 1 and 2
	int ntmp = f.n[1];
	f.n[1] = f.n[2];
	f.n[2] = ntmp;

	// flip neighbors
	ntmp = f.m_nbr[0];
	f.m_nbr[0] = f.m_nbr[2];
	f.m_nbr[2] = ntmp;

	// flip edges
	ntmp = f.m_edge[0];
	f.m_edge[0] = f.m_edge[3];
	f.m_edge[2] = ntmp;

	// flip normals
	f.m_fn = -f.m_fn;
	f.m_nn[0] = -f.m_nn[0];
	f.m_nn[1] = -f.m_nn[1];
	f.m_nn[2] = -f.m_nn[2];
}

//-----------------------------------------------------------------------------
void flipQuad4(FSFace& f)
{
	// flip nodes 1 and 3
	int ntmp = f.n[1];
	f.n[1] = f.n[3];
	f.n[3] = ntmp;

	// flip neighbors
	ntmp = f.m_nbr[0];
	f.m_nbr[0] = f.m_nbr[3];
	f.m_nbr[3] = ntmp;

	ntmp = f.m_nbr[1];
	f.m_nbr[1] = f.m_nbr[2];
	f.m_nbr[2] = ntmp;

	// flip edges
	ntmp = f.m_edge[0];
	f.m_edge[0] = f.m_edge[3];
	f.m_edge[3] = ntmp;

	ntmp = f.m_edge[1];
	f.m_edge[1] = f.m_edge[2];
	f.m_edge[2] = ntmp;

	// flip normals
	f.m_fn = -f.m_fn;
	f.m_nn[0] = -f.m_nn[0];
	f.m_nn[1] = -f.m_nn[1];
	f.m_nn[2] = -f.m_nn[2];
	f.m_nn[3] = -f.m_nn[3];
}

//-----------------------------------------------------------------------------
bool FEFixSurfaceMesh::FixElementWinding(FSSurfaceMesh* pm)
{
	// clear tags
	pm->TagAllFaces(0);

	// loop over all triangles
	int NF = pm->Faces();
	for (int i = 0; i < NF; ++i)
	{
		// get an face
		FSFace& f0 = pm->Face(i);

		// proceed if it has not been processed
		if (f0.m_ntag == 0)
		{
			// this element will be the starter element
			// all elements connected to this element will now be wound
			// in the same direction.
			f0.m_ntag = 1;
			std::stack<FSFace*> S;
			S.push(&f0);
			while (S.empty() == false)
			{
				// pop an face
				FSFace* pf = S.top(); S.pop();

				// loop over the neighbors
				int nn = pf->Nodes();
				for (int j = 0; j < nn; ++j)
				{
					int n0 = pf->n[j];
					int n1 = pf->n[(j + 1) % nn];
					int fj = pf->m_nbr[j];
					if (fj >= 0)
					{
						FSFace* pfj = &pm->Face(fj);
						if (pfj->m_ntag == 0)
						{
							int nnj = pfj->Nodes();
							// find the shared edge
							for (int k = 0; k < nnj; ++k)
							{
								int k0 = pfj->n[k];
								int k1 = pfj->n[(k + 1) % nnj];

								if ((k0 == n0) && (k1 == n1))
								{
									// winding is wrong, so flip the element
									// flip nodes
									if (nnj == 3) flipTri3(*pfj);
									else if (nnj == 4) flipQuad4(*pfj);
									else
									{
										assert(false);
										return false;
									}

									pfj->m_ntag = 1;
									S.push(pfj);
									break;
								}
								else if ((k0 == n1) && (k1 == n0))
								{
									// winding is correct, just push it
									pfj->m_ntag = 1;
									S.push(pfj);
									break;
								}
							}
							assert(pfj->m_ntag == 1);
						}
					}
				}
			}
		}
	}

	// done
	return true;
}

//-----------------------------------------------------------------------------
bool FEFixSurfaceMesh::InvertMesh(FSSurfaceMesh* pm)
{
	// loop over all faces
	int NF = pm->Faces();
	for (int i = 0; i < NF; ++i)
	{
		// get a face
		FSFace& face = pm->Face(i);

		int nn = face.Nodes();
		if (nn == 3) flipTri3(face);
		else if (nn == 4) flipQuad4(face);
		else {
			assert(false);
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEFixSurfaceMesh::FillAllHoles(FSSurfaceMesh* pm)
{
	if (pm->IsType(FE_FACE_TRI3))
	{
		// fill all the holes
		FEFillHole fill;
		m_mod = &fill;
		fill.FillAllHoles(pm);

		// copy the error string
		std::string err = fill.GetErrorString();
		if (err.empty() == false) SetError(err.c_str());
		m_mod = nullptr;
	}
	else if (pm->IsType(FE_FACE_QUAD4))
	{
		FEFillQuadHole fill;
		m_mod = &fill;
		fill.FillAllHoles(pm);

		// copy the error string
		std::string err = fill.GetErrorString();
		if (err.empty() == false) SetError(err.c_str());
		m_mod = nullptr;
	}
	else
	{
		SetError("Can only fill holes of tri and quad meshes.");
		return false;
	}

	// rebuild the mesh
	pm->RebuildMesh();

	// all done
	return true;
}

//-----------------------------------------------------------------------------
bool FEFixSurfaceMesh::RemoveDuplicateEdges(FSSurfaceMesh* pm)
{
	pm->RemoveDuplicateEdges();

	// rebuild the mesh
	pm->RebuildMesh();

	//all done
	return true;
}
