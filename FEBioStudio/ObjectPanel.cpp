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
#include "ObjectPanel.h"
#include <QPushButton>
#include <QGridLayout>
#include <QMenu>
#include <QLineEdit>
#include <QLabel>
#include "MatEditButton.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GLWLib/convert.h>
#include "MainWindow.h"
#include "GLDocument.h"

class CObjectPanelUI
{
public:
	QMenu*			menu;
	QLineEdit*		name;
	QLabel*			type;
	CMatEditButton*	mat;
	CMainWindow*	m_wnd;

public:
	void setup(QWidget* w)
	{
		name = new QLineEdit;
		name->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
		type = new QLabel;
		mat = new CMatEditButton;

		menu = new QMenu(w);
		menu->setObjectName("menu");

		QPushButton* pb = new QPushButton("Convert");
		pb->setMenu(menu);

		QGridLayout* objGrid = new QGridLayout;
		QLabel* nameLabel = new QLabel("Name:"); nameLabel->setBuddy(name);
		QLabel* typeLabel = new QLabel("Type:"); typeLabel->setBuddy(type);

		objGrid->addWidget(nameLabel, 0, 0);
		objGrid->addWidget(name, 0, 1, 1, 2);
		objGrid->addWidget(mat, 0, 3, 2, 2);
		objGrid->addWidget(typeLabel, 1, 0);
		objGrid->addWidget(type, 1, 1);
		objGrid->addWidget(pb, 1, 2);

		w->setLayout(objGrid);

		QObject::connect(mat, SIGNAL(materialChanged(GLMaterial)), w, SLOT(onMaterialChanged(GLMaterial)));
	}
};

CObjectPanel::CObjectPanel(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new CObjectPanelUI)
{
	ui->m_wnd = wnd;
	ui->setup(this);
}

void CObjectPanel::onMaterialChanged(GLMaterial mat)
{
	CGLDocument* doc = ui->m_wnd->GetGLDocument();
	GObject* po = (doc ? doc->GetActiveObject() : nullptr);
	if (po)
	{
		po->SetMaterial(mat);
		ui->m_wnd->RedrawGL();
	}
}

void CObjectPanel::Update()
{
	CGLDocument* doc = ui->m_wnd->GetGLDocument();
	GObject* po = (doc ? doc->GetActiveObject() : nullptr);
	ui->menu->clear();
	if (po)
	{
		if (dynamic_cast<GMeshObject*>(po))
		{
			// mesh objects can be converted to surface objects
			ui->menu->addAction("Editable Surface")->setData(CONVERT_TO_EDITABLE_SURFACE);
			ui->menu->setEnabled(true);
		}
		else if (dynamic_cast<GSurfaceMeshObject*>(po))
		{
			// surface mesh objects can be converted to mesh objects
			ui->menu->addAction("Editable Mesh")->setData(CONVERT_TO_EDITABLE_MESH);
			ui->menu->setEnabled(true);
		}
		else if (po->GetFEMesh())
		{
			// if it has a mesh, we can do either
			ui->menu->addAction("Editable Mesh")->setData(CONVERT_TO_EDITABLE_MESH);
			ui->menu->addAction("Editable Surface")->setData(CONVERT_TO_EDITABLE_SURFACE);

			if (dynamic_cast<GPrimitive*>(po))
			{
				ui->menu->addAction("Multi-block Mesh")->setData(CONVERT_TO_MULTIBLOCK);
			}

			ui->menu->setEnabled(true);
		}
		else if (dynamic_cast<GShellPrimitive*>(po))
		{
			ui->menu->addAction("Multi-patch Mesh")->setData(CONVERT_TO_MULTIPATCH);
			ui->menu->setEnabled(true);
		}
		else if (dynamic_cast<GPrimitive*>(po))
		{
			ui->menu->addAction("Multiblock Mesh")->setData(CONVERT_TO_MULTIBLOCK);
			ui->menu->setEnabled(true);

		}
		else ui->menu->setEnabled(false);

		if (isEnabled() == false) setEnabled(true);

		ui->name->setText(QString::fromStdString(po->GetName()));
		ui->mat->setMaterial(po->GetMaterial());

		std::string stype = doc->GetTypeString(po);
		ui->type->setText(QString::fromStdString(stype));
	}
	else 
	{
		setEnabled(false);
		ui->name->setText("");
		ui->type->setText("");
		ui->mat->setMaterial(GLMaterial());
	}
}
