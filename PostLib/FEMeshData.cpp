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

#include "FEMeshData_T.h"
#include "FEDataManager.h"
#include "FEPostModel.h"
#include "FEPointCongruency.h"
#include <MeshLib/MeshMetrics.h>

using namespace Post;
using namespace std;

extern int ET_HEX[12][2];
extern int DIAG_HEX[16][2];

//-----------------------------------------------------------------------------
Post::FEPostMesh* Post::FEMeshData::GetFEMesh()
{ 
	return m_state->GetFEMesh(); 
}

FEPostModel* Post::FEMeshData::GetFSModel()
{ 
	return m_state->GetFSModel(); 
}

//-----------------------------------------------------------------------------
// FEMeshDataList
//-----------------------------------------------------------------------------

void FEMeshDataList::clear()
{
	for (int i=0; i<(int) m_data.size(); ++i) delete m_data[i];
	m_data.clear();
}

//-----------------------------------------------------------------------------
// evaluate the spatial gradient of the shape functions in elem at point q at state
void Post::shape_grad(FEPostModel& fem, int elem, double q[3], int nstate, vec3f* G)
{
	// get the mesh
	FEState& state = *fem.GetState(nstate);
	FEPostMesh& m = *state.GetFEMesh();

	// get the element
	FEElement_& el = m.ElementRef(elem);
	int N = el.Nodes();

	// we can only define this for solid elements
	if (el.IsSolid() == false) return;

	// get the nodal positions
	const int MN = FSElement::MAX_NODES;
	vec3f x[MN];
	for (int i = 0; i<N; i++)
	{
		int node = el.m_node[i];
		x[i] = fem.NodePosition(node, nstate);
	}

	// get the shape function derivatives
	double Hr[MN], Hs[MN], Ht[MN];
	el.shape_deriv(Hr, Hs, Ht, q[0], q[1], q[2]);

	// evaluate jacobian
	mat3d J; J.zero();
	for (int i = 0; i<N; i++)
	{
		J[0][0] += x[i].x*Hr[i]; J[0][1] += x[i].x*Hs[i]; J[0][2] += x[i].x*Ht[i];
		J[1][0] += x[i].y*Hr[i]; J[1][1] += x[i].y*Hs[i]; J[1][2] += x[i].y*Ht[i];
		J[2][0] += x[i].z*Hr[i]; J[2][1] += x[i].z*Hs[i]; J[2][2] += x[i].z*Ht[i];
	}

	// invert jacobian
	J.invert();

	// evaluate dH/dX = J^(-T)*dH/dr
	for (int i = 0; i<N; i++)
	{
		G[i].x = J[0][0] * Hr[i] + J[1][0] * Hs[i] + J[2][0] * Ht[i];
		G[i].y = J[0][1] * Hr[i] + J[1][1] * Hs[i] + J[2][1] * Ht[i];
		G[i].z = J[0][2] * Hr[i] + J[1][2] * Hs[i] + J[2][2] * Ht[i];
	}
}

//-----------------------------------------------------------------------------
// evaluate the spatial gradient of the shape functions in elem at point q at state
void Post::shape_grad_ref(FEPostModel& fem, int elem, double q[3], int nstate, vec3f* G)
{
	// get the mesh
	FEState& state = *fem.GetState(nstate);
	FEPostMesh& m = *state.GetFEMesh();

	// get the element
	FEElement_& el = m.ElementRef(elem);
	int N = el.Nodes();

	// we can only define this for solid elements
	if (el.IsSolid() == false) return;

	// get the (reference) nodal positions
	const int MN = FSElement::MAX_NODES;
	vec3f x[MN];
	for (int i = 0; i < N; i++)
	{
		int node = el.m_node[i];
		x[i] = fem.NodePosition(node, 0);
	}

	// get the shape function derivatives
	double Hr[MN], Hs[MN], Ht[MN];
	el.shape_deriv(Hr, Hs, Ht, q[0], q[1], q[2]);

	// evaluate jacobian
	mat3d J; J.zero();
	for (int i = 0; i < N; i++)
	{
		J[0][0] += x[i].x * Hr[i]; J[0][1] += x[i].x * Hs[i]; J[0][2] += x[i].x * Ht[i];
		J[1][0] += x[i].y * Hr[i]; J[1][1] += x[i].y * Hs[i]; J[1][2] += x[i].y * Ht[i];
		J[2][0] += x[i].z * Hr[i]; J[2][1] += x[i].z * Hs[i]; J[2][2] += x[i].z * Ht[i];
	}

	// invert jacobian
	J.invert();

	// evaluate dH/dX = J^(-T)*dH/dr
	for (int i = 0; i < N; i++)
	{
		G[i].x = J[0][0] * Hr[i] + J[1][0] * Hs[i] + J[2][0] * Ht[i];
		G[i].y = J[0][1] * Hr[i] + J[1][1] * Hs[i] + J[2][1] * Ht[i];
		G[i].z = J[0][2] * Hr[i] + J[1][2] * Hs[i] + J[2][2] * Ht[i];
	}
}

//-----------------------------------------------------------------------------
// This function calculates the deformation gradient for a given state with respect to a user-
// defined reference state.
mat3d deform_grad(FEPostModel& fem, int n, double r, double s, double t, int nstate, int nref = 0)
{
	// get the mesh
	FEState& state = *fem.GetState(nstate);
	Post::FEPostMesh& m = *state.GetFEMesh();

	// get the element
	FEElement_& el = m.ElementRef(n);
	int N = el.Nodes();

	// we can only define this for solid elements
	if (el.IsSolid() == false)
	{
		// for non-solid elements, let's return the identity for now
		return mat3d::identity();
	}

	// get the nodal positions
	const int MN = FSElement::MAX_NODES;
	vec3f X[MN], x[MN];
	if (nref < 0)
	{
		Post::FERefState* ref = state.m_ref;
		for (int i = 0; i < N; i++)
		{
			int node = el.m_node[i];
			X[i] = ref->m_Node[node].m_rt;
			x[i] = fem.NodePosition(node, nstate);
		}
	}
	else
	{
		for (int i = 0; i < N; i++)
		{
			int node = el.m_node[i];
			X[i] = fem.NodePosition(node, nref);
			x[i] = fem.NodePosition(node, nstate);
		}
	}

	// get the shape function derivatives
	double Hr[MN], Hs[MN], Ht[MN];
	el.shape_deriv(Hr, Hs, Ht, r, s, t);

	// evaluate jacobian
	mat3d J; J.zero();
	for (int i=0; i<N; i++)
	{
		J[0][0] += X[i].x*Hr[i]; J[0][1] += X[i].x*Hs[i]; J[0][2] += X[i].x*Ht[i];
		J[1][0] += X[i].y*Hr[i]; J[1][1] += X[i].y*Hs[i]; J[1][2] += X[i].y*Ht[i];
		J[2][0] += X[i].z*Hr[i]; J[2][1] += X[i].z*Hs[i]; J[2][2] += X[i].z*Ht[i];
	}

	// invert jacobian
	if (J.invert() == false) return mat3d(0,0,0,0,0,0,0,0,0);
	
	// evaluate dH/dX = J^(-T)*dH/dr
	double HX[MN], HY[MN], HZ[MN];
	for (int i=0; i<N; i++)
	{
		HX[i] = J[0][0]*Hr[i] + J[1][0]*Hs[i] + J[2][0]*Ht[i];
		HY[i] = J[0][1]*Hr[i] + J[1][1]*Hs[i] + J[2][1]*Ht[i];
		HZ[i] = J[0][2]*Hr[i] + J[1][2]*Hs[i] + J[2][2]*Ht[i];
	}

	// evaluate deformation gradient F
	mat3d F; F.zero();
	for (int i=0; i<N; i++)
	{
		F[0][0] += x[i].x*HX[i]; F[0][1] += x[i].x*HY[i]; F[0][2] += x[i].x*HZ[i];
		F[1][0] += x[i].y*HX[i]; F[1][1] += x[i].y*HY[i]; F[1][2] += x[i].y*HZ[i];
		F[2][0] += x[i].z*HX[i]; F[2][1] += x[i].z*HY[i]; F[2][2] += x[i].z*HZ[i];
	}
	return F;
}

//-----------------------------------------------------------------------------
// This function calculates the deformation gradient for a given state with respect to a user-
// defined reference state.
mat2d deform_grad_2d(FEPostModel& fem, int n, double r, double s, int nstate, int nref = 0)
{
	// get the mesh
	FEState& state = *fem.GetState(nstate);
	Post::FEPostMesh& m = *state.GetFEMesh();

	// get the element
	FEElement_& el = m.ElementRef(n);
	int N = el.Nodes();

	// we can only define this for shell elements
	if (el.IsShell() == false)
	{
		// for non-solid elements, let's return the identity for now
		return mat2d(1.0, 0.0, 0.0, 1.0);
	}

	// get the nodal positions
	const int MN = FSElement::MAX_NODES;
	vec3f X[MN], x[MN];
	if (nref < 0)
	{
		Post::FERefState* ref = state.m_ref;
		for (int i = 0; i < N; i++)
		{
			int node = el.m_node[i];
			X[i] = ref->m_Node[node].m_rt;
			x[i] = fem.NodePosition(node, nstate);
		}
	}
	else
	{
		for (int i = 0; i < N; i++)
		{
			int node = el.m_node[i];
			X[i] = fem.NodePosition(node, nref);
			x[i] = fem.NodePosition(node, nstate);
		}
	}

	// get the shape function derivatives
	double Hr[MN], Hs[MN];
	el.shape_deriv_2d(Hr, Hs, r, s);

	// evaluate jacobian
	mat2d J; J.zero();
	for (int i = 0; i < N; i++)
	{
		J[0][0] += X[i].x * Hr[i]; J[0][1] += X[i].x * Hs[i];
		J[1][0] += X[i].y * Hr[i]; J[1][1] += X[i].y * Hs[i];
	}

	// invert jacobian
	mat2d Ji = J.inverse();

	// evaluate dH/dX = J^(-T)*dH/dr
	double HX[MN], HY[MN];
	for (int i = 0; i < N; i++)
	{
		HX[i] = Ji[0][0] * Hr[i] + Ji[1][0] * Hs[i];
		HY[i] = Ji[0][1] * Hr[i] + Ji[1][1] * Hs[i];
	}

	// evaluate deformation gradient F
	mat2d F; F.zero();
	for (int i = 0; i < N; i++)
	{
		F[0][0] += x[i].x * HX[i]; F[0][1] += x[i].x * HY[i];
		F[1][0] += x[i].y * HX[i]; F[1][1] += x[i].y * HY[i];
	}
	return F;
}

//-----------------------------------------------------------------------------
// Deformation gradient
DeformationGradient::DeformationGradient(FEState* pm, ModelDataField* pdf) : FEElemData_T<mat3f, DATA_MULT>(pm, pdf)
{
}

void DeformationGradient::eval(int n, mat3f* pv)
{
	// get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);

	// if this is not a solid element, return 0
	if (e.IsSolid() == false)
	{
		mat3f z; z.zero();
		int N = e.Nodes();
		for (int i=0; i<N; ++i) pv[i] = z;
		return;
	}

	// get the state
	int nstate = m_state->GetID();

	// loop over all the element nodes
	int N = e.Nodes();
	for (int i=0; i<N; ++i)
	{
		// get the iso-parameteric coordinates
		double q[3];
		e.iso_coord(i, q);

		// get the deformation gradient
		mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate);
		pv[i] = to_mat3f(F);
	}
}

//-----------------------------------------------------------------------------
Post::FEMeshData* StrainDataField::CreateData(FEState* pstate)
{
	switch (m_measure)
	{
	case INF_STRAIN        : return new InfStrain       (pstate, this);
	case RIGHT_CAUCHY_GREEN: return new RightCauchyGreen(pstate, this);
	case RIGHT_STRETCH     : return new RightStretch    (pstate, this);
	case LAGRANGE          : return new LagrangeStrain  (pstate, this);
	case BIOT              : return new BiotStrain      (pstate, this);
	case RIGHT_HENCKY      : return new RightHencky     (pstate, this);
	case LEFT_CAUCHY_GREEN : return new LeftCauchyGreen (pstate, this);
	case LEFT_STRETCH      : return new LeftStretch     (pstate, this);
	case LEFT_HENCKY       : return new LeftHencky      (pstate, this);
	case ALMANSI           : return new AlmansiStrain         (pstate, this);
	default:
		assert(false);
	}

	return 0;
}

//-----------------------------------------------------------------------------
// infinitesimal strain
//
void InfStrain::eval(int n, mat3fs* pv)
{
	// get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);
    // get the iso-parameteric coordinates of the element center
    double q[3];
    e.iso_coord(-1, q);

	// get the state
	int nstate = m_state->GetID();

	// get the deformation gradient
	int nref = ReferenceState();
	mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate, nref);

	// evaluate strain tensor U = F-I
	double U[3][3];
	U[0][0] = F[0][0] - 1; U[0][1] = F[0][1]    ; U[0][2] = F[0][2];
	U[1][0] = F[1][0]    ; U[1][1] = F[1][1] - 1; U[1][2] = F[1][2];
	U[2][0] = F[2][0]    ; U[2][1] = F[2][1]    ; U[2][2] = F[2][2]-1;

	// evaluate small strain tensor eij = 0.5*(Uij + Uji)
	mat3fs E;
	E.x = (float) (U[0][0]); 
	E.y = (float) (U[1][1]); 
	E.z = (float) (U[2][2]); 
	E.xy = (float) (0.5*(U[0][1] + U[1][0]));
	E.yz = (float) (0.5*(U[1][2] + U[2][1]));
	E.xz = (float) (0.5*(U[0][2] + U[2][0]));

	// a-ok
	(*pv) = E;
}

//-----------------------------------------------------------------------------
// Right Cauchy-Green tensor C
//
void RightCauchyGreen::eval(int n, mat3fs* pv)
{
	// get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);
    // get the iso-parameteric coordinates of the element center
    double q[3];
    e.iso_coord(-1, q);

	// get the state
	int nstate = m_state->GetID();

	// get the deformation gradient
	int nref = ReferenceState();
	mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate, nref);
	
	// evaluate right Cauchy-Green C = Ft*F
	double C[3][3] = {0};
	for (int k=0; k<3; k++)
	{
		C[0][0] += F[k][0]*F[k][0]; C[0][1] += F[k][0]*F[k][1]; C[0][2] += F[k][0]*F[k][2];
		C[1][0] += F[k][1]*F[k][0]; C[1][1] += F[k][1]*F[k][1]; C[1][2] += F[k][1]*F[k][2];
		C[2][0] += F[k][2]*F[k][0]; C[2][1] += F[k][2]*F[k][1]; C[2][2] += F[k][2]*F[k][2];
	}
	
	// convert C to a symmetric Matrix
	mat3fs c;
	c.x = (float) C[0][0];
	c.y = (float) C[1][1];
	c.z = (float) C[2][2];
	c.xy = (float)(0.5*(C[0][1] + C[1][0]));
	c.yz = (float)(0.5*(C[1][2] + C[2][1]));
	c.xz = (float)(0.5*(C[0][2] + C[2][0]));
	
	*pv = c;
}

//-----------------------------------------------------------------------------
// Right stretch tensor U
//
void RightStretch::eval(int n, mat3fs* pv)
{
	// get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);
    // get the iso-parameteric coordinates of the element center
    double q[3];
    e.iso_coord(-1, q);

	// get the state
	int nstate = m_state->GetID();

	// get the deformation gradient
	int nref = ReferenceState();
	mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate, nref);
	
	// evaluate right Cauchy-Green C = Ft*F
	double C[3][3] = {0};
	for (int k=0; k<3; k++)
	{
		C[0][0] += F[k][0]*F[k][0]; C[0][1] += F[k][0]*F[k][1]; C[0][2] += F[k][0]*F[k][2];
		C[1][0] += F[k][1]*F[k][0]; C[1][1] += F[k][1]*F[k][1]; C[1][2] += F[k][1]*F[k][2];
		C[2][0] += F[k][2]*F[k][0]; C[2][1] += F[k][2]*F[k][1]; C[2][2] += F[k][2]*F[k][2];
	}
	
	// convert C to a symmetric Matrix
	mat3fs c;
	c.x = (float) C[0][0];
	c.y = (float) C[1][1];
	c.z = (float) C[2][2];
	c.xy = (float)(0.5*(C[0][1] + C[1][0]));
	c.yz = (float)(0.5*(C[1][2] + C[2][1]));
	c.xz = (float)(0.5*(C[0][2] + C[2][0]));
	
	// get the eigenvalues and eigenvectors of C
	float l[3];
	vec3f v[3];
	c.eigen(v, l);
	
	// the eigenvalues of U are the sqrt(lam) of those of C
	l[0] = sqrt(l[0]);
	l[1] = sqrt(l[1]);
	l[2] = sqrt(l[2]);
	
	// build the right stretch tensor
	mat3fs U;
	U.x  = l[0]*v[0].x*v[0].x + l[1]*v[1].x*v[1].x + l[2]*v[2].x*v[2].x;
	U.y  = l[0]*v[0].y*v[0].y + l[1]*v[1].y*v[1].y + l[2]*v[2].y*v[2].y;
	U.z  = l[0]*v[0].z*v[0].z + l[1]*v[1].z*v[1].z + l[2]*v[2].z*v[2].z;
	U.xy = l[0]*v[0].x*v[0].y + l[1]*v[1].x*v[1].y + l[2]*v[2].x*v[2].y;
	U.yz = l[0]*v[0].y*v[0].z + l[1]*v[1].y*v[1].z + l[2]*v[2].y*v[2].z;
	U.xz = l[0]*v[0].x*v[0].z + l[1]*v[1].x*v[1].z + l[2]*v[2].x*v[2].z;
	
	*pv = U;
}

//-----------------------------------------------------------------------------
// Green-Lagrange strain evaluated at element center
//
void LagrangeStrain::eval(int n, mat3fs* pv)
{
	// get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);

	// get the state
	int nstate = m_state->GetID();

	if (e.IsBeam())
	{
		// get the mesh
		FEPostModel& fem = *GetFSModel();
		FEState& state = *fem.GetState(nstate);
		Post::FEPostMesh& m = *state.GetFEMesh();

		Post::FERefState* ref = state.m_ref;
		vec3f R0 = ref->m_Node[e.m_node[0]].m_rt;
		vec3f R1 = ref->m_Node[e.m_node[1]].m_rt;

		vec3f r0 = fem.NodePosition(e.m_node[0], nstate);
		vec3f r1 = fem.NodePosition(e.m_node[1], nstate);

		double L = (R1 - R0).Length();
		double l = (r1 - r0).Length();

		double lam = (L != 0 ? l / L : 0);

		mat3fs& E = *pv; 
		E.x = 0.5f * (lam * lam - 1.0f);
		E.y = 0.0f; E.z = 0.0f;
		E.xy = E.yz = E.xz = 0.f;
		
		return;
	}
	else if (e.IsSolid() == false)
	{
		*pv = mat3fs();
		return;
	}

    // get the iso-parameteric coordinates of the element center
    double q[3];
    e.iso_coord(-1, q);

	// get the deformation gradient
	int nref = ReferenceState();
	mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate, nref);
	
	// evaluate right Cauchy-Green C = Ft*F
	double C[3][3] = {0};
	for (int k=0; k<3; k++)
	{
		C[0][0] += F[k][0]*F[k][0]; C[0][1] += F[k][0]*F[k][1]; C[0][2] += F[k][0]*F[k][2];
		C[1][0] += F[k][1]*F[k][0]; C[1][1] += F[k][1]*F[k][1]; C[1][2] += F[k][1]*F[k][2];
		C[2][0] += F[k][2]*F[k][0]; C[2][1] += F[k][2]*F[k][1]; C[2][2] += F[k][2]*F[k][2];
	}
	
	// evaluate E
    mat3fs E;
    E.x = (float) (0.5*(C[0][0] - 1));
    E.y = (float) (0.5*(C[1][1] - 1));
    E.z = (float) (0.5*(C[2][2] - 1));
    E.xy = (float) (0.5*(C[0][1]));
    E.yz = (float) (0.5*(C[1][2]));
    E.xz = (float) (0.5*(C[0][2]));
	
	*pv = E;
}

//-----------------------------------------------------------------------------
// Green-Lagrange strain for 2D elements, evaluated at element center
//
void LagrangeStrain2D::eval(int n, mat3fs* pv)
{
	// get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);

	// get the state
	int nstate = m_state->GetID();

	if (e.IsShell() == false)
	{
		*pv = mat3fs();
		return;
	}

	// get the iso-parameteric coordinates of the element center
	double q[2];
	e.iso_coord_2d(-1, q);

	// get the deformation gradient
	mat2d F = deform_grad_2d(*GetFSModel(), n, q[0], q[1], nstate, -1);

	// evaluate right Cauchy-Green C = Ft*F
	double C[2][2] = { 0 };
	for (int k = 0; k < 2; k++)
	{
		C[0][0] += F[k][0] * F[k][0]; C[0][1] += F[k][0] * F[k][1];
		C[1][0] += F[k][1] * F[k][0]; C[1][1] += F[k][1] * F[k][1];
	}

	// evaluate E
	mat3fs E;
	E.x = (float)(0.5 * (C[0][0] - 1));
	E.y = (float)(0.5 * (C[1][1] - 1));
	E.z = 0.f;
	E.xy = (float)(0.5 * (C[0][1]));
	E.yz = 0.f;
	E.xz = 0.f;

	*pv = E;
}


//-----------------------------------------------------------------------------
// infinitesimal strain for 2D elements, evaluated at element center
void InfStrain2D::eval(int n, mat3fs* pv)
{
	// get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);

	// get the state
	int nstate = m_state->GetID();

	if (e.IsShell() == false)
	{
		*pv = mat3fs();
		return;
	}

	// get the iso-parameteric coordinates of the element center
	double q[2];
	e.iso_coord_2d(-1, q);

	// get the deformation gradient
	mat2d F = deform_grad_2d(*GetFSModel(), n, q[0], q[1], nstate, -1);

	// evaluate strain tensor U = F-I
	double U[2][2];
	U[0][0] = F[0][0] - 1; U[0][1] = F[0][1];
	U[1][0] = F[1][0]; U[1][1] = F[1][1] - 1;

	// evaluate small strain tensor eij = 0.5*(Uij + Uji)
	mat3fs E;
	E.x = (float)(U[0][0]);
	E.y = (float)(U[1][1]);
	E.z = 0.0f;
	E.xy = (float)(0.5 * (U[0][1] + U[1][0]));
	E.yz = 0.0f;
	E.xz = 0.0f;

	// a-ok
	(*pv) = E;
}

//-----------------------------------------------------------------------------
// Biot strain
//
void BiotStrain::eval(int n, mat3fs* pv)
{
	// get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);

	// make sure it's a solid
	if (e.IsSolid() == false)
	{
		*pv = mat3fs();
		return;
	}

    // get the iso-parameteric coordinates of the element center
    double q[3];
    e.iso_coord(-1, q);

	// get the state
	int nstate = m_state->GetID();

	// get the deformation gradient
	int nref = ReferenceState();
	mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate, nref);

	// evaluate right Cauchy-Green C = Ft*F
	double C[3][3] = {0};
	for (int k=0; k<3; k++)
	{
		C[0][0] += F[k][0]*F[k][0]; C[0][1] += F[k][0]*F[k][1]; C[0][2] += F[k][0]*F[k][2];
		C[1][0] += F[k][1]*F[k][0]; C[1][1] += F[k][1]*F[k][1]; C[1][2] += F[k][1]*F[k][2];
		C[2][0] += F[k][2]*F[k][0]; C[2][1] += F[k][2]*F[k][1]; C[2][2] += F[k][2]*F[k][2];
	}

	// convert C to a symmetric Matrix
	mat3fs c;
	c.x = (float) C[0][0];
	c.y = (float) C[1][1];
	c.z = (float) C[2][2];
	c.xy = (float)(0.5*(C[0][1] + C[1][0]));
	c.yz = (float)(0.5*(C[1][2] + C[2][1]));
	c.xz = (float)(0.5*(C[0][2] + C[2][0]));

	// get the eigenvalues and eigenvectors of C
	float l[3];
	vec3f v[3];
	c.eigen(v, l);

	// the eigenvalues of E are the sqrt(lam)-1 of those of C
	l[0] = sqrt(l[0])-1;
	l[1] = sqrt(l[1])-1;
	l[2] = sqrt(l[2])-1;

	// build the biot tensor
	mat3fs B;
	B.x  = l[0]*v[0].x*v[0].x + l[1]*v[1].x*v[1].x + l[2]*v[2].x*v[2].x;
	B.y  = l[0]*v[0].y*v[0].y + l[1]*v[1].y*v[1].y + l[2]*v[2].y*v[2].y;
	B.z  = l[0]*v[0].z*v[0].z + l[1]*v[1].z*v[1].z + l[2]*v[2].z*v[2].z;
	B.xy = l[0]*v[0].x*v[0].y + l[1]*v[1].x*v[1].y + l[2]*v[2].x*v[2].y;
	B.yz = l[0]*v[0].y*v[0].z + l[1]*v[1].y*v[1].z + l[2]*v[2].y*v[2].z;
	B.xz = l[0]*v[0].x*v[0].z + l[1]*v[1].x*v[1].z + l[2]*v[2].x*v[2].z;

	*pv = B;
}

//-----------------------------------------------------------------------------
// Hencky strain (material frame)
//
void RightHencky::eval(int n, mat3fs* pv)
{
	// get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);

	// make sure it's a solid
	if (e.IsSolid() == false)
	{
		*pv = mat3fs();
		return;
	}

    // get the iso-parameteric coordinates of the element center
    double q[3];
    e.iso_coord(-1, q);

	// get the state
	int nstate = m_state->GetID();

	// get the deformation gradient
	int nref = ReferenceState();
	mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate, nref);
	
	// evaluate right Cauchy-Green C = Ft*F
	double C[3][3] = {0};
	for (int k=0; k<3; k++)
	{
		C[0][0] += F[k][0]*F[k][0]; C[0][1] += F[k][0]*F[k][1]; C[0][2] += F[k][0]*F[k][2];
		C[1][0] += F[k][1]*F[k][0]; C[1][1] += F[k][1]*F[k][1]; C[1][2] += F[k][1]*F[k][2];
		C[2][0] += F[k][2]*F[k][0]; C[2][1] += F[k][2]*F[k][1]; C[2][2] += F[k][2]*F[k][2];
	}
	
	// convert C to a symmetric Matrix
	mat3fs c;
	c.x = (float) C[0][0];
	c.y = (float) C[1][1];
	c.z = (float)C[2][2];
	c.xy = (float)(0.5*(C[0][1] + C[1][0]));
	c.yz = (float)(0.5*(C[1][2] + C[2][1]));
	c.xz = (float)(0.5*(C[0][2] + C[2][0]));
	
	// get the eigenvalues and eigenvectors of C
	float l[3];
	vec3f v[3];
	c.eigen(v, l);
	
	// the eigenvalues of H are the log(sqrt(lam)) of those of C
	l[0] = log(sqrt(l[0]));
	l[1] = log(sqrt(l[1]));
	l[2] = log(sqrt(l[2]));
	
	// build the Hencky strain tensor
	mat3fs H;
	H.x  = l[0]*v[0].x*v[0].x + l[1]*v[1].x*v[1].x + l[2]*v[2].x*v[2].x;
	H.y  = l[0]*v[0].y*v[0].y + l[1]*v[1].y*v[1].y + l[2]*v[2].y*v[2].y;
	H.z  = l[0]*v[0].z*v[0].z + l[1]*v[1].z*v[1].z + l[2]*v[2].z*v[2].z;
	H.xy = l[0]*v[0].x*v[0].y + l[1]*v[1].x*v[1].y + l[2]*v[2].x*v[2].y;
	H.yz = l[0]*v[0].y*v[0].z + l[1]*v[1].y*v[1].z + l[2]*v[2].y*v[2].z;
	H.xz = l[0]*v[0].x*v[0].z + l[1]*v[1].x*v[1].z + l[2]*v[2].x*v[2].z;
	
	*pv = H;
}

//-----------------------------------------------------------------------------
// Left Cauchy-Green tensor B
//
void LeftCauchyGreen::eval(int n, mat3fs* pv)
{
    // get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);

	// make sure it's a solid
	if (e.IsSolid() == false)
	{
		*pv = mat3fs();
		return;
	}

    // get the iso-parameteric coordinates of the element center
    double q[3];
    e.iso_coord(-1, q);
    
	// get the state
	int nstate = m_state->GetID();

    // get the deformation gradient
	int nref = ReferenceState();
	mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate, nref);
    
    // evaluate left Cauchy-Green B = F*Ft
    double B[3][3] = {0};
    for (int k=0; k<3; k++)
    {
        B[0][0] += F[0][k]*F[0][k]; B[0][1] += F[0][k]*F[1][k]; B[0][2] += F[0][k]*F[2][k];
        B[1][0] += F[1][k]*F[0][k]; B[1][1] += F[1][k]*F[1][k]; B[1][2] += F[1][k]*F[2][k];
        B[2][0] += F[2][k]*F[0][k]; B[2][1] += F[2][k]*F[1][k]; B[2][2] += F[2][k]*F[2][k];
    }
    
    // convert B to a symmetric Matrix
    mat3fs b;
    b.x = (float) B[0][0];
    b.y = (float) B[1][1];
    b.z = (float) B[2][2];
    b.xy = (float)(0.5*(B[0][1] + B[1][0]));
    b.yz = (float)(0.5*(B[1][2] + B[2][1]));
    b.xz = (float)(0.5*(B[0][2] + B[2][0]));
    
    *pv = b;
}

//-----------------------------------------------------------------------------
// Left stretch tensor V
//
void LeftStretch::eval(int n, mat3fs* pv)
{
    // get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);

	// make sure it's a solid
	if (e.IsSolid() == false)
	{
		*pv = mat3fs();
		return;
	}

    // get the iso-parameteric coordinates of the element center
    double q[3];
    e.iso_coord(-1, q);
    
	// get the state
	int nstate = m_state->GetID();

    // get the deformation gradient
	int nref = ReferenceState();
	mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate, nref);
    
    // evaluate left Cauchy-Green B = F*Ft
    double B[3][3] = {0};
    for (int k=0; k<3; k++)
    {
        B[0][0] += F[0][k]*F[0][k]; B[0][1] += F[0][k]*F[1][k]; B[0][2] += F[0][k]*F[2][k];
        B[1][0] += F[1][k]*F[0][k]; B[1][1] += F[1][k]*F[1][k]; B[1][2] += F[1][k]*F[2][k];
        B[2][0] += F[2][k]*F[0][k]; B[2][1] += F[2][k]*F[1][k]; B[2][2] += F[2][k]*F[2][k];
    }
    
    // convert B to a symmetric Matrix
    mat3fs b;
    b.x = (float) B[0][0];
    b.y = (float) B[1][1];
    b.z = (float) B[2][2];
    b.xy = (float)(0.5*(B[0][1] + B[1][0]));
    b.yz = (float)(0.5*(B[1][2] + B[2][1]));
    b.xz = (float)(0.5*(B[0][2] + B[2][0]));
    
    // get the eigenvalues and eigenvectors of B
    float l[3];
    vec3f v[3];
    b.eigen(v, l);
    
    // the eigenvalues of V are the sqrt(lam) of those of B
    l[0] = sqrt(l[0]);
    l[1] = sqrt(l[1]);
    l[2] = sqrt(l[2]);
    
    // build the left stretch tensor
    mat3fs V;
    V.x  = l[0]*v[0].x*v[0].x + l[1]*v[1].x*v[1].x + l[2]*v[2].x*v[2].x;
    V.y  = l[0]*v[0].y*v[0].y + l[1]*v[1].y*v[1].y + l[2]*v[2].y*v[2].y;
    V.z  = l[0]*v[0].z*v[0].z + l[1]*v[1].z*v[1].z + l[2]*v[2].z*v[2].z;
    V.xy = l[0]*v[0].x*v[0].y + l[1]*v[1].x*v[1].y + l[2]*v[2].x*v[2].y;
    V.yz = l[0]*v[0].y*v[0].z + l[1]*v[1].y*v[1].z + l[2]*v[2].y*v[2].z;
    V.xz = l[0]*v[0].x*v[0].z + l[1]*v[1].x*v[1].z + l[2]*v[2].x*v[2].z;
    
    *pv = V;
}

//-----------------------------------------------------------------------------
// Hencky strain (spatial frame)
//
void LeftHencky::eval(int n, mat3fs* pv)
{
    // get the element
	FEElement_& e = GetFEState()->GetFEMesh()->ElementRef(n);

	// make sure it's a solid
	if (e.IsSolid() == false)
	{
		*pv = mat3fs();
		return;
	}

    // get the iso-parameteric coordinates of the element center
    double q[3];
    e.iso_coord(-1, q);
    
	// get the state
	int nstate = m_state->GetID();

    // get the deformation gradient
	int nref = ReferenceState();
	mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate, nref);
    
    // evaluate left Cauchy-Green B = F*Ft
    double B[3][3] = {0};
    for (int k=0; k<3; k++)
    {
        B[0][0] += F[0][k]*F[0][k]; B[0][1] += F[0][k]*F[1][k]; B[0][2] += F[0][k]*F[2][k];
        B[1][0] += F[1][k]*F[0][k]; B[1][1] += F[1][k]*F[1][k]; B[1][2] += F[1][k]*F[2][k];
        B[2][0] += F[2][k]*F[0][k]; B[2][1] += F[2][k]*F[1][k]; B[2][2] += F[2][k]*F[2][k];
    }
    
    // convert B to a symmetric Matrix
    mat3fs b;
    b.x = (float) B[0][0];
    b.y = (float) B[1][1];
    b.z = (float) B[2][2];
    b.xy = (float)(0.5*(B[0][1] + B[1][0]));
    b.yz = (float)(0.5*(B[1][2] + B[2][1]));
    b.xz = (float)(0.5*(B[0][2] + B[2][0]));
    
    // get the eigenvalues and eigenvectors of B
    float l[3];
    vec3f v[3];
    b.eigen(v, l);
    
    // the eigenvalues of H are the log(sqrt(lam)) of those of B
    l[0] = log(sqrt(l[0]));
    l[1] = log(sqrt(l[1]));
    l[2] = log(sqrt(l[2]));
    
    // build the Hencky strain tensor
    mat3fs H;
    H.x  = l[0]*v[0].x*v[0].x + l[1]*v[1].x*v[1].x + l[2]*v[2].x*v[2].x;
    H.y  = l[0]*v[0].y*v[0].y + l[1]*v[1].y*v[1].y + l[2]*v[2].y*v[2].y;
    H.z  = l[0]*v[0].z*v[0].z + l[1]*v[1].z*v[1].z + l[2]*v[2].z*v[2].z;
    H.xy = l[0]*v[0].x*v[0].y + l[1]*v[1].x*v[1].y + l[2]*v[2].x*v[2].y;
    H.yz = l[0]*v[0].y*v[0].z + l[1]*v[1].y*v[1].z + l[2]*v[2].y*v[2].z;
    H.xz = l[0]*v[0].x*v[0].z + l[1]*v[1].x*v[1].z + l[2]*v[2].x*v[2].z;
    
    *pv = H;
}

//-----------------------------------------------------------------------------
// Almansi (Eulerian) strain tensor e
//
void AlmansiStrain::eval(int n, mat3fs* pv)
{
    // get the element
	FEElement_& el = GetFEState()->GetFEMesh()->ElementRef(n);

	// make sure it's a solid
	if (el.IsSolid() == false)
	{
		*pv = mat3fs();
		return;
	}

    // get the iso-parameteric coordinates of the element center
    double q[3];
    el.iso_coord(-1, q);
    
	// get the state
	int nstate = m_state->GetID();

    // get the deformation gradient
	int nref = ReferenceState();
	mat3d F = deform_grad(*GetFSModel(), n, q[0], q[1], q[2], nstate, nref);
    
    // evaluate left Cauchy-Green B = F*Ft
    double B[3][3] = {0};
    for (int k=0; k<3; k++)
    {
        B[0][0] += F[0][k]*F[0][k]; B[0][1] += F[0][k]*F[1][k]; B[0][2] += F[0][k]*F[2][k];
        B[1][0] += F[1][k]*F[0][k]; B[1][1] += F[1][k]*F[1][k]; B[1][2] += F[1][k]*F[2][k];
        B[2][0] += F[2][k]*F[0][k]; B[2][1] += F[2][k]*F[1][k]; B[2][2] += F[2][k]*F[2][k];
    }
    
    // invert B
	mat3d bi;
    bi(0,0) = B[0][0];
    bi(1,1) = B[1][1];
    bi(2,2) = B[2][2];
    bi(0,1) = bi(1,0) = (0.5*(B[0][1] + B[1][0]));
    bi(1,2) = bi(2,1) = (0.5*(B[1][2] + B[2][1]));
    bi(0,2) = bi(2,0) = (0.5*(B[0][2] + B[2][0]));
    
    bi.invert();
    
    mat3fs e((1.-bi(0,0))/2.,
             (1.-bi(1,1))/2.,
             (1.-bi(2,2))/2.,
             -bi(0,1)/2.,
             -bi(1,2)/2.,
             -bi(0,2)/2.);
    
    *pv = e;
}

//-----------------------------------------------------------------------------
double element_volume(int ntype, vec3d* r)
{
	switch (ntype)
    {
        case FE_TET4: return tet4_volume(r); break;
        case FE_TET5: return tet5_volume(r); break;
        case FE_TET10: return tet10_volume(r); break;
        case FE_TET15: return tet15_volume(r); break;
        case FE_TET20: return tet20_volume(r); break;
        case FE_HEX8: return hex8_volume(r); break;
        case FE_HEX20: return hex20_volume(r); break;
        case FE_HEX27: return hex27_volume(r); break;
        case FE_PENTA6: return penta6_volume(r); break;
        case FE_PENTA15: return penta15_volume(r); break;
        case FE_PYRA5: return pyra5_volume(r); break;
        case FE_PYRA13: return pyra13_volume(r); break;
    }
	return 0.0;
}

//-----------------------------------------------------------------------------
// Volume ratio
//
void VolumeRatio::eval(int n, float* pv)
{
	FEPostModel& fem = *GetFSModel();
	FEPostMesh& m = *GetFEMesh();

	FEElement_* pe = &m.ElementRef(n);

	// get the initial and current nodal positions
	int N = pe->Nodes();
	vec3d X[FSElement::MAX_NODES], x[FSElement::MAX_NODES];
	int ntime = m_state->GetID();
	for (int i=0; i<N; i++) 
	{ 
		int node = pe->m_node[i];
		x[i] = to_vec3d(m_state->NodePosition(node));
		X[i] = to_vec3d(m_state->NodeRefPosition(node));
	}

	double v0 = element_volume(pe->Type(), X);
	double vt = element_volume(pe->Type(), x);

	if (v0 == 0.0) *pv = 0.f;
	else *pv = (float) (vt / v0);
}

/*
void VolumeRatio::eval(int n, float* pv)
{
	static double dN_hex[3][8] = {
		{ -.125,  .125,  .125, -.125, -.125,  .125, .125, -.125 },
		{ -.125, -.125,  .125,  .125, -.125, -.125, .125,  .125 },
		{ -.125, -.125, -.125, -.125,  .125,  .125, .125,  .125 }};

	static double dN_pen[3][6] = {
		{ -.5,  .5,   0, -.5, .5,  0 },
		{ -.5,   0,  .5, -.5,  0, .5 },
		{ -.5, -.5, -.5,  .5, .5, .5 }};

	static double dN_tet[3][4] = {
		{-1,1,0,0 },
		{-1,0,1,0 },
		{-1,0,0,1 }};

	FEPostModel& fem = *GetFSModel();
	FEPostMesh& m = *GetFEMesh();
	FEElement_* pe = &m.ElementRef(n);

	double *dN1, *dN2, *dN3;
	int N = pe->Nodes();
	switch (pe->Type())
	{
	case FE_HEX8:
	case FE_HEX20:
	case FE_HEX27:
		dN1 = dN_hex[0];
		dN2 = dN_hex[1];
		dN3 = dN_hex[2];
		N = 8;
		break;
	case FE_PENTA6:
    case FE_PENTA15:
        dN1 = dN_pen[0];
		dN2 = dN_pen[1];
		dN3 = dN_pen[2];
		N = 6;
		break;
	case FE_TET4:
	case FE_TET10:
	case FE_TET15:
		dN1 = dN_tet[0];
		dN2 = dN_tet[1];
		dN3 = dN_tet[2];
		N = 4;
		break;
	default:
		*pv = 0;
		return;
	}

	// get the initial and current nodal positions
	vec3f X[8], x[8];
	int ntime = m_state->GetID();
	for (int i=0; i<N; i++) 
	{ 
		int node = pe->m_node[i];
		X[i] = fem.NodePosition(node, 0); 
		x[i] = fem.NodePosition(node, ntime);
	}

	// calculate (average) partial derivatives
	double dNx[8], dNy[8], dNz[8];

	double J[9] = {0}, Ji[9], detJ;
	for (int i=0; i<N; ++i)
	{
		J[0] += dN1[i]*X[i].x;
		J[1] += dN1[i]*X[i].y;
		J[2] += dN1[i]*X[i].z;
		J[3] += dN2[i]*X[i].x;
		J[4] += dN2[i]*X[i].y;
		J[5] += dN2[i]*X[i].z;
		J[6] += dN3[i]*X[i].x;
		J[7] += dN3[i]*X[i].y;
		J[8] += dN3[i]*X[i].z;
	}
	detJ = J[0]*J[4]*J[8] + J[1]*J[5]*J[6] + J[2]*J[3]*J[7]
		  -J[2]*J[4]*J[6] - J[1]*J[3]*J[8] - J[0]*J[5]*J[7];

	if (detJ <= 0) { *pv = -1; return; }

	detJ = 1.0/detJ;

	Ji[0] = detJ*( J[4]*J[8] - J[5]*J[7]);
	Ji[1] = detJ*(-J[1]*J[8] + J[2]*J[7]);
	Ji[2] = detJ*( J[1]*J[5] - J[2]*J[4]);
	Ji[3] = detJ*(-J[3]*J[8] + J[5]*J[6]);
	Ji[4] = detJ*( J[0]*J[8] - J[2]*J[6]);
	Ji[5] = detJ*(-J[0]*J[5] + J[2]*J[3]);
	Ji[6] = detJ*( J[3]*J[7] - J[4]*J[6]);
	Ji[7] = detJ*(-J[0]*J[7] + J[1]*J[6]);
	Ji[8] = detJ*( J[0]*J[4] - J[1]*J[3]);

	for (int i=0; i<N; ++i)
	{
		dNx[i] = Ji[0]*dN1[i] + Ji[1]*dN2[i] + Ji[2]*dN3[i];
		dNy[i] = Ji[3]*dN1[i] + Ji[4]*dN2[i] + Ji[5]*dN3[i];
		dNz[i] = Ji[6]*dN1[i] + Ji[7]*dN2[i] + Ji[8]*dN3[i];
	}

	// calculate average def gradient
	double F[9] = {0}, detF;
	for (int i=0; i<N; ++i)
	{
		F[0] += dNx[i]*x[i].x;
		F[1] += dNy[i]*x[i].x;
		F[2] += dNz[i]*x[i].x;
		F[3] += dNx[i]*x[i].y;
		F[4] += dNy[i]*x[i].y;
		F[5] += dNz[i]*x[i].y;
		F[6] += dNx[i]*x[i].z;
		F[7] += dNy[i]*x[i].z;
		F[8] += dNz[i]*x[i].z;
	}

	detF = F[0]*F[4]*F[8] + F[1]*F[5]*F[6] + F[2]*F[3]*F[7]
		  -F[2]*F[4]*F[6] - F[1]*F[3]*F[8] - F[0]*F[5]*F[7];

	*pv = (float) detF;
}
*/

void ElementVolume::eval(int iel, float* pv)
{
	FEPostMesh& m = *GetFEState()->GetFEMesh();
	FEElement_& el = m.ElementRef(iel);
	int nn = el.Nodes();

	int ntime = GetFEState()->GetID();
	FEPostModel& fem = *GetFSModel();
	vec3d rt[FSElement::MAX_NODES];
	for (int i = 0; i < nn; ++i) rt[i] = to_vec3d(fem.NodePosition(el.m_node[i], ntime));

	double vol = 0.0;
	switch (el.Type())
	{
	case FE_HEX8   : vol = hex8_volume   (rt); break;
	case FE_HEX20  : vol = hex20_volume  (rt); break;
	case FE_HEX27  : vol = hex27_volume  (rt); break;
	case FE_TET4   : vol = tet4_volume   (rt); break;
	case FE_TET5   : vol = tet5_volume   (rt); break;
	case FE_TET10  : vol = tet10_volume  (rt); break;
	case FE_TET15  : vol = tet15_volume  (rt); break;
	case FE_TET20  : vol = tet20_volume  (rt); break;
	case FE_PENTA6 : vol = penta6_volume (rt); break;
	case FE_PENTA15: vol = penta15_volume(rt); break;
	case FE_PYRA5  : vol = pyra5_volume  (rt); break;
	case FE_PYRA13 : vol = pyra13_volume (rt); break;
	default:
		break;
	}
	*pv = (float)vol;
}

//-----------------------------------------------------------------------------
// This function calculates the aspect ratio of the element.
// For hex, wedge and quad elements, the AR is defined as the ratio of the 
// largets diagonal to the smallest diagonal. For tet and tri elements it is
// defined as the largest so smallest edge.
//
void AspectRatio::eval(int iel, float* pv)
{
	FEPostMesh& m = *GetFEState()->GetFEMesh();
	FEElement_& el = m.ElementRef(iel);
	int nn = el.Nodes();
	if (el.Type() == FE_HEX20) nn = 8;
	if (el.Type() == FE_HEX27) nn = 8;

	int nd = -1;
	int (*pd)[2];
	switch (el.Type())
	{
	case FE_HEX8 : nd = 16; pd = DIAG_HEX; break;
	case FE_HEX20: nd = 16; pd = DIAG_HEX; break;
	case FE_HEX27: nd = 16; pd = DIAG_HEX; break;
	default:
		*pv = -1.f;
		return;
	}

	int ntime = GetFEState()->GetID();
	FEPostModel& fem = *GetFSModel();
	vec3f rt[8];
	for (int i=0; i<nn; ++i) rt[i] = fem.NodePosition(el.m_node[i], ntime);

	vec3f et;
	float l;
	float minl = 1e20f, maxl = -1e20f;
	for (int i=0; i<nd; ++i)
	{
		et = rt[ pd[i][1]] - rt[ pd[i][0] ];
		l = et.Length();
		if (l < minl) minl = l;
		if (l > maxl) maxl = l;
	}

	float ar;
	if (minl == 0.f) ar = 1e20f;
	else ar = maxl / minl;

	*pv = ar;
}

//-----------------------------------------------------------------------------
// Calculate the max edge angle of an element
void MaxEdgeAngle::eval(int iel, float* pv)
{
	FEPostModel& fem = *GetFSModel();
	FEPostMesh& m = *GetFEState()->GetFEMesh();
	FEElement_& el = m.ElementRef(iel);

	if (el.Nodes() != 8) { *pv = 90.f; return; }

	int ntime = GetFEState()->GetID();
	vec3f rt[8];
	for (int i=0; i<8; ++i) rt[i] = fem.NodePosition(el.m_node[i], ntime);

	vec3f et[12];
	for (int i = 0; i<12; ++i)
	{
		et[i] = rt[ ET_HEX[i][1]] - rt[ ET_HEX[i][0] ];
		et[i].Normalize();
	}

	float w, wmax = 1.f;

	// node 0: 0, 3, 8
	w = -(et[0]*et[3]); if (w < wmax) wmax = w;
	w = -(et[3]*et[8]); if (w < wmax) wmax = w;
	w =   et[8]*et[0] ; if (w < wmax) wmax = w;

	// node 1: 0, 1, 9
	w = -(et[0]*et[9]); if (w < wmax) wmax = w;
	w = -(et[1]*et[0]); if (w < wmax) wmax = w;
	w =   et[9]*et[1] ; if (w < wmax) wmax = w;

	// node 2: 1, 2, 11
	w = -(et[ 1]*et[ 2]); if (w < wmax) wmax = w;
	w =   et[ 2]*et[11] ; if (w < wmax) wmax = w;
	w = -(et[11]*et[ 1]); if (w < wmax) wmax = w;

	// node 3: 2, 3, 10
	w = -(et[ 2]*et[ 3]); if (w < wmax) wmax = w;
	w =   et[ 3]*et[10];  if (w < wmax) wmax = w;
	w = -(et[10]*et[ 2]); if (w < wmax) wmax = w;

	// node 4: 4, 7, 8
	w = -(et[4]*et[7]); if (w < wmax) wmax = w;
	w =   et[7]*et[8] ; if (w < wmax) wmax = w;
	w = -(et[8]*et[4]); if (w < wmax) wmax = w;

	// node 5: 4, 5, 9
	w = -(et[4]*et[5]); if (w < wmax) wmax = w;
	w = -(et[5]*et[9]); if (w < wmax) wmax = w;
	w =   et[9]*et[4] ; if (w < wmax) wmax = w;

	// node 6: 5, 6, 11
	w = -(et[ 5]*et[ 6]); if (w < wmax) wmax = w;
	w = -(et[ 6]*et[11]); if (w < wmax) wmax = w;
	w =   et[11]*et[ 5] ; if (w < wmax) wmax = w;

	// node 7: 6, 7, 10
	w = -(et[ 6]*et[ 7]); if (w < wmax) wmax = w;
	w = -(et[ 7]*et[10]); if (w < wmax) wmax = w;
	w =   et[10]*et[ 6] ; if (w < wmax) wmax = w;

	*pv = (float)(acos(wmax)*180.0/PI);
}

//-----------------------------------------------------------------------------
void MinEdgeAngle::eval(int iel, float* pv)
{
	FEPostModel& fem = *GetFSModel();
	FEPostMesh& m = *GetFEState()->GetFEMesh();
	FEElement_& el = m.ElementRef(iel);
	if (el.Nodes() != 8) { *pv = 90.f; return; }

	int ntime = GetFEState()->GetID();
	vec3f rt[8];
	for (int i=0; i<8; ++i) rt[i] = fem.NodePosition(el.m_node[i], ntime);

	vec3f et[12];
	for (int i=0; i<12; ++i)
	{
		et[i] = rt[ ET_HEX[i][1]] - rt[ ET_HEX[i][0] ];
		et[i].Normalize();
	}

	float w, wmin = -1.f;

	// node 0: 0, 3, 8
	w = -(et[0]*et[3]); if (w > wmin) wmin = w;
	w = -(et[3]*et[8]); if (w > wmin) wmin = w;
	w =   et[8]*et[0] ; if (w > wmin) wmin = w;

	// node 1: 0, 1, 9
	w = -(et[0]*et[9]); if (w > wmin) wmin = w;
	w = -(et[1]*et[0]); if (w > wmin) wmin = w;
	w =   et[9]*et[1] ; if (w > wmin) wmin = w;

	// node 2: 1, 2, 11
	w = -(et[ 1]*et[ 2]); if (w > wmin) wmin = w;
	w =   et[ 2]*et[11] ; if (w > wmin) wmin = w;
	w = -(et[11]*et[ 1]); if (w > wmin) wmin = w;

	// node 3: 2, 3, 10
	w = -(et[ 2]*et[ 3]); if (w > wmin) wmin = w;
	w =   et[ 3]*et[10] ; if (w > wmin) wmin = w;
	w = -(et[10]*et[ 2]); if (w > wmin) wmin = w;

	// node 4: 4, 7, 8
	w = -(et[4]*et[7]); if (w > wmin) wmin = w;
	w =   et[7]*et[8] ; if (w > wmin) wmin = w;
	w = -(et[8]*et[4]); if (w > wmin) wmin = w;

	// node 5: 4, 5, 9
	w = -(et[4]*et[5]); if (w > wmin) wmin = w;
	w = -(et[5]*et[9]); if (w > wmin) wmin = w;
	w =   et[9]*et[4] ; if (w > wmin) wmin = w;

	// node 6: 5, 6, 11
	w = -(et[ 5]*et[ 6]); if (w > wmin) wmin = w;
	w = -(et[ 6]*et[11]); if (w > wmin) wmin = w;
	w =   et[11]*et[ 5] ; if (w > wmin) wmin = w;

	// node 7: 6, 7, 10
	w = -(et[ 6]*et[ 7]); if (w > wmin) wmin = w;
	w = -(et[ 7]*et[10]); if (w > wmin) wmin = w;
	w =   et[10]*et[ 6] ; if (w > wmin) wmin = w;

	*pv = (float)(acos(wmin)*180.0/PI);
}

//-----------------------------------------------------------------------------
void NodePosition::eval(int n, vec3f* pv)
{
	int ntime = m_state->GetID();
	*pv = GetFSModel()->NodePosition(n, ntime);
}

//-----------------------------------------------------------------------------
void NodeInitPos::eval(int n, vec3f* pv)
{
	*pv = GetFSModel()->NodePosition(n, 0);
}

//=============================================================================

Curvature::Curvature(FEState* state, CurvatureField* pdf) : FEFaceData_T<float, DATA_NODE>(state, pdf), m_pdf(pdf)
{ 
	m_face.assign(state->GetFEMesh()->Faces(), 1); 
}

void Curvature::eval(int n, float* f)
{ 
	eval_curvature(n, f, m_pdf->m_measure); 
}

//-----------------------------------------------------------------------------
void Curvature::level(int n, int l, set<int>& nl1)
{
	// get the model's surface
	FEPostModel* pfem = GetFSModel();
	FEPostMesh* pmesh = GetFEState()->GetFEMesh();

	// add the first node
	nl1.insert(n);

	// loop over all levels
	vector<int> nl2; nl2.reserve(64);
	for (int k=0; k<=l; ++k)
	{
		// reset face marks
		set<int>::iterator it;
		for (it = nl1.begin(); it != nl1.end(); ++it)
		{
			// get the node-face list
			const vector<NodeFaceRef>& nfl = pmesh->NodeFaceList(*it);
			int NF = nfl.size();

			// add the other nodes
			for (int i=0; i<NF; ++i)
			{
				if (m_face[nfl[i].fid] == 1)
				{
					FSFace& f = pmesh->Face(nfl[i].fid); 
					f.m_ntag = 0;
				}
			}
		}

		// loop over all nodes
		nl2.clear();
		for (it = nl1.begin(); it != nl1.end(); ++it)
		{
			// get the node-face list
			const vector<NodeFaceRef>& nfl = pmesh->NodeFaceList(*it);
			int NF = nfl.size();

			// add the other nodes
			for (int i=0; i<NF; ++i)
			{
				FSFace& f = pmesh->Face(nfl[i].fid);
				if (m_face[nfl[i].fid] == 1)
				{
					if (f.m_ntag == 0)
					{
						int ne = f.Nodes();
						for (int j=0; j<ne; ++j) if (f.n[j] != *it) nl2.push_back(f.n[j]);
						f.m_ntag = 1;
					}
				}
			}
		}

		// merge sets
		if (!nl2.empty()) nl1.insert(nl2.begin(), nl2.end());
	}
}

//-----------------------------------------------------------------------------
void Curvature::set_facelist(vector<int>& L)
{
	for (int i=0; i<(int)m_face.size(); ++i) m_face[i] = 0;
	for (int i=0; i<(int)L.size(); ++i) m_face[L[i]] = 1;
}

//-----------------------------------------------------------------------------
void Curvature::eval_curvature(int n, float* f, int m)
{
	// get the model's surface
	FEPostModel* pfem = GetFSModel();
	FSMeshBase* pmesh = GetFEState()->GetFEMesh();

	// get the face
	FSFace& face = pmesh->Face(n);
	for (int i=0; i<face.Nodes(); ++i)
	{
		int in = face.n[i];
		f[i] = nodal_curvature(in, m);
	}
}

//-----------------------------------------------------------------------------
float Curvature::nodal_curvature(int n, int measure)
{
	// get the model's surface
	FEPostModel* pfem = GetFSModel();
	FEPostMesh* pmesh = GetFEState()->GetFEMesh();
	int ntime = m_state->GetID();

	// get the reference nodal position
	vec3f r0 = pfem->NodePosition(n, ntime);

	// get the node-face list
	const vector<NodeFaceRef>& nfl = pmesh->NodeFaceList(n);
	int NF = nfl.size();

	// estimate surface normal
	vec3f sn(0,0,0);
	for (int i=0; i<NF; ++i) 
	{
		if (m_face[nfl[i].fid] == 1)
		{
			FSFace& f = pmesh->Face(nfl[i].fid);
			sn += pfem->FaceNormal(f, ntime);
		}
	}
	sn.Normalize();

	// array of nodal points
	vector<vec3f> x; x.reserve(128);

	// number of levels
	int nlevels = m_pdf->m_nlevels - 1;
	if (nlevels <  0) nlevels = 0;
	if (nlevels > 10) nlevels = 10;

	// get the neighbors
	set<int> nl1;
	level(n, nlevels, nl1);

	// get the node coordinates
	set<int>::iterator it;
	for (it=nl1.begin(); it != nl1.end(); ++it)
	{
		if (*it != n) x.push_back(pfem->NodePosition(*it, ntime));
	}

	// evaluate curvature
	float K = FEMeshMetrics::eval_curvature(x, r0, sn, measure, m_pdf->m_bext == 0, m_pdf->m_nmax);

	// return result
	return (float) K;
}

//=============================================================================
int PrincCurvatureVector::m_nlevels = 1;
int PrincCurvatureVector::m_nmax    = 1;
int PrincCurvatureVector::m_bext    = 0;

//-----------------------------------------------------------------------------
void PrincCurvatureVector::level(int n, int l, set<int>& nl1)
{
	// get the model's surface
	FEPostModel* pfem = GetFSModel();
	FEPostMesh* pmesh = GetFEMesh();

	// add the first node
	nl1.insert(n);

	// loop over all levels
	vector<int> nl2; nl2.reserve(64);
	for (int k=0; k<=l; ++k)
	{
		// reset face marks
		set<int>::iterator it;
		for (it = nl1.begin(); it != nl1.end(); ++it)
		{
			// get the node-face list
			const vector<NodeFaceRef>& nfl = pmesh->NodeFaceList(*it);
			int NF = nfl.size();

			// add the other nodes
			for (int i=0; i<NF; ++i)
			{
				if (m_face[nfl[i].fid] == 1)
				{
					FSFace& f = pmesh->Face(nfl[i].fid); 
					f.m_ntag = 0;
				}
			}
		}

		// loop over all nodes
		nl2.clear();
		for (it = nl1.begin(); it != nl1.end(); ++it)
		{
			// get the node-face list
			const vector<NodeFaceRef>& nfl = pmesh->NodeFaceList(*it);
			int NF = nfl.size();

			// add the other nodes
			for (int i=0; i<NF; ++i)
			{
				FSFace& f = pmesh->Face(nfl[i].fid);
				if (m_face[nfl[i].fid] == 1)
				{
					if (f.m_ntag == 0)
					{
						int ne = f.Nodes();
						for (int j=0; j<ne; ++j) if (f.n[j] != *it) nl2.push_back(f.n[j]);
						f.m_ntag = 1;
					}
				}
			}
		}

		// merge sets
		if (!nl2.empty()) nl1.insert(nl2.begin(), nl2.end());
	}
}

//-----------------------------------------------------------------------------
void PrincCurvatureVector::set_facelist(vector<int>& L)
{
	for (int i=0; i<(int)m_face.size(); ++i) m_face[i] = 0;
	for (int i=0; i<(int)L.size(); ++i) m_face[L[i]] = 1;
}

//-----------------------------------------------------------------------------
void PrincCurvatureVector::eval(int n, vec3f* fv, int m)
{
	// get the model's surface
	FEPostModel* pfem = GetFSModel();
	FEPostMesh* pmesh = GetFEMesh();

	// get the face
	FSFace& face = pmesh->Face(n);
	for (int i=0; i<face.Nodes(); ++i)
	{
		int in = face.n[i];
		fv[i] = nodal_curvature(in, m);
	}
}

//-----------------------------------------------------------------------------
vec3f PrincCurvatureVector::nodal_curvature(int n, int m)
{
	// get the model's surface
	FEPostModel* pfem = GetFSModel();
	FEPostMesh* pmesh = GetFEMesh();
	int ntime = m_state->GetID();

	// get the reference nodal position
	vec3f r0 = pfem->NodePosition(n, ntime);

	// get the node-face list
	const vector<NodeFaceRef>& nfl = pmesh->NodeFaceList(n);
	int NF = nfl.size();

	// estimate surface normal
	vec3f sn(0,0,0);
	for (int i=0; i<NF; ++i) 
	{
		if (m_face[nfl[i].fid] == 1)
		{
			FSFace& f = pmesh->Face(nfl[i].fid);
			sn += pfem->FaceNormal(f, ntime);
		}
	}
	sn.Normalize();

	// array of nodal points
	vector<vec3f> x; x.reserve(128);
	vector<vec3f> y; y.reserve(128);

	// number of levels
	int nlevels = m_nlevels - 1;
	if (nlevels <  0) nlevels = 0;
	if (nlevels > 10) nlevels = 10;

	// get the neighbors
	set<int> nl1;
	level(n, nlevels, nl1);

	// get the node coordinates
	set<int>::iterator it;
	for (it=nl1.begin(); it != nl1.end(); ++it)
	{
		if (*it != n) x.push_back(pfem->NodePosition(*it, ntime));
	}
	y.resize(x.size());
	int nn = x.size();

	// for less than three neighbors, we assume K = 0
	vec3f kv;
	if (nn < 3) kv = vec3f(0,0,0);
	else if ((nn < 5) || (m_bext == 0))
	{
		// for less than 5 points, or if the extended quadric flag is zero,
		// we use the quadric method

		// construct local coordinate system
		vec3f e3 = sn;

		vec3f qx(1.f-sn.x*sn.x, -sn.y*sn.x, -sn.z*sn.x);
		if (qx.Length() < 1e-5) qx = vec3f(-sn.x*sn.y, 1.f-sn.y*sn.y, -sn.z*sn.y);
		qx.Normalize();
		vec3f e1 = qx;

		vec3f e2 = e3 ^ e1;

		mat3f Q;
		Q[0][0] = e1.x; Q[1][0] = e2.x; Q[2][0] = e3.x;
		Q[0][1] = e1.y; Q[1][1] = e2.y; Q[2][1] = e3.y;
		Q[0][2] = e1.z; Q[1][2] = e2.z; Q[2][2] = e3.z;
		mat3f Qt = Q.transpose();

		// map coordinates
		for (int i=0; i<nn; ++i)
		{
			vec3f tmp = x[i] - r0;
			y[i] = Q*tmp;
		}

		// setup the linear system
		matrix R(nn, 3);
		vector<double> r(nn);
		for (int i=0; i<nn; ++i)
		{
			vec3f& p = y[i];
			R[i][0] = p.x*p.x;
			R[i][1] = p.x*p.y;
			R[i][2] = p.y*p.y;
			r[i] = p.z;
		}

		// solve for quadric coefficients
		vector<double> q(3);
		R.lsq_solve(q, r);
		double a = q[0], b = q[1], c = q[2];

		// calculate curvature
		double alpha = 0.5*atan2(b, a - c);
		mat3f S;
		S[0][0] =  cos(alpha); S[0][1] = -sin(alpha); S[0][2] = 0.0;
		S[1][0] =  sin(alpha); S[1][1] =  cos(alpha); S[1][2] = 0.0;
		S[2][0] =         0.0; S[2][1] =        0.0; S[2][2] = 1.0;
	
		if (m == 0) {
			kv = vec3f(1,0,0);
			kv = (Qt*S)*kv;
		}
		if (m == 1) {
			kv = vec3f(0,1,0);
			kv = (Qt*S)*kv;
		}
	}
	else
	{
		// loop until converged
		const int NMAX = 100;
		if (m_nmax > NMAX) m_nmax = NMAX;
		if (m_nmax < 1) m_nmax = 1;
		int niter = 0;
		while (niter < m_nmax)
		{
			// construct local coordinate system
			vec3f e3 = sn;

			vec3f qx(1.f-sn.x*sn.x, -sn.y*sn.x, -sn.z*sn.x);
			if (qx.Length() < 1e-5) qx = vec3f(-sn.x*sn.y, 1.f-sn.y*sn.y, -sn.z*sn.y);
			qx.Normalize();
			vec3f e1 = qx;

			vec3f e2 = e3 ^ e1;

			mat3f Q;
			Q[0][0] = e1.x; Q[1][0] = e2.x; Q[2][0] = e3.x;
			Q[0][1] = e1.y; Q[1][1] = e2.y; Q[2][1] = e3.y;
			Q[0][2] = e1.z; Q[1][2] = e2.z; Q[2][2] = e3.z;
			mat3f Qt = Q.transpose();

			// map coordinates
			for (int i=0; i<nn; ++i)
			{
				vec3f tmp = x[i] - r0;
				y[i] = Q*tmp;
			}

			// setup the linear system
			matrix R(nn, 5);
			vector<double> r(nn);
			for (int i=0; i<nn; ++i)
			{
				vec3f& p = y[i];
				R[i][0] = p.x*p.x;
				R[i][1] = p.x*p.y;
				R[i][2] = p.y*p.y;
				R[i][3] = p.x;
				R[i][4] = p.y;
				r[i] = p.z;
			}

			// solve for quadric coefficients
			vector<double> q(5);
			R.lsq_solve(q, r);
			double a = q[0], b = q[1], c = q[2], d = q[3], e = q[4];

			// calculate curvature
			double alpha = 0.5*atan2(b, a - c);
			mat3f S;
			S[0][0] =  cos(alpha); S[0][1] = -sin(alpha); S[0][2] = 0.0;
			S[1][0] =  sin(alpha); S[1][1] =  cos(alpha); S[1][2] = 0.0;
			S[2][0] =         0.0; S[2][1] =        0.0; S[2][2] = 1.0;
	
			if (m == 0) {
				kv = vec3f(1,0,0);
				kv = (Qt*S)*kv;
			}
			if (m == 1) {
				kv = vec3f(0,1,0);
				kv = (Qt*S)*kv;
			}

			// setup the new normal
			sn = vec3f(- (float) d, - (float) e, 1.f);
			sn.Normalize();
			sn = Qt*sn;

			// increase counter
			niter++;
		}
	}

	// return result
	return kv;
}


int SurfaceCongruency::m_nlevels = 1;
int SurfaceCongruency::m_nmax = 1;
int SurfaceCongruency::m_bext = 0;

//-----------------------------------------------------------------------------
void SurfaceCongruency::set_facelist(vector<int>& L)
{
	for (int i=0; i<(int)m_face.size(); ++i) m_face[i] = 0;
	for (int i=0; i<(int)L.size(); ++i) m_face[L[i]] = 1;
}

//-----------------------------------------------------------------------------
void SurfaceCongruency::eval(int n, float* f)
{
	FEPointCongruency map;
	map.SetLevels(m_nlevels);
	map.m_nmax = m_nmax;
	map.m_bext = m_bext;

	// get the model's surface
	FEPostModel* pfem = GetFSModel();
	FEPostMesh* pmesh = GetFEMesh();

	// get the face
	FSFace& face = pmesh->Face(n);
	int ntime = m_state->GetID();
	for (int i = 0; i<face.Nodes(); ++i)
	{
		int in = face.n[i];
		f[i] = (float) map.Congruency(pmesh, in).Ke;
	}
}

//-----------------------------------------------------------------------------
void FEFacetArea::eval(int n, float* f)
{
	FEPostMesh* pmesh = GetFEMesh();
	FSFace& face = pmesh->Face(n);

	std::vector<Post::NODEDATA>& data = m_state->m_NODE;

	vector<vec3d> r(face.Nodes());
	for (int i = 0; i < face.Nodes(); ++i) r[i] = to_vec3d(data[face.n[i]].m_rt);

	// NOTE: Passing the face type doesn't work! 
	f[0] = (float)pmesh->FaceArea(r, face.Nodes());
}

//=============================================================================
void VolumeStrain::eval(int n, float* pv)
{
	// shape function derivatives evaluated at center of element
	static double dN_hex[3][8] = {
		{ -.125,  .125,  .125, -.125, -.125,  .125, .125, -.125 },
		{ -.125, -.125,  .125,  .125, -.125, -.125, .125,  .125 },
		{ -.125, -.125, -.125, -.125,  .125,  .125, .125,  .125 }};

	static double dN_pen[3][6] = {
		{ -.5,  .5,   0, -.5, .5,  0 },
		{ -.5,   0,  .5, -.5,  0, .5 },
		{ -.5, -.5, -.5,  .5, .5, .5 }};

	static double dN_tet4[3][4] = {
		{-1,1,0,0 },
		{-1,0,1,0 },
		{-1,0,0,1 }};

	static double dN_tet10[3][10] = {
		{-3, -1,  0,  0, 4, 0, 0, 0, 0, 0 },
		{-3,  0, -1,  0, 0, 0, 4, 0, 0, 0 },
		{-3,  0,  0, -1, 0, 0, 0, 4, 0, 0 } };

	FEElement_* pe = &GetFEMesh()->ElementRef(n);

	// TODO: Some higher-order elements are mapped to lower-order elements.
	int N = pe->Nodes();
	if (pe->Type() == FE_HEX20) N = 8;
	if (pe->Type() == FE_HEX27) N = 8;
	if (pe->Type() == FE_TET15) N = 10;
	if (pe->Type() == FE_PENTA15) N = 6;

	double *dN1, *dN2, *dN3;
	switch (pe->Type())
	{
	case FE_HEX8:
	case FE_HEX20:
	case FE_HEX27:
		dN1 = dN_hex[0];
		dN2 = dN_hex[1];
		dN3 = dN_hex[2];
		break;
	case FE_PENTA6:
	case FE_PENTA15:
		dN1 = dN_pen[0];
		dN2 = dN_pen[1];
		dN3 = dN_pen[2];
		break;
	case FE_TET4:
		dN1 = dN_tet4[0];
		dN2 = dN_tet4[1];
		dN3 = dN_tet4[2];
		break;
	case FE_TET10:
	case FE_TET15:
		dN1 = dN_tet10[0];
		dN2 = dN_tet10[1];
		dN3 = dN_tet10[2];
		break;
	default:
		*pv = 0;
		return;
	}

	// get the initial and current nodal positions
	vec3f X[FSElement::MAX_NODES], x[FSElement::MAX_NODES];
	FEPostModel* fem = GetFSModel();
	int ntime = m_state->GetID();
	for (int i = 0; i<N; i++)
	{ 
		int node = pe->m_node[i];
		x[i] = m_state->NodePosition(node);
		X[i] = m_state->NodeRefPosition(node);
	}

	// calculate partial derivatives
	double dNx[FSElement::MAX_NODES], dNy[FSElement::MAX_NODES], dNz[FSElement::MAX_NODES];
	double J[9] = {0}, Ji[9], detJ;
	for (int i=0; i<N; ++i)
	{
		J[0] += dN1[i]*X[i].x;
		J[1] += dN1[i]*X[i].y;
		J[2] += dN1[i]*X[i].z;
		J[3] += dN2[i]*X[i].x;
		J[4] += dN2[i]*X[i].y;
		J[5] += dN2[i]*X[i].z;
		J[6] += dN3[i]*X[i].x;
		J[7] += dN3[i]*X[i].y;
		J[8] += dN3[i]*X[i].z;
	}
	detJ = J[0]*J[4]*J[8] + J[1]*J[5]*J[6] + J[2]*J[3]*J[7]
		  -J[2]*J[4]*J[6] - J[1]*J[3]*J[8] - J[0]*J[5]*J[7];

	if (detJ <= 0) { *pv = -1; return; }

	detJ = 1.0/detJ;

	Ji[0] = detJ*( J[4]*J[8] - J[5]*J[7]);
	Ji[1] = detJ*(-J[1]*J[8] + J[2]*J[7]);
	Ji[2] = detJ*( J[1]*J[5] - J[2]*J[4]);
	Ji[3] = detJ*(-J[3]*J[8] + J[5]*J[6]);
	Ji[4] = detJ*( J[0]*J[8] - J[2]*J[6]);
	Ji[5] = detJ*(-J[0]*J[5] + J[2]*J[3]);
	Ji[6] = detJ*( J[3]*J[7] - J[4]*J[6]);
	Ji[7] = detJ*(-J[0]*J[7] + J[1]*J[6]);
	Ji[8] = detJ*( J[0]*J[4] - J[1]*J[3]);

	for (int i=0; i<N; ++i)
	{
		dNx[i] = Ji[0]*dN1[i] + Ji[1]*dN2[i] + Ji[2]*dN3[i];
		dNy[i] = Ji[3]*dN1[i] + Ji[4]*dN2[i] + Ji[5]*dN3[i];
		dNz[i] = Ji[6]*dN1[i] + Ji[7]*dN2[i] + Ji[8]*dN3[i];
	}

	// calculate deformation gradient
	double F[9] = {0}, detF;
	for (int i=0; i<N; ++i)
	{
		F[0] += dNx[i]*x[i].x;
		F[1] += dNy[i]*x[i].x;
		F[2] += dNz[i]*x[i].x;
		F[3] += dNx[i]*x[i].y;
		F[4] += dNy[i]*x[i].y;
		F[5] += dNz[i]*x[i].y;
		F[6] += dNx[i]*x[i].z;
		F[7] += dNy[i]*x[i].z;
		F[8] += dNz[i]*x[i].z;
	}

	detF = F[0]*F[4]*F[8] + F[1]*F[5]*F[6] + F[2]*F[3]*F[7]
		  -F[2]*F[4]*F[6] - F[1]*F[3]*F[8] - F[0]*F[5]*F[7];

	*pv = (float) log(detF);
}

//=============================================================================
// Element pressure
//-----------------------------------------------------------------------------
ElemPressure::ElemPressure(FEState* pm, ModelDataField* pdf) : FEElemData_T<float, DATA_ITEM>(pm, pdf)
{
	// find the stress field
	FEPostModel& fem = *pm->GetFSModel();
	FEDataManager* pdm = fem.GetDataManager();
	m_nstress = pdm->FindDataField("stress");
}

//-----------------------------------------------------------------------------
void ElemPressure::eval(int n, float* pv)
{
	// get the state
	FEState& state = *GetFEState();

	// get stress field
	FEMeshData& rd = state.m_Data[m_nstress];
	FEElemData_T<mat3fs,DATA_ITEM>& dm = dynamic_cast<FEElemData_T<mat3fs,DATA_ITEM>&>(rd);

	// evaluate pressure
	if (dm.active(n))
	{
		mat3fs m;
		dm.eval(n, &m);

		*pv = -m.tr() / 3.f;
	}
	else *pv = 0.f;
}


//=============================================================================
// Element nodal pressure
//-----------------------------------------------------------------------------
ElemNodalPressure::ElemNodalPressure(FEState* state, ModelDataField* pdf) : FEElemData_T<float, DATA_MULT>(state, pdf)
{
	// find the stress field
	FEPostModel& fem = *state->GetFSModel();
	FEDataManager* pdm = fem.GetDataManager();
	m_nstress = pdm->FindDataField("nodal stress");
}

//-----------------------------------------------------------------------------
void ElemNodalPressure::eval(int n, float* pv)
{
	// get the state
	FEState& state = *GetFEState();

	// get stress field
	FEMeshData& rd = state.m_Data[m_nstress];
	FEElemData_T<mat3fs,DATA_MULT>& dm = dynamic_cast<FEElemData_T<mat3fs,DATA_MULT>&>(rd);

	// get number of nodes for this element
	int neln = state.GetFEMesh()->ElementRef(n).Nodes();

	// evaluate pressure
	if (dm.active(n))
	{
		mat3fs m[FSElement::MAX_NODES];
		dm.eval(n, m);

		for (int i=0; i<neln; ++i) pv[i] = -m[i].tr() / 3.f;
	}
	else 
	{
		for (int i=0; i<neln; ++i) pv[i] = 0.f;
	}
}

//=============================================================================
// Solid stress in a biphasic/multiphasic material
//-----------------------------------------------------------------------------
SolidStress::SolidStress(FEState* state, ModelDataField* pdf) : FEElemData_T<mat3fs, DATA_ITEM>(state, pdf)
{
	// find the stress field
	FEPostModel& fem = *state->GetFSModel();
	FEDataManager* pdm = fem.GetDataManager();
	m_nstress = pdm->FindDataField("stress");
	m_nflp = pdm->FindDataField("fluid pressure");
}

//-----------------------------------------------------------------------------
void SolidStress::eval(int n, mat3fs* pv)
{
	// get the state
	FEState& state = *GetFEState();
	
	// get stress field
	FEMeshData& rd = state.m_Data[m_nstress];
	FEElemData_T<mat3fs,DATA_ITEM>& dm = dynamic_cast<FEElemData_T<mat3fs,DATA_ITEM>&>(rd);
	
	// get fluid pressure field
	FEMeshData& rdp = state.m_Data[m_nflp];
	FEElemData_T<float,DATA_ITEM>& dmp = dynamic_cast<FEElemData_T<float,DATA_ITEM>&>(rdp);
	
	// evaluate pressure
	if (dm.active(n) && dmp.active(n))
	{
		mat3fs m;
		dm.eval(n, &m);
		
		float p;
		dmp.eval(n, &p);
		
		m.x += p;
		m.y += p;
		m.z += p;
		
		*pv = m;
	}
	else if (dm.active(n))
	{
		mat3fs m;
		dm.eval(n, &m);
		
		*pv = m;
	}
	else *pv = mat3fs(0.f,0.f,0.f,0.f,0.f,0.f);
}

FEElementMaterial::FEElementMaterial(FEState* state, ModelDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf)
{

}

void FEElementMaterial::eval(int n, float* pv)
{
	// get the state
	FEState& state = *GetFEState();

	FEPostMesh* mesh = state.GetFEMesh();
	FSElement& el = mesh->Element(n);
	pv[0] = (float) el.m_MatID;
}

//=============================================================================
FESurfaceNormal::FESurfaceNormal(FEState* state, ModelDataField* pdf) : FEFaceData_T<vec3f, DATA_ITEM>(state, pdf)
{

}

void FESurfaceNormal::eval(int n, vec3f* pv)
{
	FEPostMesh* mesh = GetFEState()->GetFEMesh();
	if (mesh)
	{
		if ((n >= 0) && (n < mesh->Faces()))
		{
			FSFace& face = mesh->Face(n);
			*pv = face.m_fn;
		}
	}
}
