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
#include "FESurfaceIntersect.h"

int FESurfaceIntersect::Apply(FSSurface* psrc, FSSurface* ptrg, double mindist)
{
	// get the parent mesh 
	// and make sure it is a triangle mesh
	FSMesh* pm = psrc->GetMesh();

	// build the slave surface node list
	FSNodeList* pn = psrc->BuildNodeList();

	// build the target facet list
	FSFaceList* pfs = psrc->BuildFaceList();
	FSFaceList* pft = ptrg->BuildFaceList();

	// clear the tags
	pm->TagAllNodes(0);

	// loop over all the surface nodes
	int N = pn->Size();
	FSNodeList::Iterator it = pn->First();
	for (int i=0; i<N; ++i, ++it)
	{
		// get the next node
		FSNode& node = *(it->m_pi);

		// find the distance to the target surface
		double D = Distance(*pft, node.r);

		// tag the node is the (signed) distance is less the min
		if (D <= mindist)
		{
			node.m_ntag = 1;
		}
	}

	// select all nodes that are tagged
	for (int i=0; i<pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.m_ntag == 1) node.Select(); else node.Unselect();
	}

	// select all faces that attach to a node that is tagged
	int NF = pm->Faces();
	for (int i=0; i<NF; ++i) pm->Face(i).Unselect();

	FSFaceList::Iterator pi;
	for (pi = pfs->First(); pi != pfs->End(); ++pi)
	{
		FSFace& f = *(pi->m_pi);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j)
		{
			if (pm->Node(f.n[j]).m_ntag == 1) f.Select();
		}
	}

	int nsel = 0;
	for (int i=0; i<NF; ++i) if (pm->Face(i).IsSelected()) nsel++;

	// clean up
	delete pn;
	delete pfs;
	delete pft;

	return nsel;
}

bool tri_distance(vec3d p[3], const vec3d& r, double& L)
{
	// set origin to p[0]
	vec3d e1 = p[1] - p[0];
	vec3d e2 = p[2] - p[0];
	vec3d t = r - p[0];

	// calculate normal
	vec3d n = e1 ^ e2; n.Normalize();

	// calculate signed distance to plane
	L = t*n;

	// get projection
	vec3d q = t - n*L;

	double g[2][2];
	g[0][0] = e1*e1; 
	g[0][1] = g[1][0] = e1*e2;
	g[1][1] = e2*e2;

	double D = g[0][0]*g[1][1] - g[0][1]*g[0][1];
	if (D == 0) return false;

	double G[2][2];
	G[0][0] = g[1][1] / D;
	G[0][1] = G[1][0] = -g[0][1]/D;
	G[1][1] = g[0][0] / D;

	vec3d E1 = e1*G[0][0] + e2*G[0][1];
	vec3d E2 = e1*G[1][0] + e2*G[1][1];

	double a1 = q*E1;
	double a2 = q*E2;

	const double e = -0.01;
	return (a1>=e)&&(a1<=1-e)&&(a2>=e)&&(a2<=1-e)&&(a1+a2<=1-e);
}

double FESurfaceIntersect::Distance(FSFaceList& s, const vec3d& r)
{
	double Dmin = 1e99, D;

	// loop over all the (triangle) facets
	FSFaceList::Iterator pf;
	vec3d p[3];
	for (pf = s.First(); pf != s.End(); ++pf)
	{
		FSFace& f = *(pf->m_pi);
		FSCoreMesh& m = *(pf->m_pm);
		if (f.Type() == FE_FACE_TRI3)
		{
			// get the vertex coordinates
			p[0] = m.Node(f.n[0]).r;
			p[1] = m.Node(f.n[1]).r;
			p[2] = m.Node(f.n[2]).r;

			// calculate the distance
			if (tri_distance(p, r, D))
			{
				// store the min distance
				if (fabs(D) < fabs(Dmin)) Dmin = D;
			}
		}
	}
	
	return Dmin;
}
