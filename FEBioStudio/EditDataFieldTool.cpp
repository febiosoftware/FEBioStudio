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
#include "EditDataFieldTool.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include <QMessageBox>
#include <QWidget>
#include <QBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <GeomLib/GObject.h>
#include <MeshLib/FEMesh.h>
#include <MeshLib/FEElementData.h>

class UIEditDataFieldTool : public QWidget
{
public:
	QComboBox* pc;
	QPushButton* pb;
	QLineEdit* pe;

public:
	UIEditDataFieldTool(CEditDataFieldTool* w, CMainWindow* wnd)
	{
		QFormLayout* form = new QFormLayout;

		form->addRow("Datafield:", pc = new QComboBox);
		form->addRow("New Value:", pe = new QLineEdit); pe->setValidator(new QDoubleValidator);
		pb = new QPushButton("Apply");

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		l->addWidget(pb);
		l->addStretch();

		setLayout(l);

		QObject::connect(pb, SIGNAL(clicked()), w, SLOT(OnApply()));
	}
};

CEditDataFieldTool::CEditDataFieldTool(CMainWindow* wnd) : CAbstractTool(wnd, "Edit Data Field")
{
	m_po = nullptr;
	ui = new UIEditDataFieldTool(this, wnd);
}

QWidget* CEditDataFieldTool::createUi()
{
	return ui;
}

void CEditDataFieldTool::Activate()
{
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if (doc == nullptr) return;

	FSModel* fem = doc->GetFSModel();
	if (fem == nullptr) return;

	GObject* po = GetActiveObject();
	if (po == nullptr) return;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	ui->pc->clear();
	for (int i = 0; i < pm->MeshDataFields(); ++i)
	{
		FEMeshData& md = *pm->GetMeshDataField(i);
		ui->pc->addItem(QString::fromStdString(md.GetName()));
	}

	ui->pe->setText(QString::number(0.0));
}

void CEditDataFieldTool::OnApply()
{
	FSMesh* pm = GetActiveMesh();
	if (pm == nullptr) return;

	double v = ui->pe->text().toDouble();

	int n = ui->pc->currentIndex();
	if ((n >= 0) && (n < pm->MeshDataFields()))
	{
		FEPartData* pd = dynamic_cast<FEPartData*>(pm->GetMeshDataField(n));
		if (pd && (pd->GetDataFormat() == FEMeshData::DATA_ITEM) && (pd->GetDataType() == DATA_SCALAR))
		{
			for (int i = 0; i < pm->Elements(); ++i)
			{
				FSElement& el = pm->Element(i);
				if (el.IsSelected())
				{
					int m = pd->GetElementIndex(i);
					if (m >= 0) pd->set(m, v);
				}
			}
		}
		else if (pd && (pd->GetDataFormat() == FEMeshData::DATA_MULT) && (pd->GetDataType() == DATA_SCALAR))
		{
			for (int i = 0; i < pm->Elements(); ++i)
			{
				FSElement& el = pm->Element(i);
				if (el.IsSelected())
				{
					int m = pd->GetElementIndex(i);
					if (m >= 0) {
						for (int j = 0; j < el.Nodes(); ++j)
							pd->SetValue(m, j, v);
					}
				}
			}
		}
		else QMessageBox::critical(GetMainWindow(), "Edit Data", "Failed updating mesh data.");
	}
	else QMessageBox::critical(GetMainWindow(), "Edit Data", "No valid data field selected.");

	GetMainWindow()->UpdateMeshInspector(true);
	GetMainWindow()->RedrawGL();
}
