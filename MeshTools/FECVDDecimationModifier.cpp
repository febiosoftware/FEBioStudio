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
#include "FECVDDecimationModifier.h"
#include <MeshLib/FSNodeFaceList.h>
#include <MeshLib/FSNodeNodeList.h>
#include "FEFillHole.h"
#include <GeomLib/GObject.h>
#include <MeshLib/MeshMetrics.h>
#include <MeshLib/MeshTools.h>
#include <MeshLib/FSSurfaceMesh.h>
#include "FEFixSurfaceMesh.h"
#include <algorithm>
#include <stack>
#include <FECore/matrix.h>
using namespace std;

//-----------------------------------------------------------------------------
//! Constructor
FECVDDecimationModifier::FECVDDecimationModifier() : FESurfaceModifier("CVD Decimation")
{
	AddDoubleParam(0.1, "scale", "scale");

	m_gradient = 0.0;
	m_bcvd = false;
}

//-----------------------------------------------------------------------------
// We need a simple random number generator for creating the seeds. The C rand()
// function won't do since it only generates numbers less than 32K.
int randi(int nmax)
{
	static unsigned int nseed = 47856987;
	nseed = 789789812*nseed + 38569741 + rand();
	return (int)(nseed%nmax);
}

//-----------------------------------------------------------------------------
//! Create the decimate mesh. 
//! \todo This implementation will only work with closed surfaces. 
FSSurfaceMesh* FECVDDecimationModifier::Apply(FSSurfaceMesh* pm)
{
	// make sure this is a triangle mesh
	if (pm->IsType(FE_FACE_TRI3) == false) 
	{	
		FESurfaceModifier::SetError("Invalid mesh type");
		return 0;
	}

	// some sanity checks
	if (MeshTools::IsMeshClosed(*pm) == false)
	{
		FESurfaceModifier::SetError("Mesh is not closed.");
		return 0;
	}

	// do the initialization
	if (Initialize(pm) == false) return 0;

	// Minimize energy
	if (Minimize(pm) == false) return 0;

	// create the new mesh
	// we can create either the clustered mesh or the final decimated mesh
	FSSurfaceMesh* pnew = 0;
	if (m_bcvd) 
	{
		// partition mesh based on cluster assingments
		pnew = CalculateCVD(pm); 
	}
	else 
	{
		// triangulate the decimation
		pnew = Triangulate2(pm);

		// make sure we have a mesh
		if (pnew == 0) return 0;

		// next, we use the auto-mesher to reconstruct all faces, edges and nodes
		pnew->RebuildMesh();
	
		FEFixSurfaceMesh fix;
		// TODO: I noticed that this algorithm can create duplicate elements. I'm not sure yet if
		//       this is a bug or if there is something fishy about this algorithm. In any case,
		//       for now I will explicitly check for duplicate elements and remove duplicates. Even
		//       this may not fix all problems, since sometimes duplicate elements can have different
		//       winding.
		// remove duplicate faces
		fix.RemoveDuplicateFaces(pnew);

		// TODO: Another problem I found is that some elements become non-manifold, meaning that
		//       one or two edges don't have any neighbors. If the origianl mesh is closed, then
		//       the decimated mesh must be closed as well. (Off course, this assumes that the original
		//       mesh is closed. Perhaps I can find another criteria for non-manifold).
		fix.RemoveNonManifoldFaces(pnew);

		// TODO: Some elements can be wound in the wrong direction. So, we try to find those elements
		//       and invert them.
		fix.FixElementWinding(pnew);

		// TODO: These operations can create holes in the mesh. therefore we fill all holes.
		FEFillHole fill;
		fill.FillAllHoles(pnew);
	}

	// All done!
	return pnew;
}

//-----------------------------------------------------------------------------
//! This function initializes the CVD algorithm. Cluster data is allocated and
//! initialized. We also build the edge list, that is the list with the boundary
//! edges between clusters.
//! \todo I need to create a better random seeding algorithm. If the scale is close
//! to one, this algorithm may take a while to finish.
bool FECVDDecimationModifier::Initialize(FSSurfaceMesh* pm)
{
	// nodes on original mesh
	int N0 = pm->Nodes();

	// decimation percentage
	double pct = GetFloatValue(0);

	// target number of clusters/vertices
	int NC = (int) (pct*N0);
	if (NC < 4) NC = 4;

	// allocate cluster data
	// number of clusters equals number of target nodes
	// plus one, the "null" cluster
	m_Cluster.resize(NC + 1);

	// assign all triangles to the "null" cluster
	int T0 = pm->Faces();
	m_tag.assign(T0, 0);

	// assign NC random triangles to a cluster
	// TODO: make a better algorithm
	int MAX_TRIES = 2*T0;
	int ntries = 0;
	int nc = 0;
	while (nc < NC)
	{
		int n = randi(T0);
		if (m_tag[n] == 0) m_tag[n] = ++nc;

		ntries++;
		if (ntries > MAX_TRIES) return FESurfaceModifier::SetError("Seeding failed");
	}

	// calculate the "rho" value (here, the area) and gamma (centroid) for each face
	m_rho.resize(T0);
	m_gamma.resize(T0);
	vec3d r[3];
	
	//Creating a node node list
	FSNodeNodeList NNL(pm);

	for (int i=0; i<T0; ++i)
	{
		FSFace& fi = pm->Face(i);

		// get the nodal coordinates
		r[0] = pm->Node(fi.n[0]).r;
		r[1] = pm->Node(fi.n[1]).r;
		r[2] = pm->Node(fi.n[2]).r;

		// assign the centroid
		m_gamma[i] = (r[0] + r[1] + r[2]) / 3.0;

		// calculate rho
		m_rho[i] = area_triangle(r);

		// gradient weighting
		if (m_gradient > 0.0)
		{
			double curvature1 = 0;
			double curvature2 = 0;
			for (int j = 0; j < 3; j++)
			{
				//normal vector to the jth node of the face
				//step -1
				vec3d p = to_vec3d(fi.m_nn[j]);
				vec3d r0 = pm->Node(fi.n[j]).r;

				//step -2 
				vec3d r1 = vec3d(1.f - p.x*p.x, -p.y*p.x, -p.x*p.z);
				r1.Normalize();
				vec3d r3 = p;
				vec3d r2 = r3 ^ r1;
				mat3d R;
				R[0][0] = r1.x; R[0][1] = r1.y; R[0][2] = r1.z;
				R[1][0] = r2.x; R[1][1] = r2.y; R[1][2] = r2.z;
				R[2][0] = r3.x; R[2][1] = r3.y; R[2][2] = r3.z;

				//step -3
				int current_node = fi.n[j];
				vector<vec3d> x;
				x.reserve(NNL.Valence(current_node));
				for (int k = 0; k < NNL.Valence(current_node); k++)
					x.push_back(pm->Node(NNL.Node(current_node, k)).r);
				vector<vec3d> y;
				y.resize(x.size());
				int nn = (int)x.size();

				//step 4
				// map coordinates
				for (int k = 0; k < nn; ++k)
				{
					vec3d tmp = x[k] - r0;
					y[k] = R * tmp;
				}

				//step 5
				//Prepare linear system of equations.
				matrix RR(nn, 3);
				vector<double> r(nn);
				for (int k = 0; k < nn; ++k)
				{
					vec3d& p = y[k];
					RR[k][0] = p.x*p.x;
					RR[k][1] = p.x*p.y;
					RR[k][2] = p.y*p.y;
					r[k] = p.z;
				}

				// solve for quadric coefficients
				vector<double> q(3);
				RR.lsq_solve(q, r);
				double a = q[0], b = q[1], c = q[2];

				//step 6
				//get the curvature
				double k1 = a + c + sqrt((a - c)*(a - c) + b * b);
				double k2 = a + c - sqrt((a - c)*(a - c) + b * b);
				curvature1 += k1;
				curvature2 += k2;
			}
			curvature1 /= 3.0;
			curvature2 /= 3.0;

			// calculate rho and gamma
			m_rho[i] *= pow(sqrt(curvature1 * curvature1 + curvature2 * curvature2), m_gradient);
		}

		// make sure rho is positive
		if (m_rho[i] <= 0.0) return FESurfaceModifier::SetError("Zero triangle area encountered");
	}

	// now, calculate the sgamma and srho for each cluster
	for (int i=0; i<T0; ++i)
	{
		int nc = m_tag[i];
		if (nc > 0)
		{
			m_Cluster[nc].m_srho += m_rho[i];
			m_Cluster[nc].m_sgamma += m_gamma[i]*m_rho[i];
			m_Cluster[nc].m_val++;
		}
		else m_Cluster[0].m_val++;
	}

	// build the edge list
	m_Edge.clear();
	for (int i=0; i<T0; ++i)
	{
		FSFace& fi = pm->Face(i);
		for (int j=0; j<3; ++j)
		{
			int nj = fi.m_nbr[j];

			// this algorithm currently only works with closed surfaces
			// so all neighbors must be assigned
			if (nj < 0) return FESurfaceModifier::SetError("Mesh is not closed");

			if (m_tag[i] < m_tag[nj])
			{
				EDGE e;
				e.face[0] = i;
				e.face[1] = nj;
				if (i == nj) return FESurfaceModifier::SetError("Invalid mesh connectivity");

				e.node[0] = fi.n[j];
				e.node[1] = fi.n[(j+1)%3];
				m_Edge.push_back(e);
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// This function assigns face to a new cluster
bool FECVDDecimationModifier::Swap(FSFace& face, int nface, int ncluster)
{
	if (m_tag[nface] == ncluster) return false;

	int oldCluster = m_tag[nface];
	m_tag[nface] = ncluster;

	// make sure the cluster will not dissappear
	if ((oldCluster != 0) && (m_Cluster[oldCluster].m_val == 1)) return true;

	m_Cluster[oldCluster].m_val--;
	assert((oldCluster == 0) || (m_Cluster[oldCluster].m_val > 0));
	assert(m_Cluster[oldCluster].m_val >= 0);
	m_Cluster[ncluster].m_val++;

	for (int j=0; j<3; ++j)
	{
		int nj = face.m_nbr[j];
		assert(nj >= 0);

		if (m_tag[nj] == oldCluster)
		{
			EDGE ed;
			ed.face[0] = nface;
			ed.face[1] = nj;
			if (nface == nj) return false;

			ed.node[0] = face.n[j];
			ed.node[1] = face.n[(j+1)%3];

			m_Edge.push_front(ed);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
//! Update clustering to minimize energy
bool FECVDDecimationModifier::Minimize(FSSurfaceMesh* pm)
{
	bool bconv = false;
	const int MAX_ITER = 50000;
	int niter = 0;
	do
	{
		// let's be optimistic
		bconv = true;

		// loop over all edges
		list<EDGE>::iterator it;
		for (it = m_Edge.begin(); it != m_Edge.end();)
		{
			EDGE& e = *it;

			// faces
			int m = e.face[0];
			int n = e.face[1];
			assert(m != n);

			// clusters 
			int k = m_tag[m];
			int l = m_tag[n];

			if (k == l)
			{
				it = m_Edge.erase(it);
				bconv = false;
			}
			else
			{
				Cluster& Ck = m_Cluster[k];
				Cluster& Cl = m_Cluster[l];

				// case 1 - no change
				double L1 = (Ck.m_sgamma*Ck.m_sgamma) / Ck.m_srho + (Cl.m_sgamma*Cl.m_sgamma) / Cl.m_srho;

				// case 2 - face m to Cl
				vec3d g0k = Ck.m_sgamma - m_gamma[m] * m_rho[m];
				double r0k = Ck.m_srho - m_rho[m];

				vec3d g0l = Cl.m_sgamma + m_gamma[m] * m_rho[m];
				double r0l = Cl.m_srho + m_rho[m];
				double L2 = (g0k*g0k) / r0k + (g0l*g0l) / r0l;

				// case 3 - face n to Ck
				vec3d g1k = Ck.m_sgamma + m_gamma[n] * m_rho[n];
				double r1k = Ck.m_srho + m_rho[n];

				vec3d g1l = Cl.m_sgamma - m_gamma[n] * m_rho[n];
				double r1l = Cl.m_srho - m_rho[n];
				double L3 = (g1k*g1k) / r1k + (g1l*g1l) / r1l;

				if ((k == 0) || ((l!=0) && (L2 > L1) && (L2 > L3)))
				{
					// case 2 wins
					Ck.m_srho = r0k;
					Ck.m_sgamma = g0k;
					Cl.m_srho = r0l;
					Cl.m_sgamma = g0l;

					if (Swap(pm->Face(m), m, l) == false) return FESurfaceModifier::SetError("Error during minimization");
					else it = m_Edge.erase(it);

					bconv = false;
				}
				else if ((l == 0) || ((L3 > L1) && (L3 > L2)))
				{
					// case 3 wins
					Ck.m_srho = r1k;
					Ck.m_sgamma = g1k;
					Cl.m_srho = r1l;
					Cl.m_sgamma = g1l;

					if (Swap(pm->Face(n), n, k) == false)
						return FESurfaceModifier::SetError("Error during minimization");
					else
						it = m_Edge.erase(it);

					bconv = false;
				}
				else
				{
					// case 1 wins
					// no change
					++it;
				}
			}
		}

		if (bconv)
		{
			// In some cases, (really bad meshes) a few elements will not have
			// been assigned to a cluster. If this happens, we just return an
			// error and the user needs to clean up the mesh before decimating
			int NF = pm->Faces();
			for (int i=0; i<NF; ++i)
			{
				if (m_tag[i] == 0) 
					return FESurfaceModifier::SetError("Error during minimization");;
			}

			// build the cluster's fid array
			for (int i = 0; i < m_Cluster.size(); ++i) m_Cluster[i].m_fid.clear();
			for (int i = 0; i < pm->Faces(); ++i)
			{
				Cluster& C = m_Cluster[m_tag[i]];
				C.m_fid.push_back(i);
			}

			// check one-connectedness of clusters
			pm->TagAllFaces(-1);
			for (int i = 1; i < m_Cluster.size(); ++i)
			{
				Cluster& Ci = m_Cluster[i];

				// keep track of components and max-area component
				int nc = 0;
				int imax = -1;
				double amax = 0.0;

				for (int j = 0; j < Ci.m_fid.size(); ++j)
				{
					int facej = Ci.m_fid[j];
					if (pm->Face(facej).m_ntag == -1)
					{
						double am = 0.0;
						std::stack<int> S;
						S.push(facej);
						while (S.empty() == false)
						{
							int nface = S.top(); S.pop();
							FSFace& face = pm->Face(nface);
							face.m_ntag = nc;

							am += m_rho[nface];

							for (int k = 0; k < 3; ++k)
							{
								int nk = face.m_nbr[k];
								if ((pm->Face(nk).m_ntag == -1) && (m_tag[nk] == i))
								{
									pm->Face(nk).m_ntag = nc;
									S.push(nk);
								}
							}
						}

						if ((imax == -1) || (am > amax))
						{
							imax = nc;
							amax = am;
						}

						nc++;
					}
				}

				if (nc > 1)
				{
					for (int j = 0; j < Ci.m_fid.size(); ++j)
					{
						int facej = Ci.m_fid[j];
						if (pm->Face(facej).m_ntag != imax)
						{
							m_tag[facej] = 0;
							m_Cluster[0].m_val++;
							Ci.m_srho -= m_rho[facej];
							Ci.m_sgamma -= m_gamma[facej] * m_rho[facej];
							Ci.m_val--;
						}
					}

					bconv = false;
				}
			}
		}

		niter++;
	}
	while ((bconv == false)&&(niter < MAX_ITER));

	if (niter >= MAX_ITER) return FESurfaceModifier::SetError("Error during minimization");

	return true;
}

//-----------------------------------------------------------------------------
bool FECVDDecimationModifier::NODE::AttachToCluster(int n)
{
	for (int i=0; i<nc; ++i)
	{
		if (c[i] == n) 
			return true;
	}
	if(nc>=MAX_CLUSTERS-1) 
		return false;
	c[nc++] = n;
	return true;
}

//-----------------------------------------------------------------------------
// TODO: The case n==6 has other ways to get tesselated. I only implemented a few
FSSurfaceMesh* FECVDDecimationModifier::Triangulate(FSSurfaceMesh* pm)
{
	int N = pm->Nodes();
	vector<NODE> Node; Node.resize(N);
	for (int i=0; i<N; ++i)
	{
		Node[i].nc = 0;
	}

	FSNodeFaceList NFL;
	NFL.BuildSorted(pm);
	for (int i=0; i<pm->Faces(); ++i) pm->Face(i).m_ntag = i;
	for (int i=0; i<N; ++i)
	{
		int nval = NFL.Valence(i);
		for (int j=0; j<nval; ++j)
		{
			if (Node[i].AttachToCluster(m_tag[NFL.Face(i, j)->m_ntag]) == false) return 0;
		}
	}

	// number of nodes equals number of clusters
	int nodes = (int) m_Cluster.size() - 1;

	// count the number of triangles
	int faces = 0;
	for (int i=0; i<N; ++i)
	{
		NODE& nd = Node[i];
		if      (nd.nc == 3) faces++;
		else if (nd.nc == 4) faces += 2;
		else if (nd.nc == 5) faces += 3;
		else if (nd.nc == 6) faces += 4;
	}

	// create a new mesh
	FSSurfaceMesh* pnew = new FSSurfaceMesh;
	pnew->Create(nodes, 0, faces);

	// calculate the node positions
	for (int i=0; i<nodes; ++i)
	{
		FSNode& nd = pnew->Node(i);
		Cluster& Ci = m_Cluster[i+1];
		assert(Ci.faces() > 0);
		nd.r = Ci.m_sgamma / Ci.m_srho;
	}

	// create the connectivity
	faces = 0;
	for (int i=0; i<N; ++i)
	{
		NODE& nd = Node[i];
		if (nd.nc == 3)
		{
			FSFace& face = pnew->Face(faces++);
			face.SetType(FE_FACE_TRI3);
			face.n[0] = nd.c[0]-1; assert(nd.c[0] > 0);
			face.n[1] = nd.c[1]-1; assert(nd.c[1] > 0);
			face.n[2] = nd.c[2]-1; assert(nd.c[2] > 0);
		}
		else if (nd.nc == 4)
		{
			vec3d r[4];
			r[0] = pnew->Node(nd.c[0]-1).r;
			r[1] = pnew->Node(nd.c[1]-1).r;
			r[2] = pnew->Node(nd.c[2]-1).r;
			r[3] = pnew->Node(nd.c[3]-1).r;

			vec3d ra0[3] = {r[0], r[1], r[2]}, ra1[3] = {r[2], r[3], r[0]};
			double A0 = area_triangle(ra0);
			double A1 = area_triangle(ra1);
			double Amin = (A0 < A1 ? A0 : A1);
	
			vec3d rb0[3] = {r[3], r[0], r[1]}, rb1[3] = {r[1], r[2], r[3]};
			double B0 = area_triangle(rb0);
			double B1 = area_triangle(rb1);
			double Bmin = (B0 < B1 ? B0 : B1);

			if (Amin >= Bmin)
			{
				FSFace& f0 = pnew->Face(faces++);
				f0.SetType(FE_FACE_TRI3);
				f0.n[0] = nd.c[0]-1; assert(nd.c[0] > 0);
				f0.n[1] = nd.c[1]-1; assert(nd.c[1] > 0);
				f0.n[2] = nd.c[2]-1; assert(nd.c[2] > 0);
			
				FSFace& f1 = pnew->Face(faces++);
				f1.SetType(FE_FACE_TRI3);
				f1.n[0] = nd.c[2]-1; assert(nd.c[2] > 0);
				f1.n[1] = nd.c[3]-1; assert(nd.c[3] > 0);
				f1.n[2] = nd.c[0]-1; assert(nd.c[0] > 0);
			}
			else
			{
				FSFace& f0 = pnew->Face(faces++);
				f0.SetType(FE_FACE_TRI3);
				f0.n[0] = nd.c[3]-1; assert(nd.c[3] > 0);
				f0.n[1] = nd.c[0]-1; assert(nd.c[0] > 0);
				f0.n[2] = nd.c[1]-1; assert(nd.c[1] > 0);
			
				FSFace& f1 = pnew->Face(faces++);
				f1.SetType(FE_FACE_TRI3);
				f1.n[0] = nd.c[1]-1; assert(nd.c[1] > 0);
				f1.n[1] = nd.c[2]-1; assert(nd.c[2] > 0);
				f1.n[2] = nd.c[3]-1; assert(nd.c[3] > 0);
			}
		}
		else if (nd.nc == 5)
		{
			vec3d r[5];
			r[0] = pnew->Node(nd.c[0]-1).r;
			r[1] = pnew->Node(nd.c[1]-1).r;
			r[2] = pnew->Node(nd.c[2]-1).r;
			r[3] = pnew->Node(nd.c[3]-1).r;
			r[4] = pnew->Node(nd.c[4]-1).r;

			const int LUT[5][3][3] = {
				{{0,1,2},{2,3,4},{0,2,4}},{{0,1,4},{1,2,4},{2,3,4}},{{0,1,2},{0,2,3},{0,3,4}},
				{{3,4,0},{0,1,3},{1,2,3}},{{0,1,4},{1,3,4},{1,2,3}}};

			double Amax = 0;
			int imax = 0;
			for (int i=0; i<5; ++i)
			{
				vec3d ra0[3] = {r[LUT[i][0][0]], r[LUT[i][0][1]], r[LUT[i][0][2]]};
				vec3d ra1[3] = {r[LUT[i][1][0]], r[LUT[i][1][1]], r[LUT[i][1][2]]};
				vec3d ra2[3] = {r[LUT[i][2][0]], r[LUT[i][2][1]], r[LUT[i][2][2]]};
				double A0 = area_triangle(ra0);
				double A1 = area_triangle(ra1);
				double A2 = area_triangle(ra2);
				double Amin = min(min(A0,A1),A2);

				if (Amin > Amax)
				{
					imax = i;
					Amax = Amin;
				}
			}
	
			FSFace& f0 = pnew->Face(faces++);
			f0.SetType(FE_FACE_TRI3);
			f0.n[0] = nd.c[LUT[imax][0][0]]-1; assert(nd.c[LUT[imax][0][0]] > 0);
			f0.n[1] = nd.c[LUT[imax][0][1]]-1; assert(nd.c[LUT[imax][0][1]] > 0);
			f0.n[2] = nd.c[LUT[imax][0][2]]-1; assert(nd.c[LUT[imax][0][2]] > 0);

			FSFace& f1 = pnew->Face(faces++);
			f1.SetType(FE_FACE_TRI3);
			f1.n[0] = nd.c[LUT[imax][1][0]]-1; assert(nd.c[LUT[imax][1][0]] > 0);
			f1.n[1] = nd.c[LUT[imax][1][1]]-1; assert(nd.c[LUT[imax][1][1]] > 0);
			f1.n[2] = nd.c[LUT[imax][1][2]]-1; assert(nd.c[LUT[imax][1][2]] > 0);

			FSFace& f2 = pnew->Face(faces++);
			f2.SetType(FE_FACE_TRI3);
			f2.n[0] = nd.c[LUT[imax][2][0]]-1; assert(nd.c[LUT[imax][2][0]] > 0);
			f2.n[1] = nd.c[LUT[imax][2][1]]-1; assert(nd.c[LUT[imax][2][1]] > 0);
			f2.n[2] = nd.c[LUT[imax][2][2]]-1; assert(nd.c[LUT[imax][2][2]] > 0);
		}
		else if (nd.nc == 6)
		{
			vec3d r[6];
			r[0] = pnew->Node(nd.c[0]-1).r;
			r[1] = pnew->Node(nd.c[1]-1).r;
			r[2] = pnew->Node(nd.c[2]-1).r;
			r[3] = pnew->Node(nd.c[3]-1).r;
			r[4] = pnew->Node(nd.c[4]-1).r;
			r[5] = pnew->Node(nd.c[5]-1).r;

			const int LUT[8][4][3] = {
				{{0,1,5},{1,2,3},{3,4,5},{1,3,5}},
				{{0,1,2},{2,3,4},{4,5,0},{0,2,4}},
				{{0,1,2},{0,2,3},{0,3,5},{3,4,5}},
				{{0,1,2},{0,2,5},{2,3,5},{3,4,5}},
				{{0,1,5},{1,2,5},{2,4,5},{2,3,4}},
				{{0,1,5},{1,4,5},{1,2,4},{2,3,4}},
				{{0,4,5},{0,3,4},{0,1,3},{1,2,3}},
				{{0,4,5},{0,1,4},{1,3,4},{1,2,3}}};

			double Amax = 0, Amin;
			int imax = 0;
			for (int i=0; i<8; ++i)
			{
				vec3d ra0[3] = {r[LUT[i][0][0]], r[LUT[i][0][1]], r[LUT[i][0][2]]};
				vec3d ra1[3] = {r[LUT[i][1][0]], r[LUT[i][1][1]], r[LUT[i][1][2]]};
				vec3d ra2[3] = {r[LUT[i][2][0]], r[LUT[i][2][1]], r[LUT[i][2][2]]};
				vec3d ra3[3] = {r[LUT[i][3][0]], r[LUT[i][3][1]], r[LUT[i][3][2]]};
				double A0 = area_triangle(ra0); Amin = A0;
				double A1 = area_triangle(ra1); if (A1 < Amin) Amin = A1;
				double A2 = area_triangle(ra2); if (A2 < Amin) Amin = A2;
				double A3 = area_triangle(ra3); if (A3 < Amin) Amin = A3;

				if (Amin > Amax)
				{
					imax = i;
					Amax = Amin;
				}
			}
	
			FSFace& f0 = pnew->Face(faces++);
			f0.SetType(FE_FACE_TRI3);
			f0.n[0] = nd.c[LUT[imax][0][0]]-1; assert(nd.c[LUT[imax][0][0]] > 0);
			f0.n[1] = nd.c[LUT[imax][0][1]]-1; assert(nd.c[LUT[imax][0][1]] > 0);
			f0.n[2] = nd.c[LUT[imax][0][2]]-1; assert(nd.c[LUT[imax][0][2]] > 0);

			FSFace& f1 = pnew->Face(faces++);
			f1.SetType(FE_FACE_TRI3);
			f1.n[0] = nd.c[LUT[imax][1][0]]-1; assert(nd.c[LUT[imax][1][0]] > 0);
			f1.n[1] = nd.c[LUT[imax][1][1]]-1; assert(nd.c[LUT[imax][1][1]] > 0);
			f1.n[2] = nd.c[LUT[imax][1][2]]-1; assert(nd.c[LUT[imax][1][2]] > 0);

			FSFace& f2 = pnew->Face(faces++);
			f2.SetType(FE_FACE_TRI3);
			f2.n[0] = nd.c[LUT[imax][2][0]]-1; assert(nd.c[LUT[imax][2][0]] > 0);
			f2.n[1] = nd.c[LUT[imax][2][1]]-1; assert(nd.c[LUT[imax][2][1]] > 0);
			f2.n[2] = nd.c[LUT[imax][2][2]]-1; assert(nd.c[LUT[imax][2][2]] > 0);

			FSFace& f3 = pnew->Face(faces++);
			f3.SetType(FE_FACE_TRI3);
			f3.n[0] = nd.c[LUT[imax][3][0]]-1; assert(nd.c[LUT[imax][3][0]] > 0);
			f3.n[1] = nd.c[LUT[imax][3][1]]-1; assert(nd.c[LUT[imax][3][1]] > 0);
			f3.n[2] = nd.c[LUT[imax][3][2]]-1; assert(nd.c[LUT[imax][3][2]] > 0);
		}
	}

	return pnew;
}

//-----------------------------------------------------------------------------
FSSurfaceMesh* FECVDDecimationModifier::Triangulate2(FSSurfaceMesh* pm)
{
	// let's build the node data
	int N = pm->Nodes();
	vector<NODE> Node; Node.resize(N);
	for (int i=0; i<N; ++i)
	{
		Node[i].nc = 0;
	}

	// find all clusters a node belongs to
	FSNodeFaceList NFL;
	if (NFL.BuildSorted(pm) == false) return 0;
	for (int i=0; i<pm->Faces(); ++i) pm->Face(i).m_ntag = i;
	for (int i=0; i<N; ++i)
	{
		int nval = NFL.Valence(i);
		for (int j=0; j<nval; ++j)
		{
			if (Node[i].AttachToCluster(m_tag[NFL.Face(i, j)->m_ntag]) == false) 
				return 0;
		}
	}

	// build the cluster's fid array
	for (int i = 0; i < m_Cluster.size(); ++i) m_Cluster[i].m_fid.clear();
	for (int i=0; i<pm->Faces(); ++i)
	{
		Cluster& C = m_Cluster[m_tag[i]];
		C.m_fid.push_back(i);
	}

	// number of nodes for decimanted mesh equals number of clusters
	int nodes = (int) m_Cluster.size() - 1;

	// create a new mesh
	FSSurfaceMesh* pnew = new FSSurfaceMesh;
	pnew->Create(nodes, 0, 0);
	// calculate the node positions
	for (int i=0; i<nodes; ++i)
	{
		FSNode& nd = pnew->Node(i);
		Cluster& Ci = m_Cluster[i+1];

		//nd.r = Ci.m_sgamma / Ci.m_srho;//change here //original point
		vec3d po = Ci.m_sgamma / Ci.m_srho;
		vec3d po_1 = vec3d(0,0,0); //projection of po on one of the faces
		//find projection of this center back to original surface of the mesh
		int flag =  0;
		double thickness = 0;
		for (int j =0 ;j<Ci.faces() ;j++)
		{
			FSFace& fj = pm->Face(Ci.m_fid[j]);
			vec3d p[3];
			p[0] = pm->Node(fj.n[0]).r;//p1
			p[1] = pm->Node(fj.n[1]).r;//p2
			p[2] = pm->Node(fj.n[2]).r;//p3

			vec3d u = p[1] - p[0]; //p2 - p1
			vec3d v = p[2] - p[0]; //p3 - p1
			vec3d n = u ^ v;//u x v
			vec3d w = po - p[0]; //po - p1
			double n_2 = n * n; //n^2
			vec3d u_X_w = u ^ w;//u x w
			double gama = (u_X_w * n)/n_2;
			vec3d w_X_v = w ^ v;//w x v
			double beta = (w_X_v * n)/n_2;
			double alpha = 1 - gama - beta;
			if (alpha <= 1 && alpha >= 0 && beta <= 1 && beta >= 0 && gama <= 1 && gama >= 0 )
			{
				po_1 = (p[0] * alpha) + (p[1] * beta) + (p[2] * gama);
				flag = 1;
				break;
			}
		}
		if (flag == 0) 
			nd.r = po;
		else
			nd.r = po_1;
	}

	// list of triangles
	FEFillHole fill;
	vector<FEFillHole::FACE> tri_list;
	tri_list.reserve(nodes);

	// loop over all nodes
	for (int i=0; i<N; ++i)
	{
		NODE& nd = Node[i];
		if (nd.nc == 3)
		{
			FEFillHole::FACE f;
			f.n[0] = nd.c[0]-1; assert(nd.c[0] > 0);
			f.n[1] = nd.c[1]-1; assert(nd.c[1] > 0);
			f.n[2] = nd.c[2]-1; assert(nd.c[2] > 0);
			tri_list.push_back(f);
		}
		else if (nd.nc > 3)
		{
			FEFillHole::EdgeRing ring;
			vec3d temp;
			for (int j=0; j<nd.nc; ++j) ring.add(nd.c[j]-1, pnew->Node(nd.c[j]-1).r,temp);

			vector<FEFillHole::FACE> tri;
			fill.DivideRing(ring, tri);

			// append to the tri_list
			tri_list.insert(tri_list.end(), tri.begin(), tri.end());
		}
	}

	// count the faces
	int faces = (int) tri_list.size();
	pnew->Create(0, 0, faces);

	// build the elements
	for (int i=0; i<faces; ++i)
	{
		FSFace& face = pnew->Face(i);
		FEFillHole::FACE& fi = tri_list[i];
		face.SetType(FE_FACE_TRI3);
		face.m_gid = 0;
		face.n[0] = fi.n[0];
		face.n[1] = fi.n[1];
		face.n[2] = fi.n[2];
	}

	return pnew;
}

//-----------------------------------------------------------------------------
FSSurfaceMesh* FECVDDecimationModifier::CalculateCVD(FSSurfaceMesh* pm)
{
	FSSurfaceMesh* pnew = new FSSurfaceMesh(*pm);

	int N = pnew->Faces();
	for (int i=0; i<N; ++i)
	{
		FSFace& f = pnew->Face(i);
		f.m_gid = m_tag[i];
	}

	pnew->RebuildMesh(0.0);

	return pnew;
}
