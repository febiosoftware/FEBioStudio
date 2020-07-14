///*This file is part of the FEBio Studio source code and is licensed under the MIT license
//listed below.
//
//See Copyright-FEBio-Studio.txt for details.
//
//Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
//the City of New York, and others.
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.*/
//
//#include <QVBoxLayout>
//#include <QHBoxLayout>
//#include <QListWidget>
//#include <QLabel>
//#include <QTextEdit>
//#include <QLineEdit>
//#include <QTreeWidget>
//#include <QToolBar>
//#include <QToolButton>
//#include <QPushButton>
//#include <QFileDialog>
//#include <QFileInfo>
//#include "ExportProjectWidget.h"
//#include "FEBioStudioProject.h"
//#include "IconProvider.h"
//#include "FocusWatcher.h"
//
//enum ITEMTYPES {PROJECTITEM = 1001, FOLDERITEM = 1002, FILEITEM = 1003};
//enum DATATYPES {DESCRIPTION = 1001, TAGS = 1002};
//
//class Ui::CExportProjectWidget
//{
//public:
//	QAction* addFolder;
//	QAction* delFolder;
//	QAction* addFiles;
//
//	QTreeWidget* fileTree;
//	QTextEdit* description;
//
//	QListWidget* fileTags;
//	QLineEdit* newFileTag;
//
//	void setupUI(QWidget* parent) //, FEBioStudioProject* project, bool hasDescription)
//	{
//		QVBoxLayout* filePagelayout = new QVBoxLayout;
//
//		QToolBar* toolbar = new QToolBar;
//		toolbar->addAction(addFolder = new QAction(CIconProvider::GetIcon("folder", Emblem::Plus), "New Folder", parent));
//		addFolder->setObjectName("addFolder");
//		toolbar->addAction(delFolder = new QAction(CIconProvider::GetIcon("folder", Emblem::Missing), "Delete Folder", parent));
//		delFolder->setObjectName("delFolder");
//		toolbar->addAction(addFiles = new QAction(CIconProvider::GetIcon("new"), "Add Files", parent));
//		addFiles->setObjectName("addFiles");
//		filePagelayout->addWidget(toolbar);
//
//		fileTree = new QTreeWidget;
//		fileTree->setObjectName("fileTree");
//		fileTree->setColumnCount(2);
//		fileTree->setHeaderLabels(QStringList() << "Project Files" << "Location");
//		fileTree->setDragDropMode(QAbstractItemView::InternalMove);
//		fileTree->setDragEnabled(true);
//		fileTree->setDropIndicatorShown(true);
//		fileTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
//		fileTree->setSelectionMode(QAbstractItemView::ContiguousSelection);
//
//		QTreeWidgetItem* project = new QTreeWidgetItem(PROJECTITEM);
//		project->setFirstColumnSpanned(true);
//		project->setText(0, "Project");
//		project->setIcon(0, CIconProvider::GetIcon("FEBioStudio"));
//		project->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
//		fileTree->addTopLevelItem(project);
//		fileTree->setCurrentItem(project);
//
//		filePagelayout->addWidget(fileTree);
//
//		parent->setLayout(filePagelayout);
//
//	}
//
//	void AddFile(QString& path)
//	{
//		QTreeWidgetItem* fileItem = new QTreeWidgetItem(FILEITEM);
//		fileItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
//		fileItem->setText(0, QFileInfo(path).fileName());
//		fileItem->setCheckState(0,Qt::Checked);
//		fileItem->setData(0, DESCRIPTION, "");
//		fileItem->setData(0, TAGS, QStringList());
//
//		fileItem->setText(1, path);
//
//
//		QTreeWidgetItem* current = fileTree->currentItem();
//		if(current->type() == FILEITEM)
//		{
//			current->parent()->addChild(fileItem);
//		}
//		else
//		{
//			current->addChild(fileItem);
//		}
//
//		fileTree->setCurrentItem(fileItem);
//	}
//};
//
////CExportProjectWidget::CExportProjectWidget(FEBioStudioProject* project, bool description, QWidget* parent)
//CExportProjectWidget::CExportProjectWidget(QWidget* parent)
//	: ui(new Ui::CExportProjectWidget)
//{
//	ui->setupUI(this); //, project, description);
//
////	QObject::connect(ui->newFolder, &QAction::triggered, this, &CExportProjectWidget::on_addFolder_triggered);
////	QObject::connect(ui->delFolder, &QAction::triggered, this, &CExportProjectWidget::on_delFolder_triggered);
////	QObject::connect(ui->addFiles, &QAction::triggered, this, &CExportProjectWidget::on_addFiles_triggered);
////	QObject::connect(ui->fileTree, &QTreeWidget::currentItemChanged, this, &CExportProjectWidget::on_fileTree_currentItemChanged);
////	QObject::connect(ui->fileTree, &QTreeWidget::itemChanged, this, &CExportProjectWidget::on_fileTree_itemChanged);
////	QObject::connect(ui->fileTree, &QTreeWidget::itemDoubleClicked, this, &CExportProjectWidget::on_fileTree_ItemDoubleClicked);
//	QObject::connect(new FocusWatcher(ui->description), &FocusWatcher::focusChanged, this, &CExportProjectWidget::descriptionChanged);
//
//	QMetaObject::connectSlotsByName(this);
//}
//
//QStringList CExportProjectWidget::GetFilePaths()
//{
//	QStringList filePaths;
//
//	for(int item = 0; item < ui->fileTree->topLevelItemCount(); item++)
//	{
//		if(ui->fileTree->topLevelItem(item)->checkState(0) == Qt::Checked)
//		{
//			filePaths.append(ui->fileTree->topLevelItem(item)->text(1));
//		}
//	}
//
//	return filePaths;
//}
//
//QStringList CExportProjectWidget::GetLocalFilePaths()
//{
//	QStringList localPaths;
//
//	for(int item = 0; item < ui->fileTree->topLevelItemCount(); item++)
//	{
//		if(ui->fileTree->topLevelItem(item)->checkState(0) == Qt::Checked)
//		{
//			QFileInfo info(ui->fileTree->topLevelItem(item)->text(0));
//			QString current = info.fileName();
//
//			int n = 0;
//			while(localPaths.contains(current))
//			{
//				current = info.baseName() + QString(++n) + info.suffix();
//			}
//
//			localPaths.append(current);
//		}
//	}
//
//	return localPaths;
//}
//
//QStringList CExportProjectWidget::GetFileDescriptions()
//{
//	QStringList descriptions;
//
//	for(int item = 0; item < ui->fileTree->topLevelItemCount(); item++)
//	{
//		if(ui->fileTree->topLevelItem(item)->checkState(0) == Qt::Checked)
//		{
//			descriptions.append(ui->fileTree->topLevelItem(item)->data(0, DESCRIPTION).toString());
//		}
//	}
//
//	return descriptions;
//}
//
//void CExportProjectWidget::on_addFolder_triggered()
//{
//	QTreeWidgetItem* child = new QTreeWidgetItem(FOLDERITEM);
//	child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
//	child->setFirstColumnSpanned(true);
//	child->setText(0, "New Folder");
//	child->setIcon(0, CIconProvider::GetIcon("folder"));
//
//	QTreeWidgetItem* parent;
//	if(ui->fileTree->currentItem()->type() == FILEITEM)
//	{
//		parent = ui->fileTree->currentItem()->parent();
//	}
//	else
//	{
//		parent = ui->fileTree->currentItem();
//	}
//
//	parent->addChild(child);
//	ui->fileTree->setCurrentItem(child);
//	ui->fileTree->editItem(child);
//}
//
//void CExportProjectWidget::on_delFolder_triggered()
//{
//	QTreeWidgetItem* current = ui->fileTree->currentItem();
//
//	if(current->type() == FOLDERITEM)
//	{
//		QList<QTreeWidgetItem*> children = current->takeChildren();
//
//		current->parent()->addChildren(children);
//
//		delete current;
//	}
//}
//
//void CExportProjectWidget::on_addFiles_triggered()
//{
//	QFileDialog dlg(this, "Add File");
//	dlg.setFileMode(QFileDialog::ExistingFiles);
//
//
//	if (dlg.exec())
//	{
//		QStringList files = dlg.selectedFiles();
//
//		for(auto file : files)
//		{
//			ui->AddFile(file);
//		}
//	}
//}
//
//void CExportProjectWidget::on_fileTree_currentItemChanged(QTreeWidgetItem *current)
//{
//	if(current->type() == FILEITEM)
//	{
//		ui->description->setText(current->data(0, DESCRIPTION).toString());
//
//		ui->fileTags->clear();
//		for(auto tag : current->data(0, TAGS).toStringList())
//		{
//			ui->fileTags->addItem(tag);
//		}
//	}
//
//}
//
//void CExportProjectWidget::on_fileTree_itemChanged(QTreeWidgetItem *item, int column)
//{
//	if(item->type() == FILEITEM)
//	{
//		QString extension = QFileInfo(item->text(1)).completeSuffix();
//
//		item->setText(0, QFileInfo(item->text(0)).baseName().append(".").append(extension));
//	}
//}
//
//void CExportProjectWidget::on_fileTree_itemDoubleClicked(QTreeWidgetItem * item, int column)
//{
//	if(column == 0)
//	{
//		ui->fileTree->editItem(item, column);
//	}
//}
//
//void CExportProjectWidget::descriptionChanged()
//{
//	for(auto item : ui->fileTree->selectedItems())
//	{
//		item->setData(0, DESCRIPTION, ui->description->toPlainText());
//	}
//}
//
//void CExportProjectWidget::on_addTagBtn_clicked()
//{
//	QString tag = ui->newFileTag->text();
//
//	if(!tag.isEmpty())
//	{
//		ui->fileTags->addItem(tag);
//
//		for(auto item : ui->fileTree->selectedItems())
//		{
//			item->setData(0, TAGS, item->data(0, TAGS).toStringList() << tag);
//		}
//
//	}
//
//	ui->newFileTag->clear();
//}
//
//void CExportProjectWidget::on_delTagBtn_clicked()
//{
//	for(auto item : ui->fileTags->selectedItems())
//	{
//		QString tag = item->text();
//
//		for(auto treeItem : ui->fileTree->selectedItems())
//		{
//			QStringList tags = treeItem->data(0, TAGS).toStringList();
//
//			tags.removeAll(tag);
//
//			treeItem->setData(0, TAGS, tags);
//		}
//
//		delete item;
//	}
//}
//
//
//
//
//
//
