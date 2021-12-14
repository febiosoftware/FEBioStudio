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
#include <MeshTools/FEProject.h>
#include <MeshTools/GModel.h>
#include <FEMLib/FEMultiMaterial.h>
#include <MeshTools/GGroup.h>
#include <MeshTools/FEGroup.h>
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
#include <QMessageBox>
#include <QInputDialog>
#include <FEBioLink/FEBioClass.h>

class Ui::CDlgAddDomain
{
public:
	QLabel*	name;
	QLabel* type;
	QComboBox*	dom;

public:
	void setup(QDialog* dlg)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow("Variable"   , name = new QLabel);
		form->addRow("Domain type", type = new QLabel);
		form->addRow("Domain"     , dom = new QComboBox);

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

	void setDomains(const QStringList& l)
	{
		dom->clear();
		dom->addItems(l);
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
	QLineEdit* logEdit;

	vector<CPlotVariable>	m_plt;

public:
	void setup(QDialog* dlg)
	{
		// --- plot file tab
		filter = new QLineEdit;
		varList = new QListWidget;
		domList = new QListWidget;

		QPushButton* newVar = new QPushButton("+"); newVar->setFixedWidth(20); newVar->setToolTip("<font color=\"black\">Add custom variable");
		QPushButton* add = new QPushButton("Add Domain...");
		QPushButton* del = new QPushButton("Remove");

		QHBoxLayout* fltLayout = new QHBoxLayout;
		fltLayout->setContentsMargins(0,0,0,0);
		fltLayout->addWidget(new QLabel("Filter"));
		fltLayout->addWidget(filter);
		fltLayout->addWidget(newVar);

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

		QHBoxLayout* h = new QHBoxLayout;
//		h->setContentsMargins(0,0,0,0);

		h->addLayout(varLayout);
		h->addLayout(domLayout);

		QWidget* plotTab = new QWidget;
		plotTab->setLayout(h);

		// --- log file tab
		logType = new QComboBox;
		logType->addItem("Node");
		logType->addItem("Element");
		logType->addItem("Rigid body");
        logType->addItem("Rigid connector");

		logList = new QComboBox;

		QLabel* info = new QLabel("Enter a semi-colon delimited list of variables:");

		logEdit = new QLineEdit;

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
		QObject::connect(add, SIGNAL(clicked()), dlg, SLOT(OnAddDomain()));
		QObject::connect(del, SIGNAL(clicked()), dlg, SLOT(OnRemoveDomain()));
		QObject::connect(newVar, SIGNAL(clicked()), dlg, SLOT(OnNewVariable()));
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

	void addVariable(const QString& name, bool bchecked, int nid)
	{
		QListWidgetItem* item = new QListWidgetItem(name, varList);
		item->setData(Qt::UserRole, nid);
		item->setCheckState(bchecked ? Qt::Checked : Qt::Unchecked);
	}

	int currentVariable()
	{
		int item = varList->currentRow();
		if (item < 0) return -1;
		int nid = varList->currentItem()->data(Qt::UserRole).toInt();
		return nid;
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
CDlgAddDomain::CDlgAddDomain(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddDomain)
{
	ui->setup(this);
}

void CDlgAddDomain::setVariable(const CPlotVariable& var)
{
	ui->setVariable(var);
}

void CDlgAddDomain::setDomains(const QStringList& l)
{
	ui->setDomains(l);
}

int CDlgAddDomain::selectedDomain()
{
	int n = ui->dom->currentIndex();
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

	// get all the FEBio plot classes
	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
	ui->m_plt.clear();
	vector<FEBio::FEBioClassInfo> pltClasses = FEBio::FindAllClasses(module, FEPLOTDATA_ID);
	for (int i = 0; i < pltClasses.size(); ++i)
	{
		FEBio::FEBioClassInfo& feb = pltClasses[i];
		CPlotVariable tmp(feb.sztype, false, true, DOMAIN_MESH);

		CPlotVariable* var = plt.FindVariable(feb.sztype);
		if (var)
		{
			tmp.setActive(var->isActive());
		}
		ui->m_plt.push_back(tmp);
	}

	UpdateVariables("");
	UpdateLogTable();
	UpdateLogItemList();
}

void CDlgEditOutput::UpdateVariables(const QString& flt)
{
	bool filter = (flt.isEmpty() == false);

	ui->varList->clear();
	int module = m_prj.GetModule();
	int N = ui->m_plt.size();
	for (int i = 0; i<N; ++i)
	{
		CPlotVariable& tmp = ui->m_plt[i];
		QString t = QString::fromStdString(tmp.name());
		if ((filter == false) || (t.contains(flt, Qt::CaseInsensitive)))
		{
			ui->addVariable(t, tmp.isActive(), i);
		}

/*		CPlotVariable& var = plt.PlotVariable(i);
		if (var.isShown() && (var.GetModule() & module))
		{
			QString t = QString::fromStdString(var.name());
			if ((filter == false) || ( t.contains(flt, Qt::CaseInsensitive)))
			{
				ui->addVariable(t, var.isActive(), i);
			}
		}
*/
	}
	ui->varList->sortItems();
}

void CDlgEditOutput::OnVariable(int nrow)
{
/*	ui->clearDomainList();
	if (nrow >= 0)
	{
		int nvar = ui->currentVariable();
		CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
		CPlotVariable& var = plt.PlotVariable(nvar);
		ui->setCurrentVariable(var);
	}
*/
}

void CDlgEditOutput::OnItemClicked(QListWidgetItem* item)
{
	int nvar = item->data(Qt::UserRole).toInt();
	CPlotVariable& var = ui->m_plt[nvar];
	var.setActive(item->checkState() == Qt::Checked);
}

void CDlgEditOutput::OnAddDomain()
{
/*	int nvar = ui->currentVariable();
	if (nvar == -1)
	{
		QMessageBox::information(this, "Add Domain", "Please select a plot variable first");
		return;
	}

	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
	CPlotVariable& var = plt.PlotVariable(nvar);

	FSModel& fem = m_prj.GetFSModel();
	vector<FEItemListBuilder*> list = fem.GetModel().AllNamedSelections(var.domainType());

	QStringList names;
	for (size_t i=0; i<list.size(); ++i)
	{
		names << QString::fromStdString(list[i]->GetName());
	}

	CDlgAddDomain dlg(this);
	dlg.setVariable(var);
	dlg.setDomains(names);
	if (dlg.exec())
	{
		int n = dlg.selectedDomain();
		if (n >= 0)
		{
			var.addDomain(list[n]);
			ui->setCurrentVariable(var);
		}
	}
*/
}

void CDlgEditOutput::OnRemoveDomain()
{
/*
	int nvar = ui->currentVariable();
	if (nvar == -1)
	{
		QMessageBox::information(this, "Add Domain", "Please select a plot variable first");
		return;
	}

	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
	CPlotVariable& var = plt.PlotVariable(nvar);

	QList<QListWidgetItem*> sel = ui->domList->selectedItems();
	if (sel.empty())
	{
		QMessageBox::information(this, "Add Domain", "Please select a domain first");
		return;
	}

	int nrow = ui->domList->currentRow();
	var.removeDomain(nrow);
	ui->setCurrentVariable(var);
*/
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
		FELogData& logi = log.LogData(i);

		QString type;
		switch (logi.type)
		{
		case FELogData::LD_NODE : type = "Node"; break;
		case FELogData::LD_ELEM : type = "Element"; break;
		case FELogData::LD_RIGID: type = "Rigid body"; break;
        case FELogData::LD_CNCTR: type = "Rigid connector"; break;
		}

		QString data = QString::fromStdString(logi.sdata);

		QString list;
		if (logi.type == FELogData::LD_RIGID)
		{
			if (logi.matID == -1) list = "(all rigid bodies)";
			else list = QString::fromStdString(fem.GetMaterialFromID(logi.matID)->GetName());
		}
        else if (logi.type == FELogData::LD_CNCTR)
        {
            if (logi.rcID == -1) list = "(all rigid connectors)";
            else list = QString::fromStdString(fem.GetRigidConnectorFromID(logi.rcID)->GetName());
        }
		else
		{
			if (logi.groupID == -1)
			{
				if (logi.type == FELogData::LD_NODE) list = "(all nodes)";
				else list = "(all elements)";
			}
			else
			{
				GModel& mdl = fem.GetModel();
				FEItemListBuilder* item = mdl.FindNamedSelection(logi.groupID); assert(item);
				if (item)
				{
					list = QString::fromStdString(item->GetName());
				}
			}
		}

		QString fileName = QString::fromStdString(logi.fileName);

		ui->setLogTableItem(i, type, data, list, fileName);
	}
}

void CDlgEditOutput::UpdateLogItemList()
{
	ui->logList->clear();
	FSModel& fem = m_prj.GetFSModel();
	GModel& mdl = fem.GetModel();

	int ntype = ui->logType->currentIndex();

	if (ntype == FELogData::LD_NODE ) ui->logList->addItem("(all nodes)", -1);
	if (ntype == FELogData::LD_ELEM ) ui->logList->addItem("(all elements)", -1);
	if (ntype == FELogData::LD_RIGID) ui->logList->addItem("(all rigid bodies)", -1);
    if (ntype == FELogData::LD_CNCTR) ui->logList->addItem("(all rigid connectors)", -1);

	if ((ntype == FELogData::LD_NODE) || (ntype == FELogData::LD_ELEM))
	{
		// add parts
		for (int i = 0; i<mdl.PartLists(); ++i)
		{
			GPartList* pg = mdl.PartList(i);
			ui->logList->addItem(QString::fromStdString(pg->GetName()), pg->GetID());
		}

		if (ntype == FELogData::LD_NODE)
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

		if (ntype == FELogData::LD_ELEM)
		{
			for (int i = 0; i<mdl.Objects(); ++i)
			{
				GObject* po = mdl.Object(i);
				int NES = po->FEParts();
				for (int i = 0; i<NES; ++i)
				{
					FSPart* pg = po->GetFEPart(i);
					ui->logList->addItem(QString::fromStdString(pg->GetName()), pg->GetID());
				}
			}
		}
	}
	else if (ntype == FELogData::LD_RIGID)
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
    else if (ntype == FELogData::LD_CNCTR)
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
	// extract the data
	int ntype = ui->logType->currentIndex();

	int nlist = -1;
	if (ui->logList->currentIndex() != -1)
		nlist = ui->logList->currentData().toInt();

	QString data = ui->logEdit->text();
	if (data.isEmpty())
	{
		QMessageBox::critical(this, "Add Log Data", "Please enter a data string");
		return;
	}
	ui->logEdit->setText("");

	// create new log entry
	FELogData ld;
	ld.type = ntype;
	ld.sdata = data.toStdString();
	if (ld.type == FELogData::LD_RIGID) ld.matID = nlist;
    else if (ld.type == FELogData::LD_CNCTR) ld.rcID = nlist;
	else ld.groupID = nlist;

	// add it to the list
	m_prj.GetLogDataSettings().AddLogData(ld);
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

void CDlgEditOutput::OnNewVariable()
{
	QString s = QInputDialog::getText(this, "New Variable", "Name:");
	if (s.isEmpty() == false)
	{
		CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
		plt.AddPlotVariable(s.toStdString(), true, true);
		UpdateVariables("");
	}
}

void CDlgEditOutput::onItemChanged(QTableWidgetItem* item)
{
	if (item == nullptr) return;
	int n = item->row();

	CLogDataSettings& log = m_prj.GetLogDataSettings();
	if ((n >= 0) && (n < log.LogDataSize()))
	{
		FELogData& ld = log.LogData(n);
		QString t = item->text();
		ld.fileName = t.toStdString();
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
