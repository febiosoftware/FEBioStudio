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
#include "FEMortarProjection.h"
#include <GeomLib/geomlib.h>
#include <MeshLib/MeshTools.h>
#include <MeshLib/FSMeshBuilder.h>

//-----------------------------------------------------------------------------
FEMortarProjection::FEMortarProjection()
{
}

//-----------------------------------------------------------------------------
FSMesh* FEMortarProjection::Apply(FSSurface* pslave, FSSurface* pmaster)
{
	// get the slave mesh
	FSMesh* pms = pslave->GetMesh();

	// get the master mesh
	FSMesh* pmm = pmaster->GetMesh();

	// create a new mesh
	FSMesh* pnew = new FSMesh();
	FSMesh* ptri = new FSMesh();

	// these polygons represent the projected slave, master, and intersection polygons
	const int MAX_POINTS = 10;
	POINT2D P[MAX_POINTS];
	int np = 0;
	POINT2D Q[MAX_POINTS];
	int nq = 0;
	POINT2D R[MAX_POINTS];
	int nr = 0;

	// loop over all slave facets
	FSItemListBuilder::Iterator pi = pslave->begin();
	int NFS = pslave->size();
	for (int i=0; i<NFS; ++i, ++pi)
	{
		// get the next slave facet
		FSFace* pf = pms->FacePtr(*pi);

		// get the average facet normal
		vec3d n = to_vec3d(pf->m_fn);

		// get the slave nodes
		np = pf->Nodes();
		vec3d xs[8];
		for (int k=0; k<np; ++k) xs[k] = pms->Node(pf->n[k]).r;

		// set-up the plane coordinate system
		vec3d e1 = xs[1] - xs[0];
		e1.Normalize();

		vec3d e2 = n ^ e1;
		e2.Normalize();
		
		vec3d e3 = e1 ^ e2;
		e3.Normalize();

		// coordinate center
		vec3d c = xs[0];

		// project the facet points to the plane
		for (int k=0; k<np; ++k)
		{
			vec3d q = (xs[k] - c) - e3*((xs[k] - c)*e3);
			POINT2D& pk = P[k];
			pk.x = q*e1;
			pk.y = q*e2;
		}


		// loop over all master facets
		FSItemListBuilder::Iterator pj = pmaster->begin();
		int NFM = pmaster->size();
		for (int j=0; j<NFM; ++j, ++pj)
		{
			// get the next master surface facet
			FSFace* pfm = pmm->FacePtr(*pj);

			// get the master nodes
			nq = pfm->Nodes();
			vec3d xm[8];
			for (int k=0; k<nq; ++k) xm[k] = pmm->Node(pfm->n[k]).r;

			// project the master facet onto the plane
			for (int k=0; k<nq; ++k)
			{
				vec3d x = xm[nq - k - 1];
				vec3d q = (x - c) - e3*((x - c)*e3);
				POINT2D& qk = Q[k];
				qk.x = q*e1;
				qk.y = q*e2;
			}

			// find the intersection polygon
			nr = ConvexIntersectSH(P, np, Q, nq, R);

			// let's make sure that the points in R actually lie inside P and Q
			if (nr > 0)
			{
				for (int k=0; k<nr; ++k)
				{
					POINT2D& r = R[k];
					vec3d x = e1*r.x + e2*r.y + c;

					double qr, qs;
					vec3d ps = ProjectToFace(*pms, x, *pf, qr, qs, false);
					const double eps = 1e-7;
					if ((qr < -eps) || (qs < -eps) || (qr+qs > 1+eps))
					{
						// error
						int a = 0;
					}

					vec3d pm = ProjectToFace(*pmm, x, *pfm, qr, qs, false);
					if ((qr < -eps) || (qs < -eps) || (qr+qs > 1+eps))
					{
						// error
						int a = 0;
					}

				}
			}

			// triangulate the polygon
			if (nr >= 3)
			{
				// create a new patch
				ptri->Create(nr+1, nr);

				// evaluate the center of the patch
				POINT2D d;
				d.x = d.y = 0;
				for (int k=0; k<nr; ++k)
				{
					d.x += R[k].x;
					d.y += R[k].y;
				}
				d.x /= nr; d.y /= nr;

				// add the nodes
				ptri->Node(0).r = e1*d.x + e2*d.y + c;
				for (int k=0; k<nr; ++k)
				{
					FSNode& n = ptri->Node(k+1);
					n.r = e1*R[k].x + e2*R[k].y + c;
				}

				// create the elements
				for (int k=0; k<nr; ++k)
				{
					FSElement& el = ptri->Element(k);
					el.SetType(FE_TRI3);
					el.m_gid = 0;
					el.m_node[0] = 0;
					el.m_node[1] = k+1;
					el.m_node[2] = 1 + (k+1)%nr;
				}

				// not sure if I need to do this
				ptri->RebuildMesh();

				// add the triangle to the mesh
				FSMeshBuilder meshBuilder(*pnew);
				meshBuilder.Attach(*ptri);
			}
		}
	}

	// cleanup
	delete ptri;

	return pnew;
}
