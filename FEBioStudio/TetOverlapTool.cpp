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

#include "stdafx.h"
#include "TetOverlapTool.h"
#include "Document.h"
#include <GeomLib/GObject.h>
#include <MeshTools/TetOverlap.h>

// constructor
CTetOverlapTool::CTetOverlapTool(CMainWindow* wnd) : CBasicTool(wnd, "Tet Overlap", HAS_APPLY_BUTTON)
{
	m_ncount = 0;
	addIntProperty(&m_ncount, "Overlapping tets")->setFlags(CProperty::Visible);

	SetInfo("Finds tet elements that overlap. Overlapping tets are selected.");
}

// method called when user presses Apply button (optional)
bool CTetOverlapTool::OnApply()
{
	m_ncount = 0;

	FSMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return SetErrorString("You need to select a meshed object");

	if (mesh->IsType(FE_TET4) == false)
	{
		return SetErrorString("This tool only works with tet4 meshes.");
	}

	// do the overlap test
	TetOverlap tetOverlap;

	std::vector<std::pair<int, int> > tetList;
	if (tetOverlap.Apply(mesh, tetList) == false)
	{
		return SetErrorString("The tool has failed.");
	}

	// select all these elements
	for (int i = 0; i < mesh->Elements(); ++i)
	{
		mesh->Element(i).Unselect();
	}

	for (auto p : tetList)
	{
		mesh->Element(p.first).Select();
		mesh->Element(p.second).Select();
	}

	m_ncount = 0;
	for (int i = 0; i < mesh->Elements(); ++i)
	{
		if (mesh->Element(i).IsSelected()) m_ncount++;
	}

	return true;
}
