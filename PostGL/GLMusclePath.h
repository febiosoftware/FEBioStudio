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
#include "GLPlot.h"

class FSMesh;

namespace Post {

class GLMusclePath : public CGLPlot
{
	enum { START_POINT, END_POINT, METHOD, SUBDIVISIONS, MAX_SMOOTH_ITERS, SMOOTH_TOL, SMOOTH_FACTOR, SNAP_TOL, SEARCH_RADIUS, SELECTION_RADIUS, NORMAL_TOL, PATH_RADIUS, COLOR, COLOR0, COLOR1, RENDER_MODE };

public:
	class PathData;

public:
	GLMusclePath();
	~GLMusclePath();

	void Render(CGLContext& rc) override;

	void Update() override;
	void Update(int ntime, float dt, bool breset) override;

	bool UpdateData(bool bsave = true) override;

	double DataValue(int field, int step);

	void SwapEndPoints();

public:
	bool Intersects(Ray& ray, Intersection& q);
	FESelection* SelectComponent(int index);
	void ClearSelection();

	PathData* GetPath(int n) { return m_path[n]; }

protected:
	void UpdatePath(int ntime);
	void UpdatePathData(int ntime);
	void ClearPaths();
	void ClearInitPath();
	void Reset();

	bool UpdateStraightPath(PathData* path, int ntime);
	bool UpdateWrappingPath(PathData* path, int ntime, bool reset = true);

private:
	PathData* m_initPath; // used as initial path
	std::vector<PathData*>	m_path;	// points defining the path

	// information to track motion of origin
	int		m_closestFace;	// surface face closest to origin
	vec3d	m_qr;

	// the material IDs of the start and end points
	int		m_part[2];

	// values that require re-evaluation upon change
	int		m_node0;
	int		m_node1;
	int		m_method;
	int		m_ndiv;
	int		m_maxIter;
	double	m_tol;
	double	m_searchRadius;
	double	m_normalTol;
	double	m_snaptol;

	// the currently selected point
	int	m_selectedPoint;
};

}
