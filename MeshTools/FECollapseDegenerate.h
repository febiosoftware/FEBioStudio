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
#include <string>

// Merges nodes that are closer than 'tol' to each other when they belong to the same solid element, and converts the
// resulting degenerate solid elements to the matching lower element: hex -> penta/pyramid/tet, penta -> pyramid/tet,
// pyramid -> tet (linear and quadratic: HEX8/20/27, PENTA6/15, PYRA5/13, TET4/10). Elements that collapse to
// zero volume (fewer than four distinct corner nodes) are tagged for removal (m_ntag = 1; all other elements get m_ntag = 0).
//
// tol <= 0 selects an automatic tolerance (1e-3 times the average element size).
// The mesh must be a mesh whose topology is rebuilt afterwards (faces, edges, ...). Unused nodes are not removed.
// Returns the number of elements that were modified, or -1 on failure (err is set).
int CollapseDegenerateElements(FSMesh& mesh, double tol, std::string& err);

// Mesh-panel modifier wrapping the above.
class FECollapseDegenerateElements : public FEModifier
{
public:
	FECollapseDegenerateElements();
	FSMesh* Apply(FSMesh* pm) override;
};
