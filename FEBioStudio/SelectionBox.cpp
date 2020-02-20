#include "stdafx.h"
#include "SelectionBox.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolButton>
#include <QGridLayout>

class Ui::CSelectionBox
{
public:
	QLineEdit*		name;
	QLabel*			type;
	QListWidget*	list;
	QWidget*		nameType;
	QToolButton*	pb1;
	QToolButton*	pb2;
	QToolButton*	pb3;
	QToolButton*	pb4;

public:
	void setupUi(QWidget* parent)
	{
		nameType = new QWidget;

		QFormLayout* form = new QFormLayout;
		form->addRow("Name:", name = new QLineEdit); name->setObjectName("name");
		form->addRow("Type:", type = new QLabel(""));
		nameType->setLayout(form);

		list = new QListWidget;
		list->setSelectionMode(QAbstractItemView::ExtendedSelection);
		//	list->setFixedHeight(200);

		list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

		const int W = 24;
		pb1 = new QToolButton; pb1->setObjectName("addButton"); pb1->setIcon(QIcon(":/icons/selectAdd.png")); 
		pb1->setAutoRaise(true); pb1->setFixedSize(W, W); pb1->setIconSize(pb1->size());
		pb2 = new QToolButton; pb2->setObjectName("subButton"); pb2->setIcon(QIcon(":/icons/selectSub.png"));
		pb2->setAutoRaise(true); pb2->setFixedSize(W, W); pb2->setIconSize(pb2->size());
		pb3 = new QToolButton; pb3->setObjectName("delButton"); pb3->setIcon(QIcon(":/icons/selectDel.png"));
		pb3->setAutoRaise(true); pb3->setFixedSize(W, W); pb3->setIconSize(pb3->size());
		pb4 = new QToolButton; pb4->setObjectName("selButton"); pb4->setIcon(QIcon(":/icons/select.png"   ));
		pb4->setAutoRaise(true); pb4->setFixedSize(W, W); pb4->setIconSize(pb4->size());

		QVBoxLayout* buttonLayout = new QVBoxLayout;
		buttonLayout->addWidget(pb1);
		buttonLayout->addWidget(pb2);
		buttonLayout->addWidget(pb3);
		buttonLayout->addWidget(pb4);
		buttonLayout->addStretch();
		buttonLayout->setMargin(0);

		QGridLayout* mainLayout = new QGridLayout;
		mainLayout->addWidget(nameType, 0, 0);
		mainLayout->addWidget(list, 1, 0);
		mainLayout->addLayout(buttonLayout, 1, 1);
		mainLayout->setMargin(0);

		parent->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(parent);
	}
};

CSelectionBox::CSelectionBox(QWidget* parent) : QWidget(parent), ui(new Ui::CSelectionBox)
{
	ui->setupUi(this);
}

void CSelectionBox::showNameType(bool b)
{
	ui->nameType->setVisible(b);
}

void CSelectionBox::setName(const QString& name)
{
	ui->name->setText(name);
}

void CSelectionBox::setType(const QString& type)
{
	ui->type->setText(type);
}

void CSelectionBox::enableAddButton(bool b)
{
	ui->pb1->setEnabled(b);
}

void CSelectionBox::enableRemoveButton(bool b)
{
	ui->pb2->setEnabled(b);
}

void CSelectionBox::enableDeleteButton(bool b)
{
	ui->pb3->setEnabled(b);
}

void CSelectionBox::enableSelectButton(bool b)
{
	ui->pb4->setEnabled(b);
}

void CSelectionBox::enableAllButtons(bool b)
{
	enableAddButton(b);
	enableRemoveButton(b);
	enableDeleteButton(b);
	enableSelectButton(b);
}

void CSelectionBox::on_addButton_clicked()
{
	emit addButtonClicked();
}

void CSelectionBox::on_subButton_clicked()
{
	emit subButtonClicked();
}

void CSelectionBox::on_delButton_clicked()
{
	emit delButtonClicked();
}

void CSelectionBox::on_selButton_clicked()
{
	emit selButtonClicked();
}

void CSelectionBox::on_name_textEdited(const QString& t)
{
	emit nameChanged(t);
}

void CSelectionBox::clearData()
{
	ui->list->clear();
	m_data.clear();
}

void CSelectionBox::addData(const QString& item, int data, int fmt)
{
	// make sure the data does not exist yet
	for (int i=0; i<m_data.size(); ++i)
		if (m_data[i] == data)
		{
			return;
		}

	m_data.push_back(data);
	ui->list->addItem(item);

	if (fmt != 0)
	{
		QListWidgetItem* w = ui->list->item(ui->list->count() - 1);
		w->setTextColor(Qt::red);
	}
}

void CSelectionBox::addData(const vector<int>& data)
{
	for (int i=0; i<data.size(); ++i)
	{
		QString t = QString::number(data[i]);
		addData(t, data[i]);
	}
}

void CSelectionBox::removeData(int ndata)
{
	for (int j = 0; j<m_data.size(); ++j)
	{
		if (m_data[j] == ndata)
		{
			m_data.erase(m_data.begin() + j);
			delete ui->list->takeItem(j);
			break;
		}
	}
}

void CSelectionBox::removeData(const vector<int>& data)
{
	for (int i=0; i<data.size(); ++i)
	{
		int ndata = data[i];

		for (int j=0; j<m_data.size(); ++j)
		{
			if (m_data[j] == ndata)
			{
				m_data.erase(m_data.begin() + j);
				delete ui->list->takeItem(j);
				break;
			}
		}
	}
}

void CSelectionBox::getSelectedItems(vector<int>& sel)
{
	QModelIndexList selItems = ui->list->selectionModel()->selectedRows();
	if (selItems.empty() == false)
	{
		for (QModelIndexList::iterator it = selItems.begin(); it != selItems.end(); ++it)
		{
			int row = it->row();
			sel.push_back(m_data[row]);
		}
	}
}

void CSelectionBox::getAllItems(vector<int>& data)
{
	data = m_data;
}

void CSelectionBox::getAllNames(QStringList& names)
{
	names.clear();

	int N = ui->list->count();
	for (int i = 0; i<N; ++i)
	{
		QListWidgetItem* item = ui->list->item(i);
		QString t = item->text();
		names.push_back(t);
	}
}

void CSelectionBox::getSelectedItems(list<int>& sel)
{
	QModelIndexList selItems = ui->list->selectionModel()->selectedRows();
	if (selItems.empty() == false)
	{
		for (QModelIndexList::iterator it = selItems.begin(); it != selItems.end(); ++it)
		{
			int row = it->row();
			sel.push_back(m_data[row]);
		}
	}
}

void CSelectionBox::removeSelectedItems()
{
	int N = ui->list->count();
	for (int i=0; i<N; ++i)
	{
		QListWidgetItem* item = ui->list->item(i);
		if (item->isSelected())
		{
			item = ui->list->takeItem(i);
			delete item;

			m_data.erase(m_data.begin() + i);
			N--;
			i--;
		}
	}
}
