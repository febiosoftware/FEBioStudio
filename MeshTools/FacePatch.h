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

class GObject;
class FSMeshBase;
class GOCCObject;

// A triangulated patch of a geometry object's face (object-local coordinates).
struct FacePatch
{
	std::vector<vec3d>	pts;	// unique points
	std::vector<int>	tri;	// 3 point indices per triangle

	bool empty() const { return tri.empty(); }
	int Triangles() const { return (int)tri.size() / 3; }
};

// Extract the triangulation of the given faces (zero-based GFace indices) of an object from
// its render mesh. One patch is returned per face. Faces without triangles are skipped.
bool ExtractFacePatches(GObject* po, const std::vector<int>& faceIds, std::vector<FacePatch>& patches);

// Build a single patch from the given faces (zero-based face indices) of a mesh. Quads are split into
// two triangles, higher-order faces use their corner nodes only. Mesh nodes are shared by index.
bool ExtractMeshFacePatch(FSMeshBase* mesh, const std::vector<int>& faceIds, FacePatch& patch);

// diagonal of the bounding box of a set of patches
double PatchesDiagonal(const std::vector<FacePatch>& patches);

// merge patches into a single triangulation (points are not merged between patches)
FacePatch MergePatches(const std::vector<FacePatch>& patches);

// Returns true if the patch lies in a plane. If so, the plane's point and unit normal are returned.
bool IsPlanarPatch(const FacePatch& patch, vec3d& center, vec3d& normal, double relTol = 1e-4);

// Rebuild a set of *planar* patches as true OpenCascade faces (outer boundary plus holes), sew them into
// a shell and return it as a new GOCCObject. Returns nullptr if any patch is not planar, if the boundary
// can't be resolved, or if OCC is not available. errMsg (optional) receives the reason.
GOCCObject* BuildPlanarOCCShell(const std::vector<FacePatch>& patches, std::string* errMsg = nullptr);
