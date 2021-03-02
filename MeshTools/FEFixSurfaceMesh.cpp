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
#include "FEFixSurfaceMesh.h"
#include "FEFillHole.h"
#include <MeshLib/MeshMetrics.h>
#include <MeshLib/FESurfaceMesh.h>
#include <MeshLib/FENodeFaceList.h>

//-----------------------------------------------------------------------------
FEFixSurfaceMesh::FEFixSurfaceMesh() : FESurfaceModifier("Fix mesh")
{
	AddIntParam(0, "Task:", "Task:")->SetEnumNames("Remove duplicate faces\0Remove non-manifold faces\0Fix winding\0Invert mesh\0Fill all holes\0Removed duplicate edges\0");
}

//-----------------------------------------------------------------------------
FESurfaceMesh* FEFixSurfaceMesh::Apply(FESurfaceMesh* pm)
{
	// create a copy of the mesh
	FESurfaceMesh* pnew = new FESurfaceMesh(*pm);

	// apply the task on this mesh
	int task = GetIntValue(0);
	bool ret = false;
	switch (task)
	{
	case 0: ret = RemoveDuplicateFaces(pnew); break;
	case 1: ret = RemoveNonManifoldFaces(pnew); break;
	case 2: ret = FixElementWinding(pnew); break;
	case 3: ret = InvertMesh(pnew); break;
	case 4: ret = FillAllHoles(pnew); break;
	case 5: ret = RemoveDuplicateEdges(pnew); break;
	}

	if (ret == false)
	{
		delete pnew;
		pnew = 0;
	}

	return pnew;
}

//-----------------------------------------------------------------------------
bool FEFixSurfaceMesh::RemoveDuplicateFaces(FESurfaceMesh* pm)
{
	// clear all tags
	pm->TagAllFaces(0);

	// build the node-face table
	FENodeFaceList NFT;
	NFT.Build(pm);

	// loop over all elements
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		int n = NFT.Valence(i);
		for (int j = 0; j < n - 1; ++j)
		{
			FEFace& fj = *NFT.Face(i, j);
			if (fj.m_ntag == 0)
			{
				for (int k = j + 1; k < n; ++k)
				{
					FEFace& fk = *NFT.Face(i, k);
					if (fj == fk)
					{
						fk.m_ntag = 1;
					}
				}
			}
		}
	}

	// remove tagged faces
	pm->DeleteTaggedFaces(1);

	// rebuild the mesh
	pm->RebuildMesh();

	// done
	return true;
}

//-----------------------------------------------------------------------------
bool FEFixSurfaceMesh::RemoveNonManifoldFaces(FESurfaceMesh* pm)
{
	// clear all tags
	pm->TagAllFaces(0);

	// loop over all face
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FEFace& face = pm->Face(i);
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
void flipTri3(FEFace& f)
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
void flipQuad4(FEFace& f)
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
bool FEFixSurfaceMesh::FixElementWinding(FESurfaceMesh* pm)
{
	// clear tags
	pm->TagAllFaces(0);

	// loop over all triangles
	int NF = pm->Faces();
	for (int i = 0; i < NF; ++i)
	{
		// get an face
		FEFace& f0 = pm->Face(i);

		// proceed if it has not been processed
		if (f0.m_ntag == 0)
		{
			// this element will be the starter element
			// all elements connected to this element will now be wound
			// in the same direction.
			f0.m_ntag = 1;
			stack<FEFace*> S;
			S.push(&f0);
			while (S.empty() == false)
			{
				// pop an face
				FEFace* pf = S.top(); S.pop();

				// loop over the neighbors
				int nn = pf->Nodes();
				for (int j = 0; j < nn; ++j)
				{
					int n0 = pf->n[j];
					int n1 = pf->n[(j + 1) % nn];
					int fj = pf->m_nbr[j];
					if (fj >= 0)
					{
						FEFace* pfj = &pm->Face(fj);
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
bool FEFixSurfaceMesh::InvertMesh(FESurfaceMesh* pm)
{
	// loop over all faces
	int NF = pm->Faces();
	for (int i = 0; i < NF; ++i)
	{
		// get a face
		FEFace& face = pm->Face(i);

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
bool FEFixSurfaceMesh::FillAllHoles(FESurfaceMesh* pm)
{
	// fill all the holes
	FEFillHole fill;
	fill.FillAllHoles(pm);

	// rebuild the mesh
	pm->RebuildMesh();

	// all done
	return true;
}

//-----------------------------------------------------------------------------
bool FEFixSurfaceMesh::RemoveDuplicateEdges(FESurfaceMesh* pm)
{
	pm->RemoveDuplicateEdges();

	// rebuild the mesh
	pm->RebuildMesh();

	//all done
	return true;
}
