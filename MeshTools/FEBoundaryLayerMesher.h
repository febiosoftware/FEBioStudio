/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
// This is a routine for creating a boundary layer on a selected
// boundary surface of a hex8/penta6 mesh.
// The layer of hex/penta elements adjacent to the selected surface is
// subdivided into a (optionally biased) mesh along the topological
// normal (not the true normal) to the selected boundary.
// If two adjacent faces of the same element are selected, this represents
// an external corner in the mesh, which requires special treatment.
// Any other combination of multiple face selections on the same element
// cannot be accommodated and the routine exits without any changes.
// Two adjacent faces form an internal corner of the mesh if corresponding
// hex elements don't share a face, but share a single common neighboring
// element which only has an edge on the selected surface. These internal
// corners are accommodated in this routine.
class FEBoundaryLayerMesher : public FEModifier
{
public:
	FEBoundaryLayerMesher();

	FEMesh* Apply(FEMesh* pm);

protected:
	bool BoundaryLayer(FEMesh* pm);
};
