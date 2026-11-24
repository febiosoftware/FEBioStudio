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

class FSMesh;
class Transform;

// A line segment (global coordinates), used to assemble sweep paths.
struct PathSegment
{
	vec3d a, b;
};

// Sweeps a shell mesh (TRI3/QUAD4/TRI6/QUAD8) along a path into a solid mesh (PENTA6/HEX8/PENTA15/HEX20).
// The solid mesh is created in global coordinates.
class ShellSweeper
{
public:
	ShellSweeper();

	std::vector<vec3d>	m_path;			// ordered path points (global coordinates); the section follows the path from the first point
	int		m_layers;					// number of layers; 0 = one layer per path segment
	bool	m_rotate;					// rotate the section to follow the path tangent (parallel transport); otherwise only translate
	bool	m_autoPartition;			// partition the resulting surface (as the Mesh panel's Auto Partition)
	double	m_creaseAngle;				// crease angle (degrees)
	std::vector<vec3d>	m_orient;			// optional: desired normal of the section plane at each path point (same size as m_path; empty = the
										// section stays perpendicular to the path). The section is tilted by the smallest rotation taking the path tangent to this
										// direction (interpolated between path points).
	std::vector<double>	m_covariance;		// optional: area-normalized covariance matrix (xx,xy,xz,yy,yz,zz; global) of the target shape at each path
										// point (6 values per path point). When set, the section is deformed (within its plane) so that its covariance
										// matches, which scales it and turns e.g. a circle into an ellipse. Overrides m_scale.
	vec3d	m_startNormal;				// normal of the section plane (used with m_covariance; zero = use the path tangent)
	double	m_startCov[6];				// covariance of the section itself (used with m_covariance)
	// optional outline matching (arbitrary section shapes): the outlines (line segments, global coordinates; every edge of the section
	// is fine since only the outermost crossing along each ray from the centroid is used) of the start section (first entry) and of the
	// target sections, with the path index of each (m_outlineIndex[0] = 0, the last must be the last path point). Requires m_layers = 0
	// and m_covariance (which is used if the outlines can't be matched). Each node keeps its relative radius and polar angle between
	// the centroid and the outline; the in-plane rotation of each target is chosen so that the outline radial profiles agree best.
	typedef std::vector<std::pair<vec3d, vec3d>> Outline;
	std::vector<Outline>	m_outlines;
	std::vector<int>		m_outlineIndex;
	bool	m_alignTwist = false;		// outline matching: also rotate the targets in-plane (limited to 15 degrees per section) to best align the outlines
	bool	m_smoothStart = false;	// outline matching: replace the start section's outline by a smooth one (harmonics up to 4), e.g. the jagged rim of an STL cap
	bool	m_outlineUsed = false;	// (output) true if outline matching was applied
	std::vector<double>	m_scale;			// optional: uniform scale of the section at each path point (same size as m_path; empty = no scaling)
										// (the scale is relative to the first path point and is interpolated between path points)
	double	m_collapseTol;				// nodes of an element closer than this are merged, which turns degenerate
										// hexes (e.g. at an axis of rotation) into pentas/pyramids/tets (0 = automatic)

	// Build the solid. T maps the shell mesh's local coordinates to global coordinates.
	FSMesh* Apply(const FSMesh& shell, const Transform& T);

	const std::string& GetErrorString() const { return m_err; }

public:
	// Chain unordered segments into a single open polyline. The chain is ordered so that it starts at the end
	// closest to 'near'. Fails (returns false and sets err) if the segments don't form a single open chain.
	static bool ChainSegments(const std::vector<PathSegment>& segs, const vec3d& nearPoint, std::vector<vec3d>& path, std::string& err);

	// Resample a polyline into n segments of equal arclength (n+1 points).
	static std::vector<vec3d> Resample(const std::vector<vec3d>& path, int n);

private:
	std::string	m_err;
};
