/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

CToolBox::CToolBox(QWidget* parent) : QWidget(parent) // QScrollArea(parent)
{
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	mainLayout->setAlignment(Qt::AlignTop);
	setLayout(mainLayout);
/*
	QWidget* dummy = new QWidget;
	dummy->setLayout(mainLayout);
	setWidget(dummy);
	setWidgetResizable(true);
	setFrameShape(QFrame::NoFrame);
*/
}

void CToolBox::addTool(const QString& name, QWidget* tool)
{
//	QLayout* mainLayout = widget()->layout();
	QLayout* mainLayout = layout();
	CToolItem* item = new CToolItem(name, tool);
	mainLayout->addWidget(item);
	m_items.push_back(item);
}

CToolItem* CToolBox::getToolItem(int n)
{
	return m_items.at(n);
}
