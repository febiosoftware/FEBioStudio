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

// Sweeps a linear shell mesh (TRI3/QUAD4) along a path into a solid mesh of PENTA6/HEX8 elements.
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
