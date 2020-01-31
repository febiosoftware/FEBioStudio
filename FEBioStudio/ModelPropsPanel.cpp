#include "stdafx.h"
#include "ModelPropsPanel.h"
#include "PropertyListView.h"
#include "PropertyListForm.h"
#include "ToolBox.h"
#include "SelectionBox.h"
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QBoxLayout>
#include <QMessageBox>
#include <QFormLayout>
#include <QTabWidget>
#include "Document.h"
#include "MainWindow.h"
#include "ObjectProps.h"
#include <GeomLib/GPrimitive.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEMultiMaterial.h>
#include <QGridLayout>
#include <QComboBox>
#include <QCheckBox>
#include "CColorButton.h"
#include "MeshInfoPanel.h"
#include <GLWLib/convert.h>
#include <MeshTools/GGroup.h>
#include <CUILib/ImageViewer.h>
#include <CUILib/HistogramViewer.h>
#include <PostLib/ImageModel.h>
#include <PostGL/GLPlot.h>
#include "Commands.h"

//=============================================================================
CObjectPropsPanel::CObjectPropsPanel(QWidget* parent) : QWidget(parent)
{
	QGridLayout* l = new QGridLayout;

	l->addWidget(new QLabel("Name:"), 0, 0, Qt::AlignRight);
	l->addWidget(m_name = new QLineEdit, 0, 1);
	m_name->setObjectName("name");

	l->addWidget(m_col = new CColorButton, 0, 2);
	m_col->setObjectName("col");

	l->addWidget(new QLabel("Type:"), 1, 0, Qt::AlignRight);
	l->addWidget(m_type = new QLabel, 1, 1);

	l->addWidget(new QLabel("Active:"), 2, 0, Qt::AlignRight);
	l->addWidget(m_status = new QCheckBox, 2, 1);
	m_status->setObjectName("status");

	setLayout(l);

	QMetaObject::connectSlotsByName(this);
}

void CObjectPropsPanel::setName(const QString& name)
{
	m_name->setText(name);
}

void CObjectPropsPanel::setType(const QString& name)
{
	m_type->setText(name);
}

void CObjectPropsPanel::setColor(const QColor& col)
{
	m_col->setColor(col);
}

void CObjectPropsPanel::showColor(bool b)
{
	m_col->setVisible(b);
}

void CObjectPropsPanel::showStatus(bool b)
{
	m_status->setVisible(b);
}

void CObjectPropsPanel::setNameReadOnly(bool b)
{
	m_name->setReadOnly(b);
}

void CObjectPropsPanel::setStatus(bool b)
{
	m_status->setChecked(b);
}

void CObjectPropsPanel::on_name_textEdited(const QString& t)
{
	emit nameChanged(t);
}

void CObjectPropsPanel::on_col_colorChanged(QColor c)
{
	emit colorChanged(c);
}

void CObjectPropsPanel::on_status_clicked(bool b)
{
	emit statusChanged(b);
}

//=============================================================================
CBCObjectPropsPanel::CBCObjectPropsPanel(QWidget* parent) : QWidget(parent)
{
	QGridLayout* l = new QGridLayout;

	l->addWidget(new QLabel("Name:"), 0, 0);
	l->addWidget(m_name = new QLineEdit, 0, 1);
	m_name->setObjectName("name");

	l->addWidget(new QLabel("Type:"), 1, 0);
	l->addWidget(m_type = new QLabel, 1, 1);

	l->addWidget(new QLabel("Step:"), 2, 0);
	l->addWidget(m_list = new QComboBox, 2, 1);
	m_list->setObjectName("list");

	l->addWidget(new QLabel("Active:"), 3, 0, Qt::AlignRight);
	l->addWidget(m_state = new QCheckBox, 3, 1);
	m_state->setObjectName("state");

	setLayout(l);

	QMetaObject::connectSlotsByName(this);
}

void CBCObjectPropsPanel::setStepValues(const vector<pair<QString, int> >& l)
{
	m_list->clear();
	for (size_t i=0; i<l.size(); ++i)
	{
		const pair<QString,int>& item = l[i];
		m_list->addItem(item.first, item.second);
	}
}

void CBCObjectPropsPanel::setStepID(int n)
{
	int nitem = m_list->findData(n);
	m_list->setCurrentIndex(nitem);
}

int CBCObjectPropsPanel::currentStepID()
{
	return m_list->currentData().toInt();
}

void CBCObjectPropsPanel::setName(const QString& name)
{
	m_name->setText(name);
}

void CBCObjectPropsPanel::setType(const QString& name)
{
	m_type->setText(name);
}

void CBCObjectPropsPanel::on_name_textEdited(const QString& t)
{
	emit nameChanged(t);
}

void CBCObjectPropsPanel::on_list_currentIndexChanged(int n)
{
	emit stepChanged(n);
}

void CBCObjectPropsPanel::showActiveState(bool b)
{
	m_state->setVisible(b);
}

void CBCObjectPropsPanel::setActiveState(bool b)
{
	m_state->setChecked(b);
}

void CBCObjectPropsPanel::on_state_toggled(bool b)
{
	emit stateChanged(b);
}

//=============================================================================
class Ui::CModelPropsPanel
{
	enum {
		OBJECT_PANEL,
		BCOBJECT_PANEL,
		MESHINFO_PANEL,
		PROPS_PANEL,
		SELECTION1_PANEL,
		SELECTION2_PANEL,
	};

public:
	QStackedWidget*	stack;
	QStackedWidget*	propStack;
	::CSelectionBox* sel1;
	::CSelectionBox* sel2;
	::CPropertyListView* props;
	::CPropertyListForm* form;
	CToolBox* tool;
	CObjectPropsPanel*	obj;
	CBCObjectPropsPanel*	bcobj;
	CMeshInfoPanel*	mesh;
	QTabWidget* propsTab;

	CImageViewer*		imageView;
	CHistogramViewer*	histoView;

	bool		m_showImageTools;

public:
	void setupUi(QWidget* parent)
	{
		m_showImageTools = false;

		props = new ::CPropertyListView; props->setObjectName("props");
		form  = new ::CPropertyListForm; form->setObjectName("form");

		obj = new CObjectPropsPanel;
		obj->setObjectName("object");

		bcobj = new CBCObjectPropsPanel;
		bcobj->setObjectName("bcobject");

		propStack = new QStackedWidget;
		propStack->addWidget(props);
		propStack->addWidget(form);

		propsTab = new QTabWidget;
		propsTab->addTab(propStack, "Properties");

		sel1 = new ::CSelectionBox;
		sel1->setObjectName("select1");

		sel2 = new ::CSelectionBox;
		sel2->setObjectName("select2");

		mesh = new CMeshInfoPanel;

		imageView = new CImageViewer;
		histoView = new CHistogramViewer;

		// compose toolbox
		tool = new CToolBox;
		tool->addTool("Object", obj);
		tool->addTool("Object", bcobj);
		tool->addTool("Mesh Info", mesh);
		tool->addTool("Properties", propsTab);
		tool->addTool("Selection", sel1);
		tool->addTool("Selection", sel2);

		// hide all panels initially
//		tool->getToolItem(OBJECT_PANEL)->setVisible(false);
		tool->getToolItem(BCOBJECT_PANEL)->setVisible(false);
		tool->getToolItem(MESHINFO_PANEL)->setVisible(false);
//		tool->getToolItem(PROPS_PANEL)->setVisible(false);
		tool->getToolItem(SELECTION1_PANEL)->setVisible(false);
		tool->getToolItem(SELECTION2_PANEL)->setVisible(false);

		stack = new QStackedWidget;
		QLabel* label = new QLabel("");
		label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		stack->addWidget(label);
		stack->addWidget(tool);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->setMargin(0);
		mainLayout->addWidget(stack);
		parent->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(parent);
	}

	void showObjectInfo(bool b, bool showColor = false, bool editName = true, QColor col = QColor(0,0,0), bool showActive = false, bool isActive = false) 
	{ 
		obj->showColor(showColor);
		obj->showStatus(showActive);
		if (showActive) obj->setStatus(isActive);
		if (showColor) obj->setColor(col);
		tool->getToolItem(OBJECT_PANEL)->setVisible(b); 
		obj->setNameReadOnly(!editName);
	}

	void showBCObjectInfo(bool b, bool showActiveState = false, bool isActive = false)
	{
		tool->getToolItem(BCOBJECT_PANEL)->setVisible(b);
		if (showActiveState)
			bcobj->setActiveState(isActive);
	}

	void showPropsPanel(bool b) { tool->getToolItem(PROPS_PANEL)->setVisible(b); }
	void showSelectionPanel1(bool b) { tool->getToolItem(SELECTION1_PANEL)->setVisible(b); }
	void showSelectionPanel2(bool b) { tool->getToolItem(SELECTION2_PANEL)->setVisible(b); }

	void setSelection1Title(const QString& t) { tool->getToolItem(SELECTION1_PANEL)->setTitle(t); }
	void setSelection2Title(const QString& t) { tool->getToolItem(SELECTION2_PANEL)->setTitle(t); }

	void setName(const QString& txt) { obj->setName(txt); }
	void setType(const QString& txt) { obj->setType(txt); }

	void setBCName(const QString& txt) { bcobj->setName(txt); }
	void setBCType(const QString& txt) { bcobj->setType(txt); }

	void setPropertyList(CPropertyList* pl)
	{
		propStack->setCurrentIndex(0);
		props->Update(pl);
		form->setPropertyList(0);
	}

	void setPropertyForm(CPropertyList* pl)
	{
		propStack->setCurrentIndex(1);
		props->Update(0);
		form->setPropertyList(pl);
	}

	void showImageTools(bool b, Post::CImageModel* img = nullptr)
	{
		if (b && (m_showImageTools==false))
		{
			m_showImageTools = true;

			imageView->SetImageModel(img);
			histoView->SetImageModel(img);

			propsTab->addTab(imageView, "Image Viewer");
			propsTab->addTab(histoView, "Histogram");
		}
		else if ((b == false) && m_showImageTools)
		{
			m_showImageTools = false;

			imageView->SetImageModel(nullptr);
			histoView->SetImageModel(nullptr);

			propsTab->removeTab(2);
			propsTab->removeTab(1);
		}
	}

	void showProperties(bool b)
	{
		if (b == false)
		{
			stack->setCurrentIndex(0);
			setPropertyList(0);
			showImageTools(false);
		}
		else
		{
			stack->setCurrentIndex(1);
		}
	}

	::CSelectionBox* selectionPanel(int n)
	{
		return (n==0?sel1 : sel2);
	}

	void setStepList(vector<pair<QString, int> >& l)
	{
		bcobj->setStepValues(l);
	}

	void setCurrentStepID(int n)
	{
		bcobj->setStepID(n);
	}

	int current_bcobject_value()
	{
		return bcobj->currentStepID();
	}

	void showMeshPanel(bool b)
	{
		tool->getToolItem(MESHINFO_PANEL)->setVisible(b);
	}

	void setObject(GObject* po)
	{
		mesh->setInfo(po);
	}
};

//=============================================================================
CModelPropsPanel::CModelPropsPanel(CMainWindow* wnd, QWidget* parent) : QWidget(parent), m_wnd(wnd), ui(new Ui::CModelPropsPanel)
{
	m_currentObject = 0;
	m_isUpdating = false;
	ui->setupUi(this);
}

void CModelPropsPanel::Update()
{
	// rebuild the step list
	FEModel* fem = m_wnd->GetDocument()->GetFEModel();
	int N = fem->Steps();
	vector<pair<QString,int> > steps(N);
	for (int i=0; i<N; ++i)
	{
		FEStep* step = fem->GetStep(i);
		steps[i].first = QString::fromStdString(step->GetName());
		steps[i].second = step->GetID();
	}

	m_isUpdating = true;
	ui->setStepList(steps);
	m_isUpdating = false;
}

void CModelPropsPanel::Refresh()
{
	if (m_currentObject)
	{
		m_currentObject->UpdateData(false);
		ui->props->Refresh();
	}
}

void CModelPropsPanel::SetObjectProps(FSObject* po, CPropertyList* props, int flags)
{
	if ((po == 0) && (props == 0))
	{
		ui->showProperties(false);
		m_currentObject = 0;
	}
	else
	{
		Post::CImageModel* img = dynamic_cast<Post::CImageModel*>(po);
		ui->showImageTools(img != nullptr, img);

		ui->showProperties(true);
		m_currentObject = po;
		SetSelection(0, 0);
		SetSelection(1, 0);

		ui->showBCObjectInfo(false);

		if (dynamic_cast<GObject*>(m_currentObject))
			ui->showMeshPanel(true);
		else
			ui->showMeshPanel(false);

		if (dynamic_cast<FEMaterial*>(m_currentObject))
		{
			// don't show the object info pane
			ui->showObjectInfo(false);
		}
		else
		{
			// set the object's name
			if (m_currentObject)
			{
				QString name = QString::fromStdString(m_currentObject->GetName());
				ui->setName(name);

				std::string stype = CDocument::GetTypeString(m_currentObject);
				QString type(stype.c_str());
				ui->setType(type);

				bool nameEditable = !(flags & 0x08);

				// show the color if it's a material or an object
				// TODO: maybe encode that in the flag?
				if (dynamic_cast<GObject*>(po))
				{
					GObject* go = dynamic_cast<GObject*>(po);
					ui->showObjectInfo(true, true, nameEditable, toQColor(go->GetColor()));
					ui->showMeshPanel(true);
					ui->setObject(go);
				}
				else if (dynamic_cast<GMaterial*>(po))
				{
					GMaterial* mo = dynamic_cast<GMaterial*>(po);
					ui->showObjectInfo(true, true, nameEditable, toQColor(mo->Diffuse()));
				}
				else if (dynamic_cast<GDiscreteElementSet*>(po))
				{
					GDiscreteElementSet* pd = dynamic_cast<GDiscreteElementSet*>(po);
					ui->showObjectInfo(true, true, nameEditable, toQColor(pd->GetColor()));
				}
				else if (dynamic_cast<FEStepComponent*>(po))
				{
					FEStepComponent* pc = dynamic_cast<FEStepComponent*>(po);

					ui->showObjectInfo(false, false, nameEditable);

					ui->setBCName(name);
					ui->setBCType(type);
					ui->setCurrentStepID(pc->GetStep());
					ui->showBCObjectInfo(true, true, pc->IsActive());
				}
				else if (dynamic_cast<Post::CGLObject*>(po))
				{
					Post::CGLObject* plot = dynamic_cast<Post::CGLObject*>(po);
					ui->showObjectInfo(true, false, nameEditable, QColor(0, 0, 0), true, plot->IsActive());
				}
				else ui->showObjectInfo(true, false, nameEditable);
			}
			else ui->showObjectInfo(false);
		}

		// show the property list
		if (props)
		{
			if (flags & 1)
				ui->setPropertyForm(props);
			else
				ui->setPropertyList(props);

			ui->showPropsPanel(true);
		}
		else ui->showPropsPanel(false);

		ui->showSelectionPanel1(true); ui->setSelection1Title("Selection");
		ui->showSelectionPanel2(false);
		FEBoundaryCondition* pbc = dynamic_cast<FEBoundaryCondition*>(m_currentObject);
		if (pbc) { SetSelection(0, pbc->GetItemList()); return; }

		FELoad* pbl = dynamic_cast<FELoad*>(m_currentObject);
		if (pbl) { SetSelection(0, pbl->GetItemList()); return; }

		FESoloInterface* solo = dynamic_cast<FESoloInterface*>(m_currentObject);
		if (solo) { SetSelection(0, solo->GetItemList()); return; }

		GMaterial* mat = dynamic_cast<GMaterial*>(m_currentObject);
		if (mat) { SetSelection(mat); return;	}

		FEItemListBuilder* pl = dynamic_cast<FEItemListBuilder*>(m_currentObject);
		if (pl) { SetSelection(0, pl); return; }

		FEPairedInterface* pi = dynamic_cast<FEPairedInterface*>(m_currentObject);
		if (pi)
		{
			ui->setSelection1Title("Master");
			ui->setSelection2Title("Slave");
			ui->showSelectionPanel2(true);
			SetSelection(0, pi->GetMasterSurfaceList());
			SetSelection(1, pi->GetSlaveSurfaceList());
			return;
		}

		ui->showSelectionPanel1(false);
		ui->showSelectionPanel2(false);
	}
}

void CModelPropsPanel::SetSelection(int n, FEItemListBuilder* item)
{
	CSelectionBox* sel = ui->selectionPanel(n);

	if (item == 0)
	{
		sel->setName("");
		sel->setType("");
		sel->clearData();
		return;
	}

	// set the name
	QString name = QString::fromStdString(item->GetName());
	sel->showNameType(true);
	sel->setName(name);

	sel->clearData();

	// set the type
	QString type("(unknown)");
	switch (item->Type())
	{
	case GO_PART: 
		{	
			sel->setType("Domains");
			GPartList& g = dynamic_cast<GPartList&>(*item);
			vector<GPart*> parts = g.GetPartList();
			for (int i=0; i<parts.size(); ++i)
			{
				GPart* pg = parts[i];
				if (pg) sel->addData(QString::fromStdString(pg->GetName()), pg->GetID());
				else sel->addData(QString("[invalid reference]"), -1, 1);
			}
		}
		break;
	case GO_FACE:
		{	
			sel->setType("Surfaces");
			GFaceList& g = dynamic_cast<GFaceList&>(*item);
			vector<GFace*> surfs = g.GetFaceList();
			for (int i=0; i<surfs.size(); ++i)
			{
				GFace* pg = surfs[i];
				if (pg) sel->addData(QString::fromStdString(pg->GetName()), pg->GetID());
				else sel->addData(QString("[invalid reference]"), -1, 1);
			}
		}
		break;
	case GO_EDGE: 
		{	
			sel->setType("Curves");
			GEdgeList& g = dynamic_cast<GEdgeList&>(*item);
			vector<GEdge*> edges = g.GetEdgeList();
			for (int i=0; i<edges.size(); ++i)
			{
				GEdge* pg = edges[i];
				if (pg) sel->addData(QString::fromStdString(pg->GetName()), pg->GetID());
				else sel->addData(QString("[invalid reference]"), -1, 1);
			}
		}
		break;
	case GO_NODE:
		{	
			sel->setType("Nodes");
			GNodeList& g = dynamic_cast<GNodeList&>(*item);
			vector<GNode*> nodes = g.GetNodeList();
			for (int i=0; i<nodes.size(); ++i)
			{
				GNode* pg = nodes[i];
				if (pg) sel->addData(QString::fromStdString(pg->GetName()), pg->GetID());
				else sel->addData(QString("[invalid reference]"), -1, 1);
			}
		}
		break;
	default:
		switch (item->Type())
		{
		case FE_PART   : type = "Elements"; break;
		case FE_SURFACE: type = "Facets"; break;
		case FE_EDGESET: type = "Edges"; break;
		case FE_NODESET: type = "Nodes"; break;
		default:
			assert(false);
		}
		sel->setType(type);

		// set the data
		vector<int> items;
		items.insert(items.end(), item->begin(), item->end());

//		sort(items.begin(), items.end());
//		unique(items.begin(), items.end());

		for (int i=0; i<(int)items.size();++i) sel->addData(QString::number(items[i]), items[i]);
	}
}

void CModelPropsPanel::SetSelection(GMaterial* pmat)
{
	// get the document
	CDocument* pdoc = m_wnd->GetDocument();
	FEModel& fem = *pdoc->GetFEModel();
	GModel& mdl = fem.GetModel();

	// clear the name
	::CSelectionBox* sel = ui->selectionPanel(0);
	sel->showNameType(false);

	// set the type
	sel->setType("Domains");

	// set the items
	sel->clearData();
	int N = mdl.Parts();
	for (int i = 0; i<mdl.Parts(); ++i)
	{
		GPart* pg = mdl.Part(i);
		GMaterial* pgm = fem.GetMaterialFromID(pg->GetMaterialID());
		if (pgm && (pgm->GetID() == pmat->GetID()))
		{
			int n = pg->GetID();
			sel->addData(QString::fromStdString(pg->GetName()), n);
		}
	}
}

void CModelPropsPanel::on_select1_addButtonClicked() { addSelection(0); }
void CModelPropsPanel::on_select2_addButtonClicked() { addSelection(1); }

void CModelPropsPanel::addSelection(int n)
{
	// get the document
	CDocument* pdoc = m_wnd->GetDocument();

	// get the current selection
	FESelection* ps = pdoc->GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return;

	assert(m_currentObject);
	if (m_currentObject == 0) return;

	FEModelComponent* pmc = dynamic_cast<FEModelComponent*>(m_currentObject);
	if (pmc)
	{
		// don't allow object selections 
		if (dynamic_cast<GObjectSelection*>(ps)) 
		{
			QMessageBox::critical(this, "FEBio Studio", "You cannot apply an object to a boundary condition's selection.");
			return;
		}

		// for body loads, only allow part selections
		if (dynamic_cast<FEBodyLoad*>(pmc) && (dynamic_cast<GPartSelection*>(ps) == 0))
		{
			QMessageBox::critical(this, "FEBio Studio", "You cannot apply this selection to a body load.");
			return;
		}

		// don't allow part selections, except for initial conditions
		//		if (dynamic_cast<GPartSelection*>(ps) && (dynamic_cast<FEInitialCondition*>(m_pbc)==0)) return;

		// only allow surface selections for surface loads
		if (dynamic_cast<FESurfaceLoad*>(pmc) && (dynamic_cast<GFaceSelection*>(ps) == 0) && (dynamic_cast<FEFaceSelection*>(ps) == 0))
		{
			QMessageBox::critical(this, "FEBio Studio", "You cannot apply this selection to a surface load.");
			return;
		}

		FEItemListBuilder* pl = pmc->GetItemList();
		if (pl == 0)
		{
			pdoc->DoCommand(new CCmdSetModelComponentItemList(pmc, ps->CreateItemList()));
			SetSelection(0, pmc->GetItemList());
		}
		else
		{
			// create the item list builder
			FEItemListBuilder* pg = ps->CreateItemList();

			// merge with the current list
			if (pg->Type() != pl->Type())
			{
				QMessageBox::critical(this, "FEBio Studio", "The selection is not of the correct type.");
			}
			else
			{
				list<int> l = pg->CopyItems();
				pdoc->DoCommand(new CCmdAddToItemListBuilder(pl, l));
			}
			SetSelection(0, pmc->GetItemList());
			delete pg;
		}

		emit selectionChanged();

		return;
	}

	FEPairedInterface* pi = dynamic_cast<FEPairedInterface*>(m_currentObject);
	if (pi)
	{
		if (dynamic_cast<GObjectSelection*>(ps) ||
		dynamic_cast<GPartSelection*>(ps)) return;

		FEItemListBuilder* pg = ps->CreateItemList();

		FEItemListBuilder* pl = (n==0? pi->GetMasterSurfaceList() : pi->GetSlaveSurfaceList());
		if (pl == 0)
		{
			if (n == 0) pi->SetMaster(pg);
			else pi->SetSlave(pg);
			SetSelection(n, pg);
		}
		else
		{
			// merge with the current list
			if (pg->Type() != pl->Type())
			{
				QMessageBox::critical(this, "FEBio Studio", "The selection is not of the correct type.");
			}
			else
			{
				list<int> l = pg->CopyItems();
				pdoc->DoCommand(new CCmdAddToItemListBuilder(pl, l));
			}
			SetSelection(n, pl);
			delete pg;
		}

		emit selectionChanged();
		return;
	}

	FESoloInterface* psolo = dynamic_cast<FESoloInterface*>(m_currentObject);
	if (psolo)
	{
		if (dynamic_cast<GObjectSelection*>(ps) ||
			dynamic_cast<GPartSelection*>(ps)) return;

		FEItemListBuilder* pl = psolo->GetItemList();
		if (pl == 0)
		{
			FEItemListBuilder* pg = ps->CreateItemList();
			psolo->SetItemList(pg);
			SetSelection(0, psolo->GetItemList());
		}
		else
		{
			// create the item list builder
			FEItemListBuilder* pg = ps->CreateItemList();

			// merge with the current list
			if (pg->Type() != pl->Type())
			{
				QMessageBox::critical(this, "FEBio Studio", "The selection is not of the correct type.");
			}
			else
			{
				list<int> l = pg->CopyItems();
				pdoc->DoCommand(new CCmdAddToItemListBuilder(pl, l));
			}
			SetSelection(0, psolo->GetItemList());
			delete pg;
		}

		emit selectionChanged();
		return;
	}

	GMaterial* pmat = dynamic_cast<GMaterial*>(m_currentObject);
	if (pmat)
	{
		if (dynamic_cast<GObjectSelection*>(ps))
		{
			GObjectSelection* pos = dynamic_cast<GObjectSelection*>(ps);
			int N = pos->Count();
			vector<GObject*> o(N);
			for (int i = 0; i<N; ++i) o[i] = pos->Object(i);
			pdoc->DoCommand(new CCmdAssignObjectListMaterial(o, pmat->GetID()));
		}
		else if (dynamic_cast<GPartSelection*>(ps))
		{
			GPartSelection* pps = dynamic_cast<GPartSelection*>(ps);
			int N = pps->Count();
			vector<int> p(N);
			GPartSelection::Iterator it(pps);
			for (int i = 0; i<N; ++i, ++it) p[i] = it->GetID();
			pdoc->DoCommand(new CCmdAssignPartMaterial(pdoc->GetFEModel(), p, pmat->GetID()));
		}
		else
		{
			QMessageBox::critical(this, "FEBio Studio", "You cannot assign a material to this selection.");
		}
		SetSelection(pmat);
		m_wnd->RedrawGL();

		emit selectionChanged();
		return;
	}

	FEItemListBuilder* pl = dynamic_cast<FEItemListBuilder*>(m_currentObject);
	if (pl)
	{
		// create the item list builder
		FEItemListBuilder* pg = ps->CreateItemList();

		// merge with the current list
		if (pg->Type() != pl->Type())
		{
			QMessageBox::critical(this, "FEBio Studio", "The selection is not of the correct type.");
		}
		else
		{
			list<int> l = pg->CopyItems();
			pdoc->DoCommand(new CCmdAddToItemListBuilder(pl, l));
		}
		SetSelection(0, pl);

		// don't forget to clean up
		delete pg;

		emit selectionChanged();
		return;
	}
}

void CModelPropsPanel::on_select1_subButtonClicked() { subSelection(0); }
void CModelPropsPanel::on_select2_subButtonClicked() { subSelection(1); }

void CModelPropsPanel::subSelection(int n)
{
	// get the document
	CDocument* pdoc = m_wnd->GetDocument();

	// get the current selection
	FESelection* ps = pdoc->GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return;

	FEBoundaryCondition* pbc = dynamic_cast<FEBoundaryCondition*>(m_currentObject);
	if (pbc)
	{
		// don't allow object selections 
		if (dynamic_cast<GObjectSelection*>(ps)) return;

		// don't allow part selections, except for initial conditions
		if (dynamic_cast<GPartSelection*>(ps) && (dynamic_cast<FEInitialCondition*>(pbc) == 0)) return;

		FEItemListBuilder* pl = pbc->GetItemList();
		if (pl)
		{
			// create the item list builder
			FEItemListBuilder* pg = ps->CreateItemList();

			// subtract from the current list
			if (pg->Type() == pl->Type())
			{
				list<int> l = pg->CopyItems();
				pdoc->DoCommand(new CCmdRemoveFromItemListBuilder(pl, l));
			}

			SetSelection(0, pbc->GetItemList());
			delete pg;
			emit selectionChanged();
		}
		return;
	}

	FEPairedInterface* pi = dynamic_cast<FEPairedInterface*>(m_currentObject);
	if (pi)
	{
		if (dynamic_cast<GObjectSelection*>(ps) ||
		dynamic_cast<GPartSelection*>(ps)) return;

		FEItemListBuilder* pl = (n==0? pi->GetMasterSurfaceList() : pi->GetSlaveSurfaceList());

		if (pl)
		{
			// create the item list builder
			FEItemListBuilder* pg = ps->CreateItemList();

			// subtract from the current list
			if (pg->Type() == pl->Type())
			{
				list<int> l = pg->CopyItems();
				pdoc->DoCommand(new CCmdRemoveFromItemListBuilder(pl, l));
			}

			SetSelection(n, pl);

			delete pg;

			emit selectionChanged();
			return;
		}
	}

	FESoloInterface* psi = dynamic_cast<FESoloInterface*>(m_currentObject);
	if (psi)
	{
		if (dynamic_cast<GObjectSelection*>(ps) ||
			dynamic_cast<GPartSelection*>(ps)) return;

		FEItemListBuilder* pl = psi->GetItemList();
		if (pl)
		{
			// create the item list builder
			FEItemListBuilder* pg = ps->CreateItemList();

			// subtract from the current list
			if (pg->Type() == pl->Type())
			{
				list<int> l = pg->CopyItems();
				pdoc->DoCommand(new CCmdRemoveFromItemListBuilder(pl, l));
			}

			SetSelection(0, psi->GetItemList());

			delete pg;
			emit selectionChanged();
		}
		return;
	}

	GMaterial* pmat = dynamic_cast<GMaterial*>(m_currentObject);
	if (pmat)
	{
		if (dynamic_cast<GObjectSelection*>(ps))
		{
			GObjectSelection* pos = dynamic_cast<GObjectSelection*>(ps);
			int N = pos->Count();
			vector<GObject*> o(N);
			for (int i = 0; i<N; ++i) o[i] = pos->Object(i);
			pdoc->DoCommand(new CCmdAssignObjectListMaterial(o, 0));
		}
		else if (dynamic_cast<GPartSelection*>(ps))
		{
			GPartSelection* pps = dynamic_cast<GPartSelection*>(ps);
			int N = pps->Count();
			vector<int> p(N);
			GPartSelection::Iterator it(pps);
			for (int i = 0; i<N; ++i, ++it) p[i] = it->GetID();
			pdoc->DoCommand(new CCmdAssignPartMaterial(pdoc->GetFEModel(), p, 0));
		}
		else
		{
			QMessageBox::critical(this, "FEBio Studio", "You cannot assign a material to this selection.");
		}
		SetSelection(pmat);
		m_wnd->RedrawGL();
		emit selectionChanged();
		return;
	}

	FEItemListBuilder* pl = dynamic_cast<FEItemListBuilder*>(m_currentObject);
	if (pl)
	{
		// create the item list builder
		FEItemListBuilder* pg = ps->CreateItemList();

		if (pg->Type() == pl->Type())
		{
			list<int> l = pg->CopyItems();
			pdoc->DoCommand(new CCmdRemoveFromItemListBuilder(pl, l));
		}
		SetSelection(0, pl);

		delete pg;

		emit selectionChanged();
		return;
	}
}

void CModelPropsPanel::on_select1_delButtonClicked() { delSelection(0); }
void CModelPropsPanel::on_select2_delButtonClicked() { delSelection(1); }

void CModelPropsPanel::delSelection(int n)
{
	CDocument* pdoc = m_wnd->GetDocument();

	FEItemListBuilder* pl = 0;

	FEBoundaryCondition* pbc = dynamic_cast<FEBoundaryCondition*>(m_currentObject);
	if (pbc) pl = pbc->GetItemList();

	FESoloInterface* psi = dynamic_cast<FESoloInterface*>(m_currentObject);
	if (psi) pl = psi->GetItemList();

	FEPairedInterface* pi = dynamic_cast<FEPairedInterface*>(m_currentObject);
	if (pi) pl = (n == 0 ? pi->GetMasterSurfaceList() : pi->GetSlaveSurfaceList());

	CSelectionBox* sel = ui->selectionPanel(n);

	if (pl)
	{
		list<int> items;
		sel->getSelectedItems(items);
		pdoc->DoCommand(new CCmdRemoveFromItemListBuilder(pl, items));
		SetSelection(n, pl);
		emit selectionChanged();
	}
	else if (dynamic_cast<GMaterial*>(m_currentObject))
	{
		vector<int> items;
		sel->getSelectedItems(items);
		pdoc->DoCommand(new CCmdAssignPartMaterial(pdoc->GetFEModel(), items, 0));
		SetSelection(dynamic_cast<GMaterial*>(m_currentObject));
		m_wnd->RedrawGL();
		emit selectionChanged();
	}
	else if (dynamic_cast<FEItemListBuilder*>(m_currentObject))
	{
		pl = dynamic_cast<FEItemListBuilder*>(m_currentObject);
		list<int> items;
		sel->getSelectedItems(items);
		pdoc->DoCommand(new CCmdRemoveFromItemListBuilder(pl, items));
		SetSelection(n, pl);
		emit selectionChanged();
	}
}

void CModelPropsPanel::on_select1_selButtonClicked() { selSelection(0); }
void CModelPropsPanel::on_select2_selButtonClicked() { selSelection(1); }

void CModelPropsPanel::on_select1_nameChanged(const QString& t)
{
	FEItemListBuilder* pl = 0;

	FEBoundaryCondition* pbc = dynamic_cast<FEBoundaryCondition*>(m_currentObject);
	if (pbc) pl = pbc->GetItemList();

	FESoloInterface* psi = dynamic_cast<FESoloInterface*>(m_currentObject);
	if (psi) pl = psi->GetItemList();

	FEPairedInterface* pi = dynamic_cast<FEPairedInterface*>(m_currentObject);
	if (pi) pl = pi->GetMasterSurfaceList();

	if (pl == 0) return;

	string sname = t.toStdString();
	pl->SetName(sname);
}

void CModelPropsPanel::on_select2_nameChanged(const QString& t)
{
	FEItemListBuilder* pl = 0;

	FEBoundaryCondition* pbc = dynamic_cast<FEBoundaryCondition*>(m_currentObject);
	if (pbc) pl = pbc->GetItemList();

	FESoloInterface* psi = dynamic_cast<FESoloInterface*>(m_currentObject);
	if (psi) pl = psi->GetItemList();

	FEPairedInterface* pi = dynamic_cast<FEPairedInterface*>(m_currentObject);
	if (pi) pl = pi->GetSlaveSurfaceList();

	if (pl == 0) return;

	string sname = t.toStdString();
	pl->SetName(sname);
}

void CModelPropsPanel::selSelection(int n)
{
	CDocument* pdoc = m_wnd->GetDocument();
	FEModel* ps = pdoc->GetFEModel();

	assert(m_currentObject);
	if (m_currentObject == 0) return;

	CSelectionBox* sel = ui->selectionPanel(n);

	// get the selection list
	vector<int> l;
	sel->getSelectedItems(l);
	if (l.empty())
	{
		QMessageBox::information(this, "FEBio Studio", "Nothing to select");
		return;
	}

	// create the selection command
	FEItemListBuilder* pl = 0;

	FEBoundaryCondition* pbc = dynamic_cast<FEBoundaryCondition*>(m_currentObject);
	if (pbc) pl = pbc->GetItemList();

	FESoloInterface* psi = dynamic_cast<FESoloInterface*>(m_currentObject);
	if (psi) pl = psi->GetItemList();

	FEPairedInterface* pi = dynamic_cast<FEPairedInterface*>(m_currentObject);
	if (pi) pl = (n==0? pi->GetMasterSurfaceList() : pi->GetSlaveSurfaceList());

	GGroup* pg = dynamic_cast<GGroup*>(m_currentObject);
	if (pg) pl = pg;

	FEGroup* pf = dynamic_cast<FEGroup*>(m_currentObject);
	if (pf) pl = pf;

	CCommand* pcmd = 0;
	if (pl)
	{
		switch (pl->Type())
		{
		case GO_NODE: pcmd = new CCmdSelectNode(ps, &l[0], (int)l.size(), false); break;
		case GO_EDGE: pcmd = new CCmdSelectEdge(ps, &l[0], (int)l.size(), false); break;
		case GO_FACE: pcmd = new CCmdSelectSurface(ps, &l[0], (int)l.size(), false); break;
		case GO_PART: pcmd = new CCmdSelectPart(ps, &l[0], (int)l.size(), false); break;
		default:
			if (dynamic_cast<FEGroup*>(pl))
			{
				FEGroup* pg = dynamic_cast<FEGroup*>(pl);
				FEMesh* pm = dynamic_cast<FEMesh*>(pg->GetMesh());
				assert(pm);
				switch (pg->Type())
				{
				case FE_NODESET: pcmd = new CCmdSelectFENodes(pm, &l[0], (int)l.size(), false); break;
				case FE_EDGESET: pcmd = new CCmdSelectFEEdges(pm, &l[0], (int)l.size(), false); break;
				case FE_SURFACE: pcmd = new CCmdSelectFaces(pm, &l[0], (int)l.size(), false); break;
				case FE_PART: pcmd = new CCmdSelectElements(pm, &l[0], (int)l.size(), false); break;
				default:
					assert(false);
				}

				// make sure the parent object is selected
				GObject* po = pm->GetGObject();
				assert(po);
				if (po && !po->IsSelected())
				{
					CCmdGroup* pgc = new CCmdGroup("Select");
					pgc->AddCommand(new CCmdSelectObject(po, false));
					pgc->AddCommand(pcmd);
					pcmd = pgc;
				}
			}
		}
	}
	else if (dynamic_cast<GMaterial*>(m_currentObject))
	{
		pcmd = new CCmdSelectPart(ps, &l[0], (int)l.size(), false);
	}

	// execute command
	if (pcmd)
	{
		pdoc->DoCommand(pcmd);
		m_wnd->Update();
	}
}

void CModelPropsPanel::on_object_nameChanged(const QString& txt)
{
	if (m_currentObject)
	{
		std::string sname = txt.toStdString();
		m_currentObject->SetName(sname.c_str());

		emit nameChanged(txt);
	}
}

void CModelPropsPanel::on_bcobject_nameChanged(const QString& txt)
{
	if (m_currentObject)
	{
		std::string sname = txt.toStdString();
		m_currentObject->SetName(sname.c_str());

		emit nameChanged(txt);
	}
}

void CModelPropsPanel::on_object_colorChanged(const QColor& col)
{
	GObject* po = dynamic_cast<GObject*>(m_currentObject);
	if (po)
	{
		po->SetColor(toGLColor(col));
	}

	GMaterial* mo = dynamic_cast<GMaterial*>(m_currentObject);
	if (mo)
	{
		mo->AmbientDiffuse(toGLColor(col));
	}

	GDiscreteObject* pd = dynamic_cast<GDiscreteObject*>(m_currentObject);
	if (pd)
	{
		pd->SetColor(toGLColor(col));
	}

	m_wnd->RedrawGL();
}

void CModelPropsPanel::on_props_dataChanged(int n)
{
	Post::CGLObject* po = dynamic_cast<Post::CGLObject*>(m_currentObject);
	if (po) po->Update();
	m_wnd->RedrawGL();
}

void CModelPropsPanel::on_form_dataChanged(bool itemModified)
{
	m_wnd->RedrawGL();
	emit dataChanged(itemModified);
}

void CModelPropsPanel::on_bcobject_stepChanged(int n)
{
	if (m_isUpdating) return;

	FEStepComponent* pc = dynamic_cast<FEStepComponent*>(m_currentObject);
	if (pc == 0) return;

	n = ui->current_bcobject_value();
	if ((n!=-1) && (pc->GetStep() != n))
	{
		FEModel* fem = m_wnd->GetDocument()->GetFEModel();

		fem->AssignComponentToStep(pc, fem->GetStep(n));

		// Changing the step of a BC requires the whole model tree to be rebuild
		emit dataChanged(true);
	}
}

void CModelPropsPanel::on_bcobject_stateChanged(bool isActive)
{
	if (m_isUpdating) return;

	FEStepComponent* pc = dynamic_cast<FEStepComponent*>(m_currentObject);
	if (pc == 0) return;

	pc->Activate(isActive);

	emit dataChanged(false);
}

void CModelPropsPanel::on_object_statusChanged(bool b)
{
	if (m_isUpdating) return;

	Post::CGLObject* po = dynamic_cast<Post::CGLObject*>(m_currentObject);
	if (po == 0) return;

	po->Activate(b);

	emit dataChanged(false);
}
