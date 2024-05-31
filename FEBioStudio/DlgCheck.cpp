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
#include "DlgCheck.h"
#include <QBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>

class Ui::CDlgCheck
{
public:
	QListWidget* errList;

public:
	void setup(QDialog* dlg, bool askRunQuestion)
	{
		QVBoxLayout* l = new QVBoxLayout;

		errList = new QListWidget;

		QDialogButtonBox* bb = nullptr;
		if (askRunQuestion) bb = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);
		else bb = new QDialogButtonBox(QDialogButtonBox::Close);

		l->addWidget(new QLabel("There are issues with this model that may prevent it from runnnig correctly in FEBio:"));
		l->addWidget(errList);
		if (askRunQuestion)
			l->addWidget(new QLabel("Do you wish to continue?"));
		l->addWidget(bb);
			
		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgCheck::CDlgCheck(QWidget* parent, bool askRunQuestion): QDialog(parent), ui(new Ui::CDlgCheck)
{
	setWindowTitle("Model check");
	ui->setup(this, askRunQuestion);
}

void CDlgCheck::SetWarnings(const std::vector<MODEL_ERROR>& errList)
{
	ui->errList->clear();

	for (size_t i = 0; i < errList.size(); ++i)
	{
		QListWidgetItem* item = new QListWidgetItem;
		item->setText(QString::fromStdString(errList[i].second));
		if (errList[i].first == CRITICAL)
			item->setIcon(QIcon(":/icons/error.png"));
		else
			item->setIcon(QIcon(":/icons/warning.png"));

		ui->errList->addItem(item);
	}
}
