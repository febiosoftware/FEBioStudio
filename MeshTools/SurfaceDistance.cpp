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
#include "SurfaceDistance.h"
#include <MeshLib/FSMesh.h>
#include <GeomLib/GObject.h>

CSurfaceDistance::CSurfaceDistance()
{
	m_min = m_max = 0.0;
	m_mean = 0.0;
	m_stddev = 0.0;
	m_bsigned = false;
	m_ntype = 0;
	m_scale = 1.0;
	m_bclamp = false;
}

void CSurfaceDistance::SetRange(double gmin, double gmax)
{
	m_min = gmin;
	m_max = gmax;
}

bool CSurfaceDistance::Apply(GObject* pso, GObject* pmo)
{
	// Get the meshes
	FSMesh* ps = pso->GetFEMesh();
	FSMesh* pm = pmo->GetFEMesh();

	// allocate buffer for storing nodal distances
	int nodes = ps->Nodes();
	std::vector<double> dist(nodes, 0.0);

	// calculate nodal distances
	bool bret = false;
	switch (m_ntype)
	{
	case NORMAL       : bret = NormalProject(pso, pmo, dist); break;
	case CLOSEST_POINT: bret = ClosestPoint (pso, pmo, dist); break;
	}
	if (bret == false) return false;

	// apply the distance multiplier
	for (int i=0; i<nodes; ++i) dist[i] *= m_scale;

	// assign to shell thickness
	int NE = ps->Elements();
	for (int i=0; i<NE; ++i)
	{
		FSElement& el = ps->Element(i);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j)
		{
			el.m_h[j] = dist[el.m_node[j]];
		}
	}

	// calculate mean
	m_mean = 0.0;
	for (int i=0; i<nodes; ++i) m_mean += dist[i];
	m_mean /= nodes;

	// find stdev of thicknesses
	m_stddev = 0;
	for (int i=0; i<nodes; ++i)
	{
		m_stddev += (dist[i] - m_mean)*(dist[i] - m_mean);
	}
	m_stddev = sqrt(m_stddev/nodes);

	return true;
}

bool CSurfaceDistance::NormalProject(GObject* pso, GObject* pmo, std::vector<double>& dist)
{
	FSMesh* ps = pso->GetFEMesh();
	FSMesh* pm = pmo->GetFEMesh();

	// get the number of nodes
	int nodes = ps->Nodes();

	// calculate node normals
	std::vector<vec3d> nu = ps->NodeNormals();
	assert(nu.size() == nodes);

	// conver them to global coordinates and normalize
	for (int i=0; i<nodes; ++i)
	{
		nu[i] = pso->GetTransform().LocalToGlobalNormal(nu[i]);
		nu[i] = pmo->GetTransform().GlobalToLocalNormal(nu[i]);
		nu[i].Normalize();
	}

	// repeat for all nodes
	for (int i=0; i<nodes; ++i)
	{
		FSNode& nodei = ps->Node(i);

		// get the nodal coordinate
		vec3d ri = pso->GetTransform().LocalToGlobal(nodei.r);
		ri = pmo->GetTransform().GlobalToLocal(ri);
		vec3d ni = nu[i];

		// create a plane tangent to the ray
		// we'll use this as a quick way to decide if a triangle can be a candidate for intersection.
		vec3d g(0,0,1);
		if (ni*g > 0.999) g = vec3d(1,0,0);
		vec3d t1 = ni^g;
		vec3d t2 = ni^t1;

		double t1ri = t1*ri;
		double t2ri = t2*ri;

		double Dmin, D2min = -1.0;

		// look for closest node on reference surface
		int NE = pm->Elements();
		for (int j=0; j<NE; ++j)
		{
			FSElement& el = pm->Element(j);
			assert(el.IsType(FE_TRI3));
			vec3d r0 = pm->Node(el.m_node[0]).r;
			vec3d r1 = pm->Node(el.m_node[1]).r;
			vec3d r2 = pm->Node(el.m_node[2]).r;

			// in order for this triangle to be a candidate the points cannot all lie on the same side of the test planes.
			bool bcont = true;
			double s0 = t1*r0 - t1ri;
			double s1 = t1*r1 - t1ri;
			double s2 = t1*r2 - t1ri;
			const double tol = 1e-5;
			if      ((s0>-tol)&&(s1>-tol)&&(s2>-tol)) bcont = false;
			else if ((s0< tol)&&(s1< tol)&&(s2< tol)) bcont = false;

			if (bcont)
			{
				s0 = t2*r0 - t2ri;
				s1 = t2*r1 - t2ri;
				s2 = t2*r2 - t2ri;
				if      ((s0>-tol)&&(s1>-tol)&&(s2>-tol)) bcont = false;
				else if ((s0< tol)&&(s1< tol)&&(s2< tol)) bcont = false;
			}

			if (bcont)
			{
				// get the triangle normal
				vec3d v = (r1 - r0)^(r2 - r0);
				v.Normalize();

				// calculate r
				vec3d r = ri - r0;

				// calculate intersection
				double denom = nu[i]*v;
				if (denom != 0)
				{
					double t = (r*v) / denom;	// this is the (signed) distance to the plane

					// project node onto plane
					vec3d x = ri - nu[i]*t;

					// now we need to find out if this node is inside the triangle.
					// in order to do that we calculate the (r,s) isoparametric coordinates 
					// by projecting the nodes on to the edges (p0,p1) and (p0,p2)
					// then, if (r>=0 AND s>=0 AND r+s <= 1, the node is in the triangle

					vec3d u = (r1 - r0);
					vec3d v = (r2 - r0);

					double u2 = u*u, v2 = v*v, uv = u*v;

					vec3d r = x - r0;

					double g = u2*v2-uv*uv;
					if ( g != 0)
					{
						double a = (v2*(r*u) - uv*(r*v))/g;
						double b = (u2*(r*v) - uv*(r*u))/g;
						const double eps = 0.001;
						if ((a>=-eps) && (b>=-eps) && (a+b <= 1+eps))
						{
							// we have a winner !
							double d2 = t*t;
							if ((d2 < D2min) || (D2min < 0.0))
							{
								Dmin = t;
								D2min = d2;
							}
						}
					}
				}
			}
		}

		if (D2min < 0)
		{
			// we did not find a projection
			// set it to max
			Dmin = m_max;
		}

		// clamp to range
		if (m_bclamp)
		{
			if (Dmin<m_min) Dmin = m_min;
			if (Dmin>m_max) Dmin = m_max;
		}

		// store
		dist[i] = Dmin;
	}

	// if we want unsigned distance, make all values positive
	if (m_bsigned == false)
	{
		for (int i=0; i<nodes; ++i) dist[i] = fabs(dist[i]);
	}

	return true;
}

bool CSurfaceDistance::ClosestPoint(GObject* pso, GObject* pmo, std::vector<double>& dist)
{
	// get the meshes
	FSMesh* ps = pso->GetFEMesh();
	FSMesh* pm = pmo->GetFEMesh();

	// get the number of nodes
	int nodes = ps->Nodes();

	// repeat for all nodes
	for (int i=0; i<nodes; ++i)
	{
		FSNode& nodei = ps->Node(i);

		// get the global nodal coordinates
		vec3d ri = pso->GetTransform().LocalToGlobal(nodei.r);

		// convert it to the local coordinate in the master object
		ri = pmo->GetTransform().GlobalToLocal(ri);

		// repeat over all master nodes
		double Dmin = 0.0;
		for (int j=0; j<pm->Nodes(); ++j)
		{
			// get the position of the master node
			vec3d& rj = pm->Node(j).r;

			// separation vector
			vec3d r = rj - ri;

			// square length
			double d2 = r*r;

			if ((d2 < Dmin)||(j==0))
			{
				Dmin = d2;
			}			
		}

		dist[i] = sqrt(Dmin);
	}

	return true;
}
