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
#include "PropertyListForm.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include <FEMLib/FSModel.h>
#include <GeomLib/GModel.h>
#include <GeomLib/MeshLayer.h>
#include <GeomLib/FSGroup.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FEMesh.h>
#include <MeshLib/FEFace.h>
#include <MeshLib/FENode.h>
#include <MeshTools/FESelection.h>
#include <FECore/vec3d.h>
#include "Commands.h"
#include <QBoxLayout>
#include <QPushButton>

CSelectNearPlaneTool::CSelectNearPlaneTool(CMainWindow* wnd)
    :CBasicTool(wnd, "Select Near Plane"), m_direction(X), m_position(0), m_threshold(0.01), m_add(true)
{
    addEnumProperty(&m_direction, "Direction")->setEnumValues(QStringList() << "X" << "Y" << "Z" << "From Selected Face");
    addDoubleProperty(&m_position, "Position");
    addDoubleProperty(&m_threshold, "Threshold");
    addBoolProperty(&m_add, "Add to Selection");
}

QWidget* CSelectNearPlaneTool::createUi()
{
    QWidget* widget = new QWidget;

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    m_pform = new CPropertyListForm;
    m_pform->setPropertyList(this);
    layout->addWidget(m_pform);

    QPushButton* apply = new QPushButton("Select");
    layout->addWidget(apply);

    widget->setLayout(layout);

    QObject::connect(m_pform, &CPropertyListForm::dataChanged, [=](bool itemModified, int index){on_dataChanged(index);});
    QObject::connect(apply, &QPushButton::clicked, [=](){OnApply();});

    return widget;
}

bool CSelectNearPlaneTool::OnApply()
{
    auto doc = GetMainWindow()->GetModelDocument();

    if(!doc) return false;

    GObject* obj = GetMainWindow()->GetActiveObject();;

    if(!obj) return false;
    
    FSMesh* mesh = obj->GetFEMesh();

    std::vector<int> ids;

    std::vector<vec3d> normals;
    std::vector<vec3d> positions;

    switch (m_direction)
    {
    case X:
        normals.emplace_back(1,0,0);
        positions.emplace_back(m_position,0,0);
        break;
    case Y:
        normals.emplace_back(0,1,0);
        positions.emplace_back(0,m_position,0);
        break;
    case Z:
        normals.emplace_back(0,0,1);
        positions.emplace_back(0,0,m_position);
        break;
    case FACE:
    {
        FEFaceSelection* selection = dynamic_cast<FEFaceSelection*>(doc->GetCurrentSelection());

        if(!selection) return false;

        auto it = selection->begin();
        for(int index = 0; index < selection->Count(); index++, ++it)
        {
            normals.push_back(to_vec3d(it->m_fn));
            positions.push_back(mesh->FaceCenter(*it));
        }

        break;
    }
    default:
        break;
    }
    
    for(int nodeIndex = 0; nodeIndex < mesh->Nodes(); nodeIndex++)
    {
        FSNode node = mesh->Node(nodeIndex);

        for(int index = 0; index < normals.size(); index++)
        {
            double distance = abs(normals[index]*node.r - normals[index]*positions[index]);

            if(distance <= m_threshold)
            {
                ids.push_back(nodeIndex);
                break;
            }
        }
    }

    doc->SetItemMode(ITEM_NODE);

    CCmdSelectFENodes* cmd = new CCmdSelectFENodes(mesh, ids, m_add);

    doc->DoCommand(cmd);
}

void CSelectNearPlaneTool::on_dataChanged(int index)
{
    if(index == 0)
    {
        if(m_direction == FACE)
        {
            if(Property(1).flags != 0)
            {
                Property(1).setFlags(0);
                m_pform->setPropertyList(this);
            }
        }
        else
        {
            if(Property(1).flags != CProperty::Visible | CProperty::Editable)
            {
                Property(1).setFlags(CProperty::Visible | CProperty::Editable);
                m_pform->setPropertyList(this);
            }
        }
        }
}