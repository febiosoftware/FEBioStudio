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
#include "FEModifier.h"

//-----------------------------------------------------------------------------
//! This modifier converts a TET4 mesh into a hex-dominant mesh using the
//! field-guided polyhedral agglomeration method of Gao et al. (SIGGRAPH 2017),
//! https://github.com/gaoxifeng/robust_hex_dominant_meshing
//!
//! The algorithm produces hexahedra plus general polyhedra. Since FEBio does
//! not support general polyhedral elements, cells that are tetrahedra, pyramids
//! or triangular prisms become TET4, PYRA5 or PENTA6 elements, and every other
//! cell is split about an interior point into PYRA5 (on quad faces) and TET4 (on
//! triangular faces) elements. Faces with more than four edges get a (shared)
//! face-center node. The resulting mesh is conforming.
//!
//! The orientation and position fields are aligned with the sharp edges and corners
//! of the input surface (see the feature angle), so that the lattice runs along
//! the sharp edges instead of cutting them (an addition to the original method).
//!
//! Post-processing for FEBio:
//! - Coincident vertices of the polyhedral mesh are merged, and flat cells lying on
//!   the boundary (which no node movement can make valid) are removed.
//! - Sharp edges and corners are recovered exactly: corners get the nearest boundary
//!   node, and each feature curve gets a path of boundary edges (or of diagonals
//!   of boundary faces, which are then split) whose nodes are snapped onto it.
//!   The other boundary nodes are projected onto the input surface, on their side
//!   of the edges.
//! - The split point of each polyhedron is found by linear programming: the
//!   sub-element Jacobians at FEBio's integration points are affine in it.
//!   Cells without a good split point (e.g. nearly flat cells) are merged with
//!   neighbors. Cells that still have no valid split point (folded cells, or
//!   clusters of cells of zero volume) are merged with the region around them
//!   that a single interior point can see completely.
//! - All nodes are optimized with the foldover-free untangling method of Garanzha
//!   et al. (2021), at FEBio's integration points and (as a soft penalty) at the
//!   element corners; boundary nodes slide on the input surface and its edges.
//! - Pyramids that still have a folded corner (on a warped base, next to the surface)
//!   are merged with the element across their base and split again around a new point
//!   (or split into two tets if the base is on the surface): FEBio only checks the
//!   integration points, but a folded corner gives inverted elements when the mesh is
//!   refined. Nearly flat corners are replaced too, unless that would split a hex.
//! - A tet mesh whose tets are larger than the element size on average is first
//!   remeshed with MMG (if available).
class HexDominantRemesh : public FEModifier
{
	enum {
		ELEM_SIZE,
		ITERATIONS,
		THREADS,
		FEATURE_ANGLE,
		REMESH_INPUT
	};

public:
	HexDominantRemesh();
	FSMesh* Apply(FSMesh* pm) override;
	FSMesh* Apply(FSGroup* pg) override;
};
