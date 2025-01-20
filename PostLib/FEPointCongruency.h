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
#include <FSCore/math3d.h>
#include <MeshLib/Intersect.h>
#include <MeshLib/FENodeFaceList.h>
#include <set>

class FSFace;

namespace Post {

class FEPostModel;

//-----------------------------------------------------------------------------
class FEPointCongruency
{
	enum { GAUSS, MEAN, PRINC1, PRINC2, RMS, DIFF, ANGLE };

public:
	struct CONGRUENCY_DATA
	{
		double	H1, G1;			// curvature on master side
		double	H2, G2;			// curvature on slave side
		double	D;				// "Delta" value
		double	a;				// "alpha" value
		double	Kemin, Kemax;	// effective curvature
		double	Ke;				// congruency
		int		nface;			// face on which the master is projected
	};

public:
	// constructor
	FEPointCongruency();

	// measure the congruency of a point
	CONGRUENCY_DATA Congruency(FSMesh* pm, int node);

	void SetLevels(int niter) { m_nlevels = niter; }

private:
	bool Project(int nid, int& nface, vec3f& q, double rs[2], vec3f& sn);
	bool Intersect(const Ray& ray, int& nface, int nid, vec3f& q, double rs[2]);

	bool IntersectTri3 (const Ray& ray, FSFace& face, vec3f& q, double rs[2]);
	bool IntersectQuad4(const Ray& ray, FSFace& face, vec3f& q, double rs[2]);
	float nodal_curvature(int nid, vec3f& nn, int m);
	void level(int n, int l, std::set<int>& nl1);

	float face_curvature(FSFace& face, double rs[2], vec3f& sn, int m);

public:
	int	m_nlevels;
	int	m_bext;
	int	m_nmax;

private:
	FSMesh*		m_mesh;
	FSNodeFaceList	m_NFL;
};
}
