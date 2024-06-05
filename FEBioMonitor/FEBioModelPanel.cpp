/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2024 University of Utah, The Trustees of Columbia University in
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
#include "FEBioModelPanel.h"
#include <QBoxLayout>
#include <QTreeView>
#include "../FEBioStudio/MainWindow.h"
#include "FEBioMonitorDoc.h"
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QApplication>
#include <QPainter>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include "DlgMatrixInspector.h"
#include <FECore/SparseMatrix.h>

class MyItemDelegate : public QStyledItemDelegate
{
public:
	MyItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		if (index.isValid() == false) return;

		QStyleOptionViewItem& opt = const_cast<QStyleOptionViewItem&>(option);
		initStyleOption(&opt, index);

		// Draw the text of the item
		QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

		// Draw a frame around the text of the item
		painter->save();
		painter->setPen(QPen(Qt::darkGray, 1, Qt::DotLine));
		painter->drawRect(opt.rect);
		painter->restore();
	}
};

class CFEBioDataModel : public QAbstractItemModel
{
public:
	CFEBioDataModel(FEBioMonitorDoc* doc) : m_doc(doc)
	{

	}

public: // overrides from base class
	QModelIndex index(int nrow, int ncol, const QModelIndex& parent) const override 
	{ 
		return createIndex(nrow, ncol); 
	}

	Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		if (!index.isValid())
			return Qt::NoItemFlags;

		Qt::ItemFlags flags = QAbstractItemModel::flags(index);
		if (index.column() == 0) flags |= Qt::ItemIsEditable;

		return flags;
	}

	QModelIndex parent(const QModelIndex& child) const override 
	{ 
		return QModelIndex(); 
	}

	int rowCount(const QModelIndex& parent) const override 
	{ 
		if (parent.isValid()) return 0;

		int N = m_doc->GetWatchVariables();
		return N + 1; 
	}

	int columnCount(const QModelIndex& parent) const override 
	{ 
		return 2; 
	}

	QVariant data(const QModelIndex& index, int role) const override 
	{
		if (index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole))
		{
			int N = m_doc->GetWatchVariables();
			int row = index.row();
			int col = index.column();
			QString txt;
			if ((row >= 0) && (row < N))
			{
				const FEBioWatchVariable* var = m_doc->GetWatchVariable(index.row());
				if (col == 0) txt = var->name();
				else if (col == 1)
				{
					if (var->type() == FEBioWatchVariable::INVALID) txt = "[invalid]";
					else txt = var->value();
				}
			}
			return txt;
		}
		return QVariant(); 
	}

	bool setData(const QModelIndex& index, const QVariant& value, int role) override
	{
		if (index.isValid() && role == Qt::EditRole)
		{
			int N = m_doc->GetWatchVariables();
			int row = index.row();
			if ((row >= 0) && (row < N))
			{
				m_doc->SetWatchVariable(row, value.toString());
				return true;
			}
			else if (row == N)
			{
				beginInsertRows(QModelIndex(), row, row + 1);
				m_doc->AddWatchVariable(value.toString());
				endInsertRows();
			}
		}
		return false;
	}

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			if (section == 0) return "Name";
			if (section == 1) return "Value";
		}
		return QVariant();
	}

	void update()
	{
		beginResetModel();
		endResetModel();
	}

private:
	FEBioMonitorDoc* m_doc;
};

class Ui::CFEBioModelPanel
{
private:
	QTreeView* m_tree = nullptr;
	CFEBioDataModel* m_data = nullptr;

public:
	void setup(::CFEBioModelPanel* wnd)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0, 0, 0, 0);
		QSplitter* splitter = new QSplitter(Qt::Vertical);
		splitter->addWidget(m_tree = new QTreeView);

		QWidget* bottomPane = new QWidget;
		QVBoxLayout* bottomLayout = new QVBoxLayout;
		QPushButton* pb = new QPushButton("Matrix Inspector ...");
		bottomLayout->addWidget(pb);
		bottomLayout->addStretch();
		bottomPane->setLayout(bottomLayout);

		splitter->addWidget(bottomPane);
		l->addWidget(splitter);
		m_tree->setUniformRowHeights(true);
		m_tree->setItemDelegate(new MyItemDelegate);
		wnd->setLayout(l);

		QObject::connect(pb, &QPushButton::clicked, wnd, &::CFEBioModelPanel::launchMatrixInspector);
	}

	void reset()
	{
		m_tree->setModel(nullptr);
		delete m_data; m_data = nullptr;
	}

	void update()
	{
		if (m_data) m_data->update();
	}

	void setDocument(FEBioMonitorDoc* doc)
	{
		if (doc)
		{
			m_data = new CFEBioDataModel(doc);
			m_tree->setModel(m_data);
		}
		else
		{
			m_data = nullptr;
			m_tree->setModel(nullptr);
		}
	}
};

CFEBioModelPanel::CFEBioModelPanel(CMainWindow* wnd, QWidget* parent) : CWindowPanel(wnd, parent), ui(new Ui::CFEBioModelPanel)
{
	ui->setup(this);
}

FEBioMonitorDoc* CFEBioModelPanel::GetCurrentDocument()
{
	return dynamic_cast<FEBioMonitorDoc*>(GetMainWindow()->GetDocument());
}

void CFEBioModelPanel::Clear()
{
	ui->reset();
	ui->setDocument(nullptr);
}

void CFEBioModelPanel::Update(bool breset)
{
	if (breset)
	{
		ui->reset();

		FEBioMonitorDoc* doc = GetCurrentDocument();
		if (doc) ui->setDocument(doc);
	}
	else
	{
		ui->update();
	}
}

void CFEBioModelPanel::launchMatrixInspector()
{
	FEBioMonitorDoc* doc = GetCurrentDocument();
	if (doc == nullptr) return;
	if (doc->IsPaused() == false) return;

	FEGlobalMatrix* M = doc->GetStiffnessMatrix();
	if (M == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", "No stiffness matrix available.");
		return;
	}

	CDlgMatrixInspector dlg(doc, this);
	dlg.exec();
}
