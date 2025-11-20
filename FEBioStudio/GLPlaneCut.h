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
#include <FSCore/ColorMap.h>

class CGLModelScene;
class GLMesh;
class GLRenderEngine;

// class that creates and renders the plane cut.
class GLPlaneCut
{
public:
	GLPlaneCut();

	void Clear();

	bool IsValid() const;

	void Create(CGLModelScene& scene, bool showMeshData, int mode);

	void SetPlaneCoordinates(double d0, double d1, double d2, double d3)
	{
		m_plane[0] = d0;
		m_plane[1] = d1;
		m_plane[2] = d2;
		m_plane[3] = d3;
		Clear();
	}

	double* GetPlaneCoordinates() { return m_plane; }

	void Render(GLRenderEngine& re);

	bool Intersect(const vec3d& p, const Ray& ray, Intersection& q);

	void SetColorMap(const CColorMap& map) { m_col = map; }

	void SetMeshColor(GLColor c) { m_meshColor = c; }
	void RenderMesh(bool b) { m_renderMesh = b; }

private:
	void CreatePlaneCut(CGLModelScene& scene, bool showMeshData);
	void CreateHideElements(CGLModelScene& scene, bool showMeshData);

private:
	GLMesh* m_planeCut = nullptr;
	double	m_plane[4] = { 1.0, 0.0, 0.0, 0.0 };
	CColorMap m_col;
	bool m_renderMesh = false;
	GLColor m_meshColor;
};
