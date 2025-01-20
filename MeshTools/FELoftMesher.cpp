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
#include "FELoftMesher.h"
#include <MeshLib/FSCurveMesh.h>
#include <MeshLib/FSSurfaceMesh.h>
#include <MeshLib/triangulate.h>
#include <FSCore/LoadCurve.h>
using namespace std;

int closestNode(FSCurveMesh& c, const vec3d& r)
{
	double minD2 = 1e99;
	int imin = -1;
	int N = c.Nodes();
	for (int i=0; i<N; ++i)
	{
		vec3d d = c.Node(i).pos() - r;
		double D2 = d*d;
		if (D2 < minD2)
		{
			imin = i;
			minD2 = D2;
		}
	}
	assert(imin != -1);
	return imin;
}

FELoftMesher::FELoftMesher()
{
	m_elem = 0;
	m_ndivs = 1;
	m_bsmooth = true;
}

struct LOFT_FACET
{
	int n0, n1, n2;
};

struct LOFT_EDGE
{
	int n0, n1;
	int id;

	bool operator != (const LOFT_EDGE& e)
	{
		if ((n0 == e.n0) && (n1 == e.n1)) return false;
		if ((n0 == e.n1) && (n1 == e.n0)) return false;
		return true;
	}
};

struct LOFT_NODE
{
	vec3d	r;
	int		nid;
	double	s;		// parametric position on curve (from 0 to 1)
};

FSSurfaceMesh* FELoftMesher::Apply(vector<FSCurveMesh*> curve)
{
	switch(m_elem)
	{
	case 0: return BuildTriMesh(curve); break;
	case 1: return BuildQuadMesh(curve); break;
	default:
		assert(false);
	}

	return 0;
}


FSSurfaceMesh* FELoftMesher::BuildQuadMesh(vector<FSCurveMesh*> curve)
{
	// number of curves to loft
	int NC = (int)curve.size();
	if (NC <= 1) return nullptr;

	// curves must be sorted
	for (int i = 0; i<NC; ++i) curve[i]->Sort();

	// make sure that all curves have the same number of nodes
	int NCN = curve[0]->Nodes();
	for (int i=1; i<NC; ++i) if (curve[i]->Nodes() != NCN) return 0;

	// see if we need to flip any curves
	vec3d r0 = curve[0]->Node(NCN - 1).r - curve[0]->Node(0).r; r0.Normalize();
	for (int i=1; i<NC; ++i)
	{
		vec3d r1 = curve[i]->Node(NCN - 1).r - curve[i]->Node(0).r; r1.Normalize();
		if (r0*(-r1) > (r0*r1)) curve[i]->Invert();
	}

	// number of divisions between each pair of curves
	int ND = m_ndivs;
	if (ND < 1) ND = 1;

	// number of nodes
	int NN = ((NC - 1)*ND + 1)*NCN;

	// number of edges
	int NE = ((NC-1)*ND + 1)*(NCN - 1) + (NC - 1)*ND*NCN;

	// number of faces
	int NF = ND*(NC - 1)*(NCN - 1);

	// Build the mesh
	FSSurfaceMesh* mesh = new FSSurfaceMesh;
	mesh->Create(NN, NE, NF);

	// create nodes
	NN = 0;
	if ((m_bsmooth == false) || (NC == 2))
	{
		for (int i = 0; i < NC; ++i)
		{
			if (i < NC - 1)
			{
				FSCurveMesh* curve0 = curve[i];
				FSCurveMesh* curve1 = curve[i + 1];

				for (int l = 0; l < ND; ++l)
				{
					double w = (double)l / (double)ND;
					for (int j = 0; j < NCN; ++j)
					{
						vec3d r0 = curve0->Node(j).r;
						vec3d r1 = curve1->Node(j).r;
						mesh->Node(NN++).r = r1 * w + r0 * (1.0 - w);
					}
				}
			}
			else
			{
				FSCurveMesh* curvei = curve[i];
				int nn = curvei->Nodes();
				for (int j = 0; j < nn; ++j) mesh->Node(NN++).r = curvei->Node(j).r;
			}
		}
	}
	else if (NC == 3)
	{
		FSCurveMesh* curve0 = curve[0];
		FSCurveMesh* curve1 = curve[1];
		FSCurveMesh* curve2 = curve[2];

		// quadratic interpolation
		for (int l = 0; l <= 2 * ND; ++l)
		{
			double w = (double)l / (double)(2*ND);
			for (int j = 0; j < NCN; ++j)
			{
				vec3d r0 = curve0->Node(j).r;
				vec3d r1 = curve1->Node(j).r;
				vec3d r2 = curve2->Node(j).r;

				double h0 = 2 * (1 - w) * (0.5 - w);
				double h1 = 4*w*(1 - w);
				double h2 = 2 * w * (w - 0.5);
				mesh->Node(NN++).r = r0 * h0 + r1 * h1 + r2*h2;
			}
		}
	}
	else
	{
		for (int i = 0; i < NC - 1; ++i)
		{
			// cubic interpolation
			FSCurveMesh* c[4];
			int l0, l1;
			if (i == 0)
			{
				c[0] = curve[0];
				c[1] = curve[1];
				c[2] = curve[2];
				c[3] = curve[3];
				l0 = 0;
				l1 = ND;
			}
			else if (i == NC - 2)
			{
				c[0] = curve[i-2];
				c[1] = curve[i-1];
				c[2] = curve[i  ];
				c[3] = curve[i+1];
				l0 = 2*ND;
				l1 = 3*ND;
			}
			else
			{
				c[0] = curve[i - 1];
				c[1] = curve[i    ];
				c[2] = curve[i + 1];
				c[3] = curve[i + 2];
				l0 = ND;
				l1 = 2 * ND;
			}

			// cubic interpolation
			for (int l = l0; l < l1; ++l)
			{
				double w = (double)l / (double)(3 * ND);
				for (int j = 0; j < NCN; ++j)
				{
					vec3d r0 = c[0]->Node(j).r;
					vec3d r1 = c[1]->Node(j).r;
					vec3d r2 = c[2]->Node(j).r;
					vec3d r3 = c[3]->Node(j).r;

					double h0 = -9.0 * (w - 1. / 3.) * (w - 2. / 3.) * (w - 1.) * 0.5;
					double h1 = 27.0 * w * (w - 2. / 3.) * (w - 1.) * 0.5;
					double h2 = -27.0 * w * (w - 1. / 3.) * (w - 1.) * 0.5;
					double h3 = 9.0 * w * (w - 1. / 3.) * (w - 2. / 3.) * 0.5;
					mesh->Node(NN++).r = r0 * h0 + r1 * h1 + r2 * h2 + r3 * h3;
				}
			}
		}
		FSCurveMesh* curvei = curve[NC-1];
		int nn = curvei->Nodes();
		for (int j = 0; j < nn; ++j) mesh->Node(NN++).r = curvei->Node(j).r;
	}

	// create edges
	NE = 0;
	for (int i=0; i<NC; ++i)
	{
		int nd = (i < NC-1 ? ND : 1);
		for (int l=0; l<nd; ++l)
		{
			// do horizontal edges
			for (int j=0; j<NCN-1; ++j)
			{
				FSEdge& edge = mesh->Edge(NE++);
				edge.SetType(FE_EDGE2);
				edge.m_gid = (l==0 ? i : -1);
				edge.n[0] = (i*ND + l)*NCN + j;
				edge.n[1] = (i*ND + l)*NCN + j + 1;
			}

			// do vertical edges
			if (i < NC - 1)
			{
				for (int j = 0; j<NCN; ++j)
				{
					FSEdge& edge = mesh->Edge(NE++);
					edge.SetType(FE_EDGE2);

					if      (j==0    ) edge.m_gid = NC + 2*i;
					else if (j==NCN-1) edge.m_gid = NC + 2*i + 1;
					else edge.m_gid = -1;

					edge.n[0] = (i*ND + l)*NCN + j;
					edge.n[1] = (i*ND + l + 1)*NCN + j;
				}
			}
		}
	}

	// create faces
	NF = 0;
	for (int i = 0; i<NC-1; ++i)
	{
		for (int l=0; l<ND; ++l)
		{
			for (int j=0; j<NCN-1; ++j)
			{
				FSFace& face = mesh->Face(NF++);
				face.SetType(FE_FACE_QUAD4);
				face.m_gid = 0;
				face.n[0] = (i*ND + l)*NCN + j;
				face.n[1] = (i*ND + l)*NCN + j + 1;
				face.n[2] = (i*ND + l + 1)*NCN + j + 1;
				face.n[3] = (i*ND + l + 1)*NCN + j;
			}
		}
	}

	// update the mesh
	mesh->BuildMesh();

	return mesh;
}

vec3d averageCurveNormal(vector<LOFT_NODE>& curve)
{
	int N = curve.size() - 1; // assumes curve is closed (last node is the same as first node for closed curves)
	if (N == 0) return vec3d(0,0,0);
	vec3d c(0,0,0);
	for (int i=0; i<N; ++i) c += curve[i].r;
	c /= N;
	
	vec3d cN(0,0,0);
	for (int i=0; i<N; ++i)
	{
		vec3d a = curve[i].r;
		vec3d b = curve[i+1].r;

		vec3d n = (a - c)^(b - c);
		cN += n;
	}		
	cN.Normalize();

	return cN;
}

void LoftCurves(vector<LOFT_NODE>& curve1, vector<LOFT_NODE>& curve2, vector<LOFT_FACET>& mesh, vector<LOFT_EDGE>& edgeList, int edgeID)
{
	// shorthand notation
	vector<LOFT_NODE>& c1 = curve1;
	vector<LOFT_NODE>& c2 = curve2;

	// total number of nodes
	int NN0 = c1.size();
	int NN1 = c2.size();
	if ((NN0 <= 1) && (NN1 <= 1)) return;

	// set the nodes being processed
	int n0 = 0, n1 = 0;

	// this stores the nodes of the facet that will be added
	int n[3] = { 0 };

	// these are the new edges
	int l[2] = {0};

	// add the first edge
	LOFT_EDGE e0 = { c1[n0].nid, c2[n1].nid, edgeID };
	edgeList.push_back(e0);
	bool bclosed = false;

	const double eps = 1e-12;

	// repeat over all edges
	while (true)
	{
		if ((n0 < NN0-1) && (n1 < NN1-1))
		{
			// get the next nodes
			int m0 = n0 + 1;
			int m1 = n1 + 1;

			// get the nodes
			vec3d& r0 = c1[n0].r;
			vec3d& r1 = c1[m0].r;
			vec3d& r2 = c2[n1].r;
			vec3d& r3 = c2[m1].r;

			vec3d a[3] = {r0, r1, r2};
			vec3d b[3] = {r3, r2, r0};

			// get the cross-distances
			double D12 = (r1 - r2)*(r1 - r2);
			double D03 = (r0 - r3)*(r0 - r3);

			if (D12 <= D03 + eps)
			{
				n[0] = c1[n0].nid;
				n[1] = c1[m0].nid;
				n[2] = c2[n1].nid;

				l[0] = n[1];
				l[1] = n[2];

				n0 = m0;
			}
			else
			{
				n[0] = c2[m1].nid;
				n[1] = c2[n1].nid;
				n[2] = c1[n0].nid;

				l[0] = n[2];
				l[1] = n[0];

				n1 = m1;
			}
		}
		else if (n0 < NN0 - 1)
		{
			n[0] = c1[n0    ].nid;
			n[1] = c1[n0 + 1].nid;
			n[2] = c2[n1    ].nid;

			l[0] = n[1];
			l[1] = n[2];

			n0 = n0 + 1;
		}
		else if (n1 < NN1 - 1)
		{
			n[0] = c2[n1 + 1].nid;
			n[1] = c2[n1    ].nid;
			n[2] = c1[n0    ].nid;

			l[0] = n[2];
			l[1] = n[0];

			n1 = n1 + 1;
		}
		else break;

		// Add the facet
		LOFT_FACET f = { n[0], n[1], n[2] };
		mesh.push_back(f);

		// add the edge
		// We have to make sure that this edge is not repeated (which can be the case for a closed curve)
		LOFT_EDGE e = { l[0], l[1], -1 };
		if (e != e0)
		{
			edgeList.push_back(e);
		}
		else bclosed = true;
	}

	// update the edge ID of the last added edge
	if (bclosed == false)
	{
		edgeList[edgeList.size() - 1].id = edgeID + 1;
	}
}

vec3d projectToCurve(const vec3d& p, vector<LOFT_NODE>& c)
{
	const double eps = 1e-12;
	int imin = -1;
	double Dmin = 0.0;
	vec3d qmin(p);
	int N = (int)c.size() - 1;
	for (int i=0; i<N; ++i)
	{
		vec3d r0 = c[i].r;
		vec3d r1 = c[i+1].r;
		vec3d e = r1 - r0;

		double l = ((p - r0)*e) / (e*e);
		if ((l >= -eps) && (l <= 1.0+eps))
		{
			vec3d q = r0 + e*l;

			double D = (p-q)*(p-q);
			if ((D < Dmin) || (imin == -1))
			{
				imin = i;
				Dmin = D;
				qmin = q;
			}
		}
	}
	return qmin;
}

vec3d pointOnCurve(double s, vector<LOFT_NODE>& c)
{
	int N = c.size();
	if (N == 0)
	{
		assert(false);
		return vec3d(0,0,0);
	}

	if (s <= c[0  ].s) return c[0  ].r;
	if (s >= c[N-1].s) return c[N-1].r;

	for (int i=1; i<N; ++i)
	{
		if (s < c[i].s)
		{
			vec3d ra = c[i-1].r;
			vec3d rb = c[i  ].r;
			double ds = s - c[i-1].s;
			double W = c[i].s - c[i-1].s; if (W == 0.0) W = 1.0;
			double w = ds/W;

			return ra*(1.0 - w) + rb*w;
		}
	}

	// we should never get here
	assert(false);
	return vec3d(0,0,0);
}

double curveLength(vector<LOFT_NODE>& c)
{
	double L = 0.0;
	for (int i = 0; i<c.size() - 1; ++i)
	{
		vec3d ra = c[i].r;
		vec3d rb = c[i+1].r;
		L += (rb - ra).Length();
	}
	return L;
}

void updateCurve(vector<LOFT_NODE>& c)
{
	int N = c.size();
	if (N == 0) return;
	double L = curveLength(c); if (L == 0.0) L = 1.0;
	double l = 0.0;
	c[0].s = 0.0;
	vec3d rp = c[0].r;
	for (int i = 1; i<c.size(); ++i)
	{
		vec3d r = c[i].r;
		l += (r - rp).Length();
		c[i].s = l / L;
		rp = r;
	}
}

double findClosestNodes(const FSCurveMesh& c1, const FSCurveMesh& c2, int node[2])
{
	if ((c1.Nodes() == 0) || (c2.Nodes() == 0)) return 0.0;

	node[0] = node[1] = 0;
	double Dmin = (c1.Node(0).r - c2.Node(0).r).SqrLength();

	for (int i=0; i<c1.Nodes(); ++i)
	{
		vec3d ri = c1.Node(i).r;
		for (int j=0; j<c2.Nodes(); ++j)
		{
			vec3d rj = c2.Node(j).r;
			double Dij = (ri - rj).SqrLength();
			if (Dij < Dmin)
			{
				node[0] = i;
				node[1] = j;
				Dmin = Dij;
			}
		}
	}
	return Dmin;
}

void flipCurve(vector<LOFT_NODE>& curve)
{
	int N = (int)curve.size();
	for (int i=0; i<N/2; ++i)
	{
		int j = N - i - 1;
		if (i != j)
		{
			vec3d tmp = curve[i].r;
			curve[i].r = curve[j].r;
			curve[j].r = tmp;
		}
	}
}

FSSurfaceMesh* FELoftMesher::BuildTriMesh(vector<FSCurveMesh*> curve)
{
	// number of curves to loft
	int NC = (int)curve.size();
	if (NC < 2) return 0;

	// do some validation
	// make sure there is at least one edge and two nodes
	for (int i=0; i<NC; ++i)
	{
		FSCurveMesh* c = curve[i];
		if (c->Nodes() < 2) return 0;
		if (c->Edges() < 1) return 0;
	}

	// curves must be sorted
	// this assumes that the curves don't have any holes.
	for (int i = 0; i<NC; ++i) curve[i]->Sort();

	// The lofting requires a slightly different approach for closed or non-closed loops
	// (and a mix of closed and open curves is not allowed), so let's figure out what we're dealing with
	bool bclosed = curve[0]->Type() == FSCurveMesh::CLOSED_CURVE;
	for (int i=1; i<NC; ++i)
		if ((curve[i]->Type() == FSCurveMesh::CLOSED_CURVE) != bclosed) return 0;

	// next, we create the loft curves
	vector< vector<LOFT_NODE> > loftCurve(NC);

	// get the number of divisions
	int ND = m_ndivs;
	if (ND < 1) ND = 1;

	// keep track of the number of nodes
	int NN = 0;

	// if the curves are open we try to make sure they are all pointing the same direction
	if (bclosed == false)
	{
		// see if we need to flip any curves
		vec3d r0 = curve[0]->Node(curve[0]->Nodes()- 1).r - curve[0]->Node(0).r; r0.Normalize();
		for (int i = 0; i<NC; ++i)
		{
			vector<LOFT_NODE>& loft = loftCurve[i];
			FSCurveMesh& c = *curve[i];

			int NNi = c.Nodes();
			loft.resize(NNi);
			vec3d r1 = c.Node(NNi - 1).r - c.Node(0).r; r1.Normalize();
			if (r0*(-r1) >(r0*r1)) 
			{
				for (int j = 0; j<NNi; ++j)
				{
					loft[j].r = c.Node(NNi - j -1).r;
					loft[j].nid = NN + j;
				}
			}
			else
			{
				for (int j = 0; j<NNi; ++j)
				{
					loft[j].r = c.Node(j).r;
					loft[j].nid = NN + j;
				}
			}

			updateCurve(loft);

			if (i != NC - 1) NN += NNi*ND;
			else NN += NNi;
		}
	}
	else
	{
		// to get started we'll pick the two closest points in curve 1 and 2
		// starting points
		int nmin[2] = { 0, 0 };
		findClosestNodes(*curve[0], *curve[1], nmin);

		vec3d refNormal(0,0,1);
		for (int i=0; i<NC; ++i)
		{
			FSCurveMesh& c = *curve[i];
			int NNi = c.Nodes();
			vector<LOFT_NODE>& loft = loftCurve[i];
			loft.resize(NNi);

			int noff = 0;
			if (i<2) noff = nmin[i];
			else
			{
				// get the node that is closest to the previous starting node
				noff = closestNode(c, loftCurve[i-1][0].r);
			}

			// copy the nodes
			for (int j = 0; j<NNi; ++j)
			{
				loft[j].r = c.Node((j + noff) % NNi).pos();
				loft[j].nid = NN + j;
			}

			// since the curves are closed we need to copy the first node
			loft.push_back(loft[0]);

			updateCurve(loft);

			// calculate the curve "normal"
			vec3d curveNormal = averageCurveNormal(loft);

			// see if we need to flip this curve?
			if (i > 0)
			{
				if (curveNormal*refNormal < 0)
				{
					// flip the curve
					flipCurve(loft);

					// flipping the curve also flips the normal
					curveNormal = -curveNormal;
				}
			}

			// update the reference normal
			refNormal = curveNormal;

			if (i != NC - 1) NN += NNi*ND;
			else NN += NNi;
		}
	}

	// create a new mesh
	FSSurfaceMesh* mesh = new FSSurfaceMesh;
	mesh->Create(NN, 0, 0);

	// reserve space for storing facets
	vector<LOFT_FACET> face;
	vector<LOFT_EDGE> edge;

	// mesh the curves
	NN = 0;
	for (int i = 0; i<NC - 1; ++i)
	{
		vector<LOFT_NODE>& c1 = loftCurve[i  ];
		vector<LOFT_NODE>& c2 = loftCurve[i+1];

		int NN1 = curve[i  ]->Nodes();
		int NN2 = curve[i+1]->Nodes();

		// create the edges for curve 1
		for (int j=0; j<NN1-1; ++j)
		{
			LOFT_EDGE e = {c1[j].nid, c1[j+1].nid, i };
			edge.push_back(e);
		}

		// create the final edges
		if (i+1 == NC-1)
		{
			for (int j = 0; j<NN2 - 1; ++j)
			{
				LOFT_EDGE e = { c2[j].nid, c2[j + 1].nid, i+1 };
				edge.push_back(e);
			}
		}

		// loft the two curves
		if (ND > 1)
		{
			// find the projections of the nodes of curve1 onto curve2
			vector<vec3d> src(c1.size());
			vector<vec3d> trg(c1.size());
			for (int j=0; j<c1.size(); ++j)
			{
				src[j] = c1[j].r;
				trg[j] = pointOnCurve(c1[j].s, c2);
			}

			// do the lofting
			vector<LOFT_NODE> c3(c1.size());
			for (int n=0; n<ND-1; ++n)
			{
				double w = (double) (n + 1) / ND;

				// create the intermediate curve
				for (int j=0; j<c3.size(); ++j)
				{
					c3[j].nid = c1[j].nid + NN1;
					c3[j].r   = src[j] * (1.0 - w) + trg[j]*w;
				}

				// create the edges for curve 3
				for (int j = 0; j<NN1 - 1; ++j)
				{
					LOFT_EDGE e = { c3[j].nid, c3[j + 1].nid, -1 };
					edge.push_back(e);
				}

				// update the curve
				updateCurve(c3);

				// loft the curves
				LoftCurves(c1, c3, face, edge, (bclosed ? NC + i : NC + 2*i));

				// copy the nodes
				for (int j=0; j<NN1; ++j) mesh->Node(NN++).r = c1[j].r;
				c1 = c3;
			}
		}

		// do the final lofting
		LoftCurves(c1, c2, face, edge, (bclosed ? NC + i : NC + 2 * i));
		for (int j = 0; j<NN1; ++j) mesh->Node(NN++).r = c1[j].r; 

		if (i==NC-2)
		{
			for (int j = 0; j<NN2; ++j) mesh->Node(NN++).r = c2[j].r;
		}
	}

	// build the edges
	int NE = edge.size();
	mesh->Create(0, NE, 0);
	for (int i=0; i<NE; ++i)
	{
		LOFT_EDGE& e = edge[i];
		FSEdge& edge = mesh->Edge(i);
		edge.SetType(FE_EDGE2);
		edge.m_gid = e.id;
		edge.n[0] = e.n0;
		edge.n[1] = e.n1;
	}

	// Build the faces
	int NF = face.size();
	mesh->Create(0, 0, NF);

	for (int i = 0; i<NF; ++i)
	{
		LOFT_FACET& f = face[i];
		FSFace& face = mesh->Face(i);
		face.m_gid = 0;
		face.SetType(FE_FACE_TRI3);
		face.n[0] = f.n0;
		face.n[1] = f.n1;
		face.n[2] = f.n2;
	}

	// update mesh data
	mesh->BuildMesh();

	return mesh;
}
