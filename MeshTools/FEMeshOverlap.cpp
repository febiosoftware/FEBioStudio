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
#include "FEMeshOverlap.h"
#include <MeshLib/FSMesh.h>
#include <PostLib/tools.h>
#include <GeomLib/GObject.h>
using namespace MeshTools;
using namespace std;

std::vector<int> MeshTools::FindSurfaceOverlap(FSMesh* mesh, FSMeshBase* trg)
{
	// loop over all of the surface nodes
	mesh->TagAllNodes(0);
	int NN = mesh->Nodes();
	int NF = mesh->Faces();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh->Face(i);
		int nf = f.Nodes();
		for (int j = 0; j < nf; ++j)
		{
			mesh->Node(f.n[j]).m_ntag = 1;
		}
	}

	vector<vec3d> normalList = mesh->NodeNormals();

	for (int i = 0; i < mesh->Nodes(); ++i)
	{
		FSNode& node = mesh->Node(i);
		if (node.m_ntag == 1)
		{
			// convert between coordinate systems
			vec3d r_global = mesh->LocalToGlobal(node.r);
			vec3d r = trg->GlobalToLocal(r_global);
			vec3f rf = to_vec3f(r);

			// get the normal at this node
			vec3d N = normalList[i];
			N.Normalize();
			N = mesh->GetGObject()->GetTransform().LocalToGlobalNormal(N);
			N = trg->GetGObject()->GetTransform().GlobalToLocalNormal(N);
			vec3f Nf = to_vec3f(N);

			// find the normal projection onto the target surface
			bool bfound = false;
			float Dmin = 0.f;
			bool backFacing = false;
			vec3f y[FSFace::MAX_NODES], q;
			for (int n = 0; n < trg->Faces(); ++n)
			{
				FSFace& ft = trg->Face(n);

				for (int m = 0; m < ft.Nodes(); ++m) y[m] = to_vec3f(trg->Node(ft.n[m]).r);

				// project r onto the the facet along its normal
				vec3f p;
				if (ProjectToFacet(y, ft.Nodes(), rf, Nf, p, 0.01))
				{
					// return the closest projection
					float D = (p - rf)*(p - rf);
					if ((D < Dmin) || (bfound == false))
					{
						// only consider backfacing intersections
						if (Nf*(p - rf) <= 0.f)
						{
							q = p;
							Dmin = D;
							bfound = true;
							vec3f faceNormal = to_vec3f(trg->FaceNormal(ft));
							backFacing = (Nf*faceNormal < 0.f);
						}
					}
				}
			}

			if (bfound && backFacing)
			{
				node.m_ntag = 2;
			}
		}
	}

	vector<int> faceList;
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh->Face(i);
		int nf = f.Nodes();
		for (int j = 0; j < nf; ++j)
		{
			if (mesh->Node(f.n[j]).m_ntag == 2)
			{
				faceList.push_back(i);
				break;
			}
		}
	}
	return faceList;
}
