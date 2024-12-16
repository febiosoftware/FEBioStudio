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

class FSModel;
class GMesh;
class CGLContext;

// class that creates and renders the plane cut in the CGLView.
class GLPlaneCut
{
public:
	GLPlaneCut();

	void Clear();

	bool IsValid() const;

	void Create(FSModel& fem, bool showMeshData, int mode);

	void SetPlaneCoordinates(double d0, double d1, double d2, double d3)
	{
		m_plane[0] = d0;
		m_plane[1] = d1;
		m_plane[2] = d2;
		m_plane[3] = d3;
		Clear();
	}

	double* GetPlaneCoordinates() { return m_plane; }

	void Render(CGLContext& rc);

	bool Intersect(const vec3d& p, const Ray& ray, Intersection& q);

private:
	void CreatePlaneCut(FSModel& fem, bool showMeshData);
	void CreateHideElements(FSModel& fem, bool showMeshData);

private:
	GMesh* m_planeCut = nullptr;
	double	m_plane[4] = { 1.0, 0.0, 0.0, 0.0 };
};
