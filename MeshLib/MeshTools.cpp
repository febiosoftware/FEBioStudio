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

#include "MeshTools.h"
#include "FENodeNodeList.h"
#include "Intersect.h"
using namespace std;

// calculate the closest-fitting circle of a triangle
// This assumes a are in fact 2D vectors
void fitCircle(const vec3d a[3], vec3d& o, double& R)
{
	// setup linear system
	double A[2][2];
	A[0][0] = 2.0*(a[0].x - a[1].x); A[0][1] = 2.0*(a[0].y - a[1].y);
	A[1][0] = 2.0*(a[0].x - a[2].x); A[1][1] = 2.0*(a[0].y - a[2].y);

	double b[2];
	b[0] = a[0].x*a[0].x - a[1].x*a[1].x + a[0].y*a[0].y - a[1].y*a[1].y;
	b[1] = a[0].x*a[0].x - a[2].x*a[2].x + a[0].y*a[0].y - a[2].y*a[2].y;

	// solve linear system
	double D = A[0][0] * A[1][1] - A[0][1] * A[1][0];
	o.x = (A[1][1] * b[0] - A[0][1] * b[1]) / D;
	o.y = (-A[1][0] * b[0] + A[0][0] * b[1]) / D;

	// calculate radius
	R = sqrt((a[0].x - o.x)*(a[0].x - o.x) + (a[0].y - o.y)*(a[0].y - o.y));
}

double area_triangle(vec3d r[3])
{
	vec3d e1 = r[1] - r[0];
	vec3d e2 = r[2] - r[0];

	return (e1 ^ e2).Length()*0.5;
}

double area_triangle(const vec3d& a, const vec3d& b, const vec3d& c)
{
	vec3d e1 = b - a;
	vec3d e2 = c - a;

	return (e1 ^ e2).Length()*0.5;
}

//-----------------------------------------------------------------------------
// This function calculates the intersection of a ray with a triangle
// and returns true if the ray intersects the triangle.
//
bool IntersectTri(vec3d* y, vec3d r, vec3d n, vec3d& q, double& g)
{
	vec3d e[2], E[2];
	double G[2][2], Gi[2][2], D;

	// create base vectors on triangle
	e[0] = y[1] - y[0];
	e[1] = y[2] - y[0];

	// create triangle normal
	vec3d m = e[0] ^ e[1]; m.Normalize();

	double eps = 0.01;

	double d = n*m;
	if (d != 0)
	{
		// distance from r to plane of triangle
		g = m*(y[0] - r) / d;

		// intersection point with plane of triangle
		q = r + n*g;

		// next, we decompose q into its components
		// in the triangle basis
		// we need to create the dual basis
		// first, we calculate the metric tensor
		G[0][0] = e[0] * e[0]; G[0][1] = e[0] * e[1];
		G[1][0] = e[1] * e[0]; G[1][1] = e[1] * e[1];

		// and its inverse
		D = G[0][0] * G[1][1] - G[0][1] * G[1][0];
		Gi[0][0] = 1 / D*G[1][1]; Gi[0][1] = -1 / D*G[0][1];
		Gi[1][0] = -1 / D*G[1][0]; Gi[1][1] = 1 / D*G[0][0];

		// build dual basis
		E[0] = e[0] * Gi[0][0] + e[1] * Gi[0][1];
		E[1] = e[0] * Gi[1][0] + e[1] * Gi[1][1];

		// get the components
		double rp = E[0] * (q - y[0]);
		double sp = E[1] * (q - y[0]);

		// see if the intersection point is inside the triangle
		if ((rp >= -eps) && (sp >= -eps) && (rp + sp <= 1 + eps)) return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//! This function calculates the intersection of a ray with a quad
//! and returns true if the ray intersected.
//!
bool IntersectQuad(vec3d* y, vec3d r, vec3d n, vec3d& q, double& g)
{
	// first we're going to see if the ray intersects the two subtriangles
	vec3d x1[3], x2[3];
	x1[0] = y[0]; x2[0] = y[2];
	x1[1] = y[1]; x2[1] = y[3];
	x1[2] = y[3]; x2[2] = y[1];

	bool b = false;
	double rp, sp;

	double eps = 0.01;

	if (IntersectTri(x1, r, n, q, g))
	{
		// we've intersected the first triangle
		b = true;
		rp = 0;
		sp = 0;
	}
	else if (IntersectTri(x2, r, n, q, g))
	{
		// we've intersected the second triangle
		b = true;
		rp = 0;
		sp = 0;
	}

	// if one of the triangels was intersected,
	// we calculate a more accurate projection
	if (b)
	{
		mat3d A;
		vec3d dx;
		vec3d F, F1, F2, F3;
		double H[4], H1[4], H2[4];

		double l1 = rp;
		double l2 = sp;
		double l3 = g;

		int nn = 0;
		int maxn = 5;
		do
		{
			// shape functions of quad
			H[0] = 0.25*(1 - l1)*(1 - l2);
			H[1] = 0.25*(1 + l1)*(1 - l2);
			H[2] = 0.25*(1 + l1)*(1 + l2);
			H[3] = 0.25*(1 - l1)*(1 + l2);
			q = y[0] * H[0] + y[1] * H[1] + y[2] * H[2] + y[3] * H[3];

			// shape function derivatives
			H1[0] = -0.25*(1 - l2); H2[0] = -0.25*(1 - l1);
			H1[1] = 0.25*(1 - l2); H2[1] = -0.25*(1 + l1);
			H1[2] = 0.25*(1 + l2); H2[2] = 0.25*(1 + l1);
			H1[3] = -0.25*(1 + l2); H2[3] = 0.25*(1 - l1);

			// calculate residual
			F = r + n*l3 - q;

			// residual derivatives
			F1 = -y[0] * H1[0] - y[1] * H1[1] - y[2] * H1[2] - y[3] * H1[3];
			F2 = -y[0] * H2[0] - y[1] * H2[1] - y[2] * H2[2] - y[3] * H2[3];
			F3 = n;

			// set up the tangent matrix
			A[0][0] = F1.x; A[0][1] = F2.x; A[0][2] = F3.x;
			A[1][0] = F1.y; A[1][1] = F2.y; A[1][2] = F3.y;
			A[2][0] = F1.z; A[2][1] = F2.z; A[2][2] = F3.z;

			// calculate solution increment
			mat3d Ai;
			Ai = A.inverse();
			dx = -(Ai*F);

			// update solution
			l1 += dx.x;
			l2 += dx.y;
			l3 += dx.z;

			++nn;
		} while ((dx.Length() > 1e-7) && (nn < maxn));

		// store results
		rp = l1;
		sp = l2;
		g = l3;

		// see if the point is inside the quad
		if ((rp >= -1 - eps) && (rp <= 1 + eps) &&
			(sp >= -1 - eps) && (sp <= 1 + eps)) return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Project a point to the mesh
//
vec3d ClosestNodeOnSurface(FSMesh& mesh, const vec3d& r, const vec3d& t)
{
	// tag all surface nodes
	mesh.TagAllNodes(0);

	// loop over all faces to identify the nodes that are facing r
	for (int i = 0; i<mesh.Faces(); ++i)
	{
		// only pick faces that are facing r
		FSFace& f = mesh.Face(i);
		if (t* to_vec3d(f.m_fn) < 0)
		{
			int n = f.Nodes();
			for (int j = 0; j<n; ++j) mesh.Node(f.n[j]).m_ntag = 1;
		}
	}

	// find the closest tagged nodes
	double L, Lmin = 1e99;
	vec3d p = r, q;
	for (int i = 0; i<mesh.Nodes(); ++i)
	{
		FSNode& node = mesh.Node(i);
		if (node.m_ntag)
		{
			q = r + t*((node.r - r)*t);
			L = fabs((node.r - q).Length());
			if (L < Lmin)
			{
				p = node.r;
				Lmin = L;
			}
		}
	}

	return p;
}

//-----------------------------------------------------------------------------
// Project a point to the surface of a FE mesh
//
vec3d ProjectToFace(FSMesh& mesh, vec3d p, FSFace &f, double &r, double &s, bool bedge)
{
	double R[2], u[2], D;

	vec3d q(0, 0, 0), y[4];

	double gr[4] = { -1, +1, +1, -1 };
	double gs[4] = { -1, -1, +1, +1 };
	double H[4], Hr[4], Hs[4], Hrs[4];
	double normu;

	int i, j;
	int NMAX = 10, n = 0;

	// number of face nodes
	int nf = f.Nodes();

	// get the elements nodal positions
	for (i = 0; i<nf; ++i) y[i] = mesh.Node(f.n[i]).r;

	// loop until converged
	do
	{
		if (nf == 4)
		{
			// do quadrilaterals
			for (i = 0; i<4; ++i)
			{
				H[i] = 0.25*(1 + gr[i] * r)*(1 + gs[i] * s);

				Hr[i] = 0.25*gr[i] * (1 + gs[i] * s);
				Hs[i] = 0.25*gs[i] * (1 + gr[i] * r);

				Hrs[i] = 0.25*gr[i] * gs[i];
			}
		}
		else
		{
			// do triangles
			H[0] = 1 - r - s;
			H[1] = r;
			H[2] = s;
			Hr[0] = -1; Hs[0] = -1;
			Hr[1] = 1; Hs[1] = 0;
			Hr[2] = 0; Hs[2] = 1;
			Hrs[0] = Hrs[1] = Hrs[2] = 0;
		}

		// set up the system of equations
		q = vec3d(0, 0, 0);
		R[0] = R[1] = 0;
		double A[2][2] = { 0 };
		for (i = 0; i<nf; ++i)
		{
			R[0] -= (p*y[i])*Hr[i];
			R[1] -= (p*y[i])*Hs[i];

			A[0][1] += (p*y[i])*Hrs[i];
			A[1][0] += (p*y[i])*Hrs[i];

			for (j = 0; j<nf; ++j)
			{
				R[0] -= -H[j] * Hr[i] * (y[i] * y[j]);
				R[1] -= -H[j] * Hs[i] * (y[i] * y[j]);

				A[0][0] += -(y[i] * y[j])*(Hr[i] * Hr[j]);
				A[1][1] += -(y[i] * y[j])*(Hs[i] * Hs[j]);

				A[0][1] += -(y[i] * y[j])*(Hs[j] * Hr[i] + H[i] * Hrs[j]);
				A[1][0] += -(y[i] * y[j])*(Hr[j] * Hs[i] + H[i] * Hrs[j]);
			}

			q += y[i] * H[i];
		}

		// determinant of A
		D = A[0][0] * A[1][1] - A[0][1] * A[1][0];

		// solve for u = A^(-1)*R
		u[0] = (A[1][1] * R[0] - A[0][1] * R[1]) / D;
		u[1] = (A[0][0] * R[1] - A[1][0] * R[0]) / D;

		normu = sqrt(u[0] * u[0] + u[1] * u[1]);

		r += u[0];
		s += u[1];

		++n;
	} while ((normu > 1e-7) && (n < NMAX));

	if (bedge)
	{
		if (nf == 4)
		{
			if (r < -1) q = ProjectToEdge(y[0], y[3], p, s);
			else if (r >  1) q = ProjectToEdge(y[1], y[2], p, s);
			else if (s < -1) q = ProjectToEdge(y[0], y[1], p, s);
			else if (s >  1) q = ProjectToEdge(y[3], y[2], p, s);
		}
		else
		{
			if (r < 0) q = ProjectToEdge(y[0], y[2], p, s);
			else if (s < 0) q = ProjectToEdge(y[0], y[1], p, s);
			else if (r + s>1) q = ProjectToEdge(y[1], y[2], p, s);
		}
	}

	return q;
}

//-----------------------------------------------------------------------------
// Project a node to an edge
vec3d ProjectToEdge(vec3d e1, vec3d e2, vec3d p, double& r)
{
	vec3d t = e2 - e1;
	r = (p - e1)*t / (t*t);
	if (r < 0) r = 0;
	if (r > 1) r = 1;
	return e1 + t*r;
}

//-----------------------------------------------------------------------------
std::vector<vec3d> FindAllIntersections(FSMeshBase& mesh, const vec3d& x, const vec3d& n, bool forwardOnly)
{
	vector<vec3d> q;
	int NF = mesh.Faces();
	vec3d r;
	double g;
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (FindIntersection(mesh, f, x, n, r, g))
		{
			if ((forwardOnly == false) || (g > 0.0))
			{
				q.push_back(r);
			}
		}
	}
	return q;
}

//-----------------------------------------------------------------------------
bool FindIntersection(FSMeshBase& mesh, const vec3d& x, const vec3d& n, vec3d& q, bool snap)
{
	int NF = mesh.Faces();
	vec3d r, rmin;
	double g, gmin = 1e99;
	bool b = false;
	int imin = -1;
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (FindIntersection(mesh, f, x, n, r, g))
		{
			if ((g > 0.0) && (g < gmin))
			{
				gmin = g;
				rmin = r;
				b = true;
				imin = i;
			}
		}
	}

	if (b) 
	{
		q = rmin;

		if (snap && (imin != -1))
		{
			FSFace& face = mesh.Face(imin);
			int nf = face.Nodes();
			double Dmin = 0.0;
			rmin = q;
			for (int i=0; i<nf; ++i)
			{
				vec3d ri = mesh.Node(face.n[i]).r;
				double D = (ri - q).SqrLength();
				if ((i==0) || (D < Dmin))
				{
					rmin = ri;
					Dmin = D;
				}
			}
			q = rmin;
		}
	}

	return b;
}

//-----------------------------------------------------------------------------
// Find the intersection.
//
bool FindIntersection(FSMeshBase& mesh, FSFace &f, const vec3d& x, const vec3d& n, vec3d& q, double& g)
{
	int N = f.Nodes();

	vec3d y[FSFace::MAX_NODES];
	for (int i = 0; i<N; ++i) y[i] = mesh.Node(f.n[i]).r;

	// call the correct intersection function
	if (N == 3) return IntersectTri(y, x, n, q, g);
	else if (N == 4) return IntersectQuad(y, x, n, q, g);

	// if we get here, the ray did not intersect the element
	return false;
}

#include <set>
//using namespace std;

bool projectToLine(const vec3d& p, const vec3d& r0, const vec3d& r1, vec3d& q)
{
	vec3d t = r1 - r0;
	double tt = t*t;
	if (tt != 0.0)
	{
		double l = (t*(p - r0)) / tt;
		q = r0 + t*l;
		return ((l >= 0) && (l <= 1.0));
	}
	else return false;
}

bool projectToTriangle(const vec3d& p, const vec3d& r0, const vec3d& r1, const vec3d& r2, vec3d& q, Intersection* intersect)
{
	vec3d e1 = r1 - r0;
	vec3d e2 = r2 - r0;

	vec3d N = e1 ^ e2;
	double NN = N*N;
	if (NN != 0.0)
	{
		double l = -((p - r0)*N) / NN;
		q = p + N*l;

		double G[2][2];
		G[0][0] = e1 * e1; G[0][1] = e1 * e2;
		G[1][0] = e2 * e1; G[1][1] = e2 * e2;

		// and its inverse
		double D = G[0][0] * G[1][1] - G[0][1] * G[1][0];
		double Gi[2][2];
		Gi[0][0] = 1 / D*G[1][1]; Gi[0][1] = -1 / D*G[0][1];
		Gi[1][0] = -1 / D*G[1][0]; Gi[1][1] = 1 / D*G[0][0];

		// build dual basis
		vec3d E1 = e1 * Gi[0][0] + e2 * Gi[0][1];
		vec3d E2 = e1 * Gi[1][0] + e2 * Gi[1][1];

		// get the components
		double rp = E1 * (q - r0);
		double sp = E2 * (q - r0);

		if (intersect)
		{
			intersect->r[0] = (float)rp;
			intersect->r[1] = (float)sp;
			intersect->point = q;
		}

		return ((rp >= 0) && (sp >= 0) && (rp + sp <= 1.0));

		//		vec3d s = q - r0;
		//		return ((N*(e1 ^ s) >= 0) && (N*(e2 ^ s) <= 0));
	}
	else return false;
}


//-----------------------------------------------------------------------------
// Project a point to the surface of a FE mesh
//
bool projectToQuad(const vec3d& p, const vec3d y[4], vec3d& q, Intersection* intersect)
{
	double R[2], u[2], D;

	double gr[4] = { -1, +1, +1, -1 };
	double gs[4] = { -1, -1, +1, +1 };
	double H[4], Hr[4], Hs[4], Hrs[4];
	double normu;

	int i, j;
	int NMAX = 10, n = 0;

	// loop until converged
	double r = 0.0, s = 0.0;
	do
	{
		// do quadrilaterals
		for (i = 0; i<4; ++i)
		{
			H[i] = 0.25*(1 + gr[i] * r)*(1 + gs[i] * s);

			Hr[i] = 0.25*gr[i] * (1 + gs[i] * s);
			Hs[i] = 0.25*gs[i] * (1 + gr[i] * r);

			Hrs[i] = 0.25*gr[i] * gs[i];
		}

		// set up the system of equations
		q = vec3d(0, 0, 0);
		R[0] = R[1] = 0;
		double A[2][2] = { 0 };
		for (i = 0; i<4; ++i)
		{
			R[0] -= (p*y[i])*Hr[i];
			R[1] -= (p*y[i])*Hs[i];

			A[0][1] += (p*y[i])*Hrs[i];
			A[1][0] += (p*y[i])*Hrs[i];

			for (j = 0; j<4; ++j)
			{
				R[0] -= -H[j] * Hr[i] * (y[i] * y[j]);
				R[1] -= -H[j] * Hs[i] * (y[i] * y[j]);

				A[0][0] += -(y[i] * y[j])*(Hr[i] * Hr[j]);
				A[1][1] += -(y[i] * y[j])*(Hs[i] * Hs[j]);

				A[0][1] += -(y[i] * y[j])*(Hs[j] * Hr[i] + H[i] * Hrs[j]);
				A[1][0] += -(y[i] * y[j])*(Hr[j] * Hs[i] + H[i] * Hrs[j]);
			}

			q += y[i] * H[i];
		}

		// determinant of A
		D = A[0][0] * A[1][1] - A[0][1] * A[1][0];

		// solve for u = A^(-1)*R
		u[0] = (A[1][1] * R[0] - A[0][1] * R[1]) / D;
		u[1] = (A[0][0] * R[1] - A[1][0] * R[0]) / D;

		normu = sqrt(u[0] * u[0] + u[1] * u[1]);

		r += u[0];
		s += u[1];

		++n;
	}
	while ((normu > 1e-7) && (n < NMAX));

	if (intersect)
	{
		intersect->r[0] = (float)r;
		intersect->r[1] = (float)s;
		intersect->point = q;
	}

	const double eps = 0.001;
	return ((r > -1.0 - eps) && (r < 1.0 + eps) && (s > -1.0 - eps) && (s < 1.0 + eps));
}

vec3d projectToEdge(const FSMeshBase& m, const vec3d& p, int gid)
{
	double Dmin = 1e99;
	vec3d rmin = p;
	for (int i = 0; i<m.Edges(); ++i)
	{
		const FSEdge& edge = m.Edge(i);
		if (edge.m_gid == gid)
		{
			// calculate distance to edge nodes
			int ne = edge.Nodes();
			for (int j = 0; j<ne; ++j)
			{
				vec3d rj = m.Node(edge.n[j]).r;
				double dj = (p - rj).SqrLength();
				if (dj < Dmin)
				{
					rmin = rj;
					Dmin = dj;
				}
			}

			// try to project it on the edge
			vec3d q(0, 0, 0);
			if (projectToLine(p, m.Node(edge.n[0]).r, m.Node(edge.n[1]).r, q))
			{
				double dj = (p - q).SqrLength();
				if (dj < Dmin)
				{
					rmin = q;
					Dmin = dj;
				}
			}
		}
	}

	return rmin;
}

vec3d projectToSurface(const FSMeshBase& m, const vec3d& p, int gid, int* faceID, Intersection* intersect)
{
	double Dmin = 1e99;
	vec3d rmin = p;
	vec3d r[4];
	if (faceID) *faceID = -1;
	if (intersect) intersect->m_faceIndex = -1;
	for (int i = 0; i<m.Faces(); ++i)
	{
		const FSFace& face = m.Face(i);
		if ((face.m_gid == gid) || (gid == -1))
		{
			int nf = face.Nodes();
			for (int j = 0; j<nf; ++j) r[j] = m.Node(face.n[j]).r;

			// calculate distance to face nodes
			for (int j = 0; j<nf; ++j)
			{
				double dj = (p - r[j]).SqrLength();
				if (dj < Dmin)
				{
					rmin = r[j];
					Dmin = dj;
				}
			}

			// try to project it on the face
			vec3d q(0, 0, 0);
			bool bproject = false;
			Intersection is;
			if (nf == 3) bproject = projectToTriangle(p, r[0], r[1], r[2], q, &is);
			else if (nf == 4) bproject = projectToQuad(p, r, q, &is);

			if (bproject)
			{
				double dj = (p - q).SqrLength();
				if (dj < Dmin)
				{
					rmin = q;
					Dmin = dj;
					if (faceID) *faceID = i;
					if (intersect)
					{
						*intersect = is;
						intersect->m_faceIndex = i;
					}
				}
			}
		}
	}

	return rmin;
}

vec3d projectToSurfaceEdges(const FSMeshBase& m, const vec3d& p, int gid, int* faceID, Intersection* intersect)
{
	const double eps = 1e-5;
	double Dmin = 1e99;
	vec3d rmin = p;
	vec3d r[4];
	if (faceID) *faceID = -1;
	if (intersect) intersect->m_faceIndex = -1;
	for (int i = 0; i < m.Faces(); ++i)
	{
		const FSFace& face = m.Face(i);
		if ((face.m_gid == gid) || (gid == -1))
		{
			int nf = face.Nodes();
			for (int j = 0; j < nf; ++j) r[j] = m.Node(face.n[j]).r;

			int ne = face.Edges();
			for (int j = 0; j < ne; ++j)
			{
				vec3d a = r[j];
				vec3d b = r[(j + 1) % nf];

				double R2 = (b - a).SqrLength();
				if (R2 != 0)
				{
					double l = ((p - a) * (b - a)) / R2;
					if ((l >= -eps) && (l <= 1 + eps))
					{
						vec3d q = a + (b - a)*l;
						double dj = (p - q).SqrLength();
						if (dj < Dmin)
						{
							Dmin = dj;
							rmin = q;
							vec3d qtmp;
							Intersection is;
							if      (nf == 3) projectToTriangle(q, r[0], r[1], r[2], qtmp, &is);
							else if (nf == 4) projectToQuad(q, r, qtmp, &is);

							if (faceID) *faceID = i;
							if (intersect)
							{
								*intersect = is;
								intersect->m_faceIndex = i;
							}
						}
					}
				}
			}
		}
	}

	return rmin;
}

vec3d projectToPatch(const FSMeshBase& m, const vec3d& p, int gid, int faceID, int l)
{
	double Dmin = 1e99;
	vec3d rmin = p;
	vec3d r[3];

	set<int> patch;
	patch.insert(faceID);
	for (int i = 0; i<l; ++i)
	{
		set<int> tmp;
		for (set<int>::iterator it = patch.begin(); it != patch.end(); ++it)
		{
			const FSFace& face = m.Face(*it);
			int ne = face.Edges();
			for (int j = 0; j<ne; ++j)
			{
				if (face.m_nbr[j] >= 0)
					tmp.insert(face.m_nbr[j]);
			}
		}

		for (set<int>::iterator it = tmp.begin(); it != tmp.end(); ++it)
		{
			patch.insert(*it);
		}
	}

	for (set<int>::iterator it = patch.begin(); it != patch.end(); ++it)
	{
		const FSFace& face = m.Face(*it);
		if (face.m_gid == gid)
		{
			assert(face.Type() == FE_FACE_TRI3);
			int nf = face.Nodes();
			for (int j = 0; j<nf; ++j) r[j] = m.Node(face.n[j]).r;

			// calculate distance to face nodes
			for (int j = 0; j<nf; ++j)
			{
				double dj = (p - r[j]).SqrLength();
				if (dj < Dmin)
				{
					rmin = r[j];
					Dmin = dj;
				}
			}

			// try to project it on the face
			vec3d q(0, 0, 0);
			if (projectToTriangle(p, r[0], r[1], r[2], q))
			{
				double dj = (p - q).SqrLength();
				if (dj < Dmin)
				{
					rmin = q;
					Dmin = dj;
				}
			}
		}
	}

	return rmin;
}


//-----------------------------------------------------------------------------
// See if two edges intersect in plane (n0, N)
// returns:
// 0 = no intersection
// 1 = intersects at node n0
// 2 = intersects at node n1
// 3 = proper intersection
// 4 = edges coincide at points
// 5 = edges are identical
int edgeIntersect(const vec3d& r0, const vec3d& r1, const vec3d& r2, const vec3d& r3, vec3d N, vec3d& q, double& L, const double eps)
{
	// see if the edges are identical
	if ((r0 == r2) && (r1 == r3)) return 5;
	if ((r0 == r3) && (r1 == r2)) return 5;

	// see if any nodes coincides
	if ((r0 == r2) || (r0 == r3)) return 4;
	if ((r1 == r2) || (r1 == r3)) return 4;

	double tol = eps;

	//		vec3d T = N^(r1 - r0);
	//		N = (r1 - r0) ^ T;

	// project all points onto the plane defined by (c0;N)
	vec3d a = r1 - N*((r1 - r0)*N);
	vec3d b = r2 - N*((r2 - r0)*N);
	vec3d c = r3 - N*((r3 - r0)*N);

	vec3d t = a - r0;
	vec3d r = c - b;
	vec3d p = b - r0;

	// Rotate the vectors into this plane
	quatd Q(N, vec3d(0, 0, 1)), Qi = Q.Inverse();
	Q.RotateVector(t);
	Q.RotateVector(r);
	Q.RotateVector(p);

	double A[2][2], B[2];
	A[0][0] = t.x; A[0][1] = -r.x; B[0] = p.x;
	A[1][0] = t.y; A[1][1] = -r.y; B[1] = p.y;

	// calculate inverse
	double D = A[0][0] * A[1][1] - A[0][1] * A[1][0];
	if (D != 0.0)
	{
		double Ai[2][2];
		Ai[0][0] = A[1][1] / D; Ai[0][1] = -A[0][1] / D;
		Ai[1][0] = -A[1][0] / D; Ai[1][1] = A[0][0] / D;

		double lm = Ai[0][0] * B[0] + Ai[0][1] * B[1];
		double mu = Ai[1][0] * B[0] + Ai[1][1] * B[1];

		if ((mu >= 0) && (mu <= 1))
		{
			if ((lm >= 0) && (lm <= tol))
			{
				q = r0;
				return 1;
			}

			if ((lm <= 1.0) && (lm >= 1.0 - tol))
			{
				q = r1;
				return 2;
			}

			if ((lm > tol) && (lm < 1 - tol))
			{
				// it's a proper intersection
				q = r0 + (r1 - r0)*lm;

				// calculate the 
				vec3d s = r2 + (r3 - r2)*mu;
				L = (s - q).Length();

				return 3;
			}
		}
	}

	return 0;
}

// same as above, except the normal is defined by the cross product of the two edges
int edgeIntersect(const vec3d& r0, const vec3d& r1, const vec3d& r2, const vec3d& r3, vec3d& q, double& L, const double eps)
{
	// see if the edges are identical
	if ((r0 == r2) && (r1 == r3)) return 5;
	if ((r0 == r3) && (r1 == r2)) return 5;

	// see if any nodes coincides
	if ((r0 == r2) || (r0 == r3)) return 4;
	if ((r1 == r2) || (r1 == r3)) return 4;

	double tol = eps;

	// calculate the normal
	vec3d N = (r1 - r0) ^ (r3 - r2);
	N.Normalize();
	if (N.SqrLength() == 0.0) return 0;

	// project all points onto the plane defined by (c0;N)
	vec3d a = r1 - N*((r1 - r0)*N);
	vec3d b = r2 - N*((r2 - r0)*N);
	vec3d c = r3 - N*((r3 - r0)*N);

	vec3d t = a - r0;
	vec3d r = c - b;
	vec3d p = b - r0;

	// Rotate the vectors into this plane
	quatd Q(N, vec3d(0, 0, 1)), Qi = Q.Inverse();
	Q.RotateVector(t);
	Q.RotateVector(r);
	Q.RotateVector(p);

	double A[2][2], B[2];
	A[0][0] = t.x; A[0][1] = -r.x; B[0] = p.x;
	A[1][0] = t.y; A[1][1] = -r.y; B[1] = p.y;

	// calculate inverse
	double D = A[0][0] * A[1][1] - A[0][1] * A[1][0];
	if (D != 0.0)
	{
		double Ai[2][2];
		Ai[0][0] = A[1][1] / D; Ai[0][1] = -A[0][1] / D;
		Ai[1][0] = -A[1][0] / D; Ai[1][1] = A[0][0] / D;

		double lm = Ai[0][0] * B[0] + Ai[0][1] * B[1];
		double mu = Ai[1][0] * B[0] + Ai[1][1] * B[1];

		if ((mu >= 0) && (mu <= 1))
		{
			if ((lm >= 0) && (lm <= tol))
			{
				q = r0;
				return 1;
			}

			if ((lm <= 1.0) && (lm >= 1.0 - tol))
			{
				q = r1;
				return 2;
			}

			if ((lm > tol) && (lm < 1 - tol))
			{
				// it's a proper intersection
				q = r0 + (r1 - r0)*lm;

				// calculate the 
				vec3d s = r2 + (r3 - r2)*mu;
				L = (s - q).Length();

				return 3;
			}
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
// this calculates the ratio of the shortest diagonal to the longest edge
double TriangleQuality(vec3d r[3])
{
	// get the edge lengths
	double l01 = (r[0] - r[1])*(r[0] - r[1]);
	double l12 = (r[1] - r[2])*(r[1] - r[2]);
	double l20 = (r[2] - r[0])*(r[2] - r[0]);

	// get the max edge length
	double lmax = l01;
	if (l12 > lmax) lmax = l12;
	if (l20 > lmax) lmax = l20;

	// calculate the diagonals
	double D[3];
	for (int i = 0; i<3; ++i)
	{
		int a = i;
		int b = (i + 1) % 3;
		int c = (i + 2) % 3;
		vec3d ab = r[b] - r[a];
		vec3d ac = r[c] - r[a];
		double l = (ac*ab) / (ab*ab);
		vec3d p = r[a] + ab*l;
		vec3d pc = r[c] - p;
		D[i] = pc*pc;
	}

	// get the smallest diagonal
	double dmin = D[0];
	if (D[1] < dmin) dmin = D[1];
	if (D[2] < dmin) dmin = D[2];

	// the quality is the ratio of shortest diagonal to longest edge
	double Q2 = dmin / lmax;

	return sqrt(Q2);
}

//-----------------------------------------------------------------------------
double TriMaxDihedralAngle(const FSMeshBase& mesh, const FSFace& face)
{
	if (face.Type() != FE_FACE_TRI3) return 0.;

	double maxAngle = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (face.m_nbr[i] >= 0)
		{
			const FSFace& fi = mesh.Face(face.m_nbr[i]);
			double a = acos(face.m_fn * fi.m_fn);
			if (a > maxAngle) maxAngle = a;
		}
	}
	return maxAngle * 180.0 / PI;
}


//-----------------------------------------------------------------------------
bool FindElementRef(FSCoreMesh& m, const vec3f& p, int& nelem, double r[3])
{
	vec3d y[FSElement::MAX_NODES];
	int NE = m.Elements();
	for (int i = 0; i<NE; ++i)
	{
		FEElement_& e = m.ElementRef(i);
		int ne = e.Nodes();
		nelem = i;

		// do a quick bounding box test
		vec3d r0 = m.Node(e.m_node[0]).r;
		vec3d r1 = r0;
		for (int j = 1; j<ne; ++j)
		{
			vec3d& rj = m.Node(e.m_node[j]).r;
			if (rj.x < r0.x) r0.x = rj.x;
			if (rj.y < r0.y) r0.y = rj.y;
			if (rj.z < r0.z) r0.z = rj.z;
			if (rj.x > r1.x) r1.x = rj.x;
			if (rj.y > r1.y) r1.y = rj.y;
			if (rj.z > r1.z) r1.z = rj.z;
		}

		double dx = fabs(r0.x - r1.x);
		double dy = fabs(r0.y - r1.y);
		double dz = fabs(r0.z - r1.z);

		double R = dx;
		if (dy > R) R = dy;
		if (dz > R) R = dz;
		double eps = R*0.001;

		r0.x -= eps;
		r0.y -= eps;
		r0.z -= eps;

		r1.x += eps;
		r1.y += eps;
		r1.z += eps;

		if ((p.x >= r0.x) && (p.x <= r1.x) &&
			(p.y >= r0.y) && (p.y <= r1.y) &&
			(p.z >= r0.z) && (p.z <= r1.z))
		{
			if (ProjectInsideElement(m, e, p, r)) return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
bool ProjectInsideElement(FSCoreMesh& m, FEElement_& el, const vec3f& p, double r[3])
{
	r[0] = r[1] = r[2] = 0.f;
	int ne = el.Nodes();
	vec3f x[FSElement::MAX_NODES];
	for (int i = 0; i<ne; ++i) x[i] = to_vec3f(m.Node(el.m_node[i]).r);

	project_inside_element(el, p, r, x);

	return IsInsideElement(el, r, 0.001);
}

//-----------------------------------------------------------------------------
bool ProjectToElement(FSElement& el, const vec3f& p, vec3f* x0, vec3f* xt, vec3f& q)
{
	int ne = el.Nodes();
	BOX box;
	for (int i = 0; i < ne; ++i) box += to_vec3d(x0[i]);
	if (box.IsInside(to_vec3d(p)) == false) return false;

	double r[3] = { 0,0,0 };
	project_inside_element(el, p, r, x0);
	if (IsInsideElement(el, r, 0.001))
	{
		q = el.eval(xt, r[0], r[1], r[2]);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool IsInsideElement(FEElement_& el, double r[3], const double tol)
{
	switch (el.Type())
	{
	case FE_TET4:
	case FE_TET10:
	case FE_TET15:
	case FE_TET20:
		return (r[0] >= -tol) && (r[1] >= -tol) && (r[2] >= -tol) && (r[0] + r[1] + r[2] <= 1.0 + tol);
	case FE_HEX8:
	case FE_HEX20:
	case FE_HEX27:
	case FE_PYRA5:
    case FE_PYRA13:
		return ((r[0] >= -1.0 - tol) && (r[0] <= 1.0 + tol) &&
			(r[1] >= -1.0 - tol) && (r[1] <= 1.0 + tol) &&
			(r[2] >= -1.0 - tol) && (r[2] <= 1.0 + tol));
	}
	return false;
}

//-----------------------------------------------------------------------------
void project_inside_element(FEElement_& el, const vec3f& p, double r[3], vec3f* x)
{
	const double tol = 0.0001;
	const int nmax = 10;

	int ne = el.Nodes();
	double dr[3], R[3];
	mat3d K;
	double u2, N[FSElement::MAX_NODES], G[3][FSElement::MAX_NODES];
	int n = 0;
	do
	{
		el.shape(N, r[0], r[1], r[2]);
		el.shape_deriv(G[0], G[1], G[2], r[0], r[1], r[2]);

		R[0] = p.x;
		R[1] = p.y;
		R[2] = p.z;
		for (int i = 0; i<ne; ++i)
		{
			R[0] -= N[i] * x[i].x;
			R[1] -= N[i] * x[i].y;
			R[2] -= N[i] * x[i].z;
		}

		K.zero();
		for (int i = 0; i<ne; ++i)
		{
			K[0][0] -= G[0][i] * x[i].x; K[0][1] -= G[1][i] * x[i].x; K[0][2] -= G[2][i] * x[i].x;
			K[1][0] -= G[0][i] * x[i].y; K[1][1] -= G[1][i] * x[i].y; K[1][2] -= G[2][i] * x[i].y;
			K[2][0] -= G[0][i] * x[i].z; K[2][1] -= G[1][i] * x[i].z; K[2][2] -= G[2][i] * x[i].z;
		}

		K.invert();

		dr[0] = K[0][0] * R[0] + K[0][1] * R[1] + K[0][2] * R[2];
		dr[1] = K[1][0] * R[0] + K[1][1] * R[1] + K[1][2] * R[2];
		dr[2] = K[2][0] * R[0] + K[2][1] * R[1] + K[2][2] * R[2];

		r[0] -= dr[0];
		r[1] -= dr[1];
		r[2] -= dr[2];

		u2 = dr[0] * dr[0] + dr[1] * dr[1] + dr[2] * dr[2];
		++n;
	} while ((u2 > tol*tol) && (n < nmax));
}

//-----------------------------------------------------------------------------
bool project_inside_element2d(FEElement_& el, vec3d* x, const vec2d& p, double q[2])
{
	if (el.IsShell() == false) return false;

	const double tol = 1e-7;
	const int nmax = 10;

	int ne = el.Nodes();
	double dq[2], R[2];
	mat2d K;
	double u2, N[FSElement::MAX_NODES], G[2][FSElement::MAX_NODES];
	int n = 0;
	do
	{
		el.shape_2d(N, q[0], q[1]);
		el.shape_deriv_2d(G[0], G[1], q[0], q[1]);

		R[0] = p.x();
		R[1] = p.y();
		for (int i = 0; i < ne; ++i)
		{
			R[0] -= N[i] * x[i].x;
			R[1] -= N[i] * x[i].y;
		}

		K.zero();
		for (int i = 0; i < ne; ++i)
		{
			K[0][0] -= G[0][i] * x[i].x; K[0][1] -= G[1][i] * x[i].x;
			K[1][0] -= G[0][i] * x[i].y; K[1][1] -= G[1][i] * x[i].y;
		}

		mat2d Ki = K.inverse();

		dq[0] = Ki[0][0] * R[0] + Ki[0][1] * R[1];
		dq[1] = Ki[1][0] * R[0] + Ki[1][1] * R[1];

		q[0] -= dq[0];
		q[1] -= dq[1];

		u2 = dq[0] * dq[0] + dq[1] * dq[1];
		++n;
	}
	while ((u2 > tol * tol) && (n < nmax));

	bool inside = false;
	switch (el.Type())
	{
	case FE_TRI3:
		inside = (q[0] >= -tol) && (q[1] >= -tol) && (q[0] + q[1] <= 1.0 + tol);
		break;
	case FE_QUAD4:
		inside =((q[0] >= -1.0 - tol) && (q[0] <= 1.0 + tol) && (q[1] >= -1.0 - tol) && (q[1] <= 1.0 + tol));
		break;
	default:
		assert(false);
	}
	return inside;
}


//-----------------------------------------------------------------------------
bool ProjectInsideReferenceElement(FSCoreMesh& m, FEElement_& el, const vec3f& p, double r[3])
{
	r[0] = r[1] = r[2] = 0.f;
	int ne = el.Nodes();
	vec3f x[FSElement::MAX_NODES];
	for (int i = 0; i<ne; ++i) x[i] = to_vec3f(m.Node(el.m_node[i]).r);

	project_inside_element(el, p, r, x);

	return IsInsideElement(el, r, 0.001);
}

//-------------------------------------------------------------------------------
std::vector<vec3d> FindShortestPath(FSMesh& mesh, int m0, int m1)
{
	// helper class for finding neighbors
	FSNodeNodeList NNL(&mesh);

	const double INF = 1e34;
	int N = mesh.Nodes();
	vector<double> dist(N, INF);

	mesh.TagAllNodes(-1);

	int ncurrent = m0;
	mesh.Node(m0).m_ntag = m0;
	dist[m0] = 0.0;
	double L0 = 0.0;
	while (ncurrent != m1)
	{
		// get the position of the current node
		FSNode& node0 = mesh.Node(ncurrent);
		vec3d rc = node0.pos();

		// update neighbor distances
		int nval = NNL.Valence(ncurrent);
		for (int i = 0; i < nval; ++i)
		{
			int mi = NNL.Node(ncurrent, i);
			assert(mi != ncurrent);

			FSNode& nodei = mesh.Node(mi);
			if (dist[mi] > 0)
			{
				vec3d ri = mesh.Node(mi).pos();
				double dL = (ri - rc).Length();

				double L1 = L0 + dL;
				if (L1 < dist[mi])
				{
					dist[mi] = L1;
					nodei.m_ntag = ncurrent;
				}
				else L1 = dist[mi];
			}
		}

		// choose the next node as the unvisited node with the smallest distance
		int nmin = -1;
		double dmin = 0.0;
		for (int i = 0; i < mesh.Nodes(); ++i)
		{
			if (dist[i] > 0)
			{
				if ((nmin == -1) || (dist[i] < dmin))
				{
					nmin = i;
					dmin = dist[i];
				}
			}
		}

		if (nmin == -1)
		{
			// hmmm, something went wrong
			assert(false);
			return std::vector<vec3d>();
		}

		L0 = dist[nmin];
		dist[nmin] = -1.0;
		ncurrent = nmin;
	}

	// build the path
	// NOTE: This traverses the path in reverse!
	std::vector<vec3d> tmp;
	ncurrent = m1;
	tmp.push_back(mesh.Node(m1).pos());
	do
	{
		int parentNode = mesh.Node(ncurrent).m_ntag;

		vec3d rc = mesh.Node(parentNode).pos();
		tmp.push_back(rc);

		ncurrent = parentNode;

	} while (ncurrent != m0);

	// invert the temp path to get the final path
	std::vector<vec3d> path;
	size_t n = tmp.size();
	if (n > 0)
	{
		path.resize(n);
		for (size_t i = 0; i < n; ++i)
		{
			path[i] = tmp[n - i - 1];
		}
	}

	return path;
}
