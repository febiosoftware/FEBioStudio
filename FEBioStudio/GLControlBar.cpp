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
#include "GLControlBar.h"
#include "MainWindow.h"
#include "Document.h"
#include "GLModelDocument.h"
#include "GLView.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QValidator>
#include <QToolButton>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <GeomLib/GCurveMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include "Logger.h"

class CGLControlBar_UI
{
public:
	QLineEdit*	x;
	QLineEdit*	y;
	QLineEdit*	z;

	CMainWindow*	m_wnd;

	QWidget* edit;

	QToolButton*	meshItemButton[4];
	QToolButton*	meshToolButton[3];
	QDoubleSpinBox*	maxAngle;
	QToolButton*	cull;
	QToolButton*	noint;
	QToolButton*	showMesh;
	QToolButton*	toggleLight;

public:
	void setup(CGLControlBar* bar)
	{
		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0,0,0,0);

		QToolButton* lockPivot   = addButton(QIcon(":/icons/pivot.png"), "Lock pivot");
		QToolButton* snapToGrid  = addButton(QIcon(":/icons/snaptogrid.png"), "Snap to grid"); snapToGrid->setChecked(true);
		QToolButton* snapToNode  = addButton(QIcon(":/icons/snaptonode.png"), "Snap to node");
		QToolButton* hide        = addButton(QIcon(":/icons/hide.png"), "Hide selection", false);
		QToolButton* showAll     = addButton(QIcon(":/icons/show.png"), "Show all", false);
		QToolButton* selectHide  = addButton(QIcon(":/icons/select_hide.png"), "Select and hide", true);
		QToolButton* toggleViz   = addButton(QIcon(":/icons/toggle_visible.png"), "Toggle visibility", false);
		QToolButton* zoomSelect  = addButton(QIcon(":/icons/zoom_select.png"), "Zoom to selection", false); 
		QToolButton* zoomAll = addButton(QIcon(":/icons/zoom_all.png"), "Zoom to extents", false);
		showMesh = addButton(QIcon(":/icons/show_mesh.png"), "Toggle mesh lines", true);
		toggleLight = addButton(QIcon(":/icons/light.png"), "Toggle light", true);

		// mesh editing tool buttons
		edit = new QWidget;

		meshItemButton[0] = addButton(QIcon(":/icons/selElem.png"), "Select elements", true); 
		meshItemButton[1] = addButton(QIcon(":/icons/selFace.png"), "Select faces", true);
		meshItemButton[2] = addButton(QIcon(":/icons/selEdge.png"), "Select edges", true);
		meshItemButton[3] = addButton(QIcon(":/icons/selNode.png"), "Select nodes", true);

		meshToolButton[0] = addButton(QIcon(":/icons/select_connected.png"), "Select connected");

		maxAngle = new QDoubleSpinBox; maxAngle->setRange(0.0, 180); maxAngle->setSingleStep(0.5);
		maxAngle->setMaximumWidth(60);

		meshToolButton[1] = addButton(QIcon(":/icons/brush.png"), "Brush Select");
		meshToolButton[2] = addButton(QIcon(":/icons/select_path.png"), "Select by closest path");
		cull    = addButton(QIcon(":/icons/backface.png"), "Select backfacing");
		noint   = addButton(QIcon(":/icons/ignore.png"), "Ignore interior");

		QHBoxLayout* hl = new QHBoxLayout;
		hl->setContentsMargins(0,0,0,0);

		QFrame* sep = new QFrame;
		sep->setFrameShape(QFrame::VLine);
//		sep->setFrameShadow(QFrame::Sunken);
		hl->addWidget(sep);

		hl->addWidget(meshItemButton[0]);
		hl->addWidget(meshItemButton[1]);
		hl->addWidget(meshItemButton[2]);
		hl->addWidget(meshItemButton[3]);

		sep = new QFrame;
		sep->setFrameShape(QFrame::VLine);
		hl->addWidget(sep);

		hl->addWidget(meshToolButton[0]);
		hl->addWidget(maxAngle);
		hl->addWidget(meshToolButton[1]);
		hl->addWidget(meshToolButton[2]);

		sep = new QFrame;
		sep->setFrameShape(QFrame::VLine);
		hl->addWidget(sep);

		hl->addWidget(cull);
		hl->addWidget(noint);
		hl->addStretch();

		QButtonGroup* bg = new QButtonGroup(bar);
		bg->setExclusive(false);
		bg->addButton(meshItemButton[0], 0);
		bg->addButton(meshItemButton[1], 1);
		bg->addButton(meshItemButton[2], 2);
		bg->addButton(meshItemButton[3], 3);

		QButtonGroup* bg2 = new QButtonGroup(bar);
		bg2->setExclusive(false);
		bg2->addButton(meshToolButton[0], 0);
		bg2->addButton(meshToolButton[1], 1);
		bg2->addButton(meshToolButton[2], 2);

		edit->setLayout(hl);

		// assemble tools
		h->addWidget(lockPivot);

		h->addWidget(x = new QLineEdit); x->setMaximumWidth(100); x->setValidator(new QDoubleValidator); x->setReadOnly(true);
		h->addWidget(y = new QLineEdit); y->setMaximumWidth(100); y->setValidator(new QDoubleValidator); y->setReadOnly(true);
		h->addWidget(z = new QLineEdit); z->setMaximumWidth(100); z->setValidator(new QDoubleValidator); z->setReadOnly(true);

		h->addWidget(snapToGrid);
		h->addWidget(snapToNode);
		h->addWidget(hide);
		h->addWidget(showAll);
		h->addWidget(toggleViz);
		h->addWidget(selectHide);
		h->addWidget(zoomSelect);
		h->addWidget(zoomAll);
		h->addWidget(showMesh);
		h->addWidget(toggleLight);
		h->addWidget(edit);
		h->addStretch();

		bar->setLayout(h);

		QObject::connect(x, SIGNAL(textEdited(const QString&)), bar, SLOT(onPivotChanged()));
		QObject::connect(y, SIGNAL(textEdited(const QString&)), bar, SLOT(onPivotChanged()));
		QObject::connect(z, SIGNAL(textEdited(const QString&)), bar, SLOT(onPivotChanged()));
		QObject::connect(lockPivot, SIGNAL(clicked(bool)), bar, SLOT(onPivotClicked(bool)));
		QObject::connect(snapToGrid, SIGNAL(clicked(bool)), bar, SLOT(onSnapToGridClicked(bool)));
		QObject::connect(snapToNode, SIGNAL(clicked(bool)), bar, SLOT(onSnapToNodeClicked(bool)));
		QObject::connect(hide, SIGNAL(clicked(bool)), bar, SLOT(onHideSelection(bool)));
		QObject::connect(showAll, SIGNAL(clicked(bool)), bar, SLOT(onShowAll(bool)));
		QObject::connect(selectHide, SIGNAL(clicked(bool)), bar, SLOT(onSelectAndHide(bool)));
		QObject::connect(toggleViz, SIGNAL(clicked(bool)), bar, SLOT(onToggleVisibleClicked(bool)));
		QObject::connect(zoomSelect, SIGNAL(clicked(bool)), bar, SLOT(onZoomSelectClicked(bool)));
		QObject::connect(zoomAll, SIGNAL(clicked(bool)), bar, SLOT(onZoomAllClicked(bool)));
		QObject::connect(showMesh, SIGNAL(clicked(bool)), bar, SLOT(onToggleMesh(bool)));
		QObject::connect(toggleLight, SIGNAL(clicked(bool)), bar, SLOT(onToggleLight(bool)));
		QObject::connect(bg , SIGNAL(idClicked(int)), bar, SLOT(onMeshButtonClicked(int)));
		QObject::connect(bg2, SIGNAL(idClicked(int)), bar, SLOT(onMeshToolClicked(int)));
		QObject::connect(maxAngle, SIGNAL(valueChanged(double)), bar, SLOT(onMaxAngleChanged(double)));
		QObject::connect(cull, SIGNAL(clicked(bool)), bar, SLOT(onSelectBackfacing(bool)));
		QObject::connect(noint, SIGNAL(clicked(bool)), bar, SLOT(onIgnoreInterior(bool)));
	}

	QToolButton* addButton(const QIcon& icon, const QString& toolTip, bool isCheckable = true)
	{
		QToolButton* b = new QToolButton;
		b->setIcon(icon);
		b->setCheckable(isCheckable);
		b->setAutoRaise(true);
		b->setToolTip(QString("<font color=\"black\">") + toolTip);
		return b;
	}

	vec3d getPivot()
	{
		vec3d r;
		r.x = x->text().toDouble();
		r.y = y->text().toDouble();
		r.z = z->text().toDouble();
		return r;
	}

	void showEditButtons(bool b, bool showElem = true, bool showFace = true)
	{
		edit->setVisible(b);
		if (b)
		{
			meshItemButton[0]->setEnabled(showElem);
			meshItemButton[1]->setEnabled(showFace);
		}
	}

	void checkButton(int id)
	{
		for (int i = 0; i<4; ++i)
			if (id != i) meshItemButton[i]->setChecked(false);

		if (id == -1) return;
		if (meshItemButton[id]->isChecked() == false)
		{
			meshItemButton[id]->blockSignals(true);
			meshItemButton[id]->setChecked(true);
			meshItemButton[id]->blockSignals(false);
		}
	}
};

CGLControlBar::CGLControlBar(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new CGLControlBar_UI)
{
	ui->m_wnd = wnd;
	ui->setup(this);
	ui->showEditButtons(false);
}

void CGLControlBar::toggleSelectConnected()
{
	ui->meshToolButton[0]->toggle();
	onMeshToolClicked(0);
}

void CGLControlBar::SetMeshItem(int n)
{
	CGLDocument* pdoc = ui->m_wnd->GetGLDocument();
	pdoc->SetItemMode(n);
	switch (n)
	{
	case ITEM_ELEM: ui->checkButton(0); break;
	case ITEM_FACE: ui->checkButton(1); break;
	case ITEM_EDGE: ui->checkButton(2); break;
	case ITEM_NODE: ui->checkButton(3); break;
	default:
		ui->checkButton(-1);
	}
}

void CGLControlBar::Update()
{
	CGLView* view = ui->m_wnd->GetGLView();
	
	vec3d pivot = view->GetPivotPosition();
	ui->x->setText(QString::number(pivot.x));	
	ui->y->setText(QString::number(pivot.y));
	ui->z->setText(QString::number(pivot.z));

	bool pivotMode = view->GetPivotUserMode();

	ui->x->setReadOnly(!pivotMode);
	ui->y->setReadOnly(!pivotMode);
	ui->z->setReadOnly(!pivotMode);

	CGLDocument* pdoc = ui->m_wnd->GetGLDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& vs = ui->m_wnd->GetGLView()->GetViewSettings();

	ui->showMesh->setChecked(vs.m_bmesh);

	GObject* po = view->GetActiveObject();

	int meshMode = ui->m_wnd->GetMeshMode();

	// for post-docs we always use mesh_mode_volume
	if (dynamic_cast<CGLModelDocument*>(ui->m_wnd->GetDocument())) meshMode = MESH_MODE_VOLUME;

	if (meshMode == MESH_MODE_VOLUME)
	{
		if (po && po->GetFEMesh())
		{
			ui->showEditButtons(true);

			int item = pdoc->GetItemMode();
			switch (item)
			{
			case ITEM_MESH: ui->checkButton(-1); break;
			case ITEM_ELEM: ui->checkButton(0); break;
			case ITEM_FACE: ui->checkButton(1); break;
			case ITEM_EDGE: ui->checkButton(2); break;
			case ITEM_NODE: ui->checkButton(3); break;
			}

			ui->meshToolButton[0]->setChecked(vs.m_bconn);
			ui->meshToolButton[1]->setChecked(vs.m_bselbrush);
			ui->meshToolButton[2]->setChecked(vs.m_bselpath);
			ui->maxAngle->setValue(vs.m_fconn);
			ui->cull->setChecked(!vs.m_bcullSel);
			ui->noint->setChecked(vs.m_bext);

			return;
		}
	}
	else
	{
		GSurfaceMeshObject* ps = dynamic_cast<GSurfaceMeshObject*>(po);
		if (ps)
		{
			ui->showEditButtons(true, false);
			int item = pdoc->GetItemMode();
			if (item == ITEM_ELEM) { pdoc->SetItemMode(ITEM_MESH); item = ITEM_MESH; }
			switch (item)
			{
			case ITEM_MESH: ui->checkButton(-1); break;
				//		case ITEM_ELEM: ui->checkButton(0); break;
			case ITEM_FACE: ui->checkButton(1); break;
			case ITEM_EDGE: ui->checkButton(2); break;
			case ITEM_NODE: ui->checkButton(3); break;
			}

			ui->meshToolButton[0]->setChecked(vs.m_bconn);
			ui->meshToolButton[1]->setChecked(vs.m_bselbrush);
			ui->meshToolButton[2]->setChecked(vs.m_bselpath);
			ui->maxAngle->setValue(vs.m_fconn);
			ui->cull->setChecked(!vs.m_bcullSel);
			ui->noint->setChecked(vs.m_bext);

			return;
		}

		GCurveMeshObject* pc = dynamic_cast<GCurveMeshObject*>(po);
		if (pc)
		{
			ui->showEditButtons(true, false, false);
			int item = pdoc->GetItemMode();
			if (item == ITEM_ELEM) { pdoc->SetItemMode(ITEM_MESH); item = ITEM_MESH; }
			switch (item)
			{
			case ITEM_MESH: ui->checkButton(-1); break;
	//		case ITEM_ELEM: ui->checkButton(0); break;
	//		case ITEM_FACE: ui->checkButton(1); break;
			case ITEM_EDGE: ui->checkButton(2); break;
			case ITEM_NODE: ui->checkButton(3); break;
			}

			ui->meshToolButton[0]->setChecked(vs.m_bconn);
			ui->meshToolButton[1]->setChecked(vs.m_bselbrush);
			ui->meshToolButton[2]->setChecked(vs.m_bselpath);
			ui->maxAngle->setValue(vs.m_fconn);
			ui->cull->setChecked(!vs.m_bcullSel);
			ui->noint->setChecked(vs.m_bext);

			return;
		}
	}

	ui->showEditButtons(false);
}

void CGLControlBar::onPivotChanged()
{
	vec3d p = ui->getPivot();
	CGLView* view = ui->m_wnd->GetGLView();
	view->SetPivotPosition(p);
}

void CGLControlBar::onPivotClicked(bool b)
{
	CGLView* view = ui->m_wnd->GetGLView();
	view->SetPivotUserMode(b);
	Update();
}

void CGLControlBar::onSnapToGridClicked(bool b)
{
	GLViewSettings& view = ui->m_wnd->GetGLView()->GetViewSettings();
	view.m_snapToGrid = b;
}

void CGLControlBar::onSnapToNodeClicked(bool b)
{
	GLViewSettings& view = ui->m_wnd->GetGLView()->GetViewSettings();
	view.m_snapToNode = b;
}

void CGLControlBar::onToggleVisibleClicked(bool b)
{
	ui->m_wnd->on_actionToggleVisible_triggered();
}

void CGLControlBar::onHideSelection(bool b)
{
	ui->m_wnd->on_actionHideSelection_triggered();
}

void CGLControlBar::onShowAll(bool b)
{
	ui->m_wnd->on_actionUnhideAll_triggered();
}

void CGLControlBar::onSelectAndHide(bool b)
{
	GLViewSettings& vs = ui->m_wnd->GetGLView()->GetViewSettings();
	vs.m_selectAndHide = b;
}

void CGLControlBar::onZoomSelectClicked(bool b)
{
	ui->m_wnd->on_actionZoomSelect_triggered();
}

void CGLControlBar::onZoomAllClicked(bool b)
{
	ui->m_wnd->on_actionZoomExtents_triggered();
}

void CGLControlBar::onToggleMesh(bool b)
{
	ui->m_wnd->on_actionShowMeshLines_toggled(b);
}

void CGLControlBar::onToggleLight(bool b)
{
	ui->m_wnd->on_actionToggleLight_triggered();
}

void CGLControlBar::onMeshButtonClicked(int id)
{
	CGLDocument* pdoc = ui->m_wnd->GetGLDocument();
	if (pdoc == nullptr) return;

	for (int i = 0; i<4; ++i)
		if (id != i) ui->meshItemButton[i]->setChecked(false);

	if (ui->meshItemButton[id]->isChecked() == false) id = -1;

	int newMode = 0;
	switch (id)
	{
	case -1: newMode = ITEM_MESH; break;
	case 0: newMode = ITEM_ELEM; break;
	case 1: newMode = ITEM_FACE; break;
	case 2: newMode = ITEM_EDGE; break;
	case 3: newMode = ITEM_NODE; break;
	}

	pdoc->SetItemMode(newMode);

	ui->m_wnd->RedrawGL();
}

void CGLControlBar::onMeshToolClicked(int id)
{
	CGLView* glw = ui->m_wnd->GetGLView();
	if (glw == nullptr) return;

	for (int i = 0; i < 3; ++i)
		if (id != i) ui->meshToolButton[i]->setChecked(false);

	GLViewSettings& view = glw->GetViewSettings();

	if (ui->meshToolButton[id]->isChecked() == false) id = -1;

	view.m_bconn     = (id == 0);
	view.m_bselbrush = (id == 1);
	view.m_bselpath  = (id == 2);

	ui->m_wnd->RedrawGL();
}

void CGLControlBar::onMaxAngleChanged(double v)
{
	GLViewSettings& view = ui->m_wnd->GetGLView()->GetViewSettings();
	view.m_fconn = v;
}

void CGLControlBar::onSelectBackfacing(bool b)
{
	GLViewSettings& view = ui->m_wnd->GetGLView()->GetViewSettings();
	view.m_bcullSel = !b;
}

void CGLControlBar::onIgnoreInterior(bool b)
{
	GLViewSettings& view = ui->m_wnd->GetGLView()->GetViewSettings();
	view.m_bext = b;
}
