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
#pragma once
#include "FSTriMesh.h"

class PathOnMesh
{
public:
	struct POINT {
		vec3d	r;		// spatial position of point
		int		nface;	// face on which the point lies (or -1 if it's not on the mesh)
		vec2d	q;		// projection coordinates in face
	};

public:
	PathOnMesh(const FSTriMesh& mesh) : m_mesh(mesh) {}
	PathOnMesh(const FSTriMesh& mesh, const std::vector<vec3d>& points);

	size_t Points() const { return m_pt.size(); }

	POINT& Point(size_t i) { return m_pt[i]; }
	const POINT& Point(size_t i) const { return m_pt[i]; }

	POINT& operator [] (size_t i) { return m_pt[i]; }

	const FSTriMesh& GetMesh() const { return m_mesh; }

private:
	const FSTriMesh& m_mesh;
	std::vector<POINT>	m_pt;
};

// this function takes a curve and projects it onto the mesh. 
// The projected curve should approximate a geodesic, or the shortest
// path between the end-points of the initially provided curve.
PathOnMesh ProjectToGeodesic(
	const FSTriMesh& mesh,		// the mesh to find the geodesic on
	const std::vector<vec3d>& path,	// initial guess of path
	int maxIters,				// max nr of smoothing iterations
	double tol,					// convergence tolerance
	double snapTol = 0.0);		// snap tolerance
