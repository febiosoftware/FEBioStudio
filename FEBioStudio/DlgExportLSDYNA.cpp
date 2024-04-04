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
#include "DlgExportLSDYNA.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QMessageBox>
#include "DataFieldSelector.h"
using namespace Post;

class CDlgExportLSDYNAPlot_UI
{
public:
	QCheckBox*			m_flag[6];
	CDataFieldSelector*	m_code[6];
	FEPostModel*		m_fem;

public:
	void setup(QDialog* dlg)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow(m_flag[0] = new QCheckBox("stress"        ), m_code[0] = new CDataFieldSelector); m_code[0]->BuildMenu(m_fem, Data_Tensor_Type::TENSOR_TENSOR2);
		form->addRow(m_flag[1] = new QCheckBox("plastic strain"), m_code[1] = new CDataFieldSelector); m_code[1]->BuildMenu(m_fem, Data_Tensor_Type::TENSOR_SCALAR);
		form->addRow(m_flag[2] = new QCheckBox("displacement"  ), m_code[2] = new CDataFieldSelector); m_code[2]->BuildMenu(m_fem, Data_Tensor_Type::TENSOR_VECTOR);
		form->addRow(m_flag[3] = new QCheckBox("velocity"      ), m_code[3] = new CDataFieldSelector); m_code[3]->BuildMenu(m_fem, Data_Tensor_Type::TENSOR_VECTOR);
		form->addRow(m_flag[4] = new QCheckBox("acceleration"  ), m_code[4] = new CDataFieldSelector); m_code[4]->BuildMenu(m_fem, Data_Tensor_Type::TENSOR_VECTOR);
		form->addRow(m_flag[5] = new QCheckBox("temperature"   ), m_code[5] = new CDataFieldSelector); m_code[5]->BuildMenu(m_fem, Data_Tensor_Type::TENSOR_SCALAR);

		if (m_code[0]->setCurrentValue("stress"        )) m_flag[0]->setChecked(true);
		if (m_code[1]->setCurrentValue("plastic strain")) m_flag[1]->setChecked(true);
		if (m_code[2]->setCurrentValue("displacement"  )) m_flag[2]->setChecked(true);
		if (m_code[3]->setCurrentValue("velocity"      )) m_flag[3]->setChecked(true);
		if (m_code[4]->setCurrentValue("acceleration"  )) m_flag[4]->setChecked(true);
		if (m_code[5]->setCurrentValue("temperature"   )) m_flag[5]->setChecked(true);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgExportLSDYNAPlot::CDlgExportLSDYNAPlot(FEPostModel* fem, QWidget* parent) : QDialog(parent), ui(new CDlgExportLSDYNAPlot_UI)
{
	for (int i = 0; i < 6; ++i)
	{
		m_flag[i] = false;
		m_code[i] = -1;
	}

	ui->m_fem = fem;
	ui->setup(this);
}

void CDlgExportLSDYNAPlot::accept()
{
	for (int i = 0; i < 6; ++i)
	{
		m_flag[i] = ui->m_flag[i]->isChecked();
		m_code[i] = ui->m_code[i]->currentValue();
		if (m_flag[i] && (m_code[i] == -1))
		{
			QMessageBox::critical(this, "FEBio Studio", QString("Please select a valid field for \"%1\" or uncheck this item").arg(ui->m_flag[i]->text()));
			return;
		}
	}

	QDialog::accept();
}
