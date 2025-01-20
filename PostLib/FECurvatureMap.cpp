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
#include "FECurvatureMap.h"
using namespace Post;
using namespace std;

//-----------------------------------------------------------------------------
void FECongruencyMap::Surface::BuildNodeList(FSMesh& mesh)
{
	// tag all nodes that belong to this surface
	int N = mesh.Nodes();
	for (int i=0; i<N; ++i) mesh.Node(i).m_ntag = -1;
	int nn = 0;
	for (int i=0; i<Faces(); ++i)
	{
		FSFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) 
		{
			FSNode& node = mesh.Node(f.n[j]);
			if (node.m_ntag == -1) node.m_ntag = nn++;
		}
	}

	// create the global node list
	m_node.resize(nn);
	for (int i=0; i<N; ++i)
	{
		FSNode& node = mesh.Node(i);
		if (node.m_ntag >= 0) m_node[node.m_ntag].node = i;
	}

	// reset values
	for (int i=0; i<nn; ++i) m_node[i].val = 0.f;

	// create the local node list
	m_lnode.resize(Faces()*4);
	for (int i=0; i<Faces(); ++i)
	{
		FSFace& f = mesh.Face(m_face[i]);
		if (f.Nodes() == 4)
		{
			m_lnode[4*i  ] = mesh.Node(f.n[0]).m_ntag; assert(m_lnode[4*i  ] >= 0);
			m_lnode[4*i+1] = mesh.Node(f.n[1]).m_ntag; assert(m_lnode[4*i+1] >= 0);
			m_lnode[4*i+2] = mesh.Node(f.n[2]).m_ntag; assert(m_lnode[4*i+2] >= 0);
			m_lnode[4*i+3] = mesh.Node(f.n[3]).m_ntag; assert(m_lnode[4*i+3] >= 0);
		}
		else if (f.Nodes() == 3)
		{
			m_lnode[4*i  ] = mesh.Node(f.n[0]).m_ntag; assert(m_lnode[4*i  ] >= 0);
			m_lnode[4*i+1] = mesh.Node(f.n[1]).m_ntag; assert(m_lnode[4*i+1] >= 0);
			m_lnode[4*i+2] = mesh.Node(f.n[2]).m_ntag; assert(m_lnode[4*i+2] >= 0);
			m_lnode[4*i+3] = m_lnode[4*i+2];
		}
		else assert(false);
	}

	// create the node-facet look-up table
	m_NLT.resize(Nodes());
	for (int i=0; i<Faces(); ++i)
	{
		FSFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j)
		{
			int inode = m_lnode[4*i+j];
			m_NLT[inode].push_back(i);
		}
	}
}

//-----------------------------------------------------------------------------
void FECongruencyMap::Apply(FEPostModel& fem)
{
	// store the model
	m_pfem = &fem;

	// add a new field 
	fem.AddDataField(new FEDataField_T<FEFaceData<float, DATA_NODE> >(&fem), "congruency");
	int NDATA = fem.GetDataManager()->DataFields()-1;

	// get the mesh
	FSMesh& mesh = *fem.GetFEMesh(0);

	// build the node lists
	m_surf1.BuildNodeList(mesh);
	m_surf2.BuildNodeList(mesh);
	int N = mesh.Nodes();

	// repeat for all steps
	int nstep = fem.GetStates();
	for (int n=0; n<nstep; ++n)
	{
		FEState* ps = fem.GetState(n);
		FEFaceData<float,DATA_NODE>& df = dynamic_cast<FEFaceData<float,DATA_NODE>&>(ps->m_Data[NDATA]);

		// evaluate the nodal values for both surfaces
		EvalSurface(m_surf1, ps);
		EvalSurface(m_surf2, ps);

		// loop over all nodes of surface 1
		vector<float> a(m_surf1.Nodes());
		for (int i=0; i<m_surf1.Nodes(); ++i)
		{
			int inode = m_surf1.m_node[i].node;
			FSNode& node = mesh.Node(inode);
			vec3f r = fem.NodePosition(inode, n);
			float v0 = m_surf1.m_node[i].val;
			float v1 = project(m_surf2, r, n);
			a[i] = v0 - v1;
		}
		vector<int> nf1(m_surf1.Faces());
		for (int i = 0; i<m_surf1.Faces(); ++i) nf1[i] = mesh.Face(m_surf1.m_face[i]).Nodes();
		df.add(a, m_surf1.m_face, m_surf1.m_lnode, nf1);
	}
}

//-----------------------------------------------------------------------------
void FECongruencyMap::EvalSurface(FECongruencyMap::Surface& s, FEState* ps)
{
	FSMesh& mesh = *m_pfem->GetFEMesh(0);

	// init values and tags
	int NN = s.Nodes();
	for (int i=0; i<NN; ++i) { s.m_node[i].ntag = 0; s.m_node[i].val = 0.f; }

	// evaluate the surface data
	ValArray& d = ps->m_FaceData;
	int NF = s.Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(s.m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j)
		{
			s.m_node[s.m_lnode[4*i+j]].val += d.value(i, j);
			s.m_node[s.m_lnode[4*i+j]].ntag++;
		}
	}

	// normalize data
	for (int i=0; i<NN; ++i)
	{
		if (s.m_node[i].ntag > 0) s.m_node[i].val /= (float) s.m_node[i].ntag;
	}
}

//-----------------------------------------------------------------------------
float FECongruencyMap::project(FECongruencyMap::Surface& surf, vec3f& r, int ntime)
{
	FSMesh& mesh = *m_pfem->GetFEMesh(0);

	// find the closest surface node
	vec3f q = m_pfem->NodePosition(surf.m_node[0].node, ntime);
	float Dmin = (q - r)*(q - r);
	int imin = 0;
	for (int i=1; i<surf.Nodes(); ++i)
	{
		vec3f p = m_pfem->NodePosition(surf.m_node[i].node, ntime);
		float D = (p - r)*(p - r);
		if (D < Dmin)
		{
			q = p;
			Dmin = D;
			imin = i;
		}
	}

	// return value
	float val = 0.f;

	// loop over all facets connected to this node
	vector<int>& FT = surf.m_NLT[imin];
	for (int i=0; i<(int) FT.size(); ++i)
	{
		// get the i-th facet
		FSFace& face = mesh.Face(surf.m_face[FT[i]]);

		// project r onto the the facet
		vec3f p; float vi;
		if (ProjectToFacet(surf, FT[i], r, ntime, p, vi))
		{
			// return the closest projection
			float D = (p - r)*(p - r);
			if (D < Dmin)
			{
				q = p;
				Dmin = D;
				val = vi;
			}
		}
	}

	return val;
}

//-----------------------------------------------------------------------------
bool FECongruencyMap::ProjectToFacet(FECongruencyMap::Surface& surf, int iface, vec3f& x, int ntime, vec3f& q, float& val)
{
	// get the mesh to which this surface belongs
	FSMesh& mesh = *m_pfem->GetFEMesh(0);

	// get the face
	FSFace& f = mesh.Face(surf.m_face[iface]);
	
	// number of element nodes
	int ne = f.Nodes();
	
	// calculate normal projection of x onto element
	switch (ne)
	{
	case 3: return ProjectToTriangle(surf, iface, x, ntime, q, val); break;
	case 4: return ProjectToQuad    (surf, iface, x, ntime, q, val); break;
	default:
		assert(false);
	}
	return false;
}


//-----------------------------------------------------------------------------
// project onto a triangular face
bool FECongruencyMap::ProjectToTriangle(FECongruencyMap::Surface& surf, int iface, vec3f& x, int ntime, vec3f& q, float& val)
{
	// get the mesh to which this surface belongs
	FSMesh& mesh = *m_pfem->GetFEMesh(0);

	// get the face
	FSFace& face = mesh.Face(surf.m_face[iface]);

	// number of element nodes
	int ne = face.Nodes();

	// get the elements nodal positions
	vec3f y[3];
	for (int i=0; i<ne; ++i) y[i] = m_pfem->NodePosition(face.n[i], ntime);

	// get the nodal values
	float v[3];
	v[0] = surf.m_node[surf.m_lnode[4*iface  ]].val;
	v[1] = surf.m_node[surf.m_lnode[4*iface+1]].val;
	v[2] = surf.m_node[surf.m_lnode[4*iface+2]].val;

	// calculate base vectors 
	vec3f e1 = y[1] - y[0];
	vec3f e2 = y[2] - y[0];
	
	// calculate plane normal
	vec3f n = e1^e2; n.Normalize();
	
	// project x onto the plane
	q = x - n*((x-y[0])*n);
	
	// set up metric tensor
	double G[2][2];
	G[0][0] = e1*e1;
	G[0][1] = G[1][0] = e1*e2;
	G[1][1] = e2*e2;
	
	// invert metric tensor
	double D = G[0][0]*G[1][1] - G[0][1]*G[1][0];
	double Gi[2][2];
	Gi[0][0] = G[1][1]/D;
	Gi[1][1] = G[0][0]/D;
	Gi[0][1] = Gi[1][0] = -G[0][1]/D;
	
	// calculate dual base vectors
	vec3f E1 = e1*Gi[0][0] + e2*Gi[0][1];
	vec3f E2 = e1*Gi[1][0] + e2*Gi[1][1];
	
	// now we can calculate r and s
	vec3f t = q - y[0];
	double r = t*E1;
	double s = t*E2;

	// calculate value at this face
	val = (float)((1.0 - r - s)*v[0] + r*v[1] + s*v[2]);

	return ((r >= -m_tol) && (s >= -m_tol) && (r + s <= 1.0 + m_tol));
}

//-----------------------------------------------------------------------------
// project onto a quadrilateral surface.
bool FECongruencyMap::ProjectToQuad(FECongruencyMap::Surface& surf, int iface, vec3f& x, int ntime, vec3f& q, float& val)
{
	double R[2], u[2], D;
	double gr[4] = {-1, +1, +1, -1};
	double gs[4] = {-1, -1, +1, +1};
	double H[4], Hr[4], Hs[4], Hrs[4];
	
	int i, j;
	int NMAX = 50, n=0;

	// get the mesh to which this surface belongs
	FSMesh& mesh = *m_pfem->GetFEMesh(0);

	// get the face
	FSFace& face = mesh.Face(surf.m_face[iface]);

	// number of element nodes
	int ne = face.Nodes();

	// get the elements nodal positions
	vec3f y[4];
	for (int i=0; i<ne; ++i) y[i] = m_pfem->NodePosition(face.n[i], ntime);

	// get the nodal values
	float v[4];
	v[0] = surf.m_node[surf.m_lnode[4*iface  ]].val;
	v[1] = surf.m_node[surf.m_lnode[4*iface+1]].val;
	v[2] = surf.m_node[surf.m_lnode[4*iface+2]].val;
	v[3] = surf.m_node[surf.m_lnode[4*iface+3]].val;
	
	// evaulate scalar products
	double xy[4] = {x*y[0], x*y[1], x*y[2], x*y[3]};
	double yy[4][4];
	yy[0][0] = y[0]*y[0]; yy[1][1] = y[1]*y[1]; yy[2][2] = y[2]*y[2]; yy[3][3] = y[3]*y[3];
	yy[0][1] = yy[1][0] = y[0]*y[1];
	yy[0][2] = yy[2][0] = y[0]*y[2];
	yy[0][3] = yy[3][0] = y[0]*y[3];
	yy[1][2] = yy[2][1] = y[1]*y[2];
	yy[1][3] = yy[3][1] = y[1]*y[3];
	yy[2][3] = yy[3][2] = y[2]*y[3];

	double r = 0, s = 0;
	
	// loop until converged
	bool bconv = false;
	double normu;
	do
	{
		// evaluate shape functions and shape function derivatives.
		for (i=0; i<4; ++i)
		{
			H[i] = 0.25*(1+gr[i]*r)*(1+gs[i]*s);
			
			Hr[i] = 0.25*gr[i]*( 1 + gs[i]*s );
			Hs[i] = 0.25*gs[i]*( 1 + gr[i]*r );
			
			Hrs[i] = 0.25*gr[i]*gs[i];
		}
		
		// set up the system of equations
		R[0] = R[1] = 0;
		double A[2][2] = {0};
		for (i=0; i<4; ++i)
		{
			R[0] -= (xy[i])*Hr[i];
			R[1] -= (xy[i])*Hs[i];
			
			A[0][1] += (xy[i])*Hrs[i];
			A[1][0] += (xy[i])*Hrs[i];
			
			for (j=0; j<4; ++j)
			{
				double yij = yy[i][j];
				R[0] -= -H[j]*Hr[i]*(yij);
				R[1] -= -H[j]*Hs[i]*(yij);
				
				A[0][0] -= (yij)*(Hr[i]*Hr[j]);
				A[1][1] -= (yij)*(Hs[i]*Hs[j]);
				
				A[0][1] -= (yij)*(Hr[i]*Hs[j]+Hrs[i]*H[j]);
				A[1][0] -= (yij)*(Hs[i]*Hr[j]+Hrs[i]*H[j]);
			}
		}
		
		// determinant of A
		D = A[0][0]*A[1][1] - A[0][1]*A[1][0];
		
		// solve for u = A^(-1)*R
		u[0] = (A[1][1]*R[0] - A[0][1]*R[1])/D;
		u[1] = (A[0][0]*R[1] - A[1][0]*R[0])/D;
		
		// calculate displacement norm
		normu = u[0]*u[0]+u[1]*u[1];
		
		// check for convergence
		bconv = ((normu < 1e-10));
		if (!bconv && (n <= NMAX))
		{
			// Don't update if converged otherwise the point q
			// does not correspond with the current values for (r,s)
			r += u[0];
			s += u[1];
			++n;
		}
		else break;
	}
	while (1);
	
	// evaluate the value
	val = v[0]*H[0] + v[1]*H[1] + v[2]*H[2] + v[3]*H[3];
	
	return ((r >= -1.0 - m_tol) && (r <= 1.0+m_tol) && (s >= -1.0-m_tol) && (s <= 1.0+m_tol));
}
