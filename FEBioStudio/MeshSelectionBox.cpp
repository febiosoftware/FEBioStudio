#include "stdafx.h"
#include "MeshSelectionBox.h"
#include <QToolButton>
#include <QLineEdit>
#include <QButtonGroup>
#include <QGroupBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QValidator>
#include <QPushButton>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include "MainWindow.h"
#include "ModelDocument.h"
#include "Commands.h"

class Ui::CMeshSelectionBox
{
public:
	::CMainWindow*	wnd;
	QToolButton*	but[4];
	QLineEdit*		m_maxAngle;

	QCheckBox* maxAngleCheck;
	QLabel*			m_sel;

public:
	void setup(QWidget* w)
	{
		wnd = 0;
		QToolButton* selElem = but[0] = new QToolButton;
		selElem->setCheckable(true);
		selElem->setFixedSize(32, 32);
		selElem->setIconSize(selElem->size());
		selElem->setIcon(QIcon(":/icons/selElem.png")); selElem->setAutoRaise(true);
		selElem->setToolTip("<font color=\"black\">Select elements");

		QToolButton* selFace = but[1] = new QToolButton;
		selFace->setCheckable(true);
		selFace->setFixedSize(32, 32);
		selFace->setIconSize(selElem->size());
		selFace->setIcon(QIcon(":/icons/selFace.png")); selFace->setAutoRaise(true);
		selFace->setToolTip("<font color=\"black\">Select faces");

		QToolButton* selEdge = but[2] = new QToolButton;
		selEdge->setCheckable(true);
		selEdge->setFixedSize(32, 32);
		selEdge->setIconSize(selElem->size());
		selEdge->setIcon(QIcon(":/icons/selEdge.png")); selEdge->setAutoRaise(true);
		selEdge->setToolTip("<font color=\"black\">Select edges");

		QToolButton* selNode = but[3] = new QToolButton;
		selNode->setCheckable(true);
		selNode->setFixedSize(32, 32);
		selNode->setIconSize(selElem->size());
		selNode->setIcon(QIcon(":/icons/selNode.png")); selNode->setAutoRaise(true);
		selNode->setToolTip("<font color=\"black\">Select nodes");

		QButtonGroup* bg = new QButtonGroup(w);
		bg->setExclusive(false);
		bg->addButton(selElem, 0);
		bg->addButton(selFace, 1);
		bg->addButton(selEdge, 2);
		bg->addButton(selNode, 3);

		QGroupBox* gb = new QGroupBox("select connected");
		gb->setObjectName("selectConnected");
		gb->setCheckable(true);

		maxAngleCheck = new QCheckBox("Max angle");
		maxAngleCheck->setObjectName("maxAngleCheck");
		maxAngleCheck->setChecked(true);
		m_maxAngle = new QLineEdit;
		m_maxAngle->setObjectName("maxAngle");
		m_maxAngle->setText(QString::number(30.0));
		m_maxAngle->setValidator(new QDoubleValidator);
		QHBoxLayout* maxAngleLayout = new QHBoxLayout;
		maxAngleLayout->addWidget(maxAngleCheck);
		maxAngleLayout->addWidget(m_maxAngle);

		QCheckBox* part = new QCheckBox("Respect Partitions");
		part->setObjectName("respectPartitions");

		QCheckBox* path = new QCheckBox("Connect by shortest path");
		path->setObjectName("shortestPath");

		QPushButton* pb = new QPushButton("Selection Tools");
		QMenu* menu = new QMenu(w);
		QAction* action = menu->addAction("Hide Selection"); action->setObjectName("hideSelection");
		action = menu->addAction("Unhide All"); action->setObjectName("unhideAll");
		action = menu->addAction("Grow Selection"); action->setObjectName("growSelection");
		action = menu->addAction("Shrink Selection"); action->setObjectName("shrinkSelection");
		action = menu->addAction("Select by ID ..."); action->setObjectName("selectByID");
		action = menu->addAction("Select by value ..."); action->setObjectName("selectByValue");
		pb->setMenu(menu);

		m_sel = new QLabel(" ");

		QVBoxLayout* selectLayout = new QVBoxLayout;
		selectLayout->addLayout(maxAngleLayout);
		selectLayout->addWidget(part);
		selectLayout->addWidget(path);
		gb->setLayout(selectLayout);


		QHBoxLayout* buttonLayout = new QHBoxLayout;
		buttonLayout->addWidget(selElem);
		buttonLayout->addWidget(selFace);
		buttonLayout->addWidget(selEdge);
		buttonLayout->addWidget(selNode);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(buttonLayout);
		mainLayout->addWidget(gb);
		QCheckBox* pc = 0;
		mainLayout->addWidget(pc = new QCheckBox("Ignore Interior")); pc->setObjectName("ignoreInterior");
		mainLayout->addWidget(pc = new QCheckBox("Select and hide")); pc->setObjectName("selectAndHide");
		mainLayout->addWidget(pc = new QCheckBox("Ignore backfacing")); pc->setObjectName("ignoreBackfacing"); pc->setChecked(true);
		mainLayout->addWidget(pb);
		mainLayout->addWidget(m_sel);

		w->setLayout(mainLayout);

		QObject::connect(bg, SIGNAL(buttonClicked(int)), w, SLOT(onButtonClicked(int)));
		QMetaObject::connectSlotsByName(w);
	}

	void checkButton(int id)
	{
		for (int i=0; i<4; ++i) but[i]->setChecked(false);
		if (id != -1) but[id]->setChecked(true);
	}

	void checkMaxAngle(bool b)
	{
		maxAngleCheck->setChecked(b);
	}

	int checkedButton()
	{
		if (but[0]->isChecked()) return 0;
		if (but[1]->isChecked()) return 1;
		if (but[2]->isChecked()) return 2;
		if (but[3]->isChecked()) return 3;
		return -1;
	}
};

//=============================================================================
// CMeshSelectionBox
//=============================================================================

CMeshSelectionBox::CMeshSelectionBox(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new Ui::CMeshSelectionBox)
{
	ui->setup(this);
	ui->wnd = wnd;
}

void CMeshSelectionBox::setSelection(int n)
{
	if (n == 0)
	{
		ui->m_sel->setText("");
		return;
	}

	int nb = ui->checkedButton();
	switch (nb)
	{
	case 0: ui->m_sel->setText(QString("%1 elements selected").arg(n)); break;
	case 1: ui->m_sel->setText(QString("%1 faces selected").arg(n)); break;
	case 2: ui->m_sel->setText(QString("%1 edges selected").arg(n)); break;
	case 3: ui->m_sel->setText(QString("%1 nodes selected").arg(n)); break;
	default:
		ui->m_sel->setText("");
	}
}

void CMeshSelectionBox::setItemMode(int mode)
{
	switch (mode)
	{
	case ITEM_MESH: onButtonClicked(-1); break;
	case ITEM_ELEM: onButtonClicked(0); break;
	case ITEM_FACE: onButtonClicked(1); break;
	case ITEM_EDGE: onButtonClicked(2); break;
	case ITEM_NODE: onButtonClicked(3); break;
	}
}

void CMeshSelectionBox::onButtonClicked(int id)
{
	for (int i = 0; i<4; ++i)
		if (id != i) ui->but[i]->setChecked(false);

	if (ui->but[id]->isChecked() == false) id = -1;

	int newMode = 0;
	switch (id)
	{
	case -1: newMode = ITEM_MESH; break;
	case 0: newMode = ITEM_ELEM; break;
	case 1: newMode = ITEM_FACE; break;
	case 2: newMode = ITEM_EDGE; break;
	case 3: newMode = ITEM_NODE; break;
	}

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(ui->wnd->GetDocument());
	pdoc->SetItemMode(newMode);

	FESelection* sel = pdoc->GetCurrentSelection();
	if ((sel == 0) || (sel->Size() == 0)) setSelection(0);
	else setSelection(sel->Size());

	emit itemModeChanged(newMode);
}

void CMeshSelectionBox::showEvent(QShowEvent* ev)
{
	CDocument* pdoc = ui->wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	checkMaxAngle(view.m_bmax);
}

void CMeshSelectionBox::hideEvent(QHideEvent* ev)
{
	for (int i = 0; i<4; ++i)
		ui->but[i]->setChecked(false);

	CDocument* pdoc = ui->wnd->GetDocument();
	pdoc->SetItemMode(ITEM_MESH);
	ui->wnd->RedrawGL();
}

void CMeshSelectionBox::setSurfaceSelectionMode(bool b)
{
	ui->but[0]->setDisabled(b);
}

double CMeshSelectionBox::maxAngle()
{
	return ui->m_maxAngle->text().toDouble();
}

void CMeshSelectionBox::setMaxAngle(double w)
{
	ui->m_maxAngle->setText(QString::number(w));
}

void CMeshSelectionBox::checkMaxAngle(bool b)
{
	ui->checkMaxAngle(b);
}

void CMeshSelectionBox::on_selectConnected_clicked(bool b)
{
	CDocument* pdoc = ui->wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_bconn = b;
}

void CMeshSelectionBox::on_maxAngleCheck_toggled(bool b)
{
	CDocument* pdoc = ui->wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_bmax = b;
}

void CMeshSelectionBox::on_maxAngle_editingFinished()
{
	CDocument* pdoc = ui->wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_fconn = maxAngle();
}

void CMeshSelectionBox::on_respectPartitions_toggled(bool b)
{
	CDocument* pdoc = ui->wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_bpart = b;
}

void CMeshSelectionBox::on_shortestPath_toggled(bool b)
{
	CDocument* pdoc = ui->wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_bselpath = b;
}

void CMeshSelectionBox::on_ignoreInterior_toggled(bool b)
{
	CDocument* pdoc = ui->wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_bext = b;
}

void CMeshSelectionBox::on_ignoreBackfacing_toggled(bool b)
{
	CDocument* pdoc = ui->wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_bcullSel = b;
}

void CMeshSelectionBox::on_selectAndHide_toggled(bool b)
{
	CDocument* pdoc = ui->wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_bhide = b;
}

void CMeshSelectionBox::on_hideSelection_triggered(bool b)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(ui->wnd->GetDocument());
	if (pdoc == nullptr) return;

	FESelection* ps = pdoc->GetCurrentSelection();
	if (ps && ps->Size())
	{
		pdoc->DoCommand(new CCmdHideSelection(pdoc));
		ui->wnd->Update(0, true);
	}
}

void CMeshSelectionBox::on_unhideAll_triggered(bool b)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(ui->wnd->GetDocument());
	if (pdoc == nullptr) return;

	FEModel* ps = pdoc->GetFEModel();
	if (!ps) return;

	CCmdUnhideAll* pcmd = new CCmdUnhideAll(pdoc);
	pdoc->DoCommand(pcmd);

	ui->wnd->Update(0, true);
}

void CMeshSelectionBox::on_growSelection_triggered(bool b)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(ui->wnd->GetDocument());
	if (pdoc == nullptr) return;

	GObject* po = pdoc->GetActiveObject(); assert(po);
	FEMeshBase* pm = po->GetEditableMesh();
	if (pm == 0) return;

	int item = pdoc->GetItemMode();
	switch (item)
	{
	case ITEM_NODE: pdoc->GrowNodeSelection(pm); break;
	case ITEM_FACE: pdoc->GrowFaceSelection(pm); break;
	case ITEM_EDGE: pdoc->GrowEdgeSelection(pm); break;
	case ITEM_ELEM: pdoc->GrowElementSelection(dynamic_cast<FEMesh*>(pm)); break;
	}

	ui->wnd->Update(0, true);
}

void CMeshSelectionBox::on_shrinkSelection_triggered(bool b)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(ui->wnd->GetDocument());

	GObject* po = pdoc->GetActiveObject();
	assert(po);
	FEMeshBase* pm = po->GetEditableMesh();
	if (pm == 0) return;

	int item = pdoc->GetItemMode();
	switch (item)
	{
	case ITEM_NODE: pdoc->ShrinkNodeSelection(pm); break;
	case ITEM_FACE: pdoc->ShrinkFaceSelection(pm); break;
	case ITEM_EDGE: pdoc->ShrinkEdgeSelection(pm); break;
	case ITEM_ELEM: 
		pdoc->ShrinkElementSelection(dynamic_cast<FEMesh*>(pm)); 
		break;
	}

	ui->wnd->Update(0, true);
}

void CMeshSelectionBox::on_selectByID_triggered(bool b)
{
	CDocument* pdoc = ui->wnd->GetDocument();
	int item = pdoc->GetItemMode();
	if (item == ITEM_MESH)
	{
		QMessageBox::information(ui->wnd, "Select By ID", "You must select a sub-mesh mode before you select items by ID.");
		return;
	}

	GObject* po = pdoc->GetActiveObject(); assert(po);
	FEMesh* pm = po->GetFEMesh(); assert(pm);

	QInputDialog dlg(ui->wnd);
	bool bok;
	int nid = dlg.getInt(ui->wnd, "Select By ID", "ID", 1, 1, 10000000, 1, &bok);
	if (bok)
	{
		switch (item)
		{
		case ITEM_NODE: pdoc->DoCommand(new CCmdSelectFENodes(pm, &nid, 1, true)); break;
		case ITEM_FACE: pdoc->DoCommand(new CCmdSelectFaces(pm, &nid, 1, true)); break;
		case ITEM_EDGE: pdoc->DoCommand(new CCmdSelectFEEdges(pm, &nid, 1, true)); break;
		case ITEM_ELEM: pdoc->DoCommand(new CCmdSelectElements(pm, &nid, 1, true)); break;
		default:
			assert(false);
		}

		ui->wnd->Update(0, true);
	}
}

void CMeshSelectionBox::on_selectByValue_triggered(bool b)
{
	CDocument* pdoc = ui->wnd->GetDocument();
	int item = pdoc->GetItemMode();
	if (item != ITEM_ELEM)
	{
		QMessageBox::information(ui->wnd, "Select By Value", "This feature only works for element selections.");
		return;
	}

	GObject* po = pdoc->GetActiveObject(); assert(po);
	FEMesh* pm = po->GetFEMesh(); assert(pm);

	bool bok = true;
	double val = QInputDialog::getDouble(ui->wnd, "Select By Value", "Value:", 0.0, -1e99, 1e99, 7, &bok);
	if (bok)
	{
		/*		vector<int> id;
		double v0 = dlg.m_min;
		double v1 = dlg.m_max;
		for (int i = 0; i<pm->Elements(); ++i)
		{
		if (pm->GetElementDataTag(i) > 0)
		{
		double v = pm->GetElementValue(i);
		if ((v >= v0) && (v <= v1)) id.push_back(i);
		}
		}

		if (!id.empty())
		{
		pdoc->DoCommand(new CCmdSelectElements(pm, id, true));
		}
		*/
		ui->wnd->Update(0, true);
	}
}
