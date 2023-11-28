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
#include "FEGeodesic.h"

PathOnMesh::PathOnMesh(const FSTriMesh& mesh, const std::vector<vec3d>& points) : m_mesh(mesh)
{
	m_pt.resize(points.size());
	for (int i = 0; i < points.size(); ++i)
	{
		m_pt[i].r = points[i];
		m_pt[i].nface = -1;
		m_pt[i].q = vec2d(0, 0);
	}
}

bool ClosestPointOnRing(
	const FSTriMesh& mesh,
	const vec3d& rc, const vec3d& t,
	const vec3d& a, const vec3d& b, const vec3d& na,
	PathOnMesh::POINT& pt)
{
	const int LUT[8][2] = { {-1,-1}, {0,2},{0,1},{1,2},{1,2},{0,1},{0,2},{-1,-1} };

	int NF = mesh.Faces();
	int imin = -1;
	pt.nface = -1;
	double Dmin = 0.0;
	for (int i = 0; i < NF; ++i)
	{
		// figure out the case for this face
		// (i.e. decide if the plane (rc, t) intersects this triangle
		const FSTriMesh::FACE& face = mesh.Face(i);
		int ne = 3; // edges!
		int ncase = 0;
		const vec3d* re = face.r;
		for (int j = 0; j < ne; ++j)
		{
			double s = (re[j] - rc) * t;
			if (s > 0) ncase |= (1 << j);
		}

		if ((ncase != 0) && (ncase != 7))
		{
			int e[2] = { LUT[ncase][0], LUT[ncase][1] };

			// find the edge intersections
			vec3d er[2];
			for (int j = 0; j < 2; ++j)
			{
				int n = e[j];
				int n0 = n;
				int n1 = (n + 1) % ne;
				vec3d ra = re[n0];
				vec3d rb = re[n1];
				double la = t * (ra - rc);
				double lb = t * (rb - rc);

				assert(la * lb < 0);

				double l = (t * (rc - ra)) / (t * (rb - ra));
				vec3d p = ra + (rb - ra) * l;

				// make sure the point lies on the plane
				double e = t * (p - rc);
				assert(fabs(e) < 1e-12);

				er[j] = p;
			}

			// find the point that minimizes (a-p-b)
			vec3d dr = er[1] - er[0];
			double D = dr.SqrLength();
			if (D != 0.0)
			{
				// project point c onto the line {er[0], er[1]}
				vec3d c = rc;// (a + b) * 0.5;
				double l = (c * dr - er[0] * dr) / D;

				vec3d p;
				if (l <= 0.0) p = er[0];
				else if (l >= 1.0) p = er[1];
				else p = er[0] + (er[1] - er[0]) * l;

				double D2 = (p - c).norm2(); //(p - a).SqrLength() + (p - b).SqrLength();
				if ((imin == -1) || (D2 < Dmin))
				{
					// calculate face normal
					vec3d fn = face.fn;

					// make sure the normal is not on the wrong side
					double dot = fn * na;
					//				if (dot > normalTolerance)
					{
						imin = i;
						Dmin = D2;
						pt.r = p;

						// make sure the point lies on the plane
						double e = t * (pt.r - rc);
						assert(fabs(e) < 1e-12);

						// project normal onto plane
						fn -= t * (fn * t); fn.Normalize();

						assert(fabs(fn * t) < 1e-12);

						pt.nface = i;
					}
				}
			}
		}
	}

	return (imin != -1);
}

void ProjectPath(PathOnMesh& path, double snaptol)
{
	const FSTriMesh& mesh = path.GetMesh();

	int NP = path.Points();
	for (int i = 1; i < NP - 1; ++i)
	{
		// next point
		auto& pi = path[i];

		// approximate tangent
		vec3d a = path[i - 1].r;
		vec3d b = path[i + 1].r;
		vec3d t = b - a; t.Normalize();

		double L0 = (b - pi.r).norm() + (pi.r - a).norm();

		vec3d ri = (a + b) * 0.5;

		vec3d fn(0, 0, 0);
		if (path[i - 1].nface >= 0) fn = mesh.Face(path[i - 1].nface).fn;

		// find the closest point to ri on the ring, defined by the intersection
		// of the mesh with the plane (ri; t)
		if (ClosestPointOnRing(mesh, ri, t, a, b, fn, pi))
		{
			double L1 = (b - pi.r).norm() + (pi.r - a).norm();

			fn = vec3d(0, 0, 0);
			if (pi.nface >= 0) fn = mesh.Face(pi.nface).fn;

			if (((ri - pi.r) * fn > 0.0) || ((snaptol > 0) && (L1 > L0 * snaptol)))
			{
				pi.r = ri;
				pi.nface = -1;
			}
		}
		else pi.nface = -1;
	}
}

bool SmoothenPath(PathOnMesh& path, int maxIters, double tol, double snaptol)
{
	const FSTriMesh& mesh = path.GetMesh();

	// evaluate the initial length
	int NP = path.Points();
	double L0 = 0;
	for (int i = 0; i < NP - 1; ++i)
	{
		auto& ri = path[i];
		auto& rp = path[i + 1];
		L0 += (rp.r - ri.r).Length();
	}

	// see if we can shrink the path
	int niter = 0;
	bool done = false;
	double Lp = L0;
	do
	{
		// project the path onto the mesh
		ProjectPath(path, snaptol);

		// calculate new length
		double L1 = 0;
		for (int i = 0; i < NP - 1; ++i)
		{
			auto& ri = path[i];
			auto& rp = path[i + 1];
			L1 += (rp.r - ri.r).Length();
		}

		done = (fabs((L1 - Lp) / L0) < tol);
		Lp = L1;
		niter++;
		if (niter > maxIters) break;
	} while (!done);

	return done;
}

void StraightenPath(PathOnMesh& path)
{
	const int N = path.Points();
	if (N <= 2) return;

	int n = 1;
	while (n < N - 1)
	{
		if (path[n].nface == -1)
		{
			int m0 = n - 1; assert(m0 >= 0);
			int m1 = n + 1;
			while ((m1 < N - 1) && (path[m1].nface == -1)) m1++;
			assert(m1 < N);

			auto& p0 = path[m0];
			auto& p1 = path[m1];

			vec3d a = p0.r;
			vec3d b = p1.r;
			for (; n < m1; n++)
			{
				double l = (double)(n - m0) / (double)(m1 - m0);
				vec3d r = a + (b - a) * l;
				path[n].r = r;
			}
		}
		else n++;
	}
}

PathOnMesh ProjectToGeodesic(
	const FSTriMesh& mesh, 
	const std::vector<vec3d>& points,
	int maxIters,
	double tol,
	double snapTol)
{
	// convert to path
	PathOnMesh path(mesh, points);

	if (points.size() == 0) return path;

	// find the face that the first point is on
	path[0].nface = mesh.FindFace(path[0].r); assert(path[0].nface >= 0);

	// process the path
	ProjectPath(path, snapTol);

	// smoothen the path
	if (maxIters > 0)
	{
		SmoothenPath(path, maxIters, tol, snapTol);
	}

	// straighten the path
	StraightenPath(path);

	return path;
}
