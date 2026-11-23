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
#include <functional>

//-----------------------------------------------------------------------------
//! Untangles and optimizes a mixed TET4/PYRA5/PENTA6/HEX8 mesh by minimizing the
//! regularized distortion energy of
//!   V. Garanzha, I. Kaporin, L. Kudryavtseva, F. Protais, N. Ray, D. Sokolov,
//!   "Foldover-free maps in 50 lines of code", ACM TOG 40(4), 2021.
//! The energy is evaluated at the integration points that FEBio uses to check
//! the elements (one point for TET4, six for PENTA6, 2x2x2 for PYRA5 and HEX8),
//! relative to an ideal element (regular tet, half-cube pyramid, right prism with
//! unit edges, cube) of the element's size. The element corners (except the
//! pyramid's apex) add a regularized term that penalizes folded or badly shaped
//! corners without acting as a barrier, since FEBio only checks the integration
//! points.
//! Nodes can be free, slide in a plane, slide along a line, or be fixed. After each
//! outer iteration an optional callback can move the sliding nodes back onto their
//! surface or curve and update the plane normals and line tangents.
class FEMeshUntangler
{
public:
	enum ElemType { TET4, PYRA5, HEX8, PENTA6 };
	struct Element
	{
		ElemType	type;
		int			n[8];
	};

	enum NodeType { FREE, PLANE, LINE, FIXED };
	struct NodeConstraint
	{
		NodeType	type = FREE;
		vec3d		dir;		// plane normal (PLANE) or line tangent (LINE)
		int			id = -1;	// user data (e.g. the surface or curve a node belongs to); not used here
	};

	// moves sliding nodes back onto their surface/curve and updates the constraint directions
	typedef std::function<void(std::vector<vec3d>& r, std::vector<NodeConstraint>& c)> ProjectFnc;

	// called after each outer iteration; return false to cancel
	typedef std::function<bool(int iteration, double minScaledJacobian)> ProgressFnc;

public:
	FEMeshUntangler();

	void SetThreads(int n) { m_threads = n; }	// 0 = all cores
	void SetMaxIterations(int outer, int inner) { m_maxOuter = outer; m_maxInner = inner; }

	//! Untangle and optimize the mesh. The node positions r are updated.
	//! Returns false if the optimization was canceled.
	bool Apply(std::vector<vec3d>& r, const std::vector<Element>& elems, std::vector<NodeConstraint>& c,
		ProjectFnc project = nullptr, ProgressFnc progress = nullptr);

	//! results of the last call to Apply
	int Iterations() const { return m_iterations; }
	int InvertedElements() const { return m_inverted; }
	double MinScaledJacobian() const { return m_minJ; }	// smallest det of the normalized Jacobian (1 = ideal element)
	double MinCornerScaledJacobian() const { return m_minCornerJ; }	// the same at the element corners (not at a pyramid's apex)

	//! smallest Jacobian determinant at the integration points FEBio uses to check an element
	static double MinJacobian(const std::vector<vec3d>& r, const Element& e);

	//! smallest determinant of the Jacobian relative to the ideal element with the same mean
	//! edge length, at the integration points (1 for an ideal element, <= 0 if inverted)
	static double ScaledJacobian(const std::vector<vec3d>& r, const Element& e);
	//! the same at the corners of the element (not at the apex of a pyramid; for a tet, the same
	//! as ScaledJacobian). FEBio does not check the corners, but an element with a folded
	//! corner gives inverted elements when it is refined.
	static double CornerScaledJacobian(const std::vector<vec3d>& r, const Element& e);

private:
	int		m_threads;
	int		m_maxOuter;
	int		m_maxInner;
	int		m_iterations;
	int		m_inverted;
	double	m_minJ;
	double	m_minCornerJ;
};
