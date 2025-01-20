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
#include "GPLCObject.h"
#include "FETetGenMesher.h"
#include <GeomLib/geom.h>
#include "GSketch.h"
#include <MeshLib/triangulate.h>
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
GPLCObject::GPLCObject() : GObject(GPLC_OBJECT)
{
	SetFEMesher(new FETetGenMesher(this));
}

//-----------------------------------------------------------------------------
FSMeshBase* GPLCObject::GetEditableMesh()
{
	return GetFEMesh();
}

//-----------------------------------------------------------------------------
FSMesh* GPLCObject::BuildMesh()
{
	SetFEMesh(GetFEMesher()->BuildMesh());
	return GetFEMesh();
}

inline double Measure(vec2d& a, vec2d& b)
{
	double cw = 0.5*(1.0 - a*b);
	vec2d c(0,0);
	if (IsLeft(c, a, b) == false) cw = -cw;
	return cw;
}

//-----------------------------------------------------------------------------
// Create a PLC Object from a sketch
void GPLCObject::Create(GSketch &s)
{
	int i;
	const double eps = 0.0001;

	// add all the nodes
	int NP = s.Points();
	for (i=0; i<NP; ++i) 
	{
		GSketch::POINT& p = s.Point(i);
		AddNode(p.r);
	}

	// add all the edges
	int NE = s.Edges();
	for (i=0; i<NE; ++i)
	{
		GSketch::EDGE& e = s.Edge(i);
		switch (e.ntype)
		{
		case GSketch::LINE:
			AddLine(e.n[0], e.n[1]);
			break;
		case GSketch::ARC:
			AddCircularArc(e.nc, e.n[0], e.n[1]);
			break;
		default:
			assert(false);
		}
	}

	// Find all the faces
	for (i=0; i<NE; ++i) Edge(i)->m_ntag = 0;
	GEdge* pe = 0;
	do
	{
		// The nodal tags store the number of edges
		// that reference them
		for (i=0; i<NP; ++i) Node(i)->m_ntag = 0;
		for (i=0; i<NE; ++i)
		{
			GEdge& e = *Edge(i);
			if (e.m_ntag == 0)
			{
				Node(e.m_node[0])->m_ntag += 1;
				Node(e.m_node[1])->m_ntag += 1;
			}
		}

		// Find the lowest point. If multiple lowest points
		// exist, we pick the most left one.
		int imin = -1;
		vec3d rmin;
		for (i=0; i<NP; ++i)
		{
			GNode& node = *Node(i);
			if (node.m_ntag > 0)
			{
				if (imin == -1)
				{
					imin = i;
					rmin = node.LocalPosition();
				}
				else
				{
					vec3d r = node.LocalPosition();
					if ((r.y < rmin.y) || ((r.y == rmin.y)&&(r.x < rmin.x)))
					{
						imin = i;
						rmin = r;
					}
				}
			}
		}

		// Make sure we found a point
		if (imin == -1) break;

		// find the lowest unprocessed edge that starts from this node
		// If the edge ends at this node, we know that we have to flip it
		int ew = 0;
		pe = 0;
		vec2d te;
		for (i=0; i<NE; ++i)
		{
			GEdge& e = *Edge(i);
			if ((e.m_ntag == 0)&&(e.HasNode(imin)))
			{
				if (pe == 0)
				{
					pe = Edge(i);
					if (e.m_node[0] == imin) ew = 1; else ew = -1;
					te = (ew==1?pe->Tangent(eps):-pe->Tangent(1.0-eps));
				}
				else
				{
					vec2d t1 = (e.m_node[0]==imin?e.Tangent(eps):-e.Tangent(1.0-eps));
					double w = Measure(te, t1);
					if (w < 0)
					{
						ew = (e.m_node[0] == imin? 1 : -1);
						pe = Edge(i);
						te = t1;
					}
				}
			}
		}

		// Make sure we found an edge
		assert(pe);
		if (pe==0) break;

		// construct a face by finding a closed loop
		std::vector<int> node;
		std::vector< std::pair<int,int> > edge;
		node.clear();
		edge.clear();
		while (pe)
		{
			// en0 and en1 define the start and end-node for the current edge.
			// for a normal edge they will be 0 and 1, but for an inverted
			// edge they will 1 and 0
			int en0, en1;
			if (ew == 1) { en0 = 0; en1 = 1; } else { en0 = 1; en1 = 0; }

			int nn0 = pe->m_node[en0];
			int nn1 = pe->m_node[en1];
			if ((Node(nn0)->m_ntag == 2) || (Node(nn1)->m_ntag == 2)) pe->m_ntag = 1;

			// add the edge and start node
			node.push_back(pe->m_node[en0]);
			edge.push_back(std::pair<int, int>(pe->GetLocalID(), ew));

			// see if we reached the end of the loop
			if (pe->m_node[en1] == node[0]) break; 

			// get the end-tangent
			vec2d te;
			if (ew == 1) te = pe->Tangent(1.0);
			else te = -pe->Tangent(0.0);

			// find the edge that attaches to pe
			GEdge* pe1 = 0;
			int ew1 = 0;
			vec2d t1;
			for (int j=0; j<NE; ++j)
			{
				GEdge* pe2 = Edge(j);
				if ((pe2 != pe) && (pe2->HasNode(nn1)))
				{
					if (pe1 == 0) 
					{
						pe1 = pe2;
						ew1 = (pe2->m_node[0] == nn1? 1 : -1);

						// get the start tangent (at a slight offset)
						t1 = (ew1==1?pe2->Tangent(eps):-pe2->Tangent(1.0-eps));
					}
					else
					{
						int ew2 = (pe2->m_node[0] == nn1? 1 : -1);
						vec2d t2 = (ew2==1?pe2->Tangent(eps):-pe2->Tangent(1.0-eps));

						double w1 = Measure(te, t1);
						double w2 = Measure(te, t2);

						if (w2 > w1) 
						{
							pe1 = pe2;
							t1 = t2;
							ew1 = ew2;
						}
					}
				}
			}

			pe = pe1;
			ew = ew1;
		}

		// build the face
		AddFacet(node, edge, FACE_POLYGON);
	}
	while (1);

	// we only add one part for now
	AddSolidPart();

	// Build the GLMesh
	BuildGMesh();

	// find all vertices
	UpdateNodeTypes();
}

GObject* GPLCObject::Clone()
{
	GPLCObject* clone = new GPLCObject();

	// copy nodes
	for (int i = 0; i < Nodes(); ++i)
	{
		GNode& ni = *Node(i);
		GNode* newNode = clone->AddNode(ni.LocalPosition(), ni.Type(), true);
		newNode->SetMeshWeight(ni.GetMeshWeight());
	}

	// copy edges
	for (int i = 0; i < Edges(); ++i)
	{
		GEdge& ei = *Edge(i);
		GEdge* ec = clone->AddEdge();

		ec->m_node[0] = ei.m_node[0];
		ec->m_node[1] = ei.m_node[1];
		ec->m_cnode = ei.m_cnode;
		ec->m_ntype = ei.m_ntype;
		ec->m_orient = ei.m_orient;
		ec->SetMeshWeight(ei.GetMeshWeight());
	}

	// copy faces
	for (int i = 0; i < Faces(); ++i)
	{
		GFace& fi = *Face(i);
		GFace* fc = clone->AddFace();
		fc->m_ntype = fi.m_ntype;
		fc->m_nPID[0] = fi.m_nPID[0];
		fc->m_nPID[1] = fi.m_nPID[1];
		fc->m_nPID[2] = fi.m_nPID[2];
		fc->m_node = fi.m_node;
		fc->m_edge = fi.m_edge;
		fc->SetMeshWeight(fi.GetMeshWeight());
	}

	// copy parts
	for (int i = 0; i < Parts(); ++i)
	{
		GPart& pi = *Part(i);
		GPart* pc = clone->AddPart();
		pc->m_node = pi.m_node;
		pc->m_edge = pi.m_edge;
		pc->m_face = pi.m_face;
		pc->SetMeshWeight(pi.GetMeshWeight());
	}

	clone->Update();

	return clone;
}
