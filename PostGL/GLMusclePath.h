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
#include <FSCore/FSObjectList.h>
#include <MeshLib/FESurfaceMesh.h>

class FSMesh;

namespace Post {

class GLMusclePath : public CGLPlot
{
	enum { START_POINT, END_POINT, SUBDIVISIONS, MAX_SMOOTH_ITERS, SMOOTH_TOL, SNAP_TOL, SEARCH_RADIUS, PATH_GUIDE, PATH_RADIUS, COLOR, COLOR0, COLOR1, RENDER_MODE };

public:
	class PathData;

public:
	GLMusclePath();
	~GLMusclePath();

	void Render(GLRenderEngine& re, GLContext& rc) override;

	void Update() override;
	void Update(int ntime, float dt, bool breset) override;

	void SetModel(CGLModel* pm) override;

	bool UpdateData(bool bsave = true) override;

	double DataValue(int field, int step);

	void SwapEndPoints();

public:
	bool Intersects(Ray& ray, Intersection& q) override;
	FESelection* SelectComponent(int index) override;
	void ClearSelection() override;

	PathData* GetPath(int n) { return m_path[n]; }

	bool OverrideInitPath() const;

	std::vector<vec3d> GetInitPath() const;
	void SetInitPath(const std::vector<vec3d>& path);

protected:
	void UpdatePath(int ntime);
	void UpdatePathData(int ntime);

	bool UpdatePath(PathData* path, int ntime, bool reset = true);
	bool UpdateWrappingPath(PathData* path, int ntime, bool reset = true);
	bool UpdateGuidedPath(PathData* path, int ntime, bool reset = true);

	void ClearPaths();
	void ClearInitPath();
	void Reset();

	void BuildGuideMesh();
	void UpdateGuideMesh(int ntime);

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
	int		m_ndiv;
	int		m_maxIter;
	double	m_tol;
	double	m_searchRadius;
	double	m_snaptol;
	int		m_pathGuide;

	FSSurfaceMesh	m_guideMesh;

	// the currently selected point
	int	m_selectedPoint;
	double m_selectionRadius;
};
}
