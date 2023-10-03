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

//#include "stdafx.h"
#include "Intersect.h"

//-----------------------------------------------------------------------------
// Find intersection of a ray with a triangle
bool IntersectTriangle(const Ray& ray, const Triangle& tri, Intersection& intersection, bool evalNormal)
{
	const double tol = 0.01;

	vec3d n1 = tri.r0;
	vec3d n2 = tri.r1;
	vec3d n3 = tri.r2;

	vec3d r0 = ray.origin;
	vec3d nn = ray.direction;

	// calculate the triangle normal
	vec3d fn = tri.fn;
	if (evalNormal) { fn = (n2 - n1) ^ (n3 - n1); fn.Normalize(); }

	// find the intersection of the point with the plane
	if (fn*nn == 0.f) return false;
	double l = fn*(n1 - r0) / (fn*nn);

	//	if (l <= 0) return false;
	vec3d q = r0 + nn*l;

	// find  the natural coordinates
	vec3d e1 = n2 - n1;
	vec3d e2 = n3 - n1;

	double A[2][2] = { { e1*e1, e1*e2 }, { e2*e1, e2*e2 } };
	double D = A[0][0] * A[1][1] - A[0][1] * A[1][0];
	double Ai[2][2];
	Ai[0][0] = (A[1][1]) / D;
	Ai[1][1] = (A[0][0]) / D;
	Ai[0][1] = -A[0][1] / D;
	Ai[1][0] = -A[1][0] / D;

	vec3d E1 = e1*Ai[0][0] + e2*Ai[0][1];
	vec3d E2 = e1*Ai[1][0] + e2*Ai[1][1];

	double r = (q - n1)*E1;
	double s = (q - n1)*E2;

	intersection.point = q;
	intersection.r[0] = r;
	intersection.r[1] = s;

	return ((r >= -tol) && (s >= -tol) && (r + s <= 1.0 + tol));
}

//-----------------------------------------------------------------------------
bool FastIntersectQuad(const Ray& ray, const Quad& quad, Intersection& intersect)
{
	Triangle tri[2] = {
		{ quad.r0, quad.r1, quad.r2 },
		{ quad.r2, quad.r3, quad.r0 }};

	return IntersectTriangle(ray, tri[0], intersect) || IntersectTriangle(ray, tri[1], intersect);
}

//-----------------------------------------------------------------------------
// Find intersection of a ray with a quad
bool IntersectQuad(const Ray& ray, const Quad& quad, Intersection& intersect)
{
	const double tol = 0.01;

	vec3d nr = ray.origin;
	vec3d nn = ray.direction;

	vec3d y[4];
	y[0] = quad.r0 - nr;
	y[1] = quad.r1 - nr;
	y[2] = quad.r2 - nr;
	y[3] = quad.r3 - nr;

	double r = 0, s = 0, l = 0, normu;
	int niter = 0;
	const int NMAX = 10;
	do
	{
		// evaluate shape functions
		double H[4], Hr[4], Hs[4];
		H[0] = 0.25*(1 - r)*(1 - s);
		H[1] = 0.25*(1 + r)*(1 - s);
		H[2] = 0.25*(1 + r)*(1 + s);
		H[3] = 0.25*(1 - r)*(1 + s);

		Hr[0] = -0.25*(1 - s);
		Hr[1] = 0.25*(1 - s);
		Hr[2] = 0.25*(1 + s);
		Hr[3] = -0.25*(1 + s);

		Hs[0] = -0.25*(1 - r);
		Hs[1] = -0.25*(1 + r);
		Hs[2] = 0.25*(1 + r);
		Hs[3] = 0.25*(1 - r);

		// evaluate residual
		vec3d R = nn*l - y[0] * H[0] - y[1] * H[1] - y[2] * H[2] - y[3] * H[3];

		mat3d K;
		K[0][0] = nn.x;
		K[0][1] = -y[0].x*Hr[0] - y[1].x*Hr[1] - y[2].x*Hr[2] - y[3].x*Hr[3];
		K[0][2] = -y[0].x*Hs[0] - y[1].x*Hs[1] - y[2].x*Hs[2] - y[3].x*Hs[3];

		K[1][0] = nn.y;
		K[1][1] = -y[0].y*Hr[0] - y[1].y*Hr[1] - y[2].y*Hr[2] - y[3].y*Hr[3];
		K[1][2] = -y[0].y*Hs[0] - y[1].y*Hs[1] - y[2].y*Hs[2] - y[3].y*Hs[3];

		K[2][0] = nn.z;
		K[2][1] = -y[0].z*Hr[0] - y[1].z*Hr[1] - y[2].z*Hr[2] - y[3].z*Hr[3];
		K[2][2] = -y[0].z*Hs[0] - y[1].z*Hs[1] - y[2].z*Hs[2] - y[3].z*Hs[3];

		K = K.inverse();

		vec3d du = K*R;
		l -= du.x;
		r -= du.y;
		s -= du.z;

		normu = du.y*du.y + du.z*du.z;
		niter++;
	} 
	while ((normu > 1e-6) && (niter < NMAX));

	if (niter < NMAX)
	{
		intersect.point = nr + nn*l;
		intersect.r[0] = r;
		intersect.r[1] = s;
	
		bool b = ((r + tol >= -1) && (r - tol <= 1) && (s + tol >= -1) && (s - tol <= 1));
		return b;
	}
	else return false;
}

//-----------------------------------------------------------------------------
bool RayIntersectFace(const Ray& ray, int faceType, vec3d* rn, Intersection& q)
{
	bool bfound = false;
	switch (faceType)
	{
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
	{
		Triangle tri = { rn[0], rn[1], rn[2] };
		bfound = IntersectTriangle(ray, tri, q);
	}
	break;
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
	{
		Quad quad = { rn[0], rn[1], rn[2], rn[3] };
		bfound = FastIntersectQuad(ray, quad, q);
	}
	break;
	}
	return bfound;
}

//-----------------------------------------------------------------------------
bool FindFaceIntersection(const Ray& ray, const FSMeshBase& mesh, Intersection& q)
{
	vec3d rn[10];

	int faces = mesh.Faces();
	vec3d r, rmin;
	double gmin = 1e99;
	bool b = false;

	q.m_index = -1;
	Intersection tmp;
	for (int i=0; i<faces; ++i)
	{
		const FSFace& face = mesh.Face(i);
		if (face.IsVisible())
		{
			mesh.FaceNodeLocalPositions(face, rn);
			bool bfound = RayIntersectFace(ray, face.Type(), rn, tmp);

			if (bfound)
			{
				// signed distance
				float distance = ray.direction*(tmp.point - ray.origin);

				if ((distance > 0.f) && (distance < gmin))
				{
					gmin = distance;
					rmin = q.point;
					b = true;
					q.m_index = i;
					q.point = tmp.point;
					q.r[0] = tmp.r[0];
					q.r[1] = tmp.r[1];
				}
			}
		}
	}

	return b;
}

//-----------------------------------------------------------------------------
bool FindFaceIntersection(const Ray& ray, const GMesh& mesh, Intersection& q)
{
	vec3d rn[3];
	int faces = mesh.Faces();
	vec3d r, rmin;
	double gmin = 1e99;
	bool b = false;

	q.m_index = -1;
	Intersection tmp;
	for (int i = 0; i<faces; ++i)
	{
		const GMesh::FACE& face = mesh.Face(i);

		rn[0] = mesh.Node(face.n[0]).r;
		rn[1] = mesh.Node(face.n[1]).r;
		rn[2] = mesh.Node(face.n[2]).r;

		bool bfound = false;
		Triangle tri = { rn[0], rn[1], rn[2] };
		bfound = IntersectTriangle(ray, tri, tmp);

		if (bfound)
		{
			// signed distance
			float distance = ray.direction*(tmp.point - ray.origin);

			if ((distance > 0.f) && (distance < gmin))
			{
				gmin = distance;
				rmin = q.point;
				b = true;
				q.m_index = i;
				q.point = tmp.point;
				q.r[0] = tmp.r[0];
				q.r[1] = tmp.r[1];
			}
		}
	}

	return b;
}

//-----------------------------------------------------------------------------
bool FindElementIntersection(const Ray& ray, const FSMesh& mesh, Intersection& q, bool selectionState)
{
	vec3d rn[10];

	int elems = mesh.Elements();
	vec3d r, rmin;
	float gmin = 1e30f;
	bool b = false;

	FSFace face;
	q.m_index = -1;
	Intersection tmp;
	for (int i = 0; i<elems; ++i)
	{
		const FSElement& elem = mesh.Element(i);
		if (elem.IsVisible() && (elem.IsSelected() == selectionState))
		{
			// solid elements
			int NF = elem.Faces();
			for (int j = 0; j<NF; ++j)
			{
				bool bfound = false;
				face = elem.GetFace(j);
				switch (face.Type())
				{
				case FE_FACE_QUAD4:
				case FE_FACE_QUAD8:
				case FE_FACE_QUAD9:
				{
					rn[0] = mesh.Node(face.n[0]).r;
					rn[1] = mesh.Node(face.n[1]).r;
					rn[2] = mesh.Node(face.n[2]).r;
					rn[3] = mesh.Node(face.n[3]).r;

					Quad quad = { rn[0], rn[1], rn[2], rn[3] };
					bfound = FastIntersectQuad(ray, quad, tmp);
				}
				break;
				case FE_FACE_TRI3:
				case FE_FACE_TRI6:
				case FE_FACE_TRI7:
				case FE_FACE_TRI10:
				{
					rn[0] = mesh.Node(face.n[0]).r;
					rn[1] = mesh.Node(face.n[1]).r;
					rn[2] = mesh.Node(face.n[2]).r;

					Triangle tri = { rn[0], rn[1], rn[2] };
					bfound = IntersectTriangle(ray, tri, tmp);
				}
				break;
				default:
					assert(false);
				}

				if (bfound)
				{
					// signed distance
					float distance = ray.direction*(tmp.point - ray.origin);

					if ((distance > 0.f) && (distance < gmin))
					{
						gmin = distance;
						rmin = q.point;
						b = true;
						q.m_index = i;
						q.m_faceIndex = elem.m_face[j];
						q.point = tmp.point;
						q.r[0] = tmp.r[0];
						q.r[1] = tmp.r[1];
					}
				}
			}

			// shell elements
			int NE = elem.Edges();
			if (NE > 0)
			{
				bool bfound = false;
				if (elem.Nodes() == 4)
				{
					rn[0] = mesh.Node(elem.m_node[0]).r;
					rn[1] = mesh.Node(elem.m_node[1]).r;
					rn[2] = mesh.Node(elem.m_node[2]).r;
					rn[3] = mesh.Node(elem.m_node[3]).r;

					Quad quad = { rn[0], rn[1], rn[2], rn[3] };
					bfound = IntersectQuad(ray, quad, tmp);
				}
				else
				{
					rn[0] = mesh.Node(elem.m_node[0]).r;
					rn[1] = mesh.Node(elem.m_node[1]).r;
					rn[2] = mesh.Node(elem.m_node[2]).r;

					Triangle tri = { rn[0], rn[1], rn[2] };
					bfound = IntersectTriangle(ray, tri, tmp);
				}

				if (bfound)
				{
					// signed distance
					float distance = ray.direction*(tmp.point - ray.origin);

					if ((distance > 0.f) && (distance <= gmin))
					{
						gmin = distance;
						rmin = q.point;
						b = true;
						q.m_index = i;
						q.point = tmp.point;
						q.r[0] = tmp.r[0];
						q.r[1] = tmp.r[1];
					}
				}
			}
		}
	}

	return b;
}

//-----------------------------------------------------------------------------
bool FindFaceIntersection(const Ray& ray, const FSMeshBase& mesh, const FSFace& face, Intersection& q)
{
	q.m_index = -1;

	vec3d rn[10];
	mesh.FaceNodePosition(face, rn);

	bool bfound = false;
	switch (face.m_type)
	{
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
	{
		Triangle tri = { rn[0], rn[1], rn[2] };
		bfound = IntersectTriangle(ray, tri, q);
	}
	break;
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
	{
		Quad quad = { rn[0], rn[1], rn[2], rn[3] };
		bfound = FastIntersectQuad(ray, quad, q);
	}
	break;
	}

	return bfound;
}
