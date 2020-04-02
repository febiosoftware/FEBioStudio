#include "stdafx.h"
#include "ToolBox.h"
#include <QBoxLayout>
#include <QPushButton>

CToolItem::CToolItem(const QString& name, QWidget* tool, QWidget* parent) : QWidget(parent)
{
	pb = new QPushButton(name);
	pb->setCheckable(true);
	pb->setChecked(true);
		
	QFont font = pb->font();
	font.setBold(true);
	pb->setFont(font);

	QVBoxLayout* l = new QVBoxLayout;
	l->setMargin(0);
	l->setSpacing(0);
	l->addWidget(pb);
	l->addWidget(tool);
	l->setAlignment(Qt::AlignTop);
	setLayout(l);

	QObject::connect(pb, SIGNAL(toggled(bool)), tool, SLOT(setVisible(bool)));
}

void CToolItem::setTitle(const QString& t)
{
	pb->setText(t);
}

CToolBox::CToolBox(QWidget* parent) : QScrollArea(parent)
{
	QWidget* dummy = new QWidget;
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	mainLayout->setAlignment(Qt::AlignTop);
	dummy->setLayout(mainLayout);
	setWidget(dummy);
	setWidgetResizable(true);
	setFrameShape(QFrame::NoFrame);
}

void CToolBox::addTool(const QString& name, QWidget* tool)
{
	QLayout* mainLayout = widget()->layout();
	CToolItem* item = new CToolItem(name, tool);
	mainLayout->addWidget(item);
	m_items.push_back(item);
}

CToolItem* CToolBox::getToolItem(int n)
{
	return m_items.at(n);
}
