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
#include "DlgEditOutput.h"
#include <FEMLib/FSProject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FEMultiMaterial.h>
#include <GeomLib/GGroup.h>
#include <GeomLib/FSGroup.h>
#include <GeomLib/GObject.h>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QCompleter>
#include "CustomLineEdit.h"
#include <FEBioLink/FEBioClass.h>

class Ui::CDlgAddSelection
{
public:
	QLabel*	name;
	QLabel* type;
	QComboBox*	sel;

public:
	void setup(QDialog* dlg)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow("Variable"   , name = new QLabel);
		form->addRow("Selection type", type = new QLabel);
		form->addRow("Selection"     , sel = new QComboBox);

		QVBoxLayout* l = new QVBoxLayout;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		l->addLayout(form);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}

	void setVariable(const CPlotVariable& var)
	{
		name->setText(QString::fromStdString(var.name()));
		switch (var.domainType())
		{
		case DOMAIN_MESH: type->setText("Mesh"); break;
		case DOMAIN_PART: type->setText("Part"); break;
		case DOMAIN_SURFACE: type->setText("Surface"); break;
		default:
			type->setText("(unknown)");
		}
	}

	void setSelections(const QStringList& l)
	{
		sel->clear();
		sel->addItems(l);
	}
};

class Ui::CDlgEditOutput
{
public:
	QLineEdit*		filter;
	QListWidget*	varList;
	QListWidget*	domList;
	QTabWidget*		tab;

	QComboBox* logType;
	QTableWidget* table;
	QComboBox* logList;
	CustomLineEdit* logEdit;

	vector<CPlotVariable>	m_plt;

public:
	void setup(QDialog* dlg)
	{
		// --- plot file tab
		filter = new QLineEdit;
		varList = new QListWidget;
		domList = new QListWidget;

		QPushButton* newVar  = new QPushButton("Add..."   ); newVar->setToolTip("<font color=\"black\">Add custom variable");
		QPushButton* delVar  = new QPushButton("Delete..."); delVar->setToolTip("<font color=\"black\">Remove custom variable");
		QPushButton* editVar = new QPushButton("Edit..."  ); editVar->setToolTip("<font color=\"black\">Edit custom variable");

		QHBoxLayout* editLayout = new QHBoxLayout;
		editLayout->addWidget(newVar);
		editLayout->addWidget(editVar);
		editLayout->addWidget(delVar);
		editLayout->addStretch();

		QPushButton* add = new QPushButton("Add Selection...");
		QPushButton* del = new QPushButton("Remove");

		QHBoxLayout* fltLayout = new QHBoxLayout;
		fltLayout->setContentsMargins(0,0,0,0);
		fltLayout->addWidget(new QLabel("Filter"));
		fltLayout->addWidget(filter);

		QHBoxLayout* buttonLayout = new QHBoxLayout;
		buttonLayout->setContentsMargins(0,0,0,0);
		buttonLayout->addWidget(add);
		buttonLayout->addWidget(del);
		buttonLayout->addStretch();

		QVBoxLayout* domLayout = new QVBoxLayout;
		domLayout->setContentsMargins(0,0,0,0);
		domLayout->addLayout(buttonLayout);
		domLayout->addWidget(domList);

		QVBoxLayout* varLayout = new QVBoxLayout;
		varLayout->setContentsMargins(0,0,0,0);
		varLayout->addLayout(fltLayout);
		varLayout->addWidget(varList);
		varLayout->addLayout(editLayout);

		QHBoxLayout* h = new QHBoxLayout;
//		h->setContentsMargins(0,0,0,0);

		h->addLayout(varLayout);
		h->addLayout(domLayout);

		QWidget* plotTab = new QWidget;
		plotTab->setLayout(h);

		// --- log file tab
		logType = new QComboBox;
		logType->addItem("Node", FSLogData::LD_NODE);
		logType->addItem("Face", FSLogData::LD_FACE);
		logType->addItem("Element", FSLogData::LD_ELEM);
		logType->addItem("Rigid body", FSLogData::LD_RIGID);
		logType->addItem("Rigid connector", FSLogData::LD_CNCTR);

		logList = new QComboBox;

		QLabel* info = new QLabel("Enter a semi-colon delimited list of variables:");

		logEdit = new CustomLineEdit;
		logEdit->setWrapQuotes(false);
		logEdit->setDelimiter(";");

		QPushButton* addLogItem = new QPushButton("Add");
		QPushButton* remLogItem = new QPushButton("Remove");

		table = new QTableWidget;
		table->setColumnCount(4);
		table->verticalHeader()->hide();
		table->setHorizontalHeaderLabels(QStringList() << "Type" << "Data" << "List" << "File (optional)");
		table->horizontalHeader()->setStretchLastSection(true);
		table->setSelectionBehavior(QAbstractItemView::SelectRows);

		QFormLayout* logForm = new QFormLayout;
		logForm->setContentsMargins(0,0,0,0);
		logForm->setLabelAlignment(Qt::AlignRight);
		logForm->addRow("Type:", logType);
		logForm->addRow("List:", logList);

		QHBoxLayout* logbuttonLayout = new QHBoxLayout;
		logbuttonLayout->setContentsMargins(0,0,0,0);
		logbuttonLayout->addWidget(addLogItem);
		logbuttonLayout->addWidget(remLogItem);
		logbuttonLayout->addStretch();

		QVBoxLayout* logLayout = new QVBoxLayout;
		logLayout->addLayout(logForm);
		logLayout->addWidget(info);
		logLayout->addWidget(logEdit);
		logLayout->addLayout(logbuttonLayout);
		logLayout->addWidget(table);

		QWidget* logTab = new QWidget;
		logTab->setLayout(logLayout);

		// ---

		tab = new QTabWidget;
		tab->addTab(plotTab, "Plotfile");
		tab->addTab(logTab, "Logfile");
		
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(tab);
		mainLayout->addWidget(bb);

		dlg->setLayout(mainLayout);

		filter->setFocus();

		QObject::connect(filter, SIGNAL(textEdited(const QString&)), dlg, SLOT(onFilterChanged(const QString&)));
		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(add, SIGNAL(clicked()), dlg, SLOT(OnAddSelection()));
		QObject::connect(del, SIGNAL(clicked()), dlg, SLOT(OnRemoveSelection()));
		QObject::connect(newVar , SIGNAL(clicked()), dlg, SLOT(OnNewVariable()));
		QObject::connect(editVar, SIGNAL(clicked()), dlg, SLOT(OnEditVariable()));
		QObject::connect(delVar , SIGNAL(clicked()), dlg, SLOT(OnDeleteVariable()));
		QObject::connect(varList, SIGNAL(currentRowChanged(int)), dlg, SLOT(OnVariable(int)));
		QObject::connect(varList, SIGNAL(itemClicked(QListWidgetItem*)), dlg, SLOT(OnItemClicked(QListWidgetItem*)));

		QObject::connect(addLogItem, SIGNAL(clicked()), dlg, SLOT(onLogAdd()));
		QObject::connect(remLogItem, SIGNAL(clicked()), dlg, SLOT(onLogRemove()));
		QObject::connect(logType, SIGNAL(currentIndexChanged(int)), dlg, SLOT(UpdateLogItemList()));
		QObject::connect(table, SIGNAL(itemChanged(QTableWidgetItem*)), dlg, SLOT(onItemChanged(QTableWidgetItem*)));
	}

	void clearLists()
	{
		varList->clear();
		domList->clear();
	}

	void clearDomainList()
	{
		domList->clear();
	}

	void addVariable(const QString& name, bool bchecked, int nid, bool buser)
	{
		QListWidgetItem* item = new QListWidgetItem(name, varList);
		item->setData(Qt::UserRole, nid);
		item->setCheckState(bchecked ? Qt::Checked : Qt::Unchecked);
		if (buser)
		{
			QFont f = item->font();
			f.setBold(true);
			item->setFont(f);
		}
	}

	CPlotVariable* currentVariable()
	{
		int item = varList->currentRow();
		if (item < 0) return nullptr;
		int nid = varList->currentItem()->data(Qt::UserRole).toInt();
		return &m_plt[nid];
	}

	void removeCurrentVariable()
	{
		QListWidgetItem* it = varList->currentItem();
		if (it)
		{
			int n = it->data(Qt::UserRole).toInt();
			m_plt.erase(m_plt.begin() + n);
		}
	}

	void updateSelectionList()
	{
		clearDomainList();
		CPlotVariable* var = currentVariable();
		if (var) setCurrentVariable(*var);
	}

	void setCurrentVariable(const CPlotVariable& var)
	{
		clearDomainList();
		int N = var.Domains();
		for (int i=0; i<N; ++i)
		{
			const FEItemListBuilder* item = var.GetDomain(i);
			domList->addItem(QString::fromStdString(item->GetName()));
		}
	}

	void setCurrentVariable(const QString& s)
	{
		QList<QListWidgetItem*> sel = varList->findItems(s, Qt::MatchExactly);
		if (sel.size() == 1)
		{
			varList->setCurrentItem(sel.at(0));
		}
	}

	void clearLogTable()
	{
		table->setRowCount(0);
	}

	void setLogTableSize(int n)
	{
		table->setRowCount(n);
	}

	void setLogTableItem(int nrow, const QString& type, const QString& data, const QString& list, const QString& file)
	{
		QTableWidgetItem* item;
		item = new QTableWidgetItem; item->setText(type); item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled); table->setItem(nrow, 0, item);
		item = new QTableWidgetItem; item->setText(data); item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled); table->setItem(nrow, 1, item);
		item = new QTableWidgetItem; item->setText(list); item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled); table->setItem(nrow, 2, item);
		item = new QTableWidgetItem; item->setText(file); table->setItem(nrow, 3, item);
	}
};

//=================================================================================================
CDlgAddSelection::CDlgAddSelection(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddSelection)
{
	ui->setup(this);
}

void CDlgAddSelection::setVariable(const CPlotVariable& var)
{
	ui->setVariable(var);
}

void CDlgAddSelection::setSelections(const QStringList& l)
{
	ui->setSelections(l);
}

int CDlgAddSelection::currentSelection()
{
	int n = ui->sel->currentIndex();
	return n;
}

//=================================================================================================

CDlgEditOutput::CDlgEditOutput(FSProject& prj, QWidget* parent, int tab) : QDialog(parent), ui(new Ui::CDlgEditOutput), m_prj(prj)
{
	ui->setup(this);
	ui->tab->setCurrentIndex(tab);
	setWindowTitle("Edit Output");
}

void CDlgEditOutput::showEvent(QShowEvent* ev)
{
	ui->clearLists();

	int module = m_prj.GetModule();

	ui->m_plt.clear();

	// get all the FEBio plot classes
	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();

	// first add all existing plot variables
	for (int i = 0; i < plt.PlotVariables(); ++i)
	{
		CPlotVariable& var = plt.PlotVariable(i);
		ui->m_plt.push_back(var);
	}

	vector<FEBio::FEBioClassInfo> pltClasses = FEBio::FindAllClasses(module, FEPLOTDATA_ID);
	for (int i = 0; i < pltClasses.size(); ++i)
	{
		FEBio::FEBioClassInfo& feb = pltClasses[i];

		DOMAIN_TYPE dom = DOMAIN_MESH;
		if (feb.baseClassId == FEBio::GetBaseClassIndex("FEPlotSurfaceData")) dom = DOMAIN_SURFACE;
		if (feb.baseClassId == FEBio::GetBaseClassIndex("FEPlotDomainData" )) dom = DOMAIN_PART;
		CPlotVariable tmp(feb.sztype, false, true, dom);

		CPlotVariable* var = plt.FindVariable(feb.sztype);
		if (var == nullptr) ui->m_plt.push_back(tmp);
	}

	UpdateVariables("");
	UpdateLogTable();
	UpdateLogItemList();
}

void CDlgEditOutput::UpdateVariables(const QString& flt)
{
	bool filter = (flt.isEmpty() == false);

	ui->varList->clear();
	int N = ui->m_plt.size();
	for (int i = 0; i<N; ++i)
	{
		CPlotVariable& tmp = ui->m_plt[i];
		QString t = QString::fromStdString(tmp.name());
		if ((filter == false) || (t.contains(flt, Qt::CaseInsensitive)))
		{
			ui->addVariable(t, tmp.isActive(), i, tmp.isCustom());
		}
	}
	ui->varList->sortItems();
}

void CDlgEditOutput::OnVariable(int nrow)
{
	ui->updateSelectionList();
}

void CDlgEditOutput::OnItemClicked(QListWidgetItem* item)
{
	if (item == nullptr) return;
	int nid = item->data(Qt::UserRole).toInt();
	CPlotVariable& var = ui->m_plt[nid];
	var.setActive(item->checkState() == Qt::Checked);
}

void CDlgEditOutput::OnAddSelection()
{
	CPlotVariable* var = ui->currentVariable();
	if (var == nullptr)
	{
		QMessageBox::information(this, "Add Selection", "Please select a plot variable first");
		return;
	}

	if (var->domainType() == DOMAIN_MESH)
	{
		QMessageBox::information(this, "Add Selection", "This plot variable does not support selections.");
		return;
	}

	FSModel& fem = m_prj.GetFSModel();
	vector<FEItemListBuilder*> list = fem.GetModel().AllNamedSelections(var->domainType());

	QStringList names;
	for (size_t i=0; i<list.size(); ++i)
	{
		names << QString::fromStdString(list[i]->GetName());
	}

	CDlgAddSelection dlg(this);
	dlg.setVariable(*var);
	dlg.setSelections(names);
	if (dlg.exec())
	{
		int n = dlg.currentSelection();
		if (n >= 0)
		{
			var->addDomain(list[n]);
			ui->setCurrentVariable(*var);
		}
	}
}

void CDlgEditOutput::OnRemoveSelection()
{
	CPlotVariable* var = ui->currentVariable();
	if (var == nullptr)
	{
		QMessageBox::information(this, "Add Domain", "Please select a plot variable first");
		return;
	}

	QList<QListWidgetItem*> sel = ui->domList->selectedItems();
	if (sel.empty())
	{
		QMessageBox::information(this, "Add Selection", "Please select a selection.");
		return;
	}

	int nrow = ui->domList->currentRow();
	var->removeDomain(nrow);
	ui->setCurrentVariable(*var);
}

void CDlgEditOutput::onFilterChanged(const QString& txt)
{
	UpdateVariables(txt);	
}

void CDlgEditOutput::UpdateLogTable()
{
	FSModel& fem = m_prj.GetFSModel();

	CLogDataSettings& log = m_prj.GetLogDataSettings();
	ui->clearLogTable();
	ui->setLogTableSize(log.LogDataSize());
	for (int i=0; i<log.LogDataSize(); ++i)
	{
		FSLogData& logi = log.LogData(i);

		QString type;
		switch (logi.Type())
		{
		case FSLogData::LD_NODE : type = "Node"; break;
		case FSLogData::LD_FACE : type = "Face"; break;
		case FSLogData::LD_ELEM : type = "Element"; break;
		case FSLogData::LD_RIGID: type = "Rigid body"; break;
        case FSLogData::LD_CNCTR: type = "Rigid connector"; break;
		}

		QString data = QString::fromStdString(logi.GetDataString());

		QString list;
		if (logi.Type() == FSLogData::LD_RIGID)
		{
			FSLogRigidData& rd = dynamic_cast<FSLogRigidData&>(logi);
			if (rd.GetMatID() == -1) list = "(all rigid bodies)";
			else list = QString::fromStdString(fem.GetMaterialFromID(rd.GetMatID())->GetName());
		}
        else if (logi.Type() == FSLogData::LD_CNCTR)
        {
			FSLogConnectorData& cd = dynamic_cast<FSLogConnectorData&>(logi);
			if (cd.GetConnectorID() == -1) list = "(all rigid connectors)";
            else list = QString::fromStdString(fem.GetRigidConnectorFromID(cd.GetConnectorID())->GetName());
        }
		else
		{
			FSHasOneItemList* pil = dynamic_cast<FSHasOneItemList*>(&logi);
			FEItemListBuilder* pl = pil->GetItemList();
			if (pl == nullptr)
			{
				if (logi.Type() == FSLogData::LD_NODE) list = "(all nodes)";
				else if (logi.Type() == FSLogData::LD_ELEM) list = "(all elements)";
			}
			else
			{
				list = QString::fromStdString(pl->GetName());
			}
		}

		QString fileName = QString::fromStdString(logi.GetFileName());

		ui->setLogTableItem(i, type, data, list, fileName);
	}
}

void CDlgEditOutput::UpdateLogItemList()
{
	ui->logList->clear();
	FSModel& fem = m_prj.GetFSModel();
	GModel& mdl = fem.GetModel();

	int ntype = ui->logType->currentData().toInt();

	if (ntype == FSLogData::LD_NODE ) ui->logList->addItem("(all nodes)", -1);
	if (ntype == FSLogData::LD_ELEM ) ui->logList->addItem("(all elements)", -1);
	if (ntype == FSLogData::LD_RIGID) ui->logList->addItem("(all rigid bodies)", -1);
	if (ntype == FSLogData::LD_CNCTR) ui->logList->addItem("(all rigid connectors)", -1);

	std::vector<FEBio::FEBioClassInfo> info;
	switch (ntype)
	{
	case FSLogData::LD_NODE : info = FEBio::FindAllActiveClasses(FELOGNODEDATA_ID); break;
	case FSLogData::LD_FACE : info = FEBio::FindAllActiveClasses(FELOGFACEDATA_ID); break;
	case FSLogData::LD_ELEM : info = FEBio::FindAllActiveClasses(FELOGELEMDATA_ID); break;
	case FSLogData::LD_RIGID: info = FEBio::FindAllActiveClasses(FELOGOBJECTDATA_ID); break;
	case FSLogData::LD_CNCTR: info = FEBio::FindAllActiveClasses(FELOGNLCONSTRAINTDATA_ID); break;
	}
	
	QStringList sl;
	for (int i = 0; i < info.size(); ++i) sl << info[i].sztype;

	QCompleter* c = new QCompleter(sl);
	c->popup()->setMinimumWidth(100);
	ui->logEdit->setMultipleCompleter(c);

	if ((ntype == FSLogData::LD_NODE) || (ntype == FSLogData::LD_ELEM))
	{
		// add parts
		for (int i = 0; i<mdl.PartLists(); ++i)
		{
			GPartList* pg = mdl.PartList(i);
			ui->logList->addItem(QString::fromStdString(pg->GetName()), pg->GetID());
		}

		if (ntype == FSLogData::LD_NODE)
		{
			// add surfaces
			for (int i = 0; i<mdl.FaceLists(); ++i)
			{
				GFaceList* pg = mdl.FaceList(i);
				ui->logList->addItem(QString::fromStdString(pg->GetName()), pg->GetID());
			}

			// add edges
			for (int i = 0; i<mdl.EdgeLists(); ++i)
			{
				GEdgeList* pg = mdl.EdgeList(i);
				ui->logList->addItem(QString::fromStdString(pg->GetName()), pg->GetID());
			}

			// add nodesets
			for (int i = 0; i<mdl.NodeLists(); ++i)
			{
				GNodeList* pg = mdl.NodeList(i);
				ui->logList->addItem(QString::fromStdString(pg->GetName()), pg->GetID());
			}

			for (int i=0; i<mdl.Objects(); ++i)
			{
				GObject* po = mdl.Object(i);
				int NNS = po->FENodeSets();
				for (int i=0; i<NNS; ++i)
				{
					FSNodeSet* pns = po->GetFENodeSet(i);
					ui->logList->addItem(QString::fromStdString(pns->GetName()), pns->GetID());
				}
			}
		}

		if (ntype == FSLogData::LD_ELEM)
		{
			for (int i = 0; i<mdl.Objects(); ++i)
			{
				GObject* po = mdl.Object(i);
				int NES = po->FEElemSets();
				for (int i = 0; i<NES; ++i)
				{
					FSElemSet* pg = po->GetFEElemSet(i);
					ui->logList->addItem(QString::fromStdString(pg->GetName()), pg->GetID());
				}
			}
		}
	}
	else if (ntype == FSLogData::LD_FACE)
	{
		// add surfaces
		for (int i = 0; i < mdl.FaceLists(); ++i)
		{
			GFaceList* pg = mdl.FaceList(i);
			ui->logList->addItem(QString::fromStdString(pg->GetName()), pg->GetID());
		}

		for (int i = 0; i < mdl.Objects(); ++i)
		{
			GObject* po = mdl.Object(i);
			int NS = po->FESurfaces();
			for (int i = 0; i < NS; ++i)
			{
				FSSurface* ps = po->GetFESurface(i);
				ui->logList->addItem(QString::fromStdString(ps->GetName()), ps->GetID());
			}
		}
	}
	else if (ntype == FSLogData::LD_RIGID)
	{
		int M = fem.Materials();
		for (int i = 0; i<M; ++i)
		{
			GMaterial* pm = fem.GetMaterial(i);
			if (pm->GetMaterialProperties()->Type() == FE_RIGID_MATERIAL)
			{
				ui->logList->addItem(QString::fromStdString(pm->GetName()), pm->GetID());
			}
		}
	}
    else if (ntype == FSLogData::LD_CNCTR)
    {
        int lid = -1;
        for (int i = 0; i<fem.Steps(); ++i)
        {
            FSStep* pstep = fem.GetStep(i);
            for (int j = 0; j<pstep->RigidConnectors(); ++j)
            {
                FSRigidConnector* pc = pstep->RigidConnector(j);
                ui->logList->addItem(QString::fromStdString(pc->GetName()), ++lid);
            }
        }
    }
}

void CDlgEditOutput::onLogAdd()
{
	// extract the data class
	int ntype = ui->logType->currentData().toInt();

	// extract the list index
	int nlist = -1;
	if (ui->logList->currentIndex() != -1)
		nlist = ui->logList->currentData().toInt();

	// get the data name
	QString data = ui->logEdit->text();
	if (data.isEmpty())
	{
		QMessageBox::critical(this, "Add Log Data", "Please enter a data string");
		return;
	}
	ui->logEdit->setText("");

	FSModel& fem = m_prj.GetFSModel();
	GModel& mdl = fem.GetModel();

	// create new log entry
	FSLogData* ld = nullptr;
	switch (ntype)
	{
	case FSLogData::LD_NODE: ld = new FSLogNodeData(mdl.FindNamedSelection(nlist)); break;
	case FSLogData::LD_FACE: ld = new FSLogFaceData(mdl.FindNamedSelection(nlist)); break;
	case FSLogData::LD_ELEM: ld = new FSLogElemData(mdl.FindNamedSelection(nlist)); break;
	case FSLogData::LD_RIGID: ld = new FSLogRigidData(nlist); break;
	case FSLogData::LD_CNCTR: ld = new FSLogConnectorData(nlist); break;
	}

	assert(ld);
	if (ld)
	{
		ld->SetDataString(data.toStdString());
		m_prj.GetLogDataSettings().AddLogData(ld);
	}

	// add it to the list
	UpdateLogTable();
}

void CDlgEditOutput::onLogRemove()
{
	int nrow = ui->table->currentRow();
	if (nrow == -1) return;

	CLogDataSettings& log = m_prj.GetLogDataSettings();
	log.RemoveLogData(nrow);

	UpdateLogTable();
}

class CNewVariableDialog : public QDialog
{
private:
	QLineEdit* m_var;
	QLineEdit* m_flt;
	QLineEdit* m_aka;
	QComboBox* m_cat;

public:
	CNewVariableDialog(QWidget* parent) : QDialog(parent) 
	{
		setWindowTitle("Add New Variable");

		QFormLayout* f = new QFormLayout;
		f->addRow("Variable", m_var = new QLineEdit);
		f->addRow("Category", m_cat = new QComboBox);
		f->addRow("Filter"  , m_flt = new QLineEdit);
		f->addRow("Alias"   , m_aka = new QLineEdit);

		m_cat->addItem("Node variable");
		m_cat->addItem("Element variable");
		m_cat->addItem("Face variable");

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* l = new QVBoxLayout();
		l->addLayout(f);
		l->addWidget(bb);

		setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
	}

	QString getVariable() const { return m_var->text(); }
	QString getFilter() const { return m_flt->text(); }
	QString getAlias() const { return m_aka->text(); }
	DOMAIN_TYPE getCategory() const
	{
		int n = m_cat->currentIndex();
		switch (n)
		{
		case 0: return DOMAIN_MESH; break;
		case 1: return DOMAIN_PART; break;
		case 2: return DOMAIN_SURFACE; break;
		}
		assert(false);
		return DOMAIN_MESH;
	}

	QString getFullVariableName() const
	{
		QString var = getVariable();
		QString flt = getFilter();
		QString aka = getAlias();

		QString newvar = var;
		if (flt.isEmpty() == false) newvar += QString("[%1]").arg(flt);
		if (aka.isEmpty() == false) newvar += QString("=%1").arg(aka);

		return newvar;
	}
};

void CDlgEditOutput::OnNewVariable()
{
	CNewVariableDialog dlg(this);
	if (dlg.exec())
	{
		QString s = dlg.getFullVariableName();
		DOMAIN_TYPE n = dlg.getCategory();
		if (s.isEmpty() == false)
		{
			string varName = s.toStdString();
			// make sure we don't have it yet
			for (auto& var : ui->m_plt)
			{
				if (var.name() == varName)
				{
					QMessageBox::information(this, "FEBio Studio", "This variable already exists.");
					ui->setCurrentVariable(s);
					return;
				}
			}
			CPlotVariable var(s.toStdString(), true, true, n);
			var.setCustom(true);
			ui->m_plt.push_back(var);
			UpdateVariables("");
			ui->setCurrentVariable(s);
		}
	}
}

void CDlgEditOutput::OnEditVariable()
{
	CPlotVariable* var = ui->currentVariable();
	if (var)
	{
		if (var->isCustom() == false)
		{
			QMessageBox::critical(this, "Delete Variable", "Only custom variables can be deleted.");
		}
		else
		{
			QString s = QInputDialog::getText(this, "Edit Variable", "Variable:", QLineEdit::Normal, QString::fromStdString(var->name()));
			if (s.isEmpty() == false)
			{
				var->setName(s.toStdString());
				UpdateVariables("");
				ui->setCurrentVariable(s);
			}
		}
	}
	else QMessageBox::information(this, "Delete Variable", "Please select a custom variable.");
}

void CDlgEditOutput::OnDeleteVariable()
{
	CPlotVariable* var = ui->currentVariable();
	if (var)
	{
		if (var->isCustom() == false)
		{
			QMessageBox::critical(this, "Delete Variable", "Only custom variables can be deleted.");
		}
		else if (QMessageBox::question(this, "Delete Variable", "Are you sure you want to delete this variable?") == QMessageBox::Yes)
		{
			ui->removeCurrentVariable();
			UpdateVariables("");
		}
	}
	else QMessageBox::information(this, "Delete Variable", "Please select a custom variable.");
}

void CDlgEditOutput::onItemChanged(QTableWidgetItem* item)
{
	if (item == nullptr) return;
	int n = item->row();

	CLogDataSettings& log = m_prj.GetLogDataSettings();
	if ((n >= 0) && (n < log.LogDataSize()))
	{
		FSLogData& ld = log.LogData(n);
		QString t = item->text();
		ld.SetFileName(t.toStdString());
	}
}

void CDlgEditOutput::accept()
{
	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
	plt.Clear();

	for (int i = 0; i < ui->m_plt.size(); ++i)
	{
		CPlotVariable& plti = ui->m_plt[i];
		if (plti.isActive())
		{
			plt.AddPlotVariable(plti);
		}
	}

	QDialog::accept();
}
