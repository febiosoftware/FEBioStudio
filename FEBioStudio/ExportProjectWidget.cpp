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

#include "ExportProjectWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTreeWidget>
#include <QToolButton>
#include <QAction>
#include <QFileDialog>
#include "FEBioStudioProject.h"
#include "IconProvider.h"

class Ui::CExportProjectWidget
{
public:
	QTreeWidget* fileTree;
	QAction* addFile;
	QAction* removeFile;

	void setupUI(QWidget* parent, FEBioStudioProject* project, bool description)
	{
		QHBoxLayout* layout = new QHBoxLayout;

		fileTree = new QTreeWidget;
		fileTree->setColumnCount(description ? 1 : 2);

		if(description)
		{
			QStringList labels = {"Files", "Description"};
			fileTree->setHeaderLabels(labels);
		}
		else
		{
			fileTree->setHeaderLabel("Files");
		}

//		for(int i = 0; i < project->Files(); i++)
//		{
//			QTreeWidgetItem* fileItem = new QTreeWidgetItem(fileTree);
//			fileItem->setText(0, project->GetFileName(i));
//
//			if(description) fileItem->setText(1, "");
//		}

		layout->addWidget(fileTree);

		QVBoxLayout* buttonLayout = new QVBoxLayout;
		buttonLayout->setAlignment(Qt::AlignTop);

		addFile = new QAction;
		addFile->setIcon(CIconProvider::GetIcon("selectAdd"));
		QToolButton* addButton = new QToolButton;
		addButton->setDefaultAction(addFile);

		buttonLayout->addWidget(addButton);

		removeFile = new QAction;
		removeFile->setIcon(CIconProvider::GetIcon("selectSub"));
		QToolButton* removeButton = new QToolButton;
		removeButton->setDefaultAction(removeFile);

		buttonLayout->addWidget(removeButton);

		layout->addLayout(buttonLayout);

		parent->setLayout(layout);

	}

};

CExportProjectWidget::CExportProjectWidget(FEBioStudioProject* project, bool description, QWidget* parent)
	: ui(new Ui::CExportProjectWidget)
{
	ui->setupUI(this, project, description);

	QObject::connect(ui->addFile, &QAction::triggered, this, &CExportProjectWidget::on_addFile_triggered);
	QObject::connect(ui->removeFile, &QAction::triggered, this, &CExportProjectWidget::on_removeFile_triggered);
}

void CExportProjectWidget::on_addFile_triggered()
{
	QFileDialog dlg(this, "Add File");

	if (dlg.exec())
	{
		QStringList files = dlg.selectedFiles();

		for(auto file : files)
		{
			QTreeWidgetItem* fileItem = new QTreeWidgetItem(ui->fileTree);
			fileItem->setText(0, file);
		}
	}

}

void CExportProjectWidget::on_removeFile_triggered()
{
	for(auto item : ui->fileTree->selectedItems())
	{
		delete item;
	}
}







