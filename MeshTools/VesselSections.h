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
#include <vector>
#include <string>

// Geometry helper for vessel-like (tubular) triangulated surfaces with two open ends (e.g. an artery STL):
// finds the centerline between the two ends, and cuts planar cross-sections perpendicular to it.
class VesselSections
{
public:
	// a planar cross-section
	struct Poly
	{
		std::vector<vec3d>	pts;	// closed polygon (no repeated first point)
		vec3d	c;					// centroid
		vec3d	n;					// unit normal, pointing along the direction of travel (from the first end to the second)
		double	area = 0.0;
	};

	// Set the surface (global coordinates). Vertices are welded; the surface must have exactly two open boundary loops.
	bool Init(const std::vector<vec3d>& pts, const std::vector<int>& tri, std::string& err);

	// For a closed vessel surface that includes its end caps: remove the start cap (the planar patch containing the triangles
	// startTris, indices into tri/3) and the end cap (the largest other planar patch) from tri, so that Init can be used.
	static bool RemoveCaps(const std::vector<vec3d>& pts, std::vector<int>& tri, const std::vector<int>& startTris, vec3d& startCentroid, std::vector<int>& startCapTris, std::string& err);

	// the end of the vessel nearest to this point becomes the first section (default: arbitrary)
	void SetStartPoint(const vec3d& r) { m_hint = r; m_hasHint = true; }

	// Trace the centerline from the first end loop to the second. Called by BuildSections if needed.
	bool TraceCenterline(std::string& err);

	// Create N+1 sections (the first and last are the open end loops) distributed along the centerline.
	// adaptivity = 0: uniform spacing along the centerline; larger values put more sections where the centerline curvature
	// (relative to the vessel radius) or the change of radius along the vessel is large.
	bool BuildSections(int N, double adaptivity, std::vector<Poly>& sections, std::string& err);

	const std::vector<vec3d>& Centerline() const { return m_center; }

public:
	// area, centroid and normal of a planar polygon
	static void PolygonInfo(const std::vector<vec3d>& p, Poly& out);

	// triangulate a planar polygon (ear clipping); returns 3 indices per triangle
	static bool TriangulatePolygon(const std::vector<vec3d>& poly, std::vector<int>& tri);

private:
	struct Loops { std::vector<std::vector<vec3d>> loops; };

	// intersect the surface with the plane through o with normal n; returns the closed loops
	void Slice(const vec3d& o, const vec3d& n, std::vector<std::vector<vec3d>>& loops) const;

	// the loop around the point q (or the nearest one); returns false if there are no loops
	bool PickLoop(const std::vector<std::vector<vec3d>>& loops, const vec3d& o, const vec3d& n, const vec3d& q, std::vector<vec3d>& loop) const;

	// centerline spline
	void CurveAt(double s, vec3d& p, vec3d& t) const;
	void PrepareCurve();
	void BuildNormalField();
	vec3d PlaneNormal(double s) const;

private:
	std::vector<vec3d>	m_pts;
	std::vector<int>	m_tri;
	Poly	m_end[2];			// the open end loops
	vec3d	m_nIn[2];			// unit normals of the end loops pointing into the vessel
	double	m_diag = 1.0;
	bool	m_hasHint = false;
	vec3d	m_hint;

	std::vector<vec3d>	m_center;	// centerline points
	std::vector<vec3d>	m_nf;		// rate-limited cutting-plane normals along the centerline
	std::vector<double>	m_s;		// arclength of the centerline points
	std::vector<vec3d>	m_tv;		// tangents at the centerline points
};
