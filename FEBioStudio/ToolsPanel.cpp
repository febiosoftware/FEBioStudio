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
#include "ToolsPanel.h"
#include "ui_toolspanel.h"
#include "MainWindow.h"
#include "ConchoidFitTool.h"
#include "QuadricFitTool.h"
#include "ReadCurveTool.h"
#include "FoamGeneratorTool.h"
#include "MaterialMapTool.h"
#include "ScalarFieldTool.h"
#include "EditDataFieldTool.h"
#include "PlaneCutTool.h"
#include "FiberGeneratorTool.h"
#include "AreaCalculatorTool.h"
#include "ImportSpringsTool.h"
#include "ICPRegistrationTool.h"
#include "ImageMapTool.h"
#include "DiscreteElementNetworkTool.h"
#include "SelectNearPlaneTool.h"
#include "KinematBuildTool.h"
#include "MeshMorphTool.h"

CToolsPanel::CToolsPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CToolsPanel)
{
	m_activeTool = 0;
	initTools();
	ui->setupUi(this);
}

void CToolsPanel::Update(bool breset)
{
	if (m_activeTool)
	{
		m_activeTool->Update();
	}
}

void CToolsPanel::initTools()
{
	CMainWindow* wnd = GetMainWindow();
	tools.push_back(new CConchoidFitTool   (wnd));
	tools.push_back(new CReadCurveTool     (wnd));
	tools.push_back(new CFoamGeneratorTool (wnd));
	tools.push_back(new CMaterialMapTool   (wnd));
	tools.push_back(new CScalarFieldTool   (wnd));
	tools.push_back(new CEditDataFieldTool (wnd));
	tools.push_back(new CPlaneCutTool      (wnd));
	tools.push_back(new CFiberGeneratorTool(wnd));
    tools.push_back(new CAreaCalculatorTool(wnd));
	tools.push_back(new CImportSpringsTool (wnd));
    tools.push_back(new CQuadricFitTool    (wnd));
	tools.push_back(new CICPRegistrationTool(wnd));
    tools.push_back(new CImageMapTool      (wnd));
    tools.push_back(new CDiscreteElementNetworkTool(wnd));
    tools.push_back(new CSelectNearPlaneTool(wnd));
	tools.push_back(new CKinematBuildTool   (wnd));
	tools.push_back(new CMeshMorphTool      (wnd));
}

void CToolsPanel::on_buttons_idClicked(int id)
{
	// deactivate the active tool
	if (m_activeTool) m_activeTool->Deactivate();
	m_activeTool = 0;

	// find the tool
	QList<CAbstractTool*>::iterator it = tools.begin();
	for (int i = 0; i<id - 1; ++i, ++it);

	// activate the tool
	m_activeTool = *it;
	m_activeTool->Activate();

	// show the tab
	ui->stack->setCurrentIndex(id);
}

void CToolsPanel::hideEvent(QHideEvent* ev)
{
	if (m_activeTool)
	{
		m_activeTool->Deactivate();
	}
	ev->accept();
}

void CToolsPanel::showEvent(QShowEvent* ev)
{
	if (m_activeTool)
	{
		m_activeTool->Activate();
	}
	ev->accept();
}

bool CToolsPanel::OnPickEvent(const FESelection& sel)
{
	if (m_activeTool)
	{
		return m_activeTool->onPickEvent(sel);
	}
	else return false;
}
