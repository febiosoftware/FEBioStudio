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
#include <FECore/vec3d.h>
#include <vector>
#include <array>
#include <unordered_map>

//-----------------------------------------------------------------------------
//! Closest-point queries on a triangulated surface, accelerated with a uniform
//! grid. The surface's feature edges (dihedral angle larger than a threshold, or
//! non-manifold edges) and corners are detected as well, so that points can also
//! be projected onto the features. The feature edges split the surface into
//! patches and form feature curves (chains of feature edges between corners, or
//! closed loops); queries can be restricted to one patch or one curve.
class FESurfaceProjector
{
public:
	FESurfaceProjector();

	//! Build the search structures. featureAngle is in degrees.
	void Build(const std::vector<vec3d>& nodes, const std::vector<std::array<int, 3> >& tris, double featureAngle);

	//! Closest point q on the surface (or on one patch if patch >= 0), the unit normal n of the
	//! triangle that contains it and, optionally, its patch. Returns false if there is no such point.
	bool ClosestPoint(const vec3d& p, vec3d& q, vec3d& n, int patch = -1, int* patchOut = nullptr) const;

	//! Closest point q on the feature edges (or on one feature curve if curve >= 0) and the unit
	//! tangent t of that edge. Returns false if there is no such point.
	bool ClosestFeaturePoint(const vec3d& p, vec3d& q, vec3d& t, int curve = -1) const;

	//! Closest corner. Returns false if the surface has no corners.
	bool ClosestCorner(const vec3d& p, vec3d& q) const;

	int Triangles() const { return (int)m_tri.size(); }
	int FeatureEdges() const { return (int)m_seg.size(); }
	int Corners() const { return (int)m_corner.size(); }
	int Patches() const { return m_patches; }

	const vec3d& NodePosition(int i) const { return m_node[i]; }
	const std::vector<int>& CornerNodes() const { return m_corner; }

	//! A feature curve: its nodes in order, and the arc length at each node. A closed curve
	//! does not repeat its first node; its length includes the closing edge.
	struct Curve
	{
		std::vector<int>	nodes;
		std::vector<double>	s;
		double	length = 0.0;
		bool	closed = false;
	};
	int Curves() const { return (int)m_curve.size(); }
	const Curve& GetCurve(int i) const { return m_curve[i]; }

	//! point (and unit tangent) on a curve at arc length s (taken modulo the length for closed curves)
	vec3d CurvePoint(int curve, double s, vec3d* tangent = nullptr) const;

public:
	//! uniform grid that stores item indices per (non-empty) cell
	struct Grid
	{
		vec3d	box0;		// lower corner
		double	h = 1.0;	// cell size
		int		nx = 0, ny = 0, nz = 0;
		std::unordered_map<long long, std::vector<int> > cell;

		long long key(int i, int j, int k) const { return ((long long)k * ny + j) * nx + i; }
		void insert(int item, const vec3d& bmin, const vec3d& bmax);
	};

private:
	// Visit the grid items in shells of cells around p, nearest shells first, until no
	// item can be closer than the best one found. dist(item) returns the item's distance.
	template <class F> void search(const Grid& g, const vec3d& p, F dist) const;

private:
	std::vector<vec3d>					m_node;
	std::vector<std::array<int, 3> >	m_tri;
	std::vector<vec3d>					m_triNormal;
	std::vector<std::array<int, 2> >	m_seg;		// feature edges
	std::vector<int>					m_segCurve;	// curve of each feature edge
	std::vector<int>					m_corner;	// corner nodes
	std::vector<int>					m_triPatch;	// patch of each triangle
	int									m_patches = 0;
	std::vector<Curve>					m_curve;
	Grid	m_triGrid;
	Grid	m_segGrid;
};
