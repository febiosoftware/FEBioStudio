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

#include "SelectNearPlaneTool.h"
#include "ModelDocument.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSFace.h>
#include <MeshLib/FSNode.h>
#include <FECore/vec3d.h>
#include "Commands.h"

CSelectNearPlaneTool::CSelectNearPlaneTool(CMainWindow* wnd)
    :CBasicTool(wnd, "Select Near Plane", HAS_APPLY_BUTTON), m_direction(X), m_offset(0), m_threshold(0.01), m_add(true)
{
    addEnumProperty(&m_direction, "Direction")->setEnumValues(QStringList() << "X" << "Y" << "Z" << "From Selected Face");
    addDoubleProperty(&m_offset, "Offset");
    addDoubleProperty(&m_threshold, "Threshold");
    addBoolProperty(&m_add, "Add to Selection");

	SetApplyButtonText("Select");
}

bool CSelectNearPlaneTool::OnApply()
{
    GObject* obj = GetActiveObject();
    if(!obj) return false;
    
    FSMesh* mesh = obj->GetFEMesh();
	if (mesh == nullptr) return false;

    std::vector<std::pair<vec3d,vec3d>> planes; // position and normal

    switch (m_direction)
    {
    case X:
        planes.emplace_back(vec3d(m_offset,0,0), vec3d(1,0,0));
        break;
    case Y:
		planes.emplace_back(vec3d(0,m_offset,0), vec3d(0,1,0));
        break;
    case Z:
        planes.emplace_back(vec3d(0,0,m_offset), vec3d(0,0,1));
        break;
    case FACE:
    {
		for (int n = 0; n < mesh->Faces(); n++)
		{
			FSFace& f = mesh->Face(n);
			if (f.IsSelected())
			{
				vec3d N = mesh->FaceNormal(f);
				vec3d c = mesh->FaceCenter(f);
				planes.emplace_back(c + N * m_offset, N);
			}
        }
    }
	break;
    default:
		assert(false);
		return false;
        break;
    }
    
	std::vector<int> ids;
	for(int nodeIndex = 0; nodeIndex < mesh->Nodes(); nodeIndex++)
    {
        FSNode node = mesh->Node(nodeIndex);

        for(int index = 0; index < planes.size(); index++)
        {
			vec3d p = planes[index].first;
			vec3d N = planes[index].second;
            double distance = abs(N*node.r - N*p);
            if(distance <= m_threshold)
            {
                ids.push_back(nodeIndex);
                break;
            }
        }
    }

	CGLDocument* doc = GetDocument();
	if (doc)
	{
		doc->SetItemMode(ITEM_NODE);
		doc->DoCommand(new CCmdSelectFENodes(mesh, ids, m_add));
	}
	else return false;

	return true;
}
