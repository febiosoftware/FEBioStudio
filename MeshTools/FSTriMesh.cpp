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
#include "FSTriMesh.h"
#include <MeshLib/Intersect.h>
#include <MeshLib/MeshTools.h>

void FSTriMesh::Create(size_t faces)
{
	m_Face.resize(faces);
}

size_t FSTriMesh::FindFace(const vec3d p, double maxD) const
{
	double Dmin = 1e99;
	vec3d rmin = p;
	vec3d r[3];
	Intersection intersect;
	intersect.m_faceIndex = -1;
	for (int i = 0; i < Faces(); ++i)
	{
		const FACE& face = Face(i);

		for (int j = 0; j < 3; ++j) r[j] = face.r[j];

		// calculate distance to face nodes
		for (int j = 0; j < 3; ++j)
		{
			double dj = (p - r[j]).SqrLength();
			if ((dj < Dmin) && ((maxD <= 0) || (dj < maxD)))
			{
				rmin = r[j];
				Dmin = dj;
				intersect.m_faceIndex = i;
			}
		}
		if (Dmin == 0.0) return intersect.m_faceIndex;

		// try to project it on the face
		vec3d q(0, 0, 0);
		bool bproject = false;
		Intersection is;
		bproject = projectToTriangle(p, r[0], r[1], r[2], q, &is);

		if (bproject)
		{
			double dj = (p - q).SqrLength();
			if ((dj < Dmin) && ((maxD <= 0) || (dj < maxD)))
			{
				rmin = q;
				Dmin = dj;
				intersect = is;
				intersect.m_faceIndex = i;
			}
		}
	}

	return intersect.m_faceIndex;
}

/*size_t FSTriMesh::FindFace(const vec3d r) const
{
	for (size_t i = 0; i < m_Face.size(); ++i)
	{
		const FACE& f = m_Face[i];
		if ((f.r[0] - r).SqrLength() < 1e-12) return i;
		if ((f.r[1] - r).SqrLength() < 1e-12) return i;
		if ((f.r[2] - r).SqrLength() < 1e-12) return i;
	}
	return -1;
}
*/