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
#include "GLMesher.h"
#include "GLMesh.h"
#include <GeomLib/GObject.h>
#include <GeomLib/geom.h>
#include <MeshLib/FECurveMesh.h>
#include <MeshLib/triangulate.h>

GLMesher::GLMesher(GObject* po) : m_po(po)
{

}

GLMesh* GLMesher::CreateMesh()
{
	assert(m_po);
	if (m_po == nullptr) return nullptr;

	GObject& o = *m_po;

	// create a new mesh
	GLMesh* gmesh = new GLMesh;

	// the render mesh is created based of the faces (if there are any)
	int NF = o.Faces();
	if (NF > 0)
	{
		// repeat for all faces
		for (int j = 0; j < NF; ++j)
		{
			GFace& f = *o.Face(j);

			switch (f.m_ntype)
			{
			case FACE_POLYGON      : BuildFacePolygon(gmesh, f); break;
			case FACE_EXTRUDE      : BuildFaceExtrude(gmesh, f); break;
			case FACE_QUAD         : BuildFaceQuad(gmesh, f); break;
			case FACE_REVOLVE      : BuildFaceRevolve(gmesh, f); break;
			case FACE_REVOLVE_WEDGE: BuildFaceRevolveWedge(gmesh, f); break;
			default:
				assert(false);
			}
		}

		gmesh->Update();
	}
	else
	{
		// if there are no faces, we build a line mesh of the edge curves
		int NC = o.Edges();
		if (NC > 0)
		{
			for (int i = 0; i < NC; ++i)
			{
				GEdge& e = *o.Edge(i);
				switch (e.Type())
				{
				case EDGE_LINE: BuildEdgeLine(gmesh, e); break;
				case EDGE_MESH: BuildEdgeMesh(gmesh, e); break;
				default:
					assert(false);
				}
			}
		}
		else
		{
			// just add the nodes
			int NN = o.Nodes();
			for (int i = 0; i < NN; ++i)
			{
				GNode* ni = o.Node(i);
				gmesh->AddNode(ni->LocalPosition(), ni->GetID());
			}
			gmesh->Update();
		}
	}

	return gmesh;
}

//-----------------------------------------------------------------------------
void GLMesher::BuildEdgeLine(GLMesh* glmsh, GEdge& e)
{
	GObject& o = *m_po;
	vec3d y[2];
	y[0] = o.Node(e.m_node[0])->LocalPosition();
	y[1] = o.Node(e.m_node[1])->LocalPosition();
	int n[2] = { 0 };
	n[0] = glmsh->AddNode(y[0], e.m_node[0]);
	n[1] = glmsh->AddNode(y[1], e.m_node[1]);
	glmsh->AddEdge(n, 2, e.GetLocalID());
	glmsh->Update();
}

//-----------------------------------------------------------------------------
void GLMesher::BuildEdgeMesh(GLMesh* glmsh, GEdge& e)
{
	GObject& o = *m_po;

	// This assumes there is a mesh
	FECurveMesh* mesh = o.GetFECurveMesh(e.GetLocalID());
	assert(mesh);
	if (mesh == 0) return;

	// tag all nodes
	mesh->TagAllNodes(-1);

	// get the local edge id
	int edgeID = e.GetLocalID();
	for (int i = 0; i < mesh->Edges(); ++i)
	{
		FSEdge& edge = mesh->Edge(i);
		mesh->Node(edge.n[0]).m_ntag = 0;
		mesh->Node(edge.n[1]).m_ntag = 0;
	}

	// get all the nodes
	int NN = 0;
	for (int i = 0; i < mesh->Nodes(); ++i)
		if (mesh->Node(i).m_ntag != -1) mesh->Node(i).m_ntag = NN++;

	GLMesh glMesh;
	glMesh.Create(NN, 0);
	for (int i = 0; i < mesh->Nodes(); ++i)
	{
		FSNode& node = mesh->Node(i);
		if (node.m_ntag != -1)
		{
			GLMesh::NODE& gnode = glMesh.Node(node.m_ntag);
			gnode.r = node.pos();
		}
	}

	// add all the edges
	int n[2];
	for (int i = 0; i < mesh->Edges(); ++i)
	{
		FSEdge& e = mesh->Edge(i);
		n[0] = mesh->Node(e.n[0]).m_ntag;
		n[1] = mesh->Node(e.n[1]).m_ntag;
		glMesh.AddEdge(n, 2, edgeID);
	}

	// clean up
	delete mesh;

	// update mesh internal data structures
	glMesh.Update();

	// attach it to the render mesh
	glmsh->Attach(glMesh);
}

//-----------------------------------------------------------------------------
void GLMesher::BuildFacePolygon(GLMesh* glmesh, GFace& f)
{
	// triangulate the face
	GLMesh* pm = triangulate(f);

	// attach this mesh to our mesh
	glmesh->Attach(*pm, false);

	// don't forget to delete this mesh
	delete pm;
}

//-----------------------------------------------------------------------------
// Here, we assume that the face is built from extruding an edge.
// It must be true that the face has four nodes and four edges.
// Edges 0 and 2 must be of the same type and edges 1 and 3 must be
// straight lines.
void GLMesher::BuildFaceExtrude(GLMesh* glmesh, GFace& f)
{
	GObject& o = *m_po;

	const int M = 50;

	// get number of nodes and edges
	int NN = f.Nodes();
	int NE = f.Edges();
	assert(NN == 4);
	assert(NE == 4);

	// get the edges
	GEdge& e0 = *o.Edge(f.m_edge[0].nid);
	GEdge& e1 = *o.Edge(f.m_edge[1].nid);
	GEdge& e2 = *o.Edge(f.m_edge[2].nid);
	GEdge& e3 = *o.Edge(f.m_edge[3].nid);
	assert(e0.m_ntype == e2.m_ntype);
	assert(e1.m_ntype == EDGE_LINE);
	assert(e3.m_ntype == EDGE_LINE);

	// this is the mesh we'll be building
	GLMesh m;

	// build the face
	switch (e0.m_ntype)
	{
	case EDGE_LINE: // TODO: create a finer mesh
	{
		m.Create(4, 2, 4);
		for (int i = 0; i < 4; ++i)
		{
			GMesh::NODE& ni = m.Node(i);
			ni.r = o.Node(f.m_node[i])->LocalPosition();
			ni.pid = o.Node(f.m_node[i])->GetLocalID();
		}

		for (int i = 0; i < 4; ++i)
		{
			GMesh::EDGE& ei = m.Edge(i);
			ei.n[0] = i;
			ei.n[1] = (i + 1) % 4;
			ei.pid = o.Edge(f.m_edge[i].nid)->GetLocalID();
		}

		GMesh::FACE& f0 = m.Face(0);
		GMesh::FACE& f1 = m.Face(1);

		f0.n[0] = 0;
		f0.n[1] = 1;
		f0.n[2] = 2;
		f0.pid = f.GetLocalID();

		f1.n[0] = 2;
		f1.n[1] = 3;
		f1.n[2] = 0;
		f1.pid = f.GetLocalID();
	}
	break;
	case EDGE_3P_CIRC_ARC:
	{
		// get the nodes of the bottom edge
		vec3d r0 = o.Node(e0.m_cnode)->LocalPosition();
		int n0 = 0, n1 = 1;
		if (f.m_edge[0].nwn == -1) { n0 = 1; n1 = 0; }
		int en0 = e0.m_node[n0];
		int en1 = e0.m_node[n1];
		vec3d r1 = o.Node(en0)->LocalPosition();
		vec3d r2 = o.Node(en1)->LocalPosition();

		// get the extrusion direction
		vec3d t;
		if (e1.m_node[0] == en1) t = o.Node(e1.m_node[1])->LocalPosition() - o.Node(e1.m_node[0])->LocalPosition();
		else  t = o.Node(e1.m_node[0])->LocalPosition() - o.Node(e1.m_node[1])->LocalPosition();

		// project the points on a plane
		// NOTE: This assume the arc is on a plane!
		double z0 = r0.z;
		vec2d a0(r0.x, r0.y);
		vec2d a1(r1.x, r1.y);
		vec2d a2(r2.x, r2.y);

		// create an arc object
		GM_CIRCLE_ARC ca(a0, a1, a2, f.m_edge[0].nwn);

		// allocate the mesh
		m.Create(2 * (M + 1), 2 * M, 2 * M + 2);

		// create nodes
		for (int i = 0; i <= M; ++i)
		{
			GMesh::NODE& ni = m.Node(i);
			GMesh::NODE& nj = m.Node(i + (M + 1));

			vec2d q = ca.Point(i / (double)M);
			ni.r = vec3d(q.x(), q.y(), z0);
			nj.r = ni.r + t;
			ni.pid = -1;
			nj.pid = -1;
		}

		// mark the corner nodes
		m.Node(0).pid = o.Node(e0.m_node[n0])->GetLocalID();
		m.Node(M).pid = o.Node(e0.m_node[n1])->GetLocalID();

		int m0 = 0, m1 = 1;
		if (f.m_edge[2].nwn == -1) { m0 = 1; m1 = 0; }

		m.Node(M + 1).pid = o.Node(e2.m_node[m1])->GetLocalID();
		m.Node(2 * M + 1).pid = o.Node(e2.m_node[m0])->GetLocalID();

		// create the faces
		for (int i = 0; i < M; ++i)
		{
			GMesh::FACE& f0 = m.Face(2 * i);
			GMesh::FACE& f1 = m.Face(2 * i + 1);

			f0.n[0] = i;
			f0.n[1] = i + 1;
			f0.n[2] = (M + 1) + i + 1;
			f0.pid = f.GetLocalID();

			f1.n[0] = (M + 1) + i + 1;
			f1.n[1] = (M + 1) + i;
			f1.n[2] = i;
			f1.pid = f.GetLocalID();
		}

		// create the edges
		for (int i = 0; i < M; ++i)
		{
			GMesh::EDGE& e0 = m.Edge(2 * i);
			GMesh::EDGE& e1 = m.Edge(2 * i + 1);

			e0.n[0] = i;
			e0.n[1] = i + 1;
			e0.pid = o.Edge(f.m_edge[0].nid)->GetLocalID();

			e1.n[0] = (M + 1) + i;
			e1.n[1] = (M + 1) + i + 1;
			e1.pid = o.Edge(f.m_edge[2].nid)->GetLocalID();
		}

		GMesh::EDGE& e0 = m.Edge(2 * M);
		GMesh::EDGE& e1 = m.Edge(2 * M + 1);

		e0.n[0] = 0;
		e0.n[1] = M + 1;
		e0.pid = o.Edge(f.m_edge[3].nid)->GetLocalID();

		e1.n[0] = M;
		e1.n[1] = 2 * M + 1;
		e1.pid = o.Edge(f.m_edge[1].nid)->GetLocalID();
	}
	break;
	case EDGE_3P_ARC:
	{
		// get the nodes of the bottom edge
		vec3d r0 = o.Node(e0.m_cnode)->LocalPosition();
		int n0 = 0, n1 = 1;
		if (f.m_edge[0].nwn == -1) { n0 = 1; n1 = 0; }
		vec3d r1 = o.Node(e0.m_node[n0])->LocalPosition();
		vec3d r2 = o.Node(e0.m_node[n1])->LocalPosition();

		// get the extrusion direction
		vec3d t;
		if (f.m_edge[1].nwn == 1) t = o.Node(e1.m_node[1])->LocalPosition() - o.Node(e1.m_node[0])->LocalPosition();
		else  t = o.Node(e1.m_node[0])->LocalPosition() - o.Node(e1.m_node[1])->LocalPosition();

		// project the points on a plane
		vec2d a0(r0.x, r0.y);
		vec2d a1(r1.x, r1.y);
		vec2d a2(r2.x, r2.y);

		// create an arc object
		GM_ARC ca(a0, a1, a2, f.m_edge[0].nwn);

		// allocate the mesh
		m.Create(2 * (M + 1), 2 * M, 2 * M + 2);

		// create nodes
		for (int i = 0; i <= M; ++i)
		{
			GMesh::NODE& ni = m.Node(i);
			GMesh::NODE& nj = m.Node(i + (M + 1));

			ni.r = ca.Point(i / (double)M);
			nj.r = ni.r + t;
			ni.pid = -1;
			nj.pid = -1;
		}

		// mark the corner nodes
		m.Node(0).pid = o.Node(e0.m_node[n0])->GetLocalID();
		m.Node(M).pid = o.Node(e0.m_node[n1])->GetLocalID();

		int m0 = 0, m1 = 1;
		if (f.m_edge[2].nwn == -1) { m0 = 1; m1 = 0; }

		m.Node(M + 1).pid = o.Node(e2.m_node[m1])->GetLocalID();
		m.Node(2 * M + 1).pid = o.Node(e2.m_node[m0])->GetLocalID();

		// create the faces
		for (int i = 0; i < M; ++i)
		{
			GMesh::FACE& f0 = m.Face(2 * i);
			GMesh::FACE& f1 = m.Face(2 * i + 1);

			f0.n[0] = i;
			f0.n[1] = i + 1;
			f0.n[2] = (M + 1) + i + 1;
			f0.pid = f.GetLocalID();

			f1.n[0] = (M + 1) + i + 1;
			f1.n[1] = (M + 1) + i;
			f1.n[2] = i;
			f1.pid = f.GetLocalID();
		}

		// create the edges
		for (int i = 0; i < M; ++i)
		{
			GMesh::EDGE& e0 = m.Edge(2 * i);
			GMesh::EDGE& e1 = m.Edge(2 * i + 1);

			e0.n[0] = i;
			e0.n[1] = i + 1;
			e0.pid = o.Edge(f.m_edge[0].nid)->GetLocalID();

			e1.n[0] = (M + 1) + i;
			e1.n[1] = (M + 1) + i + 1;
			e1.pid = o.Edge(f.m_edge[2].nid)->GetLocalID();
		}

		GMesh::EDGE& e0 = m.Edge(2 * M);
		GMesh::EDGE& e1 = m.Edge(2 * M + 1);

		e0.n[0] = 0;
		e0.n[1] = M + 1;
		e0.pid = o.Edge(f.m_edge[3].nid)->GetLocalID();

		e1.n[0] = M;
		e1.n[1] = 2 * M + 1;
		e1.pid = o.Edge(f.m_edge[1].nid)->GetLocalID();
	}
	break;
	case EDGE_ZARC:
	{
		// get the nodes of the bottom edge
		vec3d r0 = vec3d(0, 0, 0);
		int n0 = 0, n1 = 1;
		if (f.m_edge[0].nwn == -1) { n0 = 1; n1 = 0; }
		vec3d r1 = o.Node(e0.m_node[n0])->LocalPosition();
		vec3d r2 = o.Node(e0.m_node[n1])->LocalPosition();

		// get the extrusion direction
		vec3d t;
		if (f.m_edge[1].nwn == 1) t = o.Node(e1.m_node[1])->LocalPosition() - o.Node(e1.m_node[0])->LocalPosition();
		else  t = o.Node(e1.m_node[0])->LocalPosition() - o.Node(e1.m_node[1])->LocalPosition();

		// project the points on a plane
		vec2d a0(r0.x, r0.y);
		vec2d a1(r1.x, r1.y);
		vec2d a2(r2.x, r2.y);

		// create an arc object
		GM_CIRCLE_ARC ca(a0, a1, a2, f.m_edge[0].nwn);

		// allocate the mesh
		m.Create(2 * (M + 1), 2 * M, 2 * M + 2);

		// create nodes
		for (int i = 0; i <= M; ++i)
		{
			GMesh::NODE& ni = m.Node(i);
			GMesh::NODE& nj = m.Node(i + (M + 1));

			ni.r = ca.Point(i / (double)M);
			nj.r = ni.r + t;
			ni.pid = -1;
			nj.pid = -1;
		}

		// mark the corner nodes
		m.Node(0).pid = o.Node(e0.m_node[n0])->GetLocalID();
		m.Node(M).pid = o.Node(e0.m_node[n1])->GetLocalID();

		int m0 = 0, m1 = 1;
		if (f.m_edge[2].nwn == -1) { m0 = 1; m1 = 0; }

		m.Node(M + 1).pid = o.Node(e2.m_node[m1])->GetLocalID();
		m.Node(2 * M + 1).pid = o.Node(e2.m_node[m0])->GetLocalID();

		// create the faces
		for (int i = 0; i < M; ++i)
		{
			GMesh::FACE& f0 = m.Face(2 * i);
			GMesh::FACE& f1 = m.Face(2 * i + 1);

			f0.n[0] = i;
			f0.n[1] = i + 1;
			f0.n[2] = (M + 1) + i + 1;
			f0.pid = f.GetLocalID();

			f1.n[0] = (M + 1) + i + 1;
			f1.n[1] = (M + 1) + i;
			f1.n[2] = i;
			f1.pid = f.GetLocalID();
		}

		// create the edges
		for (int i = 0; i < M; ++i)
		{
			GMesh::EDGE& e0 = m.Edge(2 * i);
			GMesh::EDGE& e1 = m.Edge(2 * i + 1);

			e0.n[0] = i;
			e0.n[1] = i + 1;
			e0.pid = o.Edge(f.m_edge[0].nid)->GetLocalID();

			e1.n[0] = (M + 1) + i;
			e1.n[1] = (M + 1) + i + 1;
			e1.pid = o.Edge(f.m_edge[2].nid)->GetLocalID();
		}

		GMesh::EDGE& e0 = m.Edge(2 * M);
		GMesh::EDGE& e1 = m.Edge(2 * M + 1);

		e0.n[0] = 0;
		e0.n[1] = M + 1;
		e0.pid = o.Edge(f.m_edge[3].nid)->GetLocalID();

		e1.n[0] = M;
		e1.n[1] = 2 * M + 1;
		e1.pid = o.Edge(f.m_edge[1].nid)->GetLocalID();
	}
	break;
	default:
		assert(false);
	}

	glmesh->Attach(m, false);
}

//-----------------------------------------------------------------------------
// Build a revolved surface
// The revolved surface has four edges, the two side ones of type EDGE_YARC
void GLMesher::BuildFaceRevolve(GLMesh* glmesh, GFace& f)
{
	GObject& o = *m_po;

	const int M = 50;

	int NN = f.Nodes();
	int NE = f.Edges();
	assert(NN == 4);
	assert(NE == 4);

	// get 4 corner nodes
	vec3d y[4];
	y[0] = o.Node(f.m_node[0])->LocalPosition();
	y[1] = o.Node(f.m_node[1])->LocalPosition();
	y[2] = o.Node(f.m_node[2])->LocalPosition();
	y[3] = o.Node(f.m_node[3])->LocalPosition();

	// get the edges and their windings
	int ew[4];
	GEdge* e[4];
	for (int i = 0; i < 4; ++i)
	{
		e[i] = o.Edge(f.m_edge[i].nid);
		ew[i] = f.m_edge[i].nwn;
	}

	// determine which edge is getting revolved.
	int revolvedEdge = -1;
	if      (e[0]->m_ntype == EDGE_ZARC) revolvedEdge = 1;
	else if (e[1]->m_ntype == EDGE_ZARC) revolvedEdge = 0;
	else assert(false);

	// allocate mesh
	GLMesh m;
	m.Create((M + 1) * (M + 1), 2 * M * M, 4 * M);

	// build nodes
	for (int j = 0; j <= M; ++j)
		for (int i = 0; i <= M; ++i)
		{
			double r = (double)i / (double)M;
			double s = (double)j / (double)M;

			double N1 = (1 - r) * (1 - s);
			double N2 = r * (1 - s);
			double N3 = r * s;
			double N4 = (1 - r) * s;

			// get edge points
			vec3d q[4];
			q[0] = EdgePoint(*(e[0]), (ew[0] == 1 ? r : 1.0 - r));
			q[1] = EdgePoint(*(e[1]), (ew[1] == 1 ? s : 1.0 - s));
			q[2] = EdgePoint(*(e[2]), (ew[2] != 1 ? r : 1.0 - r));
			q[3] = EdgePoint(*(e[3]), (ew[3] != 1 ? s : 1.0 - s));

			vec3d p = q[0] * (1 - s) + q[1] * r + q[2] * s + q[3] * (1 - r) \
				- (y[0] * N1 + y[1] * N2 + y[2] * N3 + y[3] * N4);

			// the transfinite interpolation doesn't quite project correctly
			// to a revolved surface, so we need to make a small adjustment.
			vec2d c(q[revolvedEdge].x, q[revolvedEdge].y);
			double R = c.norm();

			double z = p.z;
			p.z = 0;
			p.Normalize();
			p.x *= R;
			p.y *= R;
			p.z = z;

			GLMesh::NODE& n = m.Node(j * (M + 1) + i);
			n.r = p;
			n.pid = -1;
		}

	m.Node(0).pid = o.Node(f.m_node[0])->GetLocalID();
	m.Node(M).pid = o.Node(f.m_node[1])->GetLocalID();
	m.Node(M * (M + 1)).pid = o.Node(f.m_node[3])->GetLocalID();
	m.Node((M + 1) * (M + 1) - 1).pid = o.Node(f.m_node[2])->GetLocalID();

	// build the faces
	for (int j = 0; j < M; ++j)
		for (int i = 0; i < M; ++i)
		{
			GMesh::FACE& f0 = m.Face(j * (2 * M) + 2 * i);
			GMesh::FACE& f1 = m.Face(j * (2 * M) + 2 * i + 1);

			f0.n[0] = i * (M + 1) + j;
			f0.n[1] = (i + 1) * (M + 1) + j;
			f0.n[2] = (i + 1) * (M + 1) + j + 1;
			f0.pid = f.GetLocalID();

			f1.n[0] = (i + 1) * (M + 1) + j + 1;
			f1.n[1] = i * (M + 1) + j + 1;
			f1.n[2] = i * (M + 1) + j;
			f1.pid = f.GetLocalID();
		}

	// build the edges
	for (int i = 0; i < M; ++i)
	{
		GMesh::EDGE& e = m.Edge(i);
		e.n[0] = i;
		e.n[1] = i + 1;
		e.pid = o.Edge(f.m_edge[0].nid)->GetLocalID();
	}

	for (int i = 0; i < M; ++i)
	{
		GMesh::EDGE& e = m.Edge(M + i);
		e.n[0] = (i + 1) * (M + 1) - 1;
		e.n[1] = (i + 2) * (M + 1) - 1;
		e.pid = o.Edge(f.m_edge[1].nid)->GetLocalID();
	}

	for (int i = 0; i < M; ++i)
	{
		GMesh::EDGE& e = m.Edge(2 * M + i);
		e.n[0] = (M + 1) * (M + 1) - 1 - i;
		e.n[1] = (M + 1) * (M + 1) - 1 - i - 1;
		e.pid = o.Edge(f.m_edge[2].nid)->GetLocalID();
	}

	for (int i = 0; i < M; ++i)
	{
		GMesh::EDGE& e = m.Edge(3 * M + i);
		e.n[0] = (M - i) * (M + 1);
		e.n[1] = (M - i - 1) * (M + 1);
		e.pid = o.Edge(f.m_edge[3].nid)->GetLocalID();
	}
	glmesh->Attach(m, false);
}

//-----------------------------------------------------------------------------
// Build a revolved wedge surface
// The revolved surface has four edges, the two side ones of type EDGE_YARC
void GLMesher::BuildFaceRevolveWedge(GLMesh* glmesh, GFace& f)
{
	GObject& o = *m_po;

#ifdef _DEBUG
	const int M = 10;
#else
	const int M = 50;
#endif

	// get number of nodes and edges
	int NN = f.Nodes();
	int NE = f.Edges();
	assert(NN == 3);
	assert(NE == 3);

	// get the edges
	GEdge& e0 = *o.Edge(f.m_edge[0].nid);
	GEdge& e1 = *o.Edge(f.m_edge[1].nid);
	GEdge& e2 = *o.Edge(f.m_edge[2].nid);
	assert(e0.m_ntype == e2.m_ntype);
	assert((e1.m_ntype == EDGE_YARC) || (e1.m_ntype == EDGE_ZARC));

	// this is the mesh we'll be building
	GLMesh m;

	// build the mesh
	switch (e0.m_ntype)
	{
	case EDGE_LINE:
	{
		m.Create(M + 2, M, M + 2);
		vec3d r0 = o.Node(f.m_node[0])->LocalPosition();
		vec3d r1 = o.Node(f.m_node[1])->LocalPosition();
		vec3d r2 = o.Node(f.m_node[2])->LocalPosition();

		if (e1.m_ntype == EDGE_YARC)
		{
			double y0 = r0.y;
			double y1 = r1.y;

			vec2d a(r1.x, r1.z), b(r2.x, r2.z);

			// create an arc object
			GM_CIRCLE_ARC c0(vec2d(0, 0), a, b, f.m_edge[1].nwn);

			// position the center point
			GMesh::NODE& n0 = m.Node(0);
			n0.r = r0;
			n0.pid = -1;

			// create the arc points
			for (int i = 0; i <= M; ++i)
			{
				double t = (double)i / (double)M;

				GMesh::NODE& n0 = m.Node(i + 1);
				vec2d p0 = c0.Point(t);
				n0.r = vec3d(p0.x(), y1, p0.y());
				n0.pid = -1;
			}
		}
		else if (e1.m_ntype == EDGE_ZARC)
		{
			double z0 = r0.z;
			double z1 = r1.z;

			vec2d a(r1.x, r1.y), b(r2.x, r2.y);

			// create an arc object
			GM_CIRCLE_ARC c0(vec2d(0, 0), a, b, f.m_edge[1].nwn);

			// position the center point
			GMesh::NODE& n0 = m.Node(0);
			n0.r = r0;
			n0.pid = -1;

			// create the arc points
			for (int i = 0; i <= M; ++i)
			{
				double t = (double)i / (double)M;

				GMesh::NODE& n0 = m.Node(i + 1);
				vec2d p0 = c0.Point(t);
				n0.r = vec3d(p0.x(), p0.y(), z1);
				n0.pid = -1;
			}
		}

		m.Node(0).pid = o.Node(f.m_node[0])->GetLocalID();
		m.Node(1).pid = o.Node(f.m_node[1])->GetLocalID();
		m.Node(M + 1).pid = o.Node(f.m_node[2])->GetLocalID();

		// add edges
		GMesh::EDGE& e0 = m.Edge(0);
		GMesh::EDGE& e1 = m.Edge(1);
		e0.n[0] = 0; e0.n[1] = 1; e0.pid = o.Edge(f.m_edge[0].nid)->GetLocalID();
		e1.n[0] = 0; e1.n[1] = M + 1; e1.pid = o.Edge(f.m_edge[2].nid)->GetLocalID();

		for (int i = 0; i < M; ++i)
		{
			GMesh::EDGE& e = m.Edge(2 + i);
			e.n[0] = i + 1; e.n[1] = i + 2; e.pid = o.Edge(f.m_edge[1].nid)->GetLocalID();
		}

		for (int i = 0; i < M; ++i)
		{
			GMesh::FACE& f0 = m.Face(i);

			f0.n[0] = 0;
			f0.n[1] = i + 1;
			f0.n[2] = i + 2;
			f0.pid = f.GetLocalID();
		}
	}
	break;
	default:
		assert(false);
	}

	glmesh->Attach(m, false);
}

//-----------------------------------------------------------------------------
vec3d GLMesher::EdgePoint(GEdge& edge, double r)
{
	GObject& o = *m_po;
	vec3d r0 = o.Node(edge.m_node[0])->LocalPosition();
	vec3d r1 = o.Node(edge.m_node[1])->LocalPosition();
	vec3d p;
	switch (edge.m_ntype)
	{
	case EDGE_LINE:
		p = r0 * (1.0 - r) + r1 * r;
		break;
	case EDGE_ZARC:
	{
		vec2d c(0, 0);
		vec2d a(r0.x, r0.y);
		vec2d b(r1.x, r1.y);

		// create an arc object
		GM_CIRCLE_ARC ca(c, a, b, edge.m_orient);
		vec2d q = ca.Point(r);
		p = vec3d(q.x(), q.y(), r1.z);
	}
	break;
	case EDGE_3P_CIRC_ARC:
	{
		vec3d rc = o.Node(edge.m_cnode)->LocalPosition();
		GM_CIRCLE_3P_ARC c(rc, r0, r1, edge.m_orient);
		p = c.Point(r);
	}
	break;	
	default:
		assert(false);
	}
	return p;
}

//-----------------------------------------------------------------------------
void GLMesher::BuildFaceQuad(GLMesh* glmesh, GFace& f)
{
	GObject& o = *m_po;

	const int M = 10;

	int NN = f.Nodes();
	int NE = f.Edges();
	assert(NN == 4);
	assert(NE == 4);

	// get 4 corner nodes
	vec3d y[4];
	y[0] = o.Node(f.m_node[0])->LocalPosition();
	y[1] = o.Node(f.m_node[1])->LocalPosition();
	y[2] = o.Node(f.m_node[2])->LocalPosition();
	y[3] = o.Node(f.m_node[3])->LocalPosition();

	// get the edges and their windings
	int ew[4];
	GEdge* e[4];
	for (int i = 0; i < 4; ++i)
	{
		e[i] = o.Edge(f.m_edge[i].nid);
		ew[i] = f.m_edge[i].nwn;
	}

	// allocate mesh
	GLMesh m;
	m.Create((M + 1) * (M + 1), 2 * M * M, 4 * M);

	// see if this face is a sphere
	// it is assumed a sphere if all edges are 3P arcs with the same center node
	bool isSphere = true;
	double sphereRadius = 0;
	vec3d sphereCenter(0, 0, 0);
	int c0 = e[0]->m_cnode;
	for (int j = 0; j < 4; ++j)
	{
		if ((e[j]->m_ntype != EDGE_3P_CIRC_ARC) || (e[j]->m_cnode != c0))
		{
			isSphere = false;
			break;
		}
	}
	if (isSphere)
	{
		// we assume that the corner nodes are already on the sphere
		vec3d r0 = o.Node(f.m_node[0])->LocalPosition();
		sphereCenter = o.Node(c0)->LocalPosition();
		sphereRadius = (r0 - sphereCenter).Length();
	}

	// build nodes
	for (int j = 0; j <= M; ++j)
		for (int i = 0; i <= M; ++i)
		{
			double r = (double)i / (double)M;
			double s = (double)j / (double)M;

			double N1 = (1 - r) * (1 - s);
			double N2 = r * (1 - s);
			double N3 = r * s;
			double N4 = (1 - r) * s;

			// get edge points
			vec3d q[4];
			q[0] = EdgePoint(*(e[0]), (ew[0] == 1 ? r : 1.0 - r));
			q[1] = EdgePoint(*(e[1]), (ew[1] == 1 ? s : 1.0 - s));
			q[2] = EdgePoint(*(e[2]), (ew[2] != 1 ? r : 1.0 - r));
			q[3] = EdgePoint(*(e[3]), (ew[3] != 1 ? s : 1.0 - s));

			vec3d p = q[0] * (1 - s) + q[1] * r + q[2] * s + q[3] * (1 - r) \
				- (y[0] * N1 + y[1] * N2 + y[2] * N3 + y[3] * N4);

			// if this point should be on a sphere, project it to the sphere
			// since the transfinite interpolation does not respect speres exactly
			if (isSphere)
			{
				vec3d t = p - sphereCenter; t.Normalize();
				p = sphereCenter + t * sphereRadius;
			}

			GLMesh::NODE& n = m.Node(j * (M + 1) + i);
			n.r = p;
			n.pid = -1;
		}

	m.Node(0).pid = o.Node(f.m_node[0])->GetLocalID();
	m.Node(M).pid = o.Node(f.m_node[1])->GetLocalID();
	m.Node(M * (M + 1)).pid = o.Node(f.m_node[3])->GetLocalID();
	m.Node((M + 1) * (M + 1) - 1).pid = o.Node(f.m_node[2])->GetLocalID();

	// build the faces
	for (int j = 0; j < M; ++j)
		for (int i = 0; i < M; ++i)
		{
			GMesh::FACE& f0 = m.Face(j * (2 * M) + 2 * i);
			GMesh::FACE& f1 = m.Face(j * (2 * M) + 2 * i + 1);

			f0.n[0] = i * (M + 1) + j;
			f0.n[1] = (i + 1) * (M + 1) + j + 1;
			f0.n[2] = (i + 1) * (M + 1) + j;
			f0.pid = f.GetLocalID();

			f1.n[0] = (i + 1) * (M + 1) + j + 1;
			f1.n[1] = i * (M + 1) + j;
			f1.n[2] = i * (M + 1) + j + 1;
			f1.pid = f.GetLocalID();
		}

	// build the edges
	for (int i = 0; i < M; ++i)
	{
		GMesh::EDGE& e = m.Edge(i);
		e.n[0] = i;
		e.n[1] = i + 1;
		e.pid = o.Edge(f.m_edge[0].nid)->GetLocalID();
	}

	for (int i = 0; i < M; ++i)
	{
		GMesh::EDGE& e = m.Edge(M + i);
		e.n[0] = (i + 1) * (M + 1) - 1;
		e.n[1] = (i + 2) * (M + 1) - 1;
		e.pid = o.Edge(f.m_edge[1].nid)->GetLocalID();
	}

	for (int i = 0; i < M; ++i)
	{
		GMesh::EDGE& e = m.Edge(2 * M + i);
		e.n[0] = (M + 1) * (M + 1) - 1 - i;
		e.n[1] = (M + 1) * (M + 1) - 1 - i - 1;
		e.pid = o.Edge(f.m_edge[2].nid)->GetLocalID();
	}

	for (int i = 0; i < M; ++i)
	{
		GMesh::EDGE& e = m.Edge(3 * M + i);
		e.n[0] = (M - i) * (M + 1);
		e.n[1] = (M - i - 1) * (M + 1);
		e.pid = o.Edge(f.m_edge[3].nid)->GetLocalID();
	}
	glmesh->Attach(m, false);
}
