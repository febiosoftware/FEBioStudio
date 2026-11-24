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

// Helper shared by the shell extruder and the shell sweeper.
// It analyzes a shell mesh that is to be extruded/swept in layers into a solid mesh:
//   TRI3  -> PENTA6      QUAD4 -> HEX8
//   TRI6  -> PENTA15     QUAD8 -> HEX20
// All shell elements must be of one of these types, and all must be linear or all quadratic.
class ShellSection
{
public:
	// analyze the shell mesh (fails with an error message for unsupported element types)
	bool Init(const FSMesh& shell, std::string& err);

	bool quadratic = false;			// true if the elements are TRI6/QUAD8
	int nn = 0;						// number of shell nodes that are used (including mid-edge nodes)
	int ncorner = 0;				// number of those that are corner nodes

	std::vector<int> base;			// used node --> shell node index
	std::vector<int> cornerIndex;	// used node --> corner index (-1 for mid-edge nodes)
	std::vector<int> cornerNode;	// corner index --> used node
	std::vector<int> elems;			// shell element indices

	// per element: number of corners (3 or 4) and its nodes (used indices): corners first, then mid-edge nodes
	std::vector<int> ecorners;
	std::vector<std::vector<int>> enodes;

	int Elements() const { return (int)elems.size(); }

	// area-weighted normal of element e (r0 = positions of the used nodes)
	vec3d AreaNormal(int e, const std::vector<vec3d>& r0) const;

	// Build the solid mesh.
	//   r0       : positions of the used nodes of the section (used to orient the elements)
	//   nl       : number of layers
	//   layerPos : node positions for layers 0..nl, index l*nn + i
	//   midPos   : (quadratic only) positions of the corner nodes half way between layers, index l*ncorner + c for l = 0..nl-1
	FSMesh* BuildSolid(const std::vector<vec3d>& r0, int nl, const std::vector<vec3d>& layerPos, const std::vector<vec3d>& midPos) const;
};

// Eliminate degenerate elements of a newly built solid mesh, remove unused nodes, rebuild the mesh and optionally
// auto-partition the surface. Returns false (and sets err) on failure; the mesh is not deleted.
bool FinishSolidMesh(FSMesh& mesh, double collapseTol, bool autoPartition, double creaseAngle, std::string& err);

// Convert a linear shell mesh (TRI3/QUAD4) to a quadratic one (TRI6/QUAD8) by adding mid-edge nodes at the
// edge midpoints. Returns nullptr if the mesh contains other element types.
FSMesh* ConvertShellToQuadratic(const FSMesh& shell);
