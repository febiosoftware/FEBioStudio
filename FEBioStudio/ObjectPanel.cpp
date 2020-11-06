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
#include "ObjectPanel.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QFormLayout>
#include <QMenu>
#include "CColorButton.h"
#include <GeomLib/GObject.h>
#include <GLWLib/convert.h>
#include "MainWindow.h"
#include "Document.h"

CObjectPanel::CObjectPanel(CMainWindow* wnd, QWidget* parent) : QWidget(parent)
{
	name = new QLineEdit;
	name->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
	type = new QLabel;
	color = new CColorButton;

	m_wnd = wnd;

	QMenu* menu = new QMenu(this);
	menu->setObjectName("menu");
	QAction* convertAction1 = menu->addAction("Editable Surface");
	QAction* convertAction2 = menu->addAction("Editable Mesh");
	convertAction1->setObjectName("convert1");
	convertAction2->setObjectName("convert2");

	QPushButton* pb = new QPushButton("Convert");
	pb->setMenu(menu);

	QGridLayout* objGrid = new QGridLayout;
	QLabel* nameLabel = new QLabel("Name:"); nameLabel->setBuddy(name);
	QLabel* typeLabel = new QLabel("Type:"); typeLabel->setBuddy(type);

	objGrid->addWidget(nameLabel, 0, 0);
	objGrid->addWidget(name, 0, 1, 1, 2);
	objGrid->addWidget(color, 0, 3);
	objGrid->addWidget(typeLabel, 1, 0);
	objGrid->addWidget(type, 1, 1);
	objGrid->addWidget(pb, 1, 2);
	setLayout(objGrid);

	QObject::connect(color, SIGNAL(colorChanged(QColor)), this, SLOT(onColorChanged(QColor)));
}

void CObjectPanel::onColorChanged(QColor c)
{
	CGLDocument* doc = m_wnd->GetGLDocument();
	GObject* po = (doc ? doc->GetActiveObject() : nullptr);
	if (po)
	{
		po->SetColor(toGLColor(c));
		m_wnd->RedrawGL();
	}
}

void CObjectPanel::Update()
{
	CGLDocument* doc = m_wnd->GetGLDocument();
	GObject* po = (doc ? doc->GetActiveObject() : nullptr);
	if (po)
	{
		if (isEnabled() == false) setEnabled(true);

		name->setText(QString::fromStdString(po->GetName()));
		color->setColor(toQColor(po->GetColor()));

		std::string stype = doc->GetTypeString(po);
		type->setText(QString::fromStdString(stype));
	}
	else 
	{
		setEnabled(false);
		name->setText("");
		type->setText("");
		color->setColor(QColor(0,0,0));
	}
}
