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

#include <QCoreApplication>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTextBrowser>
#include <QDesktopServices>
#include <QMessageBox>
#include <FEMLib/FSProject.h>
#include "HelpDialog.h"
#include "WebDefines.h"
#include "FEBioStudio.h"
#include "MainWindow.h"
#include <FEBioLink/FEBioModule.h>
#include <FEBioLib/version.h>

class Ui::CHelpDialog
{
public:
	QPushButton* helpButton;
	QHBoxLayout* helpLayout;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout;
		helpLayout = new QHBoxLayout;

		mainLayout->addLayout(helpLayout);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        helpButton = new QPushButton("Help");
        helpButton->setCheckable(true);

        bb->addButton(helpButton, QDialogButtonBox::HelpRole);

		mainLayout->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(bb, SIGNAL(helpRequested()), parent, SLOT(on_help_clicked()));

		parent->setLayout(mainLayout);
	}
};


CHelpDialog::CHelpDialog(QWidget* parent) : QDialog(parent), ui(new Ui::CHelpDialog), m_url(UNSELECTED_HELP)
{
	ui->setupUi(this);

	m_module = FEBio::GetActiveModule();
}

CHelpDialog::~CHelpDialog() { delete ui; }

void CHelpDialog::on_help_clicked()
{
    SetURL();

    if(m_url.isEmpty())
    {
        QMessageBox::information(this, "Help", "There is currently no help article available for this item.");
    }
    else if(m_url == UNSELECTED_HELP)
    {
        QMessageBox::information(this, "Help", "Please select an item before clicking Help.");
    }
    else
    {
        QString url = QString(MANUAL_PATH) + "%1-%2/%3";
        url = url.arg(VERSION).arg(SUBVERSION).arg(m_url);

        QDesktopServices::openUrl(QUrl(url));
    }
}

void CHelpDialog::SetLeftSideLayout(QLayout* layout)
{
	ui->helpLayout->insertLayout(0, layout);
}