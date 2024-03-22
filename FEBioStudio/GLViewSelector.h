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
#include <vector>

class CGLView;
class FSMeshBase;
class FSMesh;
class GObject;
class GLViewTransform;
class QRect;
class GEdge;

//-----------------------------------------------------------------------------
class SelectRegion
{
public:
	SelectRegion() {}
	virtual ~SelectRegion() {}

	virtual bool IsInside(int x, int y) const = 0;

	// see if a line intersects this region
	// default implementation only checks if one of the end points is inside.
	// derived classes should provide a better implementation.
	virtual bool LineIntersects(int x0, int y0, int x1, int y1) const;

	// see if a triangle intersects this region
	// default implementation checks for line intersections
	virtual bool TriangleIntersect(int x0, int y0, int x1, int y1, int x2, int y2) const;
};

class BoxRegion : public SelectRegion
{
public:
	BoxRegion(int x0, int x1, int y0, int y1);
	bool IsInside(int x, int y) const;
	bool LineIntersects(int x0, int y0, int x1, int y1) const;
private:
	int	m_x0, m_x1;
	int	m_y0, m_y1;
};

class CircleRegion : public SelectRegion
{
public:
	CircleRegion(int x0, int x1, int y0, int y1);
	bool IsInside(int x, int y) const;
	bool LineIntersects(int x0, int y0, int x1, int y1) const;
private:
	int	m_xc, m_yc;
	int	m_R;
};

class FreeRegion : public SelectRegion
{
public:
	FreeRegion(std::vector<std::pair<int, int> >& pl);
	bool IsInside(int x, int y) const;
private:
	std::vector<std::pair<int, int> >& m_pl;
	int m_x0, m_x1;
	int m_y0, m_y1;
};

//! helper class for selecting items in the CGLView
class GLViewSelector
{
public:
	GLViewSelector(CGLView* glview);

	void SetStateModifiers(bool shift, bool ctrl);

	// select geometry items
	void SelectParts(int x, int y);
	void SelectSurfaces(int x, int y);
	void SelectEdges(int x, int y);
	void SelectNodes(int x, int y);
	void SelectDiscrete(int x, int y);

	void SelectObjects(int x, int y);
	void SelectPostObject(int x, int y);

	// select items of a surface mesh
	void SelectSurfaceFaces(int x, int y);
	void SelectSurfaceEdges(int x, int y);
	void SelectSurfaceNodes(int x, int y);

	// select items of an FE mesh
	void SelectFEElements(int x, int y);
	void SelectFEFaces(int x, int y);
	void SelectFEEdges(int x, int y);
	void SelectFENodes(int x, int y);

	// region select geometry items
	void RegionSelectObjects(const SelectRegion& region);
	void RegionSelectParts(const SelectRegion& region);
	void RegionSelectSurfaces(const SelectRegion& region);
	void RegionSelectEdges(const SelectRegion& region);
	void RegionSelectNodes(const SelectRegion& region);
	void RegionSelectDiscrete(const SelectRegion& region);

	// region-select items of the FE mesh
	void RegionSelectFENodes(const SelectRegion& region);
	void RegionSelectFEFaces(const SelectRegion& region);
	void RegionSelectFEEdges(const SelectRegion& region);
	void RegionSelectFEElems(const SelectRegion& region);

	void BrushSelectFaces(int x, int y, bool badd, bool binit);

	void Finish();

private:
	void TagBackfacingNodes(FSMeshBase& mesh);
	void TagBackfacingEdges(FSMeshBase& mesh);
	void TagBackfacingFaces(FSMeshBase& mesh);
	void TagBackfacingElements(FSMesh& mesh);

	GEdge* SelectClosestEdge(GObject* po, GLViewTransform& transform, QRect& rt, double& zmin);

private:
	CGLView* m_glv;

	bool	m_bshift;
	bool	m_bctrl;

	std::vector<int>	m_selFaces0;	// selected faces (before brush selection)
};
