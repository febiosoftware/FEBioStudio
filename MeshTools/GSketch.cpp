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
#include "GSketch.h"
#include <GeomLib/geom.h>

//=============================================================================
// GSketch
//-----------------------------------------------------------------------------
GSketch::GSketch(void)
{
}

//-----------------------------------------------------------------------------
GSketch::~GSketch(void)
{
	Clear();
}

//-----------------------------------------------------------------------------
void GSketch::Clear()
{
	m_pt.clear();
	m_edge.clear();
}

//-----------------------------------------------------------------------------
int GSketch::AddPoint(vec2d r)
{
	// snap tolerance
	const double eps = 1e-2;

	// see if this node already exists
	int n = -1;
	int N = (int) m_pt.size();
	for (int i=0; i<N; ++i)
	{
		vec2d& a = m_pt[i].r;
		if ((fabs(a.x - r.x) < eps) && (fabs(a.y - r.y) < eps))
		{
			n = i;
			break;
		}
	}

	// if it does not exist yet, we add it
	if (n==-1)
	{
		POINT p;
		p.r = r;
		m_pt.push_back(p);
		n = (int) m_pt.size() - 1;

		// next, we need to see if this node splits an edge
		// loop over all edges
		int NE = Edges();
		double w, D;
		for (int i=0; i<NE; ++i)
		{
			EDGE& e = m_edge[i];

			// find the projection to this edge
			vec2d q;
			w = Project(e, r, D, q);
			if ((w > 0)&&(w<1)&&(D<eps))
			{
				// project the node on the edge
				m_pt[n].r = q;

				// split the edge
				Split(i, w, n);
				break;
			}
		}
	}

	return n;
}

//-----------------------------------------------------------------------------
void GSketch::AddLine(vec2d a, vec2d b)
{
	int na = AddPoint(a);
	int nb = AddPoint(b);

	// adding the two points may have already created
	// this line (by splitting other lines) so we need
	// to see if this edge already exist or not
	for (int i=0; i<(int) m_edge.size(); ++i)
	{
		EDGE& ei = m_edge[i];
		if (ei.ntype == LINE)
		{
			if (((ei.n[0] == na)&&(ei.n[1]==nb))||
				((ei.n[0] == nb)&&(ei.n[1]==na))) return;
		}
	}

	// The edge does not exist so add it.
	EDGE e;
	e.ntype = LINE;
	e.n[0] = na;
	e.n[1] = nb;
	InsertEdge(e);
}

//-----------------------------------------------------------------------------
void GSketch::AddCircle(vec2d c, double R)
{
	int n0 = AddPoint(c);

	EDGE e;
	e.ntype = CIRCLE;
	e.n[0] = -1;
	e.n[1] = -1;
	e.nc = n0;
	e.R = R;
	m_edge.push_back(e);
}

//-----------------------------------------------------------------------------
void GSketch::AddArc(vec2d a, vec2d b, vec2d c)
{
	int n0 = AddPoint(a);
	int n1 = AddPoint(b);

	// make sure point c is on the arc
	vec2d e1 = b - a;
	vec2d e2 = c - a;
	double R = e1.norm();
	e2.unit();
	c = a + e2*R;
	int n2 = AddPoint(c);
	
	// adding the points may have already created
	// this arc (by splitting other arcs) so we need
	// to see if this edge already exist or not
	// Note that for an arc the ordering of the nodes are important
	for (int i=0; i<(int) m_edge.size(); ++i)
	{
		EDGE& ei = m_edge[i];
		if (ei.ntype == ARC)
		{
			if ((ei.n[0] == n1)&&(ei.n[1]==n2)&&(ei.nc==n0)) return;
		}
	}

	// If we get here, we did not find the edge so add it.
	EDGE e;
	e.ntype = ARC;
	e.nc = n0;
	e.n[0] = n1;
	e.n[1] = n2;
	InsertEdge(e);
}

//-----------------------------------------------------------------------------
void GSketch::InsertEdge(EDGE& e)
{
	const double eps = 0.0001;

	// See if this edge intersects any other edge
	int i = 0;
	int NE = (int) m_edge.size();
	vec2d q;
	while (i<NE)
	{
		EDGE& ei = m_edge[i];
		double w = Intersect(ei, e, q);
		if ((w>eps)&&(w<1.0-eps))
		{
			// Add the intersection point (this should also split edge i)
			int n = AddPoint(q);
			assert((int) m_edge.size() == NE+1);

			// split the new edge
			EDGE ea = e;
			EDGE eb = e;
			ea.n[1] = n;
			eb.n[0] = n;

			// Insert the new edges
			InsertEdge(ea);
			InsertEdge(eb);

			// the origianl edge e no longer
			// exists, so we stop here
			return;
		}
		else ++i;
	}

	// If we get here, the new edge does not split
	// an existing edge so we just add it to the back
	m_edge.push_back(e);
}

//-----------------------------------------------------------------------------
void GSketch::Render()
{
/*	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	int NE = (int) m_edge.size();
	if (NE > 0)
	{
		glColor3ub(255,0,255);
		for (int i=0; i<NE; ++i)
		{
			EDGE& e = m_edge[i];
			switch (e.ntype)
			{
			case LINE:
				glBegin(GL_LINES);
				{
					vec3d r0 = m_pt[e.n[0]].r;
					vec3d r1 = m_pt[e.n[1]].r;
					glVertex3d(r0.x, r0.y, 0);
					glVertex3d(r1.x, r1.y, 0);
				}
				glEnd();
				break;
			case CIRCLE:
				{
					vec3d r(m_pt[e.nc].r);
					glxCircle(r, e.R, 50);
				}
				break;
			case ARC:
				{
					vec3d r0 = m_pt[e.nc].r;
					vec3d r1 = m_pt[e.n[0]].r;
					vec3d r2 = m_pt[e.n[1]].r;

					vec2d a(r0.x, r0.y);
					vec2d b(r1.x, r1.y);
					vec2d c(r2.x, r2.y);
					GM_CIRCLE_ARC ca(a,b,c);

					glxArc(r0, ca.m_R, ca.m_w0, ca.m_w1, 30);
				}
				break;
			}
		}
	}

	int NP = (int) m_pt.size();
	if (NP > 0)
	{
		glColor3ub(0, 0, 255);
		glBegin(GL_POINTS);
		{
			for (int i=0; i<NP; ++i)
			{
				vec2d r = m_pt[i].r;
				glVertex3d(r.x, r.y, 0);
			}
		}
		glEnd();
	}

	glPopAttrib();
*/
}

//-----------------------------------------------------------------------------
// This function returns the parametric coordinate along the edge and if the 
// projection lies on the edge, the value will be between zero and one. The 
// distance to the edge is returned in the D parameter. If the parametric
// coordinate is valid, the projection point is returned in q.
double GSketch::Project(GSketch::EDGE& e, vec2d r, double& D, vec2d& q)
{
	double w = 0;
	switch (e.ntype)
	{
	case LINE:
		{
			vec2d a = m_pt[e.n[0]].r;
			vec2d b = m_pt[e.n[1]].r;
			GM_LINE line(a,b);
			w = line.Project(r, D);
			if ((w>0)&&(w<1)) q = line.Point(w);
		}
		break;
	case ARC:
		{
			vec2d a = m_pt[e.n[0]].r;
			vec2d b = m_pt[e.n[1]].r;
			vec2d c = m_pt[e.nc].r;

			GM_CIRCLE_ARC ca(c, a, b);
			w = ca.Project(r, D);
			if ((w>0)&&(w<1)) q = ca.Point(w);
		}
		break;
	default:
		assert(false);
	}
	return w;
}

//-----------------------------------------------------------------------------
// This function splits the edge at the parametric coordinate w. 
void GSketch::Split(int ie, double w, int n)
{
	// get the edge
	EDGE& e0 = m_edge[ie];
	
	// create a copy of this edge
	EDGE e1 = e0;

	// split the edge
	e0.n[1] = n;
	e1.n[0] = n;

	// add the new edge
	// TODO: Should I add to the back or insert after e0?
//	m_edge.push_back(e1);
	m_edge.insert(m_edge.begin() + ie + 1, e1);
}

//-----------------------------------------------------------------------------
// This function finds the parametric coordinate of the intersection point
double GSketch::Intersect(GSketch::EDGE &a, GSketch::EDGE &b, vec2d &q)
{
	if ((a.ntype == LINE)&&(b.ntype==LINE))
	{
		// find the intersection point
		vec2d a0 = m_pt[a.n[0]].r;
		vec2d a1 = m_pt[a.n[1]].r;
		vec2d b0 = m_pt[b.n[0]].r;
		vec2d b1 = m_pt[b.n[1]].r;

		GM_LINE la(a0, a1);
		GM_LINE lb(b0, b1);

		double w;
		if (la.Intersect(lb, w))
		{
			q = la.Point(w);
			return w;
		}
		return -1;
	}
	if ((a.ntype == ARC)&&(b.ntype==LINE))
	{
		vec2d a0 = m_pt[a.nc].r;
		vec2d a1 = m_pt[a.n[0]].r;
		vec2d a2 = m_pt[a.n[1]].r;
		GM_CIRCLE_ARC ca(a0, a1, a2);

		vec2d b0 = m_pt[b.n[0]].r;
		vec2d b1 = m_pt[b.n[1]].r;
		GM_LINE lb(b0, b1);

		double w;
		if (ca.Intersect(lb, w))
		{
			q = ca.Point(w);
			return w;			
		}
		else return -1;
	}
	if ((a.ntype == LINE)&&(b.ntype==ARC))
	{
		vec2d a0 = m_pt[a.n[0]].r;
		vec2d a1 = m_pt[a.n[1]].r;
		GM_LINE la(a0, a1);

		vec2d b0 = m_pt[b.nc].r;
		vec2d b1 = m_pt[b.n[0]].r;
		vec2d b2 = m_pt[b.n[1]].r;
		GM_CIRCLE_ARC cb(b0, b1, b2);

		double w;
		if (la.Intersect(cb, w))
		{
			q = la.Point(w);
			return w;			
		}
		else return -1;
	}
	if ((a.ntype == ARC)&&(b.ntype==ARC))
	{
		vec2d a0 = m_pt[a.nc].r;
		vec2d a1 = m_pt[a.n[0]].r;
		vec2d a2 = m_pt[a.n[1]].r;
		GM_CIRCLE_ARC ca(a0, a1, a2);

		vec2d b0 = m_pt[b.nc].r;
		vec2d b1 = m_pt[b.n[0]].r;
		vec2d b2 = m_pt[b.n[1]].r;
		GM_CIRCLE_ARC cb(b0, b1, b2);

		double w;
		if (ca.Intersect(cb, w))
		{
			q = ca.Point(w);
			return w;			
		}
		else return -1;	
	}
	assert(false);
	return -1;
}
