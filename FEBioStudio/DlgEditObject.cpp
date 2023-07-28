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
#include "DlgEditObject.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include "PropertyListForm.h"
#include "ObjectProps.h"

class Ui::CDlgEditObject
{
public:
	CPropertyListForm* m_form;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* layout = new QVBoxLayout;

		m_form = new CPropertyListForm;
		layout->addWidget(m_form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(bb);

		parent->setLayout(layout);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgEditObject::CDlgEditObject(FSBase* po, QString dlgTitle, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgEditObject)
{
	if (dlgTitle.isEmpty() == false) setWindowTitle(dlgTitle);

	m_po = po;
	ui->setupUi(this);
	ui->m_form->setPropertyList(new CObjectProps(m_po));
}

void CDlgEditObject::accept()
{
	QDialog::accept();
}
