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

#pragma once
#include "MaterialEditor.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QComboBox>
#include <QLineEdit>
#include "Document.h"
#include <FEMLib/FEMultiMaterial.h>

class MaterialEditorItem : public QTreeWidgetItem
{
public:
	MaterialEditorItem(QTreeWidget* tree) : QTreeWidgetItem(tree), m_mat(0), m_parent(0) { m_nprop = -1; m_nmat = -1; m_matClass = 0; }
	MaterialEditorItem(QTreeWidgetItem* item) : QTreeWidgetItem(item), m_mat(0), m_parent(0) { m_nprop = -1; m_nmat = -1; m_matClass = 0; }

	void SetParentMaterial(FSMaterial* pmat)
	{
		m_parent = pmat;
	}

	void SetPropertyIndex(int index, int nmat = 0) { m_nprop = index; m_nmat = nmat; }

	void SetMaterial(FSMaterial* pmat)
	{ 
		int noldmat = m_nmat;

		if (m_parent)
		{
			assert(m_nprop >= 0);

			if (m_nmat >= 0)
			{
				// This will delete the old material, so no need to do it here
				m_parent->ReplaceProperty(m_nprop, pmat, m_nmat);
			}
			else
			{
				// add a new property
				m_nmat = m_parent->AddProperty(m_nprop, pmat);
			}
		}
		else delete m_mat;
		m_mat = pmat;

		DeleteChildren();

		// add the properties
		if (pmat)
		{
			setText(1, QString(pmat->GetTypeString()));

			for (int i=0; i<pmat->Properties(); ++i)
			{
				FSProperty& mp = pmat->GetProperty(i);
				if (true)//mp.GetFlags() & FSProperty::EDITABLE)
				{
					if (mp.maxSize() == 1)
					{
						MaterialEditorItem* item = new MaterialEditorItem(this);
						item->SetParentMaterial(pmat);
						item->SetClassID(mp.GetPropertyType());
						item->SetPropertyIndex(i);
						item->setText(0, QString::fromStdString(mp.GetName()));
						item->SetMaterial(pmat->GetMaterialProperty(i));
					}
					else
					{
						for (int j = 0; j<mp.Size(); ++j)
						{
							MaterialEditorItem* item = new MaterialEditorItem(this);
							item->SetParentMaterial(pmat);
							item->SetClassID(mp.GetPropertyType());
							item->SetPropertyIndex(i, j);
							item->setText(0, QString::fromStdString(mp.GetName()));
							item->SetMaterial(pmat->GetMaterialProperty(i, j));
						}

						// Add one more so that users can create a new material for variable size properties
						if (mp.maxSize() == FSProperty::NO_FIXED_SIZE)
						{
							MaterialEditorItem* item = new MaterialEditorItem(this);
							item->SetParentMaterial(pmat);
							item->SetClassID(mp.GetPropertyType());
							item->SetPropertyIndex(i, -1);
							item->setText(0, QString::fromStdString(mp.GetName()));
						}
					}
				}
			}
		}
		else
		{
			if (m_parent == nullptr) setText(1, "(click here to choose material)");
		}

		setExpanded(true);

		if (m_parent && (noldmat == -1))
		{
			MaterialEditorItem* mi = dynamic_cast<MaterialEditorItem*>(this->parent());
			assert(mi);

			int index = mi->indexOfChild(this);

			FSProperty& mp = m_parent->GetProperty(m_nprop);
			if (mp.maxSize() == FSProperty::NO_FIXED_SIZE)
			{
				MaterialEditorItem* item = new MaterialEditorItem((QTreeWidgetItem*)0);
				item->SetParentMaterial(m_parent);
				item->SetClassID(mp.GetPropertyType());
				item->SetPropertyIndex(m_nprop, -1);
				item->setText(0, QString::fromStdString(mp.GetName()));
				mi->insertChild(index+1, item);
			}
		}
	}

	void DeleteChildren()
	{
		// delete all children
		QList<QTreeWidgetItem*> children = takeChildren();
		while (children.isEmpty() == false)
		{
			QTreeWidgetItem* child = *children.begin();
			delete child;
			children.removeFirst();
		}
	}

	FSMaterial* GetMaterial() const { return m_mat; }

	void SetClassID(int nclass) { m_matClass = nclass; }
	int GetClassID() const { return m_matClass; }

	FSMaterial* ParentMaterial() { return m_parent; }

private:
	FSMaterial*	m_parent;	// parent material (or zero)
	FSMaterial*	m_mat;		// pointer to material (can be zero!)
	int			m_nprop;	// property index of parent material
	int			m_nmat;		// material index into property's material list
	int			m_matClass;	// material category
};

class Ui::CMaterialEditor
{
public:
    QWidget* mainWidget;
	QVBoxLayout* mainLayout;
	QLineEdit*	name;
	QComboBox*	matClass;
	QTreeWidget* tree;

	QComboBox*	matList;

	FSModel* m_fem;
	GMaterial* mat;	// if nonzero, material that will be edited

public:
	void setupUi(QWidget* parent)
	{
		m_fem = nullptr;
		mat = 0;
		matList = 0;

		name = new QLineEdit;
		name->setPlaceholderText("(Leave blank for default)");

		matClass = new QComboBox;

		QFormLayout* matForm = new QFormLayout;
		matForm->addRow("Name:", name);
		matForm->addRow("Category:", matClass);

		tree = new QTreeWidget;
		tree->setMinimumWidth(400);
		tree->setColumnCount(2);
		QStringList labels;
		labels << "Property" << "Type";
		tree->setHeaderLabels(labels);
		ClearTree();

		QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

		layout->addLayout(matForm);
		layout->addWidget(tree);

        mainWidget = new QWidget;
        mainWidget->setLayout(layout);
	}

	void ClearTree()
	{
		tree->clear();
		MaterialEditorItem* item = new MaterialEditorItem(tree);
		item->setText(0, "Material");
		item->setText(1, "(click here to select material)");
		item->SetClassID(matClass->currentData().toInt());
	}

	MaterialEditorItem* topLevelItem(int n)
	{
		MaterialEditorItem* item = dynamic_cast<MaterialEditorItem*>(tree->topLevelItem(n));
		return item;
	}

	MaterialEditorItem* currentItem()
	{
		MaterialEditorItem* item = dynamic_cast<MaterialEditorItem*>(tree->currentItem());
		return item;
	}

	void addMaterialCategory(const std::string& name, int id)
	{
		matClass->addItem(QString::fromStdString(name), id);
	}

	void setMaterialCategory(int id)
	{
		int n = matClass->findData(id);
		if (n >= 0) 
		{
			matClass->blockSignals(true);
			matClass->setCurrentIndex(n);
			matClass->blockSignals(false);
		}
	}
};
