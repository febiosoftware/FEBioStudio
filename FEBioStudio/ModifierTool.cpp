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
#include "ModifierTool.h"
#include <FSCore/ClassDescriptor.h>
#include <MeshTools/FEModifier.h>
#include <MeshTools/FESelection.h>
#include "PropertyListForm.h"
#include "PropertyList.h"
#include "ObjectProps.h"
#include <GLLib/GDecoration.h>

ModifierTool::ModifierTool(CMainWindow* wnd, ClassDescriptor* cd) : CAbstractTool(wnd, cd->GetName()), m_cd(cd)
{
	m_mod = nullptr;
	ui = nullptr;
}

void ModifierTool::Activate()
{
	CAbstractTool::Activate();

	if (m_cd) {
		m_mod = dynamic_cast<FEModifier*>(m_cd->Create());
		assert(m_mod);

		CPropertyList* pl = new CObjectProps(m_mod);
		ui->setPropertyList(pl);
	}
}

void ModifierTool::Deactivate()
{
	if (ui) ui->setPropertyList(nullptr);
	if (m_mod) delete m_mod;
	m_mod = nullptr;
	CAbstractTool::Deactivate();
}

void ModifierTool::updateUi()
{
	if (ui) ui->updateData();
	CAbstractTool::updateUi();
}

QWidget* ModifierTool::createUi()
{
	ui = new CPropertyListForm();
	ui->setBackgroundRole(QPalette::Light);

	QObject::connect(ui, &CPropertyListForm::dataChanged, this, &ModifierTool::on_dataChanged);

	return ui;
}

void ModifierTool::on_dataChanged()
{
	UpdateData();
}

void ModifierTool::UpdateData()
{

}

FEModifier* ModifierTool::GetModifier() { return m_mod; }

unsigned int ModifierTool::flags() const { return (m_cd ? m_cd->Flag() : 0); }


//=============================================================================
CAddTriangleTool::CAddTriangleTool(CMainWindow* wnd, ClassDescriptor* cd) : ModifierTool(wnd, cd) { m_pick = 0; }

bool CAddTriangleTool::onPickEvent(const FESelection& sel)
{
	const FENodeSelection* nodeSel = dynamic_cast<const FENodeSelection*>(&sel);
	if (nodeSel && (nodeSel->Count() == 1))
	{
		int nid = nodeSel->NodeIndex(0);
		FEAddTriangle* mod = dynamic_cast<FEAddTriangle*>(GetModifier());
		if (mod)
		{
			const FSLineMesh* mesh = nodeSel->GetMesh();
			points.push_back(to_vec3f(mesh->NodePosition(nid)));
			mod->SetIntValue(m_pick, nid + 1);
			for (int i = m_pick + 1; i < 3; ++i) mod->SetIntValue(i, 0);

			m_pick++;
			if (m_pick >= 3)
			{
				mod->push_stack();
				m_pick = 0;
			}

			BuildDecoration();

			return true;
		}
	}
	return false;
}

void CAddTriangleTool::BuildDecoration()
{
	GCompositeDecoration* deco = new GCompositeDecoration;
	int n = (int)points.size();
	int nf = n / 3;
	for (int i = 0; i < nf; i++)
	{
		GDecoration* di = new GTriangleDecoration(points[3 * i], points[3 * i + 1], points[3 * i + 2]);
		if (i < nf - 1) di->setColor(GLColor(200, 200, 0));
		deco->AddDecoration(di);
	}
	if ((n % 3) == 2)
	{
		deco->AddDecoration(new GLineDecoration(points[n - 2], points[n - 1]));
	}
	else if ((n % 3) == 1)
	{
		deco->AddDecoration(new GPointDecoration(points[n - 1]));
	}
	SetDecoration(deco);
}

bool CAddTriangleTool::onUndoEvent()
{
	FEAddTriangle* mod = dynamic_cast<FEAddTriangle*>(GetModifier());
	if (mod)
	{
		if (points.size() > 0) points.pop_back();
		else return false;

		m_pick--;
		if (m_pick < 0)
		{
			m_pick = 2;
			mod->pop_stack();
		}
		BuildDecoration();
		return false; // fall through so selection is also undone
	}
	else return false;
}

void CAddTriangleTool::Reset()
{
	m_pick = 0;
	points.clear();
	FEAddTriangle* mod = dynamic_cast<FEAddTriangle*>(GetModifier());
	if (mod)
	{
		mod->SetIntValue(0, 0);
		mod->SetIntValue(1, 0);
		mod->SetIntValue(2, 0);
	}
	CAbstractTool::Reset();
}

//=============================================================================
CAddNodeTool::CAddNodeTool(CMainWindow* wnd, ClassDescriptor* cd) : ModifierTool(wnd, cd) {}

void CAddNodeTool::BuildDecoration()
{
	FEAddNode* mod = dynamic_cast<FEAddNode*>(GetModifier());
	if (mod)
	{
		vec3d r = mod->GetVecValue(0);
		SetDecoration(new GPointDecoration(to_vec3f(r)));
	}
}

void CAddNodeTool::UpdateData()
{
	BuildDecoration();
	updateUi();
}
