/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2026 University of Utah, The Trustees of Columbia University in
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
#include "FindIntersectionsTool.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FSMesh.h>
#include <MeshTools/FindIntersections.h>
#include <FSCore/FSLogger.h>

CFindIntersectionsTool::CFindIntersectionsTool(CMainWindow* wnd) : CBasicTool(wnd, "Find Intersections", HAS_APPLY_BUTTON)
{
}

bool CFindIntersectionsTool::OnApply()
{
	GObject* po = GetActiveObject();
	if (po == nullptr) {
		SetErrorString("No object selected.");
		return false;
	}
	FSMeshBase* pm = po->GetEditableMesh();
	if (pm == nullptr)
	{
		SetErrorString("The selected object does not have a mesh.");
		return false;
	}

	// make sure all faces are triangles
	for (int i=0; i<pm->Faces(); ++i)
	{
		const FSFace& f = pm->Face(i);
		if (f.Nodes() != 3)
		{
			SetErrorString("The mesh must be a triangular mesh.");
			return false;
		}
	}

	FindIntersections finder(*pm);
	std::vector<int> intersectingFaces = finder.FindIntersectingFaces();

	FSLogger::Write("Found %d intersecting faces.\n", (int)intersectingFaces.size());

	// select all the intersecting faces in the mesh
	pm->ClearFaceSelection();
	for (int i=0; i<(int)intersectingFaces.size(); ++i)
	{
		pm->Face(intersectingFaces[i]).Select();
	}

	return true;
}