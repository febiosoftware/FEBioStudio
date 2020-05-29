#include "stdafx.h"
#include "FEMortarProjection.h"
#include <GeomLib/geomlib.h>
#include <MeshLib/MeshTools.h>
#include <MeshLib/FEMeshBuilder.h>

//-----------------------------------------------------------------------------
FEMortarProjection::FEMortarProjection()
{
}

//-----------------------------------------------------------------------------
FEMesh* FEMortarProjection::Apply(FESurface* pslave, FESurface* pmaster)
{
	// get the slave mesh
	FEMesh* pms = pslave->GetMesh();

	// get the master mesh
	FEMesh* pmm = pmaster->GetMesh();

	// create a new mesh
	FEMesh* pnew = new FEMesh();
	FEMesh* ptri = new FEMesh();

	// these polygons represent the projected slave, master, and intersection polygons
	const int MAX_POINTS = 10;
	POINT2D P[MAX_POINTS];
	int np = 0;
	POINT2D Q[MAX_POINTS];
	int nq = 0;
	POINT2D R[MAX_POINTS];
	int nr = 0;

	// loop over all slave facets
	FEItemListBuilder::Iterator pi = pslave->begin();
	int NFS = pslave->size();
	for (int i=0; i<NFS; ++i, ++pi)
	{
		// get the next slave facet
		FEFace* pf = pms->FacePtr(*pi);

		// get the average facet normal
		vec3d n = pf->m_fn;

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
		FEItemListBuilder::Iterator pj = pmaster->begin();
		int NFM = pmaster->size();
		for (int j=0; j<NFM; ++j, ++pj)
		{
			// get the next master surface facet
			FEFace* pfm = pmm->FacePtr(*pj);

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
					FENode& n = ptri->Node(k+1);
					n.r = e1*R[k].x + e2*R[k].y + c;
				}

				// create the elements
				for (int k=0; k<nr; ++k)
				{
					FEElement& el = ptri->Element(k);
					el.SetType(FE_TRI3);
					el.m_gid = 0;
					el.m_node[0] = 0;
					el.m_node[1] = k+1;
					el.m_node[2] = 1 + (k+1)%nr;
				}

				// not sure if I need to do this
				ptri->RebuildMesh();

				// add the triangle to the mesh
				FEMeshBuilder meshBuilder(*pnew);
				meshBuilder.Attach(*ptri);
			}
		}
	}

	// cleanup
	delete ptri;

	return pnew;
}
