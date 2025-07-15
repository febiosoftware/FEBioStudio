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
#include "DlgListMaterials.h"
#include <FEMLib/GMaterial.h>
#include <QListWidget>
#include <QBoxLayout>
#include <QDialogButtonBox>

CDlgListMaterials::CDlgListMaterials(QWidget* parent) : QDialog(parent)
{
	QVBoxLayout* l = new QVBoxLayout;
	
	w = new QListWidget;
	w->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	l->addWidget(w);
	l->addWidget(bb);

	setLayout(l);

	connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void CDlgListMaterials::SetMaterials(const std::vector<GMaterial*>& materialList)
{
	matList = materialList;

	for (size_t n = 0; n < materialList.size(); ++n)
	{
		GMaterial* gm = materialList[n];
		QListWidgetItem* item = new QListWidgetItem();
		item->setText(gm->GetFullName());
		item->setData(Qt::UserRole, (qulonglong)n);
		w->addItem(item);
	}
}

std::vector<GMaterial*> CDlgListMaterials::GetSelectedMaterials()
{
	std::vector<GMaterial*> selList;

	QList<QListWidgetItem*> items = w->selectedItems();
	for (auto item : items)
	{
		int n = item->data(Qt::UserRole).toInt();
		if ((n >= 0) && (n < matList.size()))
		{
			selList.push_back(matList[n]);
		}
	}

	return selList;
}
