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
#include "DlgAddMeshData.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <FSCore/FSObject.h>
#include <QCheckBox>

class CDlgAddMeshDataUI
{
public:
	QComboBox*	m_param;
	QComboBox*	m_type;
	QCheckBox*	m_custom;

	struct Item
	{
		const char*	szname;
		const char* szparam;
		Param_Type	type;
	};

	std::vector<Item>	m_item;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* layout = new QVBoxLayout;
		m_param = new QComboBox;

		m_custom = new QCheckBox("custom");

		m_type = new QComboBox;
		m_type->addItem("scalar");
		m_type->addItem("vec3");
		m_type->setDisabled(true);

		QFormLayout* form = new QFormLayout;
		form->addRow("parameter", m_param);
		form->addRow("data type", m_type);

		layout->addLayout(form);
		layout->addWidget(m_custom);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(bb);

		dlg->setLayout(layout);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(m_custom, SIGNAL(clicked(bool)), dlg, SLOT(onCustom()));
		QObject::connect(m_param, SIGNAL(currentIndexChanged(int)), dlg, SLOT(setItem(int)));
	}
};

CDlgAddMeshData::CDlgAddMeshData(FSObject* po, QWidget* pw) : QDialog(pw), ui(new CDlgAddMeshDataUI)
{
	setMinimumWidth(300);

	ui->setup(this);

	int n = po->Parameters();
	for (int i=0; i<n; ++i)
	{
		Param& p = po->GetParam(i);
		Param_Type type = p.GetParamType();
		if      ((type == Param_FLOAT) || (type == Param_VEC3D))
		{
			CDlgAddMeshDataUI::Item item = { p.GetLongName(), p.GetShortName(), type };
			ui->m_item.push_back(item);
			ui->m_param->addItem(p.GetLongName());
		}
	}

	setItem(0);
}

void CDlgAddMeshData::setItem(int n)
{
	if ((n<0) || (n >= (int) ui->m_item.size())) return;

	CDlgAddMeshDataUI::Item item = ui->m_item[n];

	ui->m_param->setEditText(item.szname);
	switch (item.type)
	{
	case Param_FLOAT: ui->m_type->setCurrentIndex(0); break;
	case Param_VEC3D: ui->m_type->setCurrentIndex(1); break;
	}
}

void CDlgAddMeshData::onCustom()
{
	bool b = ui->m_custom->isChecked();
	if (b)
	{
		ui->m_param->setEditable(true);
		ui->m_type->setEnabled(true);
	}	
	else
	{
		ui->m_param->setEditable(false);
		ui->m_type->setEnabled(false);
	}
}

std::string CDlgAddMeshData::GetMapName()
{
	QString txt = ui->m_param->currentText();
	return txt.toStdString();
}

std::string CDlgAddMeshData::GetParamName()
{
	bool b = ui->m_custom->isChecked();
	if (b) return GetMapName();
	else
	{
		int n = ui->m_param->currentIndex();
		return ui->m_item[n].szparam;
	}
}

Param_Type CDlgAddMeshData::GetParamType()
{
	int n = ui->m_type->currentIndex();

	if (n == 0) return Param_FLOAT;
	if (n == 1) return Param_VEC3D;

	return Param_UNDEF;
}
