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
#include <QTextEdit>
#include <QTreeWidget>
#include <QToolButton>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include "FEBioStudioProject.h"
#include "IconProvider.h"
#include "FocusWatcher.h"

class Ui::CExportProjectWidget
{
public:
	QTreeWidget* fileTree;
	QPushButton* addFile;
	QTextEdit* description;

	void setupUI(QWidget* parent, FEBioStudioProject* project, bool hasDescription)
	{
		QVBoxLayout* layout = new QVBoxLayout;

		fileTree = new QTreeWidget;
		fileTree->setColumnCount(1);

		fileTree->setHeaderLabel("Files");

		QStringList filePaths = project->GetFilePaths();

		for(auto path : filePaths)
		{
			AddFile(path);
		}

		layout->addWidget(fileTree);

		addFile = new QPushButton("Add Files");
//		addFile->setIcon(CIconProvider::GetIcon("selectAdd"));
//		QToolButton* addButton = new QToolButton;
//		addButton->setDefaultAction(addFile);

		layout->addWidget(addFile);

		if(hasDescription)
		{
			layout->addWidget(new QLabel("Description:"));
			layout->addWidget(description = new QTextEdit);
		}

		parent->setLayout(layout);

	}

	void AddFile(QString& path)
	{
		for(int item = 0; item < fileTree->topLevelItemCount(); item++)
		{
			if(path == fileTree->topLevelItem(item)->text(0)) return;
		}

		QTreeWidgetItem* fileItem = new QTreeWidgetItem(fileTree);
		fileItem->setText(0, path);
		fileItem->setCheckState(0,Qt::Checked);
		fileItem->setData(0, QTreeWidgetItem::UserType, "");

		fileTree->setCurrentItem(fileItem);
	}

};

CExportProjectWidget::CExportProjectWidget(FEBioStudioProject* project, bool description, QWidget* parent)
	: ui(new Ui::CExportProjectWidget)
{
	ui->setupUI(this, project, description);

	QObject::connect(ui->addFile, &QPushButton::clicked, this, &CExportProjectWidget::on_addFile_triggered);
	QObject::connect(ui->fileTree, &QTreeWidget::currentItemChanged, this, &CExportProjectWidget::on_tree_item_changed);
	QObject::connect(new FocusWatcher(ui->description), &FocusWatcher::focusChanged, this, &CExportProjectWidget::descriptionChanged);
}

QStringList CExportProjectWidget::GetFilePaths()
{
	QStringList filePaths;

	for(int item = 0; item < ui->fileTree->topLevelItemCount(); item++)
	{
		if(ui->fileTree->topLevelItem(item)->checkState(0) == Qt::Checked)
		{
			filePaths.append(ui->fileTree->topLevelItem(item)->text(0));
		}
	}

	return filePaths;
}

QStringList CExportProjectWidget::GetLocalFilePaths()
{
	QStringList localPaths;

	for(int item = 0; item < ui->fileTree->topLevelItemCount(); item++)
	{
		if(ui->fileTree->topLevelItem(item)->checkState(0) == Qt::Checked)
		{
			QFileInfo info(ui->fileTree->topLevelItem(item)->text(0));
			QString current = info.fileName();

			int n = 0;
			while(localPaths.contains(current))
			{
				current = info.baseName() + QString(++n) + info.suffix();
			}

			localPaths.append(current);
		}
	}

	return localPaths;
}

QStringList CExportProjectWidget::GetFileDescriptions()
{
	QStringList descriptions;

	for(int item = 0; item < ui->fileTree->topLevelItemCount(); item++)
	{
		if(ui->fileTree->topLevelItem(item)->checkState(0) == Qt::Checked)
		{
			descriptions.append(ui->fileTree->topLevelItem(item)->data(0, QTreeWidgetItem::UserType).toString());
		}
	}

	return descriptions;
}

void CExportProjectWidget::on_addFile_triggered()
{
	QFileDialog dlg(this, "Add File");
	dlg.setFileMode(QFileDialog::ExistingFiles);


	if (dlg.exec())
	{
		QStringList files = dlg.selectedFiles();

		for(auto file : files)
		{
			ui->AddFile(file);
		}
	}
}

void CExportProjectWidget::on_tree_item_changed(QTreeWidgetItem *current)
{
	ui->description->setText(current->data(0, QTreeWidgetItem::UserType).toString());
}

void CExportProjectWidget::descriptionChanged()
{
	ui->fileTree->currentItem()->setData(0, QTreeWidgetItem::UserType, ui->description->toPlainText());
}








