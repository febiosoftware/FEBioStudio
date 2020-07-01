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

#include "DlgSetRepoFolder.h"
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QAction>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>

class Ui::CDlgSetRepoFolder
{
public:
	QLabel* label;
	QLineEdit* repoPath;
	QAction* openFileDialog;

	void setup(QString& defaultPath, QDialog* dlg)
	{
		QVBoxLayout* layout = new QVBoxLayout;

		layout->addWidget(label = new QLabel("Please select a location for FEBio Studio to store repository files."));

		QHBoxLayout* hLayout = new QHBoxLayout;
//		hLayout->addWidget(new QLabel("Repository Path:"));
		hLayout->addWidget(repoPath = new QLineEdit(defaultPath));

		openFileDialog = new QAction;
		openFileDialog->setObjectName("openFileDialog");
		openFileDialog->setIcon(QIcon(":/icons/open.png"));

		QToolButton* button = new QToolButton;
		button->setDefaultAction(openFileDialog);
		hLayout->addWidget(button);

		layout->addLayout(hLayout);

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                | QDialogButtonBox::Cancel);

		layout->addWidget(buttonBox);

		dlg->setLayout(layout);

		QObject::connect(openFileDialog, SIGNAL(triggered()), dlg, SLOT(on_openFileDialog_triggered()));
		QObject::connect(buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()));



	}


};


CDlgSetRepoFolder::CDlgSetRepoFolder(QString defaultPath, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgSetRepoFolder)
{
	ui->setup(defaultPath, this);
}

QString CDlgSetRepoFolder::GetRepoFolder()
{
	return ui->repoPath->text();
}

void CDlgSetRepoFolder::on_openFileDialog_triggered()
{
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::Directory);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setDirectory(ui->repoPath->text());

	if(dlg.exec())
	{
		QStringList files = dlg.selectedFiles();
		QString fileName = files.first();

		ui->repoPath->setText(fileName);
	}

}

void CDlgSetRepoFolder::accept()
{
	QString path = ui->repoPath->text();

	if (path.isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "You must enter a valid directory.");
		return;
	}

	QDir dir(path);

	if(!dir.isEmpty())
	{
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, "FEBio Studio", "This directory is not empty.\nAre you sure that you'd like to use this directory?.",
				QMessageBox::Yes|QMessageBox::No);

		if(!(reply == QMessageBox::Yes)) return;
	}


	if (!dir.exists())
	{
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, "FEBio Studio", "The directory you specified does not exist. Create it?",
				QMessageBox::Yes|QMessageBox::No);

		if(reply == QMessageBox::Yes) dir.mkpath(path);
		else return;
	}


	QDialog::accept();
}













