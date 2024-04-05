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
#include "DlgAddMeshData.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QToolButton>
#include <QItemDelegate>
#include <FSCore/FSObject.h>
#include <QCheckBox>
#include <QApplication>
#include <QClipboard>
#include <QtCore/QMimeData>
#include <GeomLib/FSGroup.h>

class CDlgAddMeshDataUI
{
public:
	QLineEdit* m_name;
	QComboBox* m_type;
	QComboBox* m_data;
	QComboBox* m_fmt;
	QComboBox* m_init;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* layout = new QVBoxLayout;

		QFormLayout* form = new QFormLayout;
		form->addRow("Name:", m_name = new QLineEdit);
		form->addRow("Type:", m_type = new QComboBox);
		form->addRow("Data type:", m_data = new QComboBox);
		form->addRow("Data format:", m_fmt = new QComboBox);
		form->addRow("Data initializer:", m_init = new QComboBox);
		layout->addLayout(form);

		m_type->addItems(QStringList() << "node data" << "surface data" << "element data" << "part data");
		m_data->addItems(QStringList() << "scalar" << "vec3" << "mat3");
		m_fmt ->addItems(QStringList() << "item" << "node" << "mult");
		m_init->addItems(QStringList() << "list" << "constant");

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(bb);

		dlg->setLayout(layout);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgAddMeshData::CDlgAddMeshData(QWidget* pw) : QDialog(pw), ui(new CDlgAddMeshDataUI)
{
	setMinimumWidth(300);
	ui->setup(this);
}

QString CDlgAddMeshData::GetName() { return ui->m_name->text(); }
DATA_CLASS CDlgAddMeshData::GetType()
{ 
	int n = ui->m_type->currentIndex(); 
	return (DATA_CLASS)n;
}

DATA_TYPE CDlgAddMeshData::GetDataType() 
{ 
	int n = ui->m_data->currentIndex();
	return (DATA_TYPE)n;
}

DATA_FORMAT CDlgAddMeshData::GetFormat() 
{ 
	int n = ui->m_fmt->currentIndex(); 
	return (DATA_FORMAT)n;
}

CDlgAddMeshData::DataInitializer CDlgAddMeshData::GetDataInitializer()
{
	int n = ui->m_init->currentIndex();
	switch (n)
	{
	case 0: return CDlgAddMeshData::INITIALIZER_LIST; break;
	case 1: return CDlgAddMeshData::INITIALIZER_CONST; break;
	}
	assert(false);
	return CDlgAddMeshData::INITIALIZER_CONST;
}

//==========================================================

class DataTableDelegate : public QItemDelegate
{
public:
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const
	{
		QLineEdit* lineEdit = new QLineEdit(parent);
		QDoubleValidator* validator = new QDoubleValidator;
		lineEdit->setValidator(validator);
		return lineEdit;
	}
};

class CDlgEditMeshDataUI
{
public:
	FEMeshData* m_data;
	QTableWidget* m_table;

public:
	void setup(QDialog* dlg)
	{
		DATA_TYPE dataType = m_data->GetDataType();
		int ncols = 0;
		switch (dataType)
		{
		case DATA_SCALAR: ncols = 1; break;
		case DATA_VEC3 : ncols = 3; break;
		case DATA_MAT3 : ncols = 9; break;
		}

		QVBoxLayout* layout = new QVBoxLayout;

		QHBoxLayout* h = new QHBoxLayout;
		QToolButton* copy = new QToolButton; copy->setIcon(QIcon(":/icons/clipboard.png")); copy->setToolTip("Copy to clipboard");
		QToolButton* paste = new QToolButton; paste->setIcon(QIcon(":/icons/paste.png")); paste->setToolTip("Paste from clipboard");
		h->addWidget(copy);
		h->addWidget(paste);
		h->addStretch();
		layout->addLayout(h);

		std::vector<double> data = m_data->GetData();
		int nrows = data.size() / ncols;

		m_table = new QTableWidget(nrows, ncols);
		m_table->setItemDelegate(new DataTableDelegate);
		m_table->setObjectName("dataTable");
		m_table->horizontalHeader()->setStretchLastSection(true);
		m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_table->setSelectionMode(QAbstractItemView::SingleSelection);
		m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

		if (ncols == 1) m_table->setHorizontalHeaderLabels(QStringList() << "value");
		if (ncols == 3) m_table->setHorizontalHeaderLabels(QStringList() << "x" << "y" << "z");
		if (ncols == 9) m_table->setHorizontalHeaderLabels(QStringList() << "xx" << "xy" << "xz" << "yx" << "yy" << "yz" << "zx" << "zy" << "zz");

		DATA_CLASS dataClass = m_data->GetDataClass();
		if (dataClass == NODE_DATA)
		{
			FSNodeSet* pg = dynamic_cast<FSNodeSet*>(m_data->GetItemList());
			if (pg)
			{
				std::vector<int> items = pg->CopyItems();
				QStringList Items;
				for (int i : items) Items.push_back(QString::number(i));
				assert(nrows == items.size());
				m_table->setVerticalHeaderLabels(Items);
			}
		}

		for (int i = 0; i < data.size(); ++i)
		{
			int r = i / ncols;
			int c = i % ncols;
			m_table->setItem(r, c, new QTableWidgetItem(QString::number(data[i])));
		}

		layout->addWidget(m_table);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(bb);

		dlg->setLayout(layout);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(copy, SIGNAL(clicked()), dlg, SLOT(OnCopyToClipboard()));
		QObject::connect(paste, SIGNAL(clicked()), dlg, SLOT(OnPasteFromClipboard()));
	}
};

CDlgEditMeshData::CDlgEditMeshData(FEMeshData* pm, QWidget* pw) : QDialog(pw), ui(new CDlgEditMeshDataUI)
{
	ui->m_data = pm;
	ui->setup(this);
}

CDlgEditMeshData::~CDlgEditMeshData()
{
	delete ui;
}

void CDlgEditMeshData::accept()
{
	std::vector<double> data;

	int rows = ui->m_table->rowCount();
	int cols = ui->m_table->columnCount();
	int ndata = rows * cols;
	data.assign(ndata, 0.0);

	for (int i=0; i<rows; ++i)
		for (int j = 0; j < cols; ++j)
		{
			double aij = ui->m_table->item(i, j)->text().toDouble();
			data[i * cols + j] = aij;
		}

	ui->m_data->SetData(data);

	QDialog::accept();
}

void CDlgEditMeshData::OnCopyToClipboard()
{
	QString txt;
	int rows = ui->m_table->rowCount();
	int cols = ui->m_table->columnCount();

	for (int i = 0; i < rows; ++i)
	{
		QString label = ui->m_table->verticalHeaderItem(i)->text();
		txt += label + "\t";
		for (int j = 0; j < cols; ++j)
		{
			double aij = ui->m_table->item(i, j)->text().toDouble();
			txt += QString::number(aij);
			if (j != cols - 1) txt += "\t";
		}
		txt += "\n";
	}
	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(txt);
}

void CDlgEditMeshData::OnPasteFromClipboard()
{
	QClipboard* clipboard = QApplication::clipboard();
	if (clipboard == nullptr) return;
	const QMimeData* mimeData = clipboard->mimeData();
	if (mimeData == nullptr) return;

	if (mimeData->hasText())
	{
		QString text = clipboard->text();

		int nrows = ui->m_table->rowCount();
		int ncols = ui->m_table->columnCount();

		QStringList lines = text.split("\n");
		int lineCount = lines.size();
		if (lineCount > nrows) lineCount = nrows;
		QString sep = "\t";

		for (int i = 0; i < lineCount; ++i)
		{
			QStringList items = lines[i].split(sep);
			int itemCount = items.size();
			if (itemCount > ncols) itemCount = ncols;
			for (int j = 0; j < itemCount; ++j)
			{
				QString s = items[j];
				double aij = s.toDouble();
				ui->m_table->item(i, j)->setText(QString::number(aij));
			}
		}
	}
}
