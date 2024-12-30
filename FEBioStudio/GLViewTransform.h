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

class CGLView;
class GLCamera;

// class that can be used to map screen to world and vice versa
// NOTE: make sure to call makeCurrent before using this class!
class GLViewTransform
{
public:
	GLViewTransform(CGLView* view);

	// convert a point in world coordinates to screen coordinates
	// the return value is a vec3d where x, y are screen coordinates
	// and z is the normalized distance to screen
	vec3d WorldToScreen(const vec3d& r);

	// calculate a ray that starts at the screen position and points forward
	Ray PointToRay(int x, int y);

	// Is the point inside the viewing frustrum (p is in device coordinates)
	bool IsVisible(const vec3d& p);

private:
	void PositionInScene(const GLCamera& cam);

private:
	CGLView*	m_view;	
	matrix		m_PM, m_PMi;
	int			m_vp[4];
	vector<double>	c, q;
};
