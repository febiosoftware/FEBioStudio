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
#include "FEFillQuadHole.h"
#include <MeshLib/FSNodeNodeList.h>
#include <MeshLib/MeshTools.h>
#include <MeshLib/FSSurfaceMesh.h>
#include <MeshLib/FSNodeEdgeList.h>
#include <FECore/matrix.h>
#include <limits.h>
using namespace std;

//-----------------------------------------------------------------------------
bool FEFillQuadHole::EdgeRing::contains(int inode)
{
	for (int i = 0; i < m_node.size(); ++i)
		if (inode == m_node[i]) return true;
	return false;
}

//-----------------------------------------------------------------------------
FEFillQuadHole::FEFillQuadHole() : FESurfaceModifier("Fill quad hole") 
{
}

//-----------------------------------------------------------------------------
// Create a new mesh where the hole is filled. The hole is defined by a node
// that lies on the edge of the hole.
FSSurfaceMesh* FEFillQuadHole::Apply(FSSurfaceMesh* pm)
{
	// make sure this is a quad mesh
	if (pm->IsType(FE_FACE_QUAD4) == false) return nullptr;

	// find a selected node
	int inode = -1;
	for (int i = 0; i < pm->Nodes(); i++) { if (pm->Node(i).IsSelected()) { inode = i; break; } }
	if (inode == -1) return nullptr;

	// build the node normals
	BuildNodalNormals(*pm);

	// find the ring that this node belongs to
	// where a ring is a closed loop of ordered edges
	EdgeRing ring;
	if (FindEdgeRing(*pm, inode, ring) == false) return 0;
	if (ring.empty()) return 0;

	// divide the ring
	vector<QUAD> quad_list;
	if (DivideRing(ring, quad_list) == false) return 0;

	// create a copy of the original mesh
	FSSurfaceMesh* pnew = new FSSurfaceMesh(*pm);

	// update the mesh
	UpdateMesh(*pnew, quad_list);

	// done
	return pnew;
}

//-----------------------------------------------------------------------------
// fill all holes
void FEFillQuadHole::FillAllHoles(FSSurfaceMesh* pm)
{
	// clear tags
	pm->TagAllNodes(0);

	// build the node-edge table
	m_NEL.Build(pm);

	// build the node normals
	BuildNodalNormals(*pm);

	// tag all the nodes that are on edge boundaries
	pm->TagAllNodes(0);
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		int fe = face.Edges();
		for (int j = 0; j < fe; ++j)
		{
			if (face.m_nbr[j] == -1)
			{
				FSEdge ej = face.GetEdge(j);
				pm->Node(ej.n[0]).m_ntag += 1;
				pm->Node(ej.n[1]).m_ntag += 1;
			}
		}
	}

	// find the edge rings
	vector<EdgeRing> ring;
	setProgress(0.0);
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		if (pm->Node(i).m_ntag > 0)
		{
			EdgeRing ri;
			if (FindEdgeRing(*pm, i, ri))
			{
				ring.push_back(ri);
				for (int j = 0; j < ri.size(); ++j) pm->Node(ri[j]).m_ntag -= 2;
			}
		}
		setProgress(100.0 * (i + 1.0) / (double)pm->Nodes());
	}

	SetError("Found %d holes", ring.size());

	int fixedHoles = 0;
	vector<QUAD> quad_list;
	for (int i = 0; i < ring.size(); ++i)
	{
		vector<QUAD> new_quads;
		if (DivideRing(ring[i], new_quads))
		{
			fixedHoles++;
			quad_list.insert(quad_list.end(), new_quads.begin(), new_quads.end());
		}
	}
	SetError("Fixed %d holes", fixedHoles);

	// allocate room for the new faces
	UpdateMesh(*pm, quad_list);
}

//-----------------------------------------------------------------------------
void FEFillQuadHole::UpdateMesh(FSSurfaceMesh& mesh, const std::vector<QUAD>& quad_list)
{
	int NF = mesh.Faces();
	int new_faces = quad_list.size();
	mesh.Create(0, 0, NF + new_faces);

	// insert the new triangles into the mesh
	for (int i = 0; i < new_faces; ++i)
	{
		FSFace& face = mesh.Face(i + NF);
		const QUAD& fi = quad_list[i];
		face.SetType(FE_FACE_QUAD4);
		face.n[0] = fi.n[0];
		face.n[1] = fi.n[1];
		face.n[2] = fi.n[2];
		face.n[3] = fi.n[3];
		face.m_gid = 0;
	}

	// next, we use the auto-mesher to reconstruct all faces, edges and nodes
	mesh.RebuildMesh();

	// see how many new faces we have
	SetError("Inserted %d new faces", new_faces);
}

//-----------------------------------------------------------------------------
void FEFillQuadHole::BuildNodalNormals(FSSurfaceMesh& mesh)
{
	// build the node normals
	m_node_normals.assign(mesh.Nodes(), vec3d(0, 0, 0));
	for (int i = 0; i < mesh.Faces(); i++)
	{
		FSFace& Face = mesh.Face(i);
		vec3d Nf = mesh.FaceNormal(i);
		for (int j = 0; j < Face.Nodes(); j++)
		{
			m_node_normals[Face.n[j]] += Nf;
		}
	}
	for (int i = 0; i < m_node_normals.size(); ++i) m_node_normals[i].Normalize();
}

//-----------------------------------------------------------------------------
vec3d FEFillQuadHole::edgeVector(FSEdge& e, FSSurfaceMesh& mesh)
{
	vec3d a = mesh.Node(e.n[0]).pos();
	vec3d b = mesh.Node(e.n[1]).pos();
	vec3d v = b - a;
	v.Normalize();
	return v;
}

//-----------------------------------------------------------------------------
bool FEFillQuadHole::FindEdgeRing(FSSurfaceMesh& mesh, int inode, FEFillQuadHole::EdgeRing& ring)
{
	// let's make sure the ring is empty
	ring.clear();

	mesh.TagAllEdges(0);

	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		int ne = face.Edges();
		for (int j = 0; j < ne; ++j)
		{
			if (face.m_nbr[j] == -1)
			{
				int ef = face.m_edge[j];
				if (ef >= 0) mesh.Edge(ef).m_ntag = 1;
			}
		}
	}

	// find an edge that this node belongs to
	int iedge = -1;
	for (int i = 0; i < mesh.Edges(); ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		if (edge.m_ntag == 1)
		{
			if ((edge.n[0] == inode) || (edge.n[1] == inode))
			{
				iedge = i;
				break;
			}
		}
	}
	if (iedge == -1) return false;

	// the tricky part is that we need to figure out the winding of the ring
	// so that we can create the faces with the correct orientation. The way
	// we do this is by first identifying a face that has this edge and then
	// see what the winding is of this face
	int iface = -1;
	FSEdge& edge = mesh.Edge(iedge);
	ring.m_winding = 0;
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		int ne = face.Edges();
		for (int j = 0; j < ne; ++j)
		{
			int jn = (j + 1) % ne;
			if ((face.n[j] == edge.n[0]) && (face.n[jn] == edge.n[1]))
			{
				// if we get here the edge is in the same direction
				ring.m_winding = -1;
				break;
			}
			else if ((face.n[j] == edge.n[1]) && (face.n[jn] == edge.n[0]))
			{
				// if we get here the edge is in the opposite winding
				ring.m_winding = 1;
				break;
			}
		}

		// we stop if we figured out the winding
		if (ring.m_winding != 0) break;
	}

	// add the initial node to the ring as a starting point
	int jnode = inode;
	ring.add(jnode, mesh.Node(jnode).r, m_node_normals[jnode]);

	// see in which direction we will be looping and if we need to flip the winding
	if (jnode == edge.n[1]) ring.m_winding *= -1;

	// now, loop over all the edges of the ring and add the nodes
	do
	{
		FSEdge& edge = mesh.Edge(iedge);
		vec3d re = edgeVector(edge, mesh);
		if (edge.n[0] == jnode)
		{
			jnode = edge.n[1];
		}
		else
		{
			jnode = edge.n[0];
			re = -re;
		}

		double minAngle = 0.0;
		int nextEdge = -1;
		int nedges = m_NEL.Edges(jnode);
		for (int k = 0; k < nedges; ++k)
		{
			int edgek = m_NEL.EdgeIndex(jnode, k);
			FSEdge& ek = mesh.Edge(edgek);
			if ((edgek != iedge) && (ek.m_ntag == 1))
			{
				if ((mesh.Node(ek.n[0]).m_ntag > 0) &&
					(mesh.Node(ek.n[1]).m_ntag > 0))
				{
					vec3d rk = edgeVector(ek, mesh);
					if (ek.n[1] == jnode)
					{
						rk = -rk;
					}
					else assert(ek.n[0] == jnode);

					double ca = re * rk;

					if ((nextEdge == -1) || (ca < minAngle))
					{
						nextEdge = edgek;
						minAngle = ca;
					}
				}
			}
		}

		// if we've reached an open-ended edge we have a problem.
		if (nextEdge == -1) return false;
		iedge = nextEdge;

		// add the node (unless we're back we're we started)
		if (jnode != inode)
		{
			// make sure the node is not already part of the ring.
			// It is possible that the ring closes on itself, but not
			// on the first node. This is an invalid topology that we cannot
			// handle. 
			if (ring.contains(jnode)) return false;
			ring.add(jnode, mesh.Node(jnode).r, m_node_normals[jnode]);
		}
	} while (jnode != inode);

	return true;
}

//------------------------------------------------------------------------------
bool isQuadConvex(const FEFillQuadHole::QUAD& q)
{
	vec3d e[4];
	for (int i = 0; i < 4; ++i)
	{
		int i0 = i;
		int i1 = (i+1)%4;
		e[i] = q.r[i1] - q.r[i0];
		e[i].Normalize();
	}

	double ang = 0.0;
	for (int i = 0; i < 4; ++i)
	{
		int i0 = i;
		int i1 = (i + 1) % 4;
		double ai = acos(e[i1] * e[i0]);
		ang += ai;
	}

	return fabs(ang/(2 * PI) - 1) < 0.1;
}

//------------------------------------------------------------------------------
double quadQuality(const FEFillQuadHole::QUAD& q)
{
	vec3d e[4];
	for (int i = 0; i < 4; ++i)
	{
		int i0 = i;
		int i1 = (i + 1) % 4;
		e[i] = q.r[i1] - q.r[i0];
		e[i].Normalize();
	}

	double cmin = 1.0;
	for (int i = 0; i < 4; ++i)
	{
		int i0 = i;
		int i1 = (i + 1) % 4;
		double ci = e[i1] * e[i0];
		if (ci < cmin) cmin = ci;
	}

	return cmin;
}

//------------------------------------------------------------------------------
bool FEFillQuadHole::DivideRing(EdgeRing& ring, vector<QUAD>& quad_list)
{
	// make sure this ring has at least four nodes
	assert(ring.size() >= 4);
	if (ring.size() < 4) return false;

	// if the ring has only four nodes, we define a new face
	if (ring.size() == 4)
	{
		QUAD f;
		if (ring.m_winding == 1)
		{
			f.n[0] = ring[0]; f.r[0] = ring.m_r[0];
			f.n[1] = ring[1]; f.r[1] = ring.m_r[1];
			f.n[2] = ring[2]; f.r[2] = ring.m_r[2];
			f.n[3] = ring[3]; f.r[3] = ring.m_r[3];
		}
		else
		{
			f.n[0] = ring[3]; f.r[0] = ring.m_r[3];
			f.n[1] = ring[2]; f.r[1] = ring.m_r[2];
			f.n[2] = ring[1]; f.r[2] = ring.m_r[1];
			f.n[3] = ring[0]; f.r[3] = ring.m_r[0];
		}
		quad_list.push_back(f);
		return true;
	}

	// find the best ear to cut off
	int N = ring.size();
	int i0max = -1;
	double Qmax = -1;
	EdgeRing left, right;
	for (int i = 0; i < N; ++i)
	{
		int i0 = i;
		int i1 = (i + 1) % N;
		int i2 = (i + 2) % N;
		int i3 = (i + 3) % N;
		QUAD quad;
		quad.n[0] = ring.m_node[i0];
		quad.n[1] = ring.m_node[i1];
		quad.n[2] = ring.m_node[i2];
		quad.n[3] = ring.m_node[i3];
		quad.r[0] = ring.m_r[i0];
		quad.r[1] = ring.m_r[i1];
		quad.r[2] = ring.m_r[i2];
		quad.r[3] = ring.m_r[i3];

		// make sure that this quad is convex 
		if (isQuadConvex(quad))
		{
			// calculate the quality metric
			double Q = quadQuality(quad);

			// if it's better, store it
			if (Q > Qmax)
			{
				i0max = i0;
				Qmax = Q;
			}
		}
	}

	assert(i0max != -1);
	if (i0max == -1) return false;
	if (i0max != -1)
	{
		int i0 = i0max;
		int i3 = (i0max + 3) % N;

		// get the left and right ears
		ring.GetLeftEar (i0, i3, left);
		ring.GetRightEar(i0, i3, right);

		vector<QUAD> quad_left, quad_right;
		bool bret1 = DivideRing(left , quad_left);
		bool bret2 = DivideRing(right, quad_right);

		if ((bret1 == false) || (bret2 == false)) return false;

		// merge the lists
		quad_list.insert(quad_list.end(), quad_left .begin(), quad_left.end());
		quad_list.insert(quad_list.end(), quad_right.begin(), quad_right.end());
	}
	return (quad_list.empty() == false);
}

//-----------------------------------------------------------------------------
// Get the right ear of this ring
void FEFillQuadHole::EdgeRing::GetRightEar(int n0, int n1, EdgeRing& ear)
{
	ear.clear();

	// add the first node
	ear.add(m_node[n0], m_r[n0], m_normal[n0]);

	// add the inside nodes
	int n = n0 + 1;
	if (n >= size()) n = 0;
	do
	{
		ear.add(m_node[n], m_r[n], m_normal[n]);
		n++;
		if (n >= size()) n = 0;
	} while (n != n1);

	// add the last node
	ear.add(m_node[n1], m_r[n1], m_normal[n1]);

	// the ear is wound in the same direction
	ear.m_winding = m_winding;
}

//-----------------------------------------------------------------------------
// Get the left ear of this ring
void FEFillQuadHole::EdgeRing::GetLeftEar(int n0, int n1, EdgeRing& ear)
{
	GetRightEar(n1, n0, ear);
}
