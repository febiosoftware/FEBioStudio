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
#include "PostToolBar.h"
#include "MainWindow.h"
#include "Document.h"
#include "PostDocument.h"
#include "DataFieldSelector.h"
#include "IconProvider.h"
#include <PostGL/GLColorMap.h>
#include <PostLib/GLModel.h>
#include <QSpinBox>

class UIPostToolBar
{
public:
	CMainWindow*	m_wnd;

	QAction*	m_actionPlay;
	QAction*	m_actionColorMap;

	CDataFieldSelector*	m_selectData;
	QSpinBox*	m_spin;

public:
	void setup(CPostToolBar* tb)
	{
		QAction* actionRefresh = addAction("Reload", "actionRefresh", "refresh");
		QAction* actionFirst = addAction("first", "actionFirst", "back");
		QAction* actionPrev = addAction("previous", "actionPrev", "prev"); actionPrev->setShortcut(Qt::Key_Left);
		m_actionPlay = addAction("Play", "actionPlay", "play"); m_actionPlay->setShortcut(Qt::Key_Space);
		m_actionPlay->setCheckable(true);

		QIcon icon(":/icons/play.png");
		QPixmap pix(":/icons/pause.png");
		icon.addPixmap(pix, QIcon::Mode::Active, QIcon::State::On);
		m_actionPlay->setIcon(icon);

		QAction* actionNext = addAction("next", "actionNext", "next"); actionNext->setShortcut(Qt::Key_Right);
		QAction* actionLast = addAction("last", "actionLast", "forward");
		QAction* actionTime = addAction("Time settings", "actionTimeSettings", "clock");

		m_selectData = new CDataFieldSelector;
		m_selectData->setWhatsThis("<font color=\"black\">Use this to select the current data variable that will be used to display the color map on the mesh.");
		m_selectData->setMinimumWidth(300);
//		m_selectData->setFixedHeight(23);
		m_selectData->setObjectName("selectData");

		actionRefresh->setWhatsThis("<font color=\"black\">Click this to reload the plot file.");
		actionFirst->setWhatsThis("<font color=\"black\">Click this to go to the first time step in the model.");
		actionPrev->setWhatsThis("<font color=\"black\">Click this to go to the previous time step in the model.");
		m_actionPlay->setWhatsThis("<font color=\"black\">Click this to toggle the animation on or off");
		actionNext->setWhatsThis("<font color=\"black\">Click this to go to the next time step");
		actionLast->setWhatsThis("<font color=\"black\">Click this to go to the last time step in the model.");
		actionTime->setWhatsThis("<font color=\"black\">Click this to open the Time Info dialog box.");

		m_actionColorMap = addAction("Toggle colormap", "actionColorMap", "colormap");
		m_actionColorMap->setCheckable(true);
		m_actionColorMap->setWhatsThis("<font color=\"black\">Click this to turn on the color map on the model.");

		QAction* selectRect = addAction("Rectangle", "postSelectRect", "selectRect", true);
		QAction* selectCircle = addAction("Circle", "postSelectCircle", "selectCircle", true);
		QAction* selectFree = addAction("Freehand", "postSelectFree", "selectFree", true);
		QAction* actionMeasureTool = addAction("Measure tool", "postActionMeasureTool", "measure");

		tb->addAction(selectRect);
		tb->addAction(selectCircle);
		tb->addAction(selectFree);
		tb->addSeparator();
		tb->addAction(actionMeasureTool);
		tb->addSeparator();

		tb->addAction(actionRefresh);
		tb->addSeparator();
		tb->addAction(actionFirst);
		tb->addAction(actionPrev);
		tb->addAction(m_actionPlay);
		tb->addAction(actionNext);
		tb->addAction(actionLast);
		tb->addWidget(m_spin = new QSpinBox);
		m_spin->setObjectName("selectTime");
		m_spin->setMinimumWidth(80);
		m_spin->setSuffix("/100");
		tb->addAction(actionTime);
		tb->addWidget(m_selectData);
		tb->addAction(m_actionColorMap);
		tb->addSeparator();
	}

	QAction* addAction(const QString& title, const QString& name, const QString& iconFile = QString(), bool bcheckable = false)
	{
		QAction* pa = new QAction(title, m_wnd);
		pa->setObjectName(name);
		if (iconFile.isEmpty() == false) pa->setIcon(CIconProvider::GetIcon(iconFile));
		if (bcheckable) pa->setCheckable(true);
		return pa;
	}
};


CPostToolBar::CPostToolBar(CMainWindow* wnd) : QToolBar(wnd), ui(new UIPostToolBar)
{
	ui->m_wnd = wnd;
	ui->setup(this);
}

void CPostToolBar::Update()
{
	CPostDocument* doc = ui->m_wnd->GetPostDocument();
	if ((doc == nullptr) || (doc->IsValid() == false))
	{
		setDisabled(true);
		hide();
		return;
	}

	Post::CGLModel* mdl = doc->GetGLModel();
	if (mdl == 0)
	{
		setDisabled(true);
		return;
	}

	Post::CGLColorMap* map = mdl->GetColorMap();

	// rebuild the menu
	Post::FEPostModel* pfem = doc->GetFSModel();
	ui->m_selectData->BuildMenu(pfem, Post::TENSOR_SCALAR);
	ui->m_selectData->blockSignals(true);
	ui->m_selectData->setCurrentValue(map->GetEvalField());
	ui->m_selectData->blockSignals(false);

	// update the color map state
	if (map->IsActive()) ui->m_actionColorMap->setChecked(true);
	else ui->m_actionColorMap->setChecked(false);

	// update the state indicator
	int ntime = mdl->CurrentTimeIndex() + 1;

	Post::FEPostModel* fem = mdl->GetFSModel();
	int states = fem->GetStates();
	QString suff = QString("/%1").arg(states);
	ui->m_spin->setSuffix(suff);
	ui->m_spin->setRange(1, states);
	ui->m_spin->setValue(ntime);
	setEnabled(true);
	if (isHidden()) show();
}

void CPostToolBar::CheckPlayButton(bool b)
{
	ui->m_actionPlay->setChecked(b);
}

void CPostToolBar::CheckColorMap(bool b)
{
	if (b != ui->m_actionColorMap->isChecked())
		ui->m_actionColorMap->setChecked(b);
}

bool CPostToolBar::IsColorMapActive()
{
	return ui->m_actionColorMap->isChecked();
}

void CPostToolBar::ToggleColorMap()
{
	ui->m_actionColorMap->toggle();
}

void CPostToolBar::SetDataField(int n)
{
	if (ui->m_selectData->currentValue() != n) ui->m_selectData->setCurrentValue(n);
}

int CPostToolBar::GetDataField()
{
	return ui->m_selectData->currentValue();
}

void CPostToolBar::SetSpinValue(int n, bool blockSignals)
{
	if (blockSignals) ui->m_spin->blockSignals(true);
	ui->m_spin->setValue(n);
	if (blockSignals) ui->m_spin->blockSignals(false);
}
