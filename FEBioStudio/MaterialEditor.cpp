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
#include "ui_materialeditor.h"
#include <QComboBox>
#include <FEMLib/FEMaterialFactory.h>
#include <FEBioLink/FEBioClass.h>
#include <FEMLib/GMaterial.h>
#include <FEMLib/FSProject.h>
#include <assert.h>

#ifndef WIN32
#define _stricmp strcasecmp
#endif

struct MATERIAL_ENTRY
{
	const char*	mat_name;
	int	mat_typeid;
};

void FillComboBox(QComboBox* pc, int nclass, int module, bool btoplevelonly)
{
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();

	std::vector<MATERIAL_ENTRY> matList;

	FEMatDescIter it = MF.FirstMaterial();
	int NMAT = MF.Materials();
	for (int i = 0; i<NMAT; ++i, ++it)
	{
		unsigned int nflags = (*it)->GetFlags();
		int mod = (*it)->GetModule();

		if (mod & module)
		{
			if ((btoplevelonly==false) || (nflags & MaterialFlags::TOPLEVEL))
			{
				if (nclass < 0x0100)
				{
					if ((*it)->GetClassID() & nclass)
					{
						const char* szname = (*it)->GetTypeString();
						matList.push_back({ szname, (*it)->GetTypeID() });
					}
				}
				else
				{
					if ((*it)->GetClassID() == nclass)
					{
						const char* szname = (*it)->GetTypeString();
						matList.push_back({ szname, (*it)->GetTypeID() });
					}
				}
			}
		}
	}

	// sort the list alphabetically
	std::sort(matList.begin(), matList.end(), [](MATERIAL_ENTRY a, MATERIAL_ENTRY b) { return _stricmp(a.mat_name, b.mat_name)<0; });

	// add it all to the combobox
	for (std::vector<MATERIAL_ENTRY>::iterator it = matList.begin(); it != matList.end(); ++it)
	{
		pc->addItem(it->mat_name, it->mat_typeid);
	}
}

void FillComboBox2(QComboBox* pc, int nclass, int module, bool btoplevelonly)
{
	if (nclass < FE_FEBIO_MATERIAL_CLASS) {
		FillComboBox(pc, nclass, module, btoplevelonly); return;
	}

	nclass -= FE_FEBIO_MATERIAL_CLASS;
	std::vector<FEBio::FEBioClassInfo> classInfo = FEBio::FindAllClasses(-1, -1, nclass, true);

	pc->clear();
	int classes = classInfo.size();
	for (int i = 0; i < classes; ++i)
	{
		FEBio::FEBioClassInfo& ci = classInfo[i];
		pc->addItem(ci.sztype, ci.classId);
	}
	pc->model()->sort(0);
}

void CMaterialEditor::SetMaterial(FSMaterial* mat)
{
	ui->ClearTree();
	MaterialEditorItem* item = ui->topLevelItem(0);
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	int nclass = MF.ClassID(mat);
	item->SetClassID(nclass);
	item->SetMaterial(mat);
	ui->setMaterialCategory(nclass);
}

QString CMaterialEditor::GetMaterialName() const
{
	return ui->name->text();
}

FSMaterial* CMaterialEditor::GetMaterial()
{
	MaterialEditorItem* it = ui->topLevelItem(0);
	return it->GetMaterial();
}

CMaterialEditor::CMaterialEditor(FSProject& prj, QWidget* parent) : CHelpDialog(parent), ui(new Ui::CMaterialEditor)
{
//	setMinimumSize(400, 400);
	setWindowTitle("Add Material");
	ui->setupUi(this);

	ui->m_fem = &prj.GetFSModel();

	SetLeftSideLayout(ui->mainLayout);

	//String to be displayed by the help dialog when no material is selected
	m_unselectedHelp = "Please select a material to view its help page.";

	QObject::connect(ui->matClass, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &CMaterialEditor::on_matClass_currentIndexChanged);
	QObject::connect(ui->tree, &QTreeWidget::currentItemChanged, this, &CMaterialEditor::on_tree_currentItemChanged);
}

void CMaterialEditor::showEvent(QShowEvent* ev)
{
	ui->matClass->clear();

	for (int i=0; i<FEMaterialFactory::Categories(); ++i)
	{
		FEMatCategory& mc = FEMaterialFactory::GetCategory(i);

		if (mc.GetModule() & m_module)
		{
			ui->addMaterialCategory(mc.GetName(), mc.GetID());
		}
	}

	GMaterial* gmat = ui->mat;
	if (gmat)
	{
		setWindowTitle("Change Material");
		ui->name->setText(QString::fromStdString(gmat->GetName()));

		// create a copy of the material (just in case we cancel)
		FSMaterial* pmat = gmat->GetMaterialProperties();
		if (pmat)
		{
			FSMaterial* pmCopy = FEMaterialFactory::Create(gmat->GetModel(), pmat->Type());
			pmCopy->copy(pmat);
			SetMaterial(pmCopy);
			ui->mat = 0;
		}
	}
}

void CMaterialEditor::SetURL()
{
	if(ui->matList)
	{
		if(ui->matList->currentIndex() != -1)
		{
			int ntype = ui->matList->currentData().toInt();
			// m_url = FEMaterialFactory::GetInstance()->Find(ntype)->GetHelpURL();
			return;
		}
	}

	m_url = UNSELECTED_HELP;
}

void CMaterialEditor::SetInitMaterial(GMaterial* pm)
{	
	ui->mat = pm;
}

void CMaterialEditor::on_matClass_currentIndexChanged(int n)
{
	ui->ClearTree();
}

void CMaterialEditor::on_tree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	if (previous)
	{
		ui->tree->removeItemWidget(previous, 1);
		ui->matList = 0;
	}

	if (current)
	{
		// get the material editor item
		MaterialEditorItem* item = dynamic_cast<MaterialEditorItem*>(current);
		assert(item);

		// get the class ID
		int nclass = item->GetClassID();

		ui->matList = new QComboBox;
		FillComboBox(ui->matList, nclass, m_module, (item->ParentMaterial() == 0));

		int index = -1;
		FSMaterial* pm = item->GetMaterial();
		if (pm)
		{
			index = ui->matList->findData(pm->Type());
			assert(index != -1);
		}
		ui->matList->setCurrentIndex(index);

		QObject::connect(ui->matList, SIGNAL(currentIndexChanged(int)), this, SLOT(materialChanged(int)));
		ui->tree->setItemWidget(current, 1, ui->matList);
	}
}

void CMaterialEditor::materialChanged(int n)
{
	FSModel* fem = ui->m_fem;
	
	MaterialEditorItem* it = ui->currentItem();
	int ntype = ui->matList->itemData(n).toInt();

	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	FSMaterial* pmat = MF.Create(fem, ntype); assert(pmat);

	it->SetMaterial(pmat);
}

void CMaterialEditor::accept()
{
	QDialog::accept();
}
