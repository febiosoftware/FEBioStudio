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

#include <WzdUpload.h>
#include "stdafx.h"
#include <QWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFrame>
#include <QAction>
#include <QToolBar>
#include <QLineEdit>
#include <QComboBox>
#include <QCompleter>
#include <QPlainTextEdit>
#include <QFormLayout>
#include <QFileDialog>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QListWidget>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QFileInfo>
#include <QLocale>
#include "IconProvider.h"
#include "FocusWatcher.h"
#include "PublicationWidgetView.h"
//#include "ExportProjectWidget.h"
#include "LocalDatabaseHandler.h"
#include "RepoConnectionHandler.h"

//ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
//    : QLabel(parent) {
//
//}
//
//ClickableLabel::~ClickableLabel() {}
//
//void ClickableLabel::mousePressEvent(QMouseEvent* event) {
//    emit clicked();
//}
//
//
//TagLabel2::TagLabel2(QString text, QWidget* parent)
//	: QFrame(parent)
//{
//	QHBoxLayout* layout = new QHBoxLayout;
//	layout->setContentsMargins(3, 0, 3, 0);
//	layout->setAlignment(Qt::AlignLeft);
//
//	layout->addWidget(label = new QLabel(text));
//
//	remove = new ClickableLabel;
//	remove->setText("x");
//
//	layout->addWidget(remove);
//	layout->setSizeConstraint(QLayout::SetFixedSize);
//
//	setLayout(layout);
//	setFrameStyle(QFrame::Box);
//
////
////	setStyleSheet("background-color : white; border: black;");
//
//
//	QObject::connect(remove, SIGNAL(clicked()), this, SLOT(deleteThis()));
//}
//
//void TagLabel2::deleteThis()
//{
//	delete this;
//}

enum ITEMTYPES {PROJECTITEM = 1001, FOLDERITEM = 1002, FILEITEM = 1003};
enum DATATYPES {DESCRIPTION = 1001, TAGS = 1002};


class Ui::CWzdUpload
{
public:
	QWizardPage* infoPage;
	QLineEdit* name;
	QPlainTextEdit* description;
	QLabel* owner;
	QLabel* version;
	QLabel* categoryLabel;
	QComboBox* categoryBox;

	QLineEdit* newTag;
	QListWidget* tags;
	::CPublicationWidgetView* pubs;

	QWizardPage* filesPage;

	QAction* addFolder;
//	QAction* delFolder;
	QAction* addFiles;
	QAction* rename;

	QTreeWidget* fileTree;
	QTreeWidgetItem* projectItem;
	QLabel* fileDescriptionLabel;
	QPlainTextEdit* fileDescription;

	QLabel* fileTagsLabel;
	QListWidget* fileTags;
	QLineEdit* newFileTag;
	QToolButton* addFileTagBtn;
	QToolButton* delFileTagBtn;

public:
	void setup(QWizard* wzd, int uploadPermissions) //, FEBioStudioProject* project)
	{
		// Project info page
		infoPage = new QWizardPage;
		QHBoxLayout* infoLayout = new QHBoxLayout;

		QFormLayout* leftLayout = new QFormLayout;
		leftLayout->addRow("Project Name: ", name = new QLineEdit);
		leftLayout->addRow("Description: ", description = new QPlainTextEdit);

		if(uploadPermissions == 1)
		{
			leftLayout->addRow("Category: ", categoryLabel = new QLabel);
			categoryBox = NULL;
		}
		else
		{
			leftLayout->addRow("Category: ", categoryBox = new QComboBox);
			categoryLabel = NULL;
		}

		leftLayout->addRow("Owner: ", owner = new QLabel);
		leftLayout->addRow("Version: ", version = new QLabel);

		infoLayout->addLayout(leftLayout);

		QVBoxLayout* rightLayout = new QVBoxLayout;


		QHBoxLayout* tagsLayout = new QHBoxLayout;
		QVBoxLayout* tagsV1 = new QVBoxLayout;
		QVBoxLayout* tagsV2 = new QVBoxLayout;
		tagsV2->setAlignment(Qt::AlignTop);

		QHBoxLayout* tagsH1 = new QHBoxLayout;
		tagsH1->addWidget(new QLabel("Tags: "));
		tagsH1->addWidget(newTag = new QLineEdit);
		tagsV1->addLayout(tagsH1);

		tags = new QListWidget;
		tags->setSelectionMode(QAbstractItemView::ExtendedSelection);
		tagsV1->addWidget(tags);

		QAction* addTag = new QAction;
		addTag->setIcon(QIcon(":/icons/selectAdd.png"));
		QToolButton* addTagBtn = new QToolButton;
		addTagBtn->setDefaultAction(addTag);
		addTagBtn->setObjectName("addTagBtn");
		tagsV2->addWidget(addTagBtn);

		QAction* delTag= new QAction;
		delTag->setIcon(QIcon(":/icons/selectSub.png"));
		QToolButton* delTagBtn = new QToolButton;
		delTagBtn->setDefaultAction(delTag);
		delTagBtn->setObjectName("delTagBtn");
		tagsV2->addWidget(delTagBtn);

		tagsLayout->addLayout(tagsV1);
		tagsLayout->addLayout(tagsV2);

		rightLayout->addLayout(tagsLayout);

		rightLayout->addWidget(new QLabel("Publications:"));

		pubs = new ::CPublicationWidgetView(::CPublicationWidgetView::EDITABLE);
		rightLayout->addWidget(pubs);

		infoLayout->addLayout(rightLayout);
		infoPage->setLayout(infoLayout);
		wzd->addPage(infoPage);

		// Files page
		filesPage = new QWizardPage;
		QVBoxLayout* filesLayout = new QVBoxLayout;

		QToolBar* toolbar = new QToolBar;
		toolbar->addAction(addFolder = new QAction(CIconProvider::GetIcon("folder", Emblem::Plus), "New Folder", wzd));
		addFolder->setObjectName("addFolder");
//		toolbar->addAction(delFolder = new QAction(CIconProvider::GetIcon("folder", Emblem::Missing), "Delete Folder", wzd));
//		delFolder->setObjectName("delFolder");
		toolbar->addAction(addFiles = new QAction(CIconProvider::GetIcon("new"), "Add Files", wzd));
		addFiles->setObjectName("addFiles");
		toolbar->addAction(rename = new QAction(CIconProvider::GetIcon("rename"), "Rename", wzd));
		rename->setObjectName("rename");
		filesLayout->addWidget(toolbar);

		fileTree = new QTreeWidget;
		fileTree->setObjectName("fileTree");
		fileTree->setColumnCount(2);
		fileTree->setHeaderLabels(QStringList() << "Project Files" << "Location");
		fileTree->setDragDropMode(QAbstractItemView::InternalMove);
		fileTree->setDragEnabled(true);
		fileTree->setDropIndicatorShown(true);
		fileTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
		fileTree->setSelectionMode(QAbstractItemView::ContiguousSelection);

		// Prevents items from being added to the root level
		fileTree->invisibleRootItem()->setFlags( fileTree->invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled );

		projectItem = new QTreeWidgetItem(PROJECTITEM);
		projectItem->setFirstColumnSpanned(true);
		projectItem->setText(0, "Project");
		projectItem->setIcon(0, CIconProvider::GetIcon("FEBioStudio"));
		projectItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
		fileTree->addTopLevelItem(projectItem);
		fileTree->setCurrentItem(projectItem);

		filesLayout->addWidget(fileTree, 1);

		QHBoxLayout* filesSubLayout = new QHBoxLayout;
		QVBoxLayout* descLayout = new QVBoxLayout;

		descLayout->addWidget(fileDescriptionLabel = new QLabel("Description:"));
		descLayout->addWidget(fileDescription = new QPlainTextEdit);
		filesSubLayout->addLayout(descLayout);

		QHBoxLayout* fileTagsLayout = new QHBoxLayout;
		QVBoxLayout* fileTagsV1 = new QVBoxLayout;
		QVBoxLayout* fileTagsV2 = new QVBoxLayout;
		fileTagsV2->setAlignment(Qt::AlignTop);

		QHBoxLayout* fileTagsH1 = new QHBoxLayout;
		fileTagsH1->addWidget(fileTagsLabel = new QLabel("Tags: "));
		fileTagsH1->addWidget(newFileTag = new QLineEdit);

		fileTagsV1->addLayout(fileTagsH1);

		fileTags = new QListWidget;
		fileTags->setSelectionMode(QAbstractItemView::ExtendedSelection);
		fileTagsV1->addWidget(fileTags);

		QAction* addFileTag = new QAction;
		addFileTag->setIcon(QIcon(":/icons/selectAdd.png"));
		addFileTagBtn = new QToolButton;
		addFileTagBtn->setDefaultAction(addFileTag);
		addFileTagBtn->setObjectName("addFileTagBtn");
		fileTagsV2->addWidget(addFileTagBtn);

		QAction* delFileTag= new QAction;
		delFileTag->setIcon(QIcon(":/icons/selectSub.png"));
		delFileTagBtn = new QToolButton;
		delFileTagBtn->setDefaultAction(delFileTag);
		delFileTagBtn->setObjectName("delFileTagBtn");
		fileTagsV2->addWidget(delFileTagBtn);

		fileTagsLayout->addLayout(fileTagsV1);
		fileTagsLayout->addLayout(fileTagsV2);

		filesSubLayout->addLayout(fileTagsLayout);

		filesLayout->addLayout(filesSubLayout);

		filesPage->setLayout(filesLayout);
		wzd->addPage(filesPage);

		fileInfoEnabled(false);

		wzd->setWindowTitle("Upload Project");
		wzd->resize(800, 600);

		fileTree->setColumnWidth(0, fileTree->width()/2);
	}

	void AddFile(QString& path)
	{
		QTreeWidgetItem* fileItem = new QTreeWidgetItem(FILEITEM);
		fileItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
		fileItem->setText(0, QFileInfo(path).fileName());
		fileItem->setCheckState(0,Qt::Checked);
		fileItem->setData(0, DESCRIPTION, "");
		fileItem->setData(0, TAGS, QStringList());

		fileItem->setText(1, path);

		QTreeWidgetItem* current = fileTree->currentItem();
		if(!current)
		{
			current = projectItem;
		}

		if(current->type() == FILEITEM)
		{
			current->parent()->addChild(fileItem);
		}
		else
		{
			current->addChild(fileItem);
		}

		fileTree->setCurrentItem(fileItem);
	}

	void fileInfoEnabled(bool enabled)
	{
		fileDescriptionLabel->setEnabled(enabled);
		fileDescription->setEnabled(enabled);

		fileTagsLabel->setEnabled(enabled);
		fileTags->setEnabled(enabled);
		newFileTag->setEnabled(enabled);
		addFileTagBtn->setEnabled(enabled);
		delFileTagBtn->setEnabled(enabled);
	}

	bool hasDuplicateNames(QTreeWidgetItem* item)
	{
		for(int child = 0; child < item->childCount(); child++)
		{
			if(item->child(child)->checkState(0) == Qt::Unchecked) continue;

			for(int child2 = 1; child2 < item->childCount(); child2++)
			{
				if(item->child(child) == item->child(child2)) continue;

				if(item->child(child2)->checkState(0) == Qt::Unchecked) continue;

				if(item->child(child)->text(0) == item->child(child2)->text(0))
				{
					return true;
				}
			}

			if(item->child(child)->type() == FOLDERITEM)
			{
				if(hasDuplicateNames(item->child(child))) return true;
			}
		}

		return false;
	}

	void getFileItems(QList<QTreeWidgetItem*>& items, QTreeWidgetItem* item)
	{
		for(int child = 0; child < item->childCount(); child++)
		{
			if(item->child(child)->checkState(0) == Qt::Unchecked) continue;

			if(item->child(child)->type() == FILEITEM)
			{
				items.append(item->child(child));
			}
			else
			{
				getFileItems(items, item->child(child));
			}
		}
	}

	void getFilePaths(QStringList& paths, QTreeWidgetItem* item)
	{
		for(int child = 0; child < item->childCount(); child++)
		{
			if(item->child(child)->checkState(0) == Qt::Unchecked) continue;

			if(item->child(child)->type() == FILEITEM)
			{
				paths.append(item->child(child)->text(1));
			}
			else
			{
				getFilePaths(paths, item->child(child));
			}
		}
	}

//	void getLocalFilePaths(QStringList& paths, QStringList& currentPath, QTreeWidgetItem* item)
//	{
//		for(int child = 0; child < item->childCount(); child++)
//		{
//			if(item->child(child)->type() == FILEITEM)
//			{
//				QString path;
//				for(auto str : currentPath)
//				{
//					path.append(str).append("/");
//				}
//
//				path.append(item->child(child)->text(0));
//
//				paths.append(path);
//			}
//			else if(item->child(child)->type() == FOLDERITEM)
//			{
//				currentPath.append(item->child(child)->text(0));
//
//				getLocalFilePaths(paths, currentPath, item->child(child));
//			}
//			else
//			{
//				getLocalFilePaths(paths, currentPath, item->child(child));
//			}
//		}
//
//		if(!currentPath.isEmpty())
//		{
//			currentPath.removeLast();
//		}
//	}

	void getLocalFilePath(QString& path, QTreeWidgetItem* item)
	{
		if(item->type() == FILEITEM)
		{
			path = item->text(0);

			getLocalFilePath(path, item->parent());
		}
		else if(item->type() == FOLDERITEM)
		{
			path = item->text(0).append("/").append(path);

			getLocalFilePath(path, item->parent());
		}
	}

};

CWzdUpload::CWzdUpload(QWidget* parent, int uploadPermissions, CLocalDatabaseHandler* dbHandler, CRepoConnectionHandler* repoHandler)//, FEBioStudioProject* project)
	: QWizard(parent), ui(new Ui::CWzdUpload), dbHandler(dbHandler), repoHandler(repoHandler)
{
	ui->setup(this, uploadPermissions); //, project);

	QObject::connect(new FocusWatcher(ui->fileDescription), &FocusWatcher::focusChanged, this, &CWzdUpload::fileDescriptionChanged);

	QMetaObject::connectSlotsByName(this);
}

void CWzdUpload::setName(QString name)
{
	ui->name->setText(name);
}

void CWzdUpload::setDescription(QString desc)
{
	ui->description->document()->setPlainText(desc);
}

void CWzdUpload::setCategories(QStringList& categories)
{
	if(ui->categoryLabel)
	{
		ui->categoryLabel->setText(categories[0]);
	}
	else
	{
		ui->categoryBox->addItems(categories);
	}
}

void CWzdUpload::setOwner(QString owner)
{
	ui->owner->setText(owner);
}

void CWzdUpload::setVersion(QString version)
{
	ui->version->setText(version);
}

void CWzdUpload::setTags(QStringList& tags)
{
	for(auto tag : tags)
	{
		ui->tags->addItem(tag);
	}
}

void CWzdUpload::setPublications(const std::vector<CPublicationWidget*>& pubs)
{
	for(auto pub : pubs)
	{
		ui->pubs->addPublicationCopy(*pub);
	}
}

void CWzdUpload::setTagList(QStringList& tags)
{
	if(ui->newTag->completer()) delete ui->newTag->completer();
	if(ui->newFileTag->completer()) delete ui->newFileTag->completer();

	ui->newTag->setCompleter(new QCompleter(tags));
	ui->newTag->completer()->setCaseSensitivity(Qt::CaseInsensitive);

	ui->newFileTag->setCompleter(new QCompleter(tags));
	ui->newFileTag->completer()->setCaseSensitivity(Qt::CaseInsensitive);
}

QString CWzdUpload::getName()
{
	return ui->name->text();
}

QString CWzdUpload::getDescription()
{
	return ui->description->document()->toPlainText();
}

QString CWzdUpload::getCategory()
{
	if(ui->categoryLabel)
	{
		return ui->categoryLabel->text();
	}
	else
	{
		return ui->categoryBox->currentText();
	}
}

QString CWzdUpload::getOwner()
{
	return ui->owner->text();
}

QString CWzdUpload::getVersion()
{
	return ui->version->text();
}


QStringList CWzdUpload::getTags()
{
	QStringList tagList;

	for(int tag = 0; tag < ui->tags->count(); ++tag)
	{
		QString tagText = ui->tags->item(tag)->text().trimmed();

		if(!tagText.isEmpty())
		{
			if(tagList.filter(tagText, Qt::CaseInsensitive).count() == 0)
			{
				tagList.append(tagText);
			}
		}
	}

	return tagList;
}

QList<QVariant> CWzdUpload::getPublicationInfo()
{
	return ui->pubs->getPublicationInfo();
}

QStringList CWzdUpload::GetFilePaths()
{
	QStringList paths;

	ui->getFilePaths(paths, ui->projectItem);

	return paths;
}

QStringList CWzdUpload::GetLocalFilePaths()
{
	QList<QTreeWidgetItem*> items;
	ui->getFileItems(items, ui->projectItem);

	QStringList paths;
	for(auto item : items)
	{
		QString path;
		ui->getLocalFilePath(path, item);
		paths.push_back(path);
	}

	return paths;
}

QList<QVariant> CWzdUpload::getFileInfo()
{
	QList<QVariant> info;
	QList<QTreeWidgetItem*> items;

	ui->getFileItems(items, ui->projectItem);

	for(auto item : items)
	{
		QVariantMap fileInfo;

		QString path;
		ui->getLocalFilePath(path, item);

		fileInfo["filename"] = path;
		fileInfo["description"] = item->data(0, DESCRIPTION);
		fileInfo["tags"] = item->data(0, TAGS);

		info.append(fileInfo);
	}

	return info;
}

void CWzdUpload::accept()
{
	if(ui->name->text().isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please enter a name for your project.");

		while(currentId() != 0)
		{
			back();
		}

		return;
	}

	QString username = getOwner();
	QString name = getName();
	QString category = getCategory();
	if(!dbHandler->isValidUpload(username, name, category))
	{
		QMessageBox::critical(this, "Upload", "You already have a project with that name in this category."
				"\n\nPlease choose a different project name.");

		while(currentId() != 0)
		{
			back();
		}

		return;
	}

	if(ui->description->toPlainText().isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please enter a description for your project.");

		while(currentId() != 0)
		{
			back();
		}

		return;
	}

	if(ui->tags->count() == 0)
	{
		QMessageBox::critical(this, "Upload", "Please add at least one tag to your project.");

		while(currentId() != 0)
		{
			back();
		}

		return;
	}

	if(ui->hasDuplicateNames(ui->projectItem))
	{
		QMessageBox::critical(this, "Upload", "You cannot have to files with the same name in the same folder.");
		return;
	}

	QStringList filePaths;

	ui->getFilePaths(filePaths, ui->projectItem);
	if(filePaths.isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please select at least one file to upload.");
		return;
	}

	qint64 totalSize = 0;
	for(auto path : filePaths)
	{
		QFileInfo info(path);
		totalSize += info.size();
	}

	qint64 currentProjectsSize = dbHandler->currentProjectsSize(repoHandler->getUsername());
	qint64 sizeLimit = repoHandler->getSizeLimit();

	if(totalSize + currentProjectsSize > sizeLimit)
	{
		QLocale locale = this->locale();

		QString message = QString("This upload would exceed your limit of %1 on the repository. Please remove some files "
				"or delete some projects from the repository.\n\n"
				"Current Project Size: %2\n"
				"Total on Repository: %3\n").arg(locale.formattedDataSize(sizeLimit))
									.arg(locale.formattedDataSize(totalSize))
									.arg(locale.formattedDataSize(currentProjectsSize));

		QMessageBox::critical(this, "Upload", message);
		return;
	}

	QList<QTreeWidgetItem*> items;
	ui->getFileItems(items, ui->projectItem);
	for(auto item : items)
	{
		if(item->data(0, DESCRIPTION).toString().isEmpty())
		{
			QMessageBox::StandardButton reply = QMessageBox::question(this, "Upload", "Some of your files are missing descriptions."
					"\n\nWould you like to upload without them?");

			if(reply == QMessageBox::Yes)
			{
				break;
			}
			else
			{
				return;
			}
		}

		if(item->data(0, TAGS).toStringList().isEmpty())
		{
			QMessageBox::StandardButton reply = QMessageBox::question(this, "Upload", "Some of your files do not have any tags."
					"\n\nWould you like to upload without them?");

			if(reply == QMessageBox::Yes)
			{
				break;
			}
			else
			{
				return;
			}
		}
	}

	if(ui->pubs->count() == 0)
	{
		QMessageBox::StandardButton reply = QMessageBox::question(this, "Upload", "You have not associated any publications with your project."
				"\n\nWould you like to upload anyway?");

		if(reply != QMessageBox::Yes)
		{

			while(currentId() != 0)
			{
				back();
			}

			return;
		}
	}

	QWizard::accept();
}

void CWzdUpload::keyPressEvent(QKeyEvent* e)
{
	// Prevent the enter key from finishing the wizard.

	if(e->key() != Qt::Key_Enter)
	{
		QWizard::keyPressEvent(e);
	}
}

void CWzdUpload::on_addTagBtn_clicked()
{
	if(!ui->newTag->text().isEmpty())
	{
		ui->tags->addItem(ui->newTag->text());
	}
	ui->newTag->clear();

}

void CWzdUpload::on_delTagBtn_clicked()
{
	QList<QListWidgetItem*> items = ui->tags->selectedItems();

	for(QListWidgetItem* item : items)
	{
		delete item;
	}
}


void CWzdUpload::on_addFolder_triggered()
{
	QTreeWidgetItem* child = new QTreeWidgetItem(FOLDERITEM);
	child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
	child->setCheckState(0,Qt::Checked);
	child->setFirstColumnSpanned(true);
	child->setText(0, "New Folder");
	child->setIcon(0, CIconProvider::GetIcon("folder"));


	QTreeWidgetItem* current = ui->fileTree->currentItem();

	if(!current)
	{
		current = ui->projectItem;
	}

	QTreeWidgetItem* parent;
	if(current->type() == FILEITEM)
	{
		parent = current->parent();
	}
	else
	{
		parent = current;
	}

	parent->addChild(child);
	ui->fileTree->setCurrentItem(child);
	ui->fileTree->editItem(child);
}

//void CDlgUpload::on_delFolder_triggered()
//{
//	QTreeWidgetItem* current = ui->fileTree->currentItem();
//
//	if(current)
//	{
//		if(current->type() == FOLDERITEM)
//		{
//			QList<QTreeWidgetItem*> children = current->takeChildren();
//
//			current->parent()->addChildren(children);
//
//			delete current;
//		}
//	}
//}

void CWzdUpload::on_addFiles_triggered()
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

void CWzdUpload::on_rename_triggered()
{
	if(ui->fileTree->currentItem())
	{
		ui->fileTree->editItem(ui->fileTree->currentItem(), 0);
	}
}

void CWzdUpload::on_fileTree_currentItemChanged(QTreeWidgetItem *current)
{
	if(current)
	{
		if(current->type() == FILEITEM)
		{
			ui->fileInfoEnabled(true);

			ui->fileDescription->setPlainText(current->data(0, DESCRIPTION).toString());

			ui->fileTags->clear();
			for(auto tag : current->data(0, TAGS).toStringList())
			{
				ui->fileTags->addItem(tag);
			}
		}
		else
		{
			ui->fileDescription->clear();
			ui->fileTags->clear();

			ui->fileInfoEnabled(false);
		}
	}
}

void CWzdUpload::on_fileTree_itemChanged(QTreeWidgetItem *item, int column)
{

	if(item->type() == FILEITEM)
	{
		// Ensure that renaming a file doesn't change its extension
		QString extension = QFileInfo(item->text(1)).completeSuffix();
		item->setText(0, QFileInfo(item->text(0)).baseName().append(".").append(extension));
	}
}

void CWzdUpload::on_fileTree_itemDoubleClicked(QTreeWidgetItem * item, int column)
{
	if(column == 0)
	{
		ui->fileTree->editItem(item, column);
	}
}

void CWzdUpload::fileDescriptionChanged()
{
	for(auto item : ui->fileTree->selectedItems())
	{
		item->setData(0, DESCRIPTION, ui->fileDescription->toPlainText());
	}
}

void CWzdUpload::on_addFileTagBtn_clicked()
{
	QString tag = ui->newFileTag->text();

	if(!tag.isEmpty())
	{
		ui->fileTags->addItem(tag);

		for(auto item : ui->fileTree->selectedItems())
		{
			item->setData(0, TAGS, item->data(0, TAGS).toStringList() << tag);
		}

	}

	ui->newFileTag->clear();
}

void CWzdUpload::on_delFileTagBtn_clicked()
{
	for(auto item : ui->fileTags->selectedItems())
	{
		QString tag = item->text();

		for(auto treeItem : ui->fileTree->selectedItems())
		{
			QStringList tags = treeItem->data(0, TAGS).toStringList();

			tags.removeAll(tag);

			treeItem->setData(0, TAGS, tags);
		}

		delete item;
	}
}


