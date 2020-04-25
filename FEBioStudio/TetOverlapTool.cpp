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
}

// method called when user presses Apply button (optional)
bool CTetOverlapTool::OnApply()
{
	m_ncount = 0;

	CDocument* doc = GetDocument();
	if ((doc == nullptr) || !doc->IsValid()) return SetErrorString("No valid document");

	GObject* po = doc->GetActiveObject();
	if ((po == 0) || (po->GetFEMesh() == 0))
	{
		return SetErrorString("You must select an object that has a mesh.");
	}

	FEMesh* mesh = po->GetFEMesh();
	if (mesh->IsType(FE_TET4) == false)
	{
		return SetErrorString("This tool only works with tet4 meshes.");
	}

	// do the overlap test
	TetOverlap tetOverlap;

	std::vector<pair<int, int> > tetList;
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
