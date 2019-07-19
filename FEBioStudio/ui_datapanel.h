#pragma once
#include "DataPanel.h"
#include "ObjectPanel.h"
#include "ToolBox.h"
#include <QBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLineEdit>
#include <QFormLayout>
#include <QComboBox>
#include <QDialogButtonBox>

//=================================================================================================
class Ui::CDlgAddDataField
{
public:
	QLineEdit*	name;
	QComboBox*	classBox;
	QComboBox*	typeBox;
	QLineEdit*	val;

public:
	void setup(QDialog* dlg)
	{
		QStringList classes; 
		classes << "Node" << "Face" << "Element";

		QStringList types;
		types << "Scalar" << "Vec3D";

		QFormLayout* form = new QFormLayout;
		form->addRow("Name" , name = new QLineEdit);
		form->addRow("Class", classBox = new QComboBox); classBox->addItems(classes);
		form->addRow("Type" , typeBox = new QComboBox); typeBox->addItems(types);
		form->addRow("Value", val = new QLineEdit); val->setValidator(new QDoubleValidator); val->setText(QString::number(0.0));

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		l->addWidget(bb);
		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

//=================================================================================================
class Ui::CDataFieldPane
{
public:
	QTableWidget*	table;

	GObject*		m_po;

	bool	m_updating;

public:
	void setup(::CDataFieldPane* pane)
	{
		m_po = 0;
		m_updating = false;

		QPushButton* newButton = new QPushButton("New...");
		QPushButton* delButton = new QPushButton("Delete");

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(newButton);
		h->addWidget(delButton);
		h->addStretch();

		table = new QTableWidget(0, 3);
		table->horizontalHeader()->setStretchLastSection(true);
		table->verticalHeader()->setVisible(false);
		table->setSelectionBehavior(QAbstractItemView::SelectRows);
		table->setSelectionMode(QAbstractItemView::SingleSelection);

		QStringList headerLabels;
		headerLabels << "Name" << "Class" << "Type";
		table->setHorizontalHeaderLabels(headerLabels);
		
		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(h);
		l->addWidget(table);
		pane->setLayout(l);

		QObject::connect(newButton, SIGNAL(clicked(bool)), pane, SLOT(onNew()));
		QObject::connect(delButton, SIGNAL(clicked(bool)), pane, SLOT(onDelete()));
		QObject::connect(table, SIGNAL(itemChanged(QTableWidgetItem*)), pane, SLOT(onItemChanged(QTableWidgetItem*)));
	}

	void clearTable()
	{
		table->setRowCount(0);
	}

	void addField(const QString& name, const QString& classType, const QString& dataType)
	{
		int rows = table->rowCount();
		table->setRowCount(rows + 1);

		QTableWidgetItem* col0 = new QTableWidgetItem(name);
		QTableWidgetItem* col1 = new QTableWidgetItem(classType); col1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		QTableWidgetItem* col2 = new QTableWidgetItem(dataType); col2->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		table->setItem(rows, 0, col0);
		table->setItem(rows, 1, col1);
		table->setItem(rows, 2, col2);
	}

	int currentRow()
	{
		return table->currentRow();
	}
};

//=================================================================================================
class Ui::CDataPanel
{
public:
	CObjectPanel*	obj;
	::CDataFieldPane*	data;
	CToolBox* tool;

public:
	void setup(::CDataPanel* panel, CMainWindow* mainWindow)
	{
		obj = new CObjectPanel(mainWindow); obj->setObjectName("objectPanel");

		data = new ::CDataFieldPane;

		tool = new CToolBox;
		tool->addTool("Object", obj);
		tool->addTool("Data fields", data);

		QVBoxLayout* l = new QVBoxLayout;
		l->setMargin(0);
		l->addWidget(tool);
		panel->setLayout(l);

		showDataFields(false);
	}

	void showDataFields(bool b)
	{
		tool->getToolItem(1)->setVisible(b);
	}
};
