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
#include "FEFillHole.h"
#include <MeshLib/FSNodeNodeList.h>
#include <MeshLib/MeshTools.h>
#include <MeshLib/FSSurfaceMesh.h>
#include <MeshLib/FSNodeEdgeList.h>
#include <FECore/matrix.h>
#include <limits.h>
using namespace std;

//-----------------------------------------------------------------------------
bool FEFillHole::EdgeRing::contains(int inode)
{
	for (int i = 0; i < m_node.size(); ++i)
		if (inode == m_node[i]) return true;
	return false;
}

//-----------------------------------------------------------------------------
FEFillHole::FEFillHole() : FESurfaceModifier("Fill hole")
{
	m_optimize = false;
	m_insertNodes = false;
}

//-----------------------------------------------------------------------------
// Create a new mesh where the hole is filled. The hole is defined by a node
// that lies on the edge of the hole.
FSSurfaceMesh* FEFillHole::Apply(FSSurfaceMesh* pm)
{
	// build the node normals
	m_node_normals.assign(pm->Nodes(), vec3d(0, 0, 0));
	for (int i = 0; i < pm->Faces(); i++)
	{
		FSFace &Face = pm->Face(i);
		for (int j = 0; j < Face.Nodes(); j++)
		{
			m_node_normals[Face.n[j]] += to_vec3d(Face.m_nn[j]);
		}
	}
	for (int i = 0; i < m_node_normals.size(); ++i) m_node_normals[i].Normalize();

	// find a selected node
	int inode = -1;
	for (int i=0; i<pm->Nodes(); i++) { if (pm->Node(i).IsSelected()) { inode = i; break; }}
	if (inode == -1) return 0;

	// find the ring that this node belongs to
	// where a ring is a closed loop of ordered edges
	EdgeRing ring;
	if (FindEdgeRing(*pm, inode, ring) == false) return 0;
	if (ring.empty()) return 0;

	if (m_insertNodes)
	{
		// AFM Method
		vector<FACE> tri_list;
		vector<vec3d>node_list;

		if (AFM(*pm,ring, tri_list, node_list) == false) return 0;
		// see how many new faces we have
		int new_faces = (int) tri_list.size();
		int new_nodes = (int) node_list.size();
		// create a copy of the original mesh
		FSSurfaceMesh* pnew = new FSSurfaceMesh(*pm);

		// allocate room for the new faces
		int NF = pnew->Faces();
		int NN = pnew->Nodes();
		pnew->Create(NN + new_nodes, 0, NF + new_faces);

		// insert the new triangles into the mesh
		for (int i=0; i<new_faces; ++i)
		{
			FSFace& face = pnew->Face(i + NF);
			FACE& fi = tri_list[i];
			face.SetType(FE_FACE_TRI3);
			face.n[0] = fi.n[0];
			face.n[1] = fi.n[1];
			face.n[2] = fi.n[2];
			face.m_gid = 0;
		}

		// calculate the node positions
		for (int i=0; i<new_nodes; ++i)
		{
			FSNode& nd = pnew->Node(i+NN);
			nd.r = node_list[i];
		}

		// next, we use the auto-mesher to reconstruct all faces, edges and nodes
		pnew->RebuildMesh();

		// done
		return pnew;
	}
	else
	{
		// divide the rings using recursive method
		vector<FACE> tri_list;
		if (m_optimize)
		{
			if (DivideRing(ring, tri_list) == false) return 0;
		}
		else
		{
			if (DivideRing1(ring, tri_list) == false) return 0;
		}

		// see how many new faces we have
		int new_faces = (int) tri_list.size();

		// create a copy of the original mesh
		FSSurfaceMesh* pnew = new FSSurfaceMesh(*pm);

		// allocate room for the new faces
		int NF = pnew->Faces();
		pnew->Create(0, 0, NF + new_faces);

		// insert the new triangles into the mesh
		for (int i=0; i<new_faces; ++i)
		{
			FSFace& face = pnew->Face(i + NF);
			FACE& fi = tri_list[i];
			face.SetType(FE_FACE_TRI3);
			face.n[0] = fi.n[0];
			face.n[1] = fi.n[1];
			face.n[2] = fi.n[2];
			face.m_gid = 0;
		}

		// next, we use the auto-mesher to reconstruct all faces, edges and nodes
		pnew->RebuildMesh();

		// done
		return pnew;
	}
}

//-----------------------------------------------------------------------------
// fill all holes
void FEFillHole::FillAllHoles(FSSurfaceMesh* pm)
{
	// clear tags
	pm->TagAllNodes(0);

	// build the node-edge table
	m_NEL.Build(pm);

	// build the node normals
	m_node_normals.assign(pm->Nodes(), vec3d(0, 0, 0));
	for (int i = 0; i < pm->Faces(); i++)
	{
		FSFace &Face = pm->Face(i);
		for (int j = 0; j < Face.Nodes(); j++)
		{
			m_node_normals[Face.n[j]] += to_vec3d(Face.m_nn[j]);
		}
	}
	for (int i = 0; i < m_node_normals.size(); ++i) m_node_normals[i].Normalize();

	// tag all the nodes that are on edge boundaries
	pm->TagAllNodes(0);
	for (int i=0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		int fe = face.Edges();
		for (int j=0; j<fe; ++j)
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
	for (int i=0; i<pm->Nodes(); ++i)
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
		setProgress(100.0*(i + 1.0) / (double)pm->Nodes());
	}

	SetError("Found %d holes", ring.size());

	int fixedHoles = 0;
	vector<FACE> tri_list;
	for (int i=0; i < ring.size(); ++i)
	{
		vector<FACE> tri;
		if (DivideRing2(ring[i], tri))
		{
			fixedHoles++;
			tri_list.insert(tri_list.end(), tri.begin(), tri.end());
		}
	}
	SetError("Fixed %d holes", fixedHoles);

	// see how many new faces we have
	int new_faces = (int) tri_list.size();
	SetError("Inserted %d new faces", new_faces);

	// allocate room for the new faces
	int NF = pm->Faces();
	pm->Create(0, 0, NF + new_faces);

	// insert the new triangles into the mesh
	for (int i=0; i<new_faces; ++i)
	{
		FSFace& face = pm->Face(i + NF);
		FACE& fi = tri_list[i];
		face.SetType(FE_FACE_TRI3);
		face.n[0] = fi.n[0];
		face.n[1] = fi.n[1];
		face.n[2] = fi.n[2];
		face.m_gid = 0;
	}

	// next, we use the auto-mesher to reconstruct all faces, edges and nodes
	pm->RebuildMesh();
}

//-----------------------------------------------------------------------------
// find the fitting sphere of a triangle
bool findCircumSphere(const vec3d* r, vec3d& c, double& R)
{
	vec3d p[2];
	p[0] = r[1] - r[0];
	p[1] = r[2] - r[0];
	vec3d n = p[0] ^ p[1];

	// setup linear system of equation
	matrix A(3, 3);
	A[0][0] = 2.0*p[0].x; A[0][1] = 2.0*p[0].y; A[0][2] = 2.0*p[0].z;
	A[1][0] = 2.0*p[1].x; A[1][1] = 2.0*p[1].y; A[1][2] = 2.0*p[1].z;
	A[2][0] = n.x; A[2][1] = n.y; A[2][2] = n.z;

	vector<double> y(3);
	y[0] = p[0].SqrLength();
	y[1] = p[1].SqrLength();
	y[2] = 0.0;

	// solve it
	vector<double> x(3);
	A.solve(x, y);

	// this will give us the center of the sphere
	c = vec3d(x[0], x[1], x[2]);

	// now evaluate the radius
	R = c.Length();

	// don't forget to add r0
	c += r[0];

	return true;
}

//-----------------------------------------------------------------------------
inline bool IsInsideSphere(const vec3d& r, const vec3d& sphereCenter, double sphereRadius)
{
	return ((r - sphereCenter).SqrLength() < sphereRadius*sphereRadius);
}

//-----------------------------------------------------------------------------
vec3d edgeVector(FSEdge& e, FSSurfaceMesh& mesh)
{
	vec3d a = mesh.Node(e.n[0]).pos();
	vec3d b = mesh.Node(e.n[1]).pos();
	vec3d v = b - a;
	v.Normalize();
	return v;
}

//-----------------------------------------------------------------------------
bool FEFillHole::FindEdgeRing(FSSurfaceMesh& mesh, int inode, FEFillHole::EdgeRing& ring)
{
	// let's make sure the ring is empty
	ring.clear();

	mesh.TagAllEdges(0);

	for (int i=0; i<mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		int ne = face.Edges();
		for (int j=0; j<ne; ++j)
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
	for (int i=0; i<mesh.Edges(); ++i)
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
	for (int i=0; i<mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		for (int j=0; j<3; ++j)
		{
			int jn = (j+1)%3;
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
	}
	while (jnode != inode);

	return true;
}

//------------------------------------------------------------------------------
bool FEFillHole::DivideRing(EdgeRing& ring, vector<FACE>& tri_list)
{
	// make sure this ring has at least three nodes
	assert(ring.size() >= 3);
	if (ring.size() < 3) return false;

	// if the ring has only three nodes, we define a new face
	if (ring.size() == 3)
	{
		FACE f;
		if (ring.m_winding == 1)
		{
			f.n[0] = ring[0]; f.r[0] = ring.m_r[0];
			f.n[1] = ring[1]; f.r[1] = ring.m_r[1];
			f.n[2] = ring[2]; f.r[2] = ring.m_r[2];
		}
		else
		{
			f.n[0] = ring[2]; f.r[0] = ring.m_r[2];
			f.n[1] = ring[1]; f.r[1] = ring.m_r[1];
			f.n[2] = ring[0]; f.r[2] = ring.m_r[0];
		}
		tri_list.push_back(f);
		return true;
	}

	// get the normal to this ring
	vec3d t = RingNormal(ring);

	if (ring.size() > 10)
	{
		// if the ring is too large, finding the optimal number of iterations is too challenging,
		// so we use a quick and dirty method. 
		// We pick a node, and find the node that is farthest away. If those two nodes split the 
		// ring in a valid split, then we divide and conquer. If not, we pick the next node and repeat.
		const int N = (const int) ring.size();
		for (int i=0; i<N; ++i)
		{
			int ni = (i+N/2)%N;
			vec3d ri = ring.m_r[ni];
			double Dmax = 0;
			int jmax = 0;
			for (int j=0; j<N; ++j)
			{
				vec3d rj = ring.m_r[j];
				double D = (ri - rj)*(ri - rj);
				if (D > Dmax)
				{
					Dmax = D;
					jmax = j;
				}
			}

			// get the dividing plane normal
			vec3d rj = ring.m_r[jmax];
			vec3d d = ri - rj;
			vec3d pn = d ^ t;

			// get the left and right ears
			EdgeRing left, right;
			ring.GetLeftEar (ni, jmax, left);
			ring.GetRightEar(ni, jmax, right);

			// make sure we actually cut off an ear
			assert(left.size() > 2);
			assert(right.size() > 2);

			// see if this split is valid
			if (IsValidSplit(left, right, ri, pn))
			{
				vector<FACE> tri_left, tri_right;
				bool bret1 = DivideRing(left , tri_left);
				bool bret2 = DivideRing(right, tri_right);
				if (bret1 && bret2)
				{
					// merge the lists
					tri_list.insert(tri_list.end(), tri_left.begin(), tri_left.end());
					tri_list.insert(tri_list.end(), tri_right.begin(), tri_right.end());
				}

				break;
			}
		}
	}
	else
	{
		// determine the optimal triangulation
		vector<FACE> tri_optimal;	// the optimal triangulation
		vector<FACE> tri_current;	// the current triangulation

		EdgeRing left, right;

		// if we get here, try to divide the ring in two rings and apply the division recursively
		double amax = 0.0;
		int N = ring.size();
		int N1 = (N%2==0?N/2:N/2+1);
		for (int i=0; i<N1; ++i)
		{
			for (int j=2; j<N-1; ++j)
			{
				// clear the current triangulation
				tri_current.clear();

				// get the dividing plane normal
				vec3d ri = ring.m_r[i      ];
				vec3d rj = ring.m_r[(i+j)%N];
				vec3d d = ri - rj;
				vec3d pn = d ^ t;

				// get the left and right ears
				ring.GetLeftEar (i, (i+j)%N, left);
				ring.GetRightEar(i, (i+j)%N, right);

				// make sure we actually cut off an ear
				assert(left.size() > 2);
				assert(right.size() > 2);

				// see if this split is valid
				if (IsValidSplit(left, right, ri, pn))
				{
					vector<FACE> tri_left, tri_right;
					bool bret1 = DivideRing(left , tri_left);
					bool bret2 = DivideRing(right, tri_right);
					if (bret1 && bret2)
					{
						// merge the lists
						tri_current.insert(tri_current.end(), tri_left.begin(), tri_left.end());
						tri_current.insert(tri_current.end(), tri_right.begin(), tri_right.end());

						// get the minimum quality
	//					double amin = min_tri_area(tri_current);
						double amin = min_tri_quality(tri_current);

						// get the minimum quality
						if (amin > amax) 
						{
							tri_optimal = tri_current;
							amax = amin;
						}
					}
				}
			}
		}

		// store the optimal triangulation
		tri_list = tri_optimal;
	}

	return (tri_list.empty() == false);
}

//------------------------------------------------------------------------------
bool FEFillHole::DivideRing1(EdgeRing& ring, vector<FACE>& tri_list)
{
	// make sure this ring has at least three nodes
	assert(ring.size() >= 3);
	if (ring.size() < 3) return false;

	// if the ring has only three nodes, we define a new face
	if (ring.size() == 3)
	{
		FACE f;
		if (ring.m_winding == 1)
		{
			f.n[0] = ring[0]; f.r[0] = ring.m_r[0];
			f.n[1] = ring[1]; f.r[1] = ring.m_r[1];
			f.n[2] = ring[2]; f.r[2] = ring.m_r[2];
		}
		else
		{
			f.n[0] = ring[2]; f.r[0] = ring.m_r[2];
			f.n[1] = ring[1]; f.r[1] = ring.m_r[1];
			f.n[2] = ring[0]; f.r[2] = ring.m_r[0];
		}
		tri_list.push_back(f);
		return true;
	}

	// get the normal to this ring
	vec3d t = RingNormal(ring);

	vector<FACE> tri_optimal;	// the optimal triangulation
	vector<FACE> tri_current;	// the current triangulation

	// if we get here, try to divide the ring in two rings and apply the division recursively
	double amax = 0.0;
	int N = ring.size();
	int N1 = (N%2==0?N/2:N/2+1);
	EdgeRing left, right;
	for (int i=0; i<N1; ++i)
	{
		for (int j=N/2; j<N-1; ++j)
		{
			if (tri_optimal.size() == N-2)
				break;
			// clear the current triangulation
			tri_current.clear();

			// get the dividing plane normal
			vec3d ri = ring.m_r[i      ];
			vec3d rj = ring.m_r[(i+j)%N];
			vec3d d = ri - rj;
			vec3d pn = d ^ t;

			// get the left and right ears
			ring.GetLeftEar (i, (i+j)%N, left);
			ring.GetRightEar(i, (i+j)%N, right);

			// make sure we actually cut off an ear
			assert(left.size() > 2);
			assert(right.size() > 2);

			// see if this split is valid
			if (IsValidSplit(left, right, ri, pn))
			{
				vector<FACE> tri_left, tri_right;
				bool bret1 = DivideRing1(left , tri_left);
				bool bret2 = DivideRing1(right, tri_right);
				if (bret1 && bret2)
				{
					// merge the lists
					tri_current.insert(tri_current.end(), tri_left.begin(), tri_left.end());
					tri_current.insert(tri_current.end(), tri_right.begin(), tri_right.end());

					// get the minimum quality
//					double amin = min_tri_area(tri_current);
					double amin = min_tri_quality(tri_current);

					// get the minimum quality
					if (amin > amax) 
					{
						tri_optimal = tri_current;
						amax = amin;
					}
				}
			}
		}
	}

	// store the optimal triangulation
	tri_list = tri_optimal;
	return (tri_list.empty() == false);
}

//------------------------------------------------------------------------------
bool FEFillHole::DivideRing2(EdgeRing& ring, vector<FACE>& tri_list)
{
	// make sure this ring has at least three nodes
	assert(ring.size() >= 3);
	if (ring.size() < 3) return false;

	// if the ring has only three nodes, we define a new face
	if (ring.size() == 3)
	{
		FACE f;
		if (ring.m_winding == 1)
		{
			f.n[0] = ring[0]; f.r[0] = ring.m_r[0];
			f.n[1] = ring[1]; f.r[1] = ring.m_r[1];
			f.n[2] = ring[2]; f.r[2] = ring.m_r[2];
		}
		else
		{
			f.n[0] = ring[2]; f.r[0] = ring.m_r[2];
			f.n[1] = ring[1]; f.r[1] = ring.m_r[1];
			f.n[2] = ring[0]; f.r[2] = ring.m_r[0];
		}
		tri_list.push_back(f);
		return true;
	}

	// calculate the angles
	int N = ring.size();
	EdgeRing left, right;
	for (int i = 0; i < N; ++i)
	{
		int i0 = i;
		int i1 = (i + 1) % N;
		int i2 = (i + 2) % N;
		vec3d r[3];
		r[0] = ring.m_r[i0];
		r[1] = ring.m_r[i1];
		r[2] = ring.m_r[i2];

		// make sure that r2 is on positive side of plane
		vec3d n = m_node_normals[ring.m_node[i0]];
		vec3d t = (r[1] - r[0]) ^ n;
		t.Normalize();
		if (ring.m_winding == 1) t = -t;
		if ((r[2] - r[1])*t > 0)
		{
			// see if this cuts off an ear
			vec3d c;
			double R;
			if (findCircumSphere(r, c, R))
			{
				// make sure none of the other points lie in this sphere
				bool valid = true;
				for (int j = 0; j < N; ++j)
				{
					if ((j != i0) && (j != i1) && (j != i2))
					{
						vec3d rj = ring.m_r[j];
						if (IsInsideSphere(rj, c, R))
						{
							valid = false;
							break;
						}
					}
				}

				if (valid)
				{
					// get the left and right ears
					ring.GetLeftEar(i0, i2, left);
					ring.GetRightEar(i0, i2, right);

					vector<FACE> tri_left, tri_right;
					bool bret1 = DivideRing2(left, tri_left);
					bool bret2 = DivideRing2(right, tri_right);

					if (bret1 == false) bret1 = DivideRing1(left, tri_left);
					if (bret2 == false) bret2 = DivideRing1(right, tri_right);

					// merge the lists
					if (bret1) tri_list.insert(tri_list.end(), tri_left.begin(), tri_left.end());
					if (bret2) tri_list.insert(tri_list.end(), tri_right.begin(), tri_right.end());

					break;
				}
			}
		}
	}

	if (tri_list.empty())
	{
		DivideRing1(ring, tri_list);
	}

	return (tri_list.empty() == false);
}

//-----------------------------------------------------------------------------
// Calculate the normal of the plane that approximate passes through the ring
vec3d FEFillHole::RingNormal(FEFillHole::EdgeRing& ring)
{
	int N = (int) ring.size();
	std::vector<vec3d>& r = ring.m_r;

	// find the plane of the ring
	// p = point on plane (here center of ring)
	vec3d p(r[0]);
	for (int i=1; i<N; ++i) p += r[i];
	p /= (double) N;

	// get the (approximate) plane normal
	vec3d t(0,0,0);
	for (int i=0; i<N-1; ++i) t += (p - r[i]) ^ (p - r[i+1]);
	t.Normalize();

	return t;
}

//-----------------------------------------------------------------------------
// Determine if this is a valid split. The split is valid if the ears lie on opposite
// side of the plane defined by (p,t)
bool FEFillHole::IsValidSplit(FEFillHole::EdgeRing& left, FEFillHole::EdgeRing& right, const vec3d& p, const vec3d& t)
{
	int n0 = GetPlaneOrientation(left , p, t);
	int n1 = GetPlaneOrientation(right, p, t);
	
	// if the ears are on either side of the plane, the product of the signs should be -1
	if (n0 * n1 < 0) return true;
	else return false;
}

//-----------------------------------------------------------------------------
// Figure out which side of the plane the ring lies. Returns +1 of the ring lies
// on the positive side, -1 if it lies on the negative side and 0 if it lies on boths sides
int FEFillHole::GetPlaneOrientation(FEFillHole::EdgeRing& ring, const vec3d& p, const vec3d& t)
{
	// loop over all the points in the ring, except the first and last
	// (we skip the first and last since they should be on the plane by design)
	int N = (int) ring.size();
	int nsign = 0;
	for (int i=1; i<N-1; ++i)
	{
		vec3d& ri = ring.m_r[i];
		double d = t*(ri - p);
		if (i==1) nsign = (d > 0 ? 1 : -1);
		else
		{
			int si = (d > 0 ? 1 : -1);
			if (si != nsign) return 0;
		}
	}
	return nsign;
}

//-----------------------------------------------------------------------------
// Find the quality of the worst triangle
double FEFillHole::min_tri_quality(vector<FACE>& tri)
{
	double qmin = 0.0;
	for (int i=0; i<(int)tri.size(); ++i)
	{
		double q = TriangleQuality(tri[i].r);
		if (i==0) qmin = q; else qmin = (q<qmin ? q : qmin);
	}
	return qmin;
}

//-----------------------------------------------------------------------------
// find the area of the smallest triangle
double FEFillHole::min_tri_area(vector<FACE>& tri)
{
	double amin = 0.0;
	for (int i=0; i<(int)tri.size(); ++i)
	{
		FACE& fi = tri[i];

		vec3d n = (fi.r[1] - fi.r[0])^(fi.r[2] - fi.r[0]);
		double a = n*n;

		if (i==0) amin = a; else amin = (a < amin ? a : amin);
	}
	return amin;
}

//-----------------------------------------------------------------------------
// Get the right ear of this ring
void FEFillHole::EdgeRing::GetRightEar(int n0, int n1, EdgeRing& ear)
{
	ear.clear();

	// add the first node
	ear.add(m_node[n0], m_r[n0],m_normal[n0]);

	// add the inside nodes
	int n = n0 + 1;
	if (n >= size()) n = 0;
	do
	{
		ear.add(m_node[n], m_r[n],m_normal[n]);
		n++;
		if (n >= size()) n = 0;
	}
	while (n != n1);

	// add the last node
	ear.add(m_node[n1], m_r[n1],m_normal[n1]);

	// the ear is wound in the same direction
	ear.m_winding = m_winding;
}

//-----------------------------------------------------------------------------
// Get the left ear of this ring
void FEFillHole::EdgeRing::GetLeftEar(int n0, int n1, EdgeRing& ear)
{
	GetRightEar(n1, n0, ear);
}


//-----------------------------------------------------------------------------
// Advancing Front Method

bool FEFillHole::AFM(FSSurfaceMesh& mesh, EdgeRing& ring, vector<FACE>& tri_list, vector<vec3d>&node_list)
{
	// make sure this ring has at least three nodes
	assert(ring.size() >= 3);
	if (ring.size() < 3) return false;

	// if the ring has only three nodes, we define a new face
	if (ring.size() == 3)
	{
		FACE f;
		if (ring.m_winding == 1)
		{
			f.n[0] = ring[0]; f.r[0] = ring.m_r[0];
			f.n[1] = ring[1]; f.r[1] = ring.m_r[1];
			f.n[2] = ring[2]; f.r[2] = ring.m_r[2];
		}
		else
		{
			f.n[0] = ring[2]; f.r[0] = ring.m_r[2];
			f.n[1] = ring[1]; f.r[1] = ring.m_r[1];
			f.n[2] = ring[0]; f.r[2] = ring.m_r[0];
		}
		tri_list.push_back(f);
		return true;
	}

	//To save all the node normals
	vector<vec3d> node_normals;
	node_normals.reserve(mesh.Nodes());
	for (int i = 0 ; i< mesh.Nodes();i++ )
	{
		vec3d temp;
		node_normals.push_back(temp);
	}
	for(int i = 0 ;i < mesh.Faces();i++)
	{
		FSFace &Face = mesh.Face(i);
		for(int j = 0; j < Face.Nodes(); j++)
		{
			node_normals[Face.n[j]] = to_vec3d(Face.m_nn[j]);
		}
	}

	FSNodeNodeList NNL(&mesh);

	int N = mesh.Nodes(); //total no of nodes.
	int new_nodes_count = 0;

	//Find Angles at each node.
	int ring_nodes = ring.size();
	double min_angle = INT_MAX;
	int index = -1;
	for(int i = 0; i<ring_nodes;i++)
	{
		vec3d a,b;
		int prev,next;
		if (i==0)
		{
			prev = ring_nodes-1;
			next = i+1;
		}
		else if (i == ring_nodes-1)
		{
			prev = i-1;
			next = 0;
		}
		else
		{
			prev = i-1;
			next = i+1;
		}
		a = ring.m_r[next] - ring.m_r[i];
		b = ring.m_r[prev] - ring.m_r[i];
		
		double cos_theta = a.Normalize() * b.Normalize();
		double angle = acos(cos_theta) * 180.0 / PI;
		//check if we want theta or 360 - theta.
		//node normal at prev and next
		if (angle < 180)
		{
			// prev node normal
			vec3d prev_nn = node_normals[ring.m_node[prev]];
			//next face normal



			vec3d np; //vector from next to previous
			if (ring.m_winding == 1)
				np = ring.m_r[next]-ring.m_r[prev];
			else
				np = ring.m_r[prev] - ring.m_r[next];
			vec3d plane_nn = prev_nn ^ np;
			double d = (plane_nn.x * ring.m_r[prev].x) + (plane_nn.y * ring.m_r[prev].y) + (plane_nn.z * ring.m_r[prev].z);
			//Side of plane
			double side = (plane_nn.x * ring.m_r[i].x) + (plane_nn.y * ring.m_r[i].y) + (plane_nn.z * ring.m_r[i].z) - d;
			if (side * ring.m_winding > 0 )
				angle = angle;
			else
				angle = 360 - angle;	
		}
		if (angle < min_angle)
		{
			min_angle = angle;
			index = i;
		}

		if(min_angle <= 85)
			break;
	}
	int prev,next;
	int current_node_id = ring.m_node[index];
	prev = index -1;
	next = index+1;
	if (index == 0)
		prev = ring_nodes-1;
	if (index == ring_nodes-1)
		next = 0;

	if (min_angle <= 85)
	{
		//Add a new face but no new node
		FACE f;
		if (ring.m_winding == 1)
		{
			f.n[0] = ring[prev]; f.r[0] = ring.m_r[prev];
			f.n[1] = ring[index]; f.r[1] = ring.m_r[index];
			f.n[2] = ring[next]; f.r[2] = ring.m_r[prev];
		}
		else
		{
			f.n[0] = ring[next]; f.r[0] = ring.m_r[next];
			f.n[1] = ring[index]; f.r[1] = ring.m_r[index];
			f.n[2] = ring[prev]; f.r[2] = ring.m_r[prev];
		}
		tri_list.push_back(f);
	}
	else if (min_angle > 85 && min_angle <= 135 )
	{
		//Add 1 new node
		//Find concave or convex
		double weight = NNL.Valence(current_node_id);
		vec3d laplacian;
		for(int k = 0; k< NNL.Valence(current_node_id);k++)
		{
			vec3d neigh_node = mesh.Node(NNL.Node(current_node_id,k)).r;
			neigh_node = neigh_node * (1/weight);
			laplacian += (neigh_node - ring.m_r[index] ); 
		}
		weight = node_normals[current_node_id] * laplacian;
		bool concave;
		if (weight <= 0)
			concave = false;
		else
			concave = true;

		vec3d v_new = newNode(ring.m_r[index], ring.m_r[next], ring.m_r[prev],node_normals[current_node_id], 0.5 ,concave);
		node_list.push_back(v_new);

		int new_node_id = N + new_nodes_count;

		FACE f1, f2;
		if (ring.m_winding == 1)
		{
			f1.n[0] = ring[prev]; f1.r[0] = ring.m_r[prev];
			f1.n[1] = ring[index]; f1.r[1] = ring.m_r[index];
			f1.n[2] = new_node_id; f1.r[2] = v_new;

			f2.n[0] = new_node_id;  f2.r[0] = v_new;
			f2.n[1] = ring[index]; f2.r[1] = ring.m_r[index];
			f2.n[2] = ring[next]; f2.r[2] = ring.m_r[next];
		}
		else
		{
			f1.n[0] = new_node_id; f1.r[0] = v_new;
			f1.n[1] = ring[index]; f1.r[1] = ring.m_r[index];
			f1.n[2] = ring[prev]; f1.r[2] = ring.m_r[prev];

			f2.n[0] = ring[next];  f2.r[0] = ring.m_r[next];
			f2.n[1] = ring[index]; f2.r[1] = ring.m_r[index];
			f2.n[2] = new_node_id; f2.r[2] = v_new;
		}
		tri_list.push_back(f1);
		tri_list.push_back(f2);

		new_nodes_count +=1;

	}
	else if (min_angle > 135 && min_angle <= 180)
	{
		//Add 2 new node
		//Find concave or convex
		double weight = NNL.Valence(current_node_id);
		vec3d laplacian;
		for(int k = 0; k< NNL.Valence(current_node_id);k++)
		{
			vec3d neigh_node = mesh.Node(NNL.Node(current_node_id,k)).r;
			neigh_node = neigh_node * (1/weight);
			laplacian += (neigh_node - ring.m_r[index] ); 
		}
		weight = node_normals[current_node_id] * laplacian;
		bool concave;
		if (weight <= 0)
			concave = false;
		else
			concave = true;

		vec3d v_new_1 = newNode(ring.m_r[index], ring.m_r[next], ring.m_r[prev],node_normals[current_node_id], 0.33 ,concave);
		vec3d v_new_2 = newNode(ring.m_r[index], ring.m_r[next], ring.m_r[prev],node_normals[current_node_id], 0.66 ,concave);
		node_list.push_back(v_new_1);
		node_list.push_back(v_new_2);
		int new_node_id_1 = N + new_nodes_count;
		int new_node_id_2 = N + new_nodes_count+1;

		FACE f1, f2,f3;
		if (ring.m_winding == 1)
		{
			f1.n[0] = ring[prev]; f1.r[0] = ring.m_r[prev];
			f1.n[1] = ring[index]; f1.r[1] = ring.m_r[index];
			f1.n[2] = new_node_id_2; f1.r[2] = v_new_2;

			f2.n[0] = new_node_id_1;  f2.r[0] = v_new_1;
			f2.n[1] = ring[index]; f2.r[1] = ring.m_r[index];
			f2.n[2] = ring[next]; f2.r[2] = ring.m_r[next];

			f3.n[0] = new_node_id_2;  f3.r[0] = v_new_2;
			f3.n[1] = ring[index]; f3.r[1] = ring.m_r[index];
			f3.n[2] = new_node_id_1; f3.r[2] = v_new_1;
		}
		else
		{
			f1.n[0] = new_node_id_2; f1.r[0] = v_new_2;
			f1.n[1] = ring[index]; f1.r[1] = ring.m_r[index];
			f1.n[2] = ring[prev]; f1.r[2] = ring.m_r[prev];

			f2.n[0] = ring[next];  f2.r[0] = ring.m_r[next];
			f2.n[1] = ring[index]; f2.r[1] = ring.m_r[index];
			f2.n[2] = new_node_id_1; f2.r[2] = v_new_1;

			f3.n[0] = new_node_id_1;  f3.r[0] = v_new_1;
			f3.n[1] = ring[index]; f3.r[1] = ring.m_r[index];
			f3.n[2] = new_node_id_2; f3.r[2] = v_new_2;
		}
		tri_list.push_back(f1);
		tri_list.push_back(f2);
		tri_list.push_back(f3);
		new_nodes_count +=2;
	}
	else
		return false;

	return true;
}



vec3d FEFillHole::newNode(vec3d current_node, vec3d next_node, vec3d prev_node,vec3d node_normal, double scale, bool concave )
{
	//bijector of V(i,i+1) and V(i,i-1)
	vec3d a = next_node - current_node;
	vec3d b = prev_node - current_node; 
	vec3d v_b = a.Normalize() + b.Normalize();
	v_b = v_b * scale;
	v_b = v_b.Normalize();

	vec3d n_i = node_normal; //node normal at index node

	double temp = n_i * v_b;
		
	vec3d v_b_1 = v_b - (n_i * temp);
	v_b_1 = v_b_1.Normalize();
		
	vec3d n_i_1 = n_i + v_b * (0.45 * temp);
	n_i_1 =  n_i_1.Normalize();

	temp = n_i * n_i_1;  
	double t = n_i_1 * v_b_1;
	double k ;
	if (t == 0)
		k = (temp-1)/0.001;
	else
		k = (temp-1)/(n_i_1 * v_b_1);

	vec3d v_B;
	if (concave) //concave
		v_B = v_b_1 - (n_i_1 * k);
	else //convex
		v_B = v_b_1 + (n_i_1 * k);

	v_B = v_B.Normalize();

	//new vertex
	vec3d v_new = current_node + v_B;
	v_new = v_new.Normalize();
	v_new = v_new/(2*(a.Length()+b.Length()));
	
	return v_new;

}
