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
#include <QWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPushButton>
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
#include <QFileInfo>
#include <QApplication>
#include <QLocale>
#include <QPalette>
#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <unordered_map>
#include "WzdUpload.h"
#include "IconProvider.h"
#include "FocusWatcher.h"
#include "PublicationWidgetView.h"
#include "ModelDatabaseHandler.h"
#include "ModelRepoConnectionHandler.h"
#include "TagWidget.h"

using std::unordered_map;
using std::out_of_range;

enum ITEMTYPES {PROJECTITEM = 1001, FOLDERITEM = 1002, FILEITEM = 1003};
enum DATATYPES {DESCRIPTION = 1001, TAGS = 1002, SIZE = 1003};

class Ui::CWzdUpload
{
public:
	QWizardPage* infoPage;
	QLineEdit* name;
	QPlainTextEdit* description;
	QLabel* owner;
	QLabel* categoryLabel;
	QComboBox* categoryBox;

    ::TagWidget* tags;
	::CPublicationWidgetView* pubs;

	QWizardPage* filesPage;

	QAction* loadJson;
	QAction* saveJson;
	QAction* addFolder;
	QAction* addFiles;
	QAction* rename;
	QAction* replaceFile;

	QTreeWidget* fileTree;
	QTreeWidgetItem* projectItem;
	QLabel* fileDescriptionLabel;
	QPlainTextEdit* fileDescription;

    ::TagWidget* fileTags;

	QWizardPage* summaryPage;

	// ID of project being modified. Since project indices start at 1, this is also
	// used as a boolean e.g. if(m_modify)
	int m_modify;
	unordered_map<QString, QTreeWidgetItem*> currentFolders;
	::CWzdUpload* m_wzd;

public:
	void setup(::CWzdUpload* wzd, int uploadPermissions, int modify)
	{
		m_wzd = wzd;
		m_modify = modify;

		// Project info page
		infoPage = new QWizardPage;
		QVBoxLayout* outerInfoLayout = new QVBoxLayout;
		outerInfoLayout->setContentsMargins(0,0,0,0);

		if(!modify)
		{
			QToolBar* infoToolbar = new QToolBar;
			infoToolbar->addAction(loadJson = new QAction(CIconProvider::GetIcon("open"), "Open", wzd));
			loadJson->setObjectName("loadJson");
			infoToolbar->addAction(saveJson = new QAction(CIconProvider::GetIcon("save"), "Save", wzd));
			saveJson->setObjectName("saveJson");
			outerInfoLayout->addWidget(infoToolbar);
		}
		

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

		infoLayout->addLayout(leftLayout);

		QVBoxLayout* rightLayout = new QVBoxLayout;

        rightLayout->addWidget(tags = new ::TagWidget);

		rightLayout->addWidget(new QLabel("Publications:"));
		pubs = new ::CPublicationWidgetView(::CPublicationWidgetView::EDITABLE, true, true);
		rightLayout->addWidget(pubs);

		infoLayout->addLayout(rightLayout);
		outerInfoLayout->addLayout(infoLayout);
		infoPage->setLayout(outerInfoLayout);
		wzd->addPage(infoPage);

		// Files page
		filesPage = new QWizardPage;
		QVBoxLayout* filesLayout = new QVBoxLayout;
		filesLayout->setContentsMargins(0,0,0,0);

		QToolBar* filesToolbar = new QToolBar;
		if(!modify)
		{
			filesToolbar->addAction(loadJson);
			filesToolbar->addAction(saveJson);
			filesToolbar->addSeparator();
		}
		
		filesToolbar->addAction(addFolder = new QAction(CIconProvider::GetIcon("folder", Emblem::Plus), "New Folder", wzd));
		addFolder->setObjectName("addFolder");
		filesToolbar->addAction(addFiles = new QAction(CIconProvider::GetIcon("new"), "Add Files", wzd));
		addFiles->setObjectName("addFiles");
		filesToolbar->addAction(rename = new QAction(CIconProvider::GetIcon("rename"), "Rename", wzd));
		rename->setObjectName("rename");
		
		replaceFile = new QAction(CIconProvider::GetIcon("document", "swap"), "Replace File", wzd);
		replaceFile->setObjectName("replaceFile");
		replaceFile->setDisabled(true);
		if(m_modify)
		{
			filesToolbar->addAction(replaceFile);
		}

		filesLayout->addWidget(filesToolbar);

		fileTree = new QTreeWidget;
		fileTree->setObjectName("fileTree");
		fileTree->setColumnCount(3);
		fileTree->setHeaderLabels(QStringList() << "Project Files" << "Location" << "Size");
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
		qint64 size = 0;
		projectItem->setData(2, SIZE, size);
		fileTree->addTopLevelItem(projectItem);
		fileTree->setCurrentItem(projectItem);

		filesLayout->addWidget(fileTree, 1);

		QHBoxLayout* filesSubLayout = new QHBoxLayout;
		QVBoxLayout* descLayout = new QVBoxLayout;

		descLayout->addWidget(fileDescriptionLabel = new QLabel("Description:"));
		descLayout->addWidget(fileDescription = new QPlainTextEdit);
		filesSubLayout->addLayout(descLayout);

        filesSubLayout->addWidget(fileTags = new ::TagWidget);

		filesLayout->addLayout(filesSubLayout);

		filesPage->setLayout(filesLayout);
		wzd->addPage(filesPage);

		fileInfoEnabled(false);

//		if(modify)
//		{
//			summaryPage = new QWizardPage();
//			QVBoxLayout* summaryLayout = new QVBoxLayout;
//
//			summaryPage->setLayout(summaryLayout);
//			wzd->addPage(summaryPage);
//		}

		if(modify)
		{
			wzd->setWindowTitle("Modify Project");
		}
		else
		{
			wzd->setWindowTitle("Upload Project");
		}
		wzd->resize(800, 600);

		fileTree->setColumnWidth(0, 350);
		fileTree->setColumnWidth(1, 310);

        QObject::connect(fileTags, &::TagWidget::TagAdded, wzd, &::CWzdUpload::on_fileTags_TagAdded);
        QObject::connect(fileTags, &::TagWidget::TagDeleted, wzd, &::CWzdUpload::on_fileTags_TagDeleted);
	}

	QTreeWidgetItem* NewFile(QString path, QString description = "", qint64 size = -1, QStringList tags = QStringList(), QString filename = "")
	{
		QFileInfo info(path);

		QTreeWidgetItem* fileItem = new QTreeWidgetItem(FILEITEM);
		fileItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
		
		if(filename.isEmpty())
		{
			fileItem->setText(0, info.fileName());
		}
		else
		{
			fileItem->setText(0, filename);
		}
		
		fileItem->setCheckState(0,Qt::Checked);
		fileItem->setData(0, DESCRIPTION, description);
		fileItem->setData(0, TAGS, tags);

		if(path.endsWith(".fsp"))
		{
			fileItem->setIcon(0, CIconProvider::GetIcon("FEBioStudio"));
		}
		else if(path.endsWith(".fsm") || path.endsWith(".fs2") || path.endsWith(".fsprj") || path.endsWith(".prv"))
		{
			fileItem->setIcon(0, CIconProvider::GetIcon("PreView"));
		}
		else if(path.endsWith(".feb"))
		{
			fileItem->setIcon(0, CIconProvider::GetIcon("febio"));
		}
		else if(path.endsWith(".xplt"))
		{
			fileItem->setIcon(0, CIconProvider::GetIcon("PostView"));
		}
		else
		{
			fileItem->setIcon(0, CIconProvider::GetIcon("new"));
		}

		if(size == -1)
		{
			fileItem->setData(2, SIZE, info.size());
		}
		else
		{
			fileItem->setData(2, SIZE, size);
		}

		fileItem->setText(1, path);

		return fileItem;
	}

	QTreeWidgetItem* NewFolder(QString name, QString location = "")
	{
		QTreeWidgetItem* child = new QTreeWidgetItem(FOLDERITEM);
		child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
		child->setCheckState(0,Qt::Checked);
		child->setText(0, name);
		child->setIcon(0, CIconProvider::GetIcon("folder"));
		quint64 size = 0;
		child->setData(2, SIZE, size);

		child->setText(1, location);

		return child;
	}

	void fileInfoEnabled(bool enabled)
	{
		fileDescriptionLabel->setEnabled(enabled);
		fileDescription->setEnabled(enabled);

		fileTags->setEnabled(enabled);
	}

	bool invalidNames(QTreeWidgetItem* item = nullptr)
	{
		std::string dissallowed = "<>:\"\\/|?*";

		if(!item) item = projectItem;

		for(int child = 0; child < item->childCount(); child++)
		{
			if(item->child(child)->checkState(0) == Qt::Unchecked) continue;

			for(auto c : dissallowed)
			{
				if(item->child(child)->text(0).contains(c)) return true;
			}

			if(item->child(child)->type() == FOLDERITEM)
			{
				return invalidNames(item->child(child));
			}
		}

		return false;
	}

	bool hasDuplicateNames(QTreeWidgetItem* item = nullptr)
	{
		if(!item) item = projectItem;

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

	void getFileItems(QList<QTreeWidgetItem*>& items, QTreeWidgetItem* item = nullptr)
	{
		if(!item) item = projectItem;

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

	void getFilePaths(QStringList& paths, QTreeWidgetItem* item = nullptr)
	{
		if(!item) item = projectItem;

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

	void getZipFilePath(QString& path, QTreeWidgetItem* item = nullptr)
	{
		if(!item) item = projectItem;

		if(item->type() == FILEITEM)
		{
			path = item->text(0);

			getZipFilePath(path, item->parent());
		}
		else if(item->type() == FOLDERITEM)
		{
			path = item->text(0).append("/").append(path);

			getZipFilePath(path, item->parent());
		}
	}

	QTreeWidgetItem* addRepoFile(QString& path, int index, QString& description, qint64 size, QStringList& tags)
	{
		int pos = path.right(path.length() - index).indexOf("/");

		if(pos == -1)
		{
			QTreeWidgetItem* child = NewFile(path, description, size, tags);

			child->setText(1, QString("{Repository}/") + path);

			return child;
		}

		QTreeWidgetItem* child = addRepoFile(path, index + (pos + 1), description, size, tags);
		QTreeWidgetItem* parent;

		try
		{
			parent = currentFolders.at(path.left(pos + index));
		}
		catch(out_of_range& e)
		{
			parent = NewFolder(path.right(path.length() - index).left(pos), QString("{Repository}/") + path.left(index + pos));

			currentFolders[path.left(pos + index)] = parent;
		}

		parent->addChild(child);

		return parent;
	}

	void updateColor(QTreeWidgetItem* item = nullptr)
	{
		if(!item) item = projectItem;

		if(item->type() != PROJECTITEM)
		{
			if(item->checkState(0) == Qt::Unchecked)
			{
				item->setForeground(0, qApp->palette().color(QPalette::Disabled, QPalette::Text));
				item->setForeground(1, qApp->palette().color(QPalette::Disabled, QPalette::Text));
				item->setForeground(2, qApp->palette().color(QPalette::Disabled, QPalette::Text));
			}
			else
			{
				item->setForeground(0, qApp->palette().color(QPalette::Active, QPalette::Text));
				item->setForeground(1, qApp->palette().color(QPalette::Active, QPalette::Text));
				item->setForeground(2, qApp->palette().color(QPalette::Active, QPalette::Text));
			}
		}

		for(int child = 0; child < item->childCount(); child++)
		{
			updateColor(item->child(child));
		}
	}

	void updateSizes(QTreeWidgetItem* item = nullptr)
	{
		if(!item) item = projectItem;

		if(item->type() == FILEITEM)
		{
			item->setText(2, m_wzd->locale().formattedDataSize(item->data(2, SIZE).toLongLong(), 2, QLocale::DataSizeTraditionalFormat));
			return;
		}

		qint64 size = 0;
		for(int child = 0; child < item->childCount(); child++)
		{
			updateSizes(item->child(child));

			if(item->child(child)->checkState(0) != Qt::Unchecked)
			{
				size += item->child(child)->data(2, SIZE).toLongLong();
			}
		}

		item->setData(2, SIZE, size);
		item->setText(2, m_wzd->locale().formattedDataSize(size, 2, QLocale::DataSizeTraditionalFormat));
	}

	bool isDeleting(QTreeWidgetItem* item = nullptr)
	{
		if(!item) item = projectItem;

		if(item->type() == FILEITEM && item->checkState(0) == Qt::Unchecked && item->text(1).startsWith("{Repository}/"))
		{
			return true;
		}

		for(int child = 0; child < item->childCount(); child++)
		{
			if(isDeleting(item->child(child)))
			{
				return true;
			}
		}

		return false;
	}

	void clearUI()
	{
		name->setText("");
		description->setPlainText("");
		tags->Clear();
        fileTags->Clear();
		pubs->clear();

		delete projectItem;

		projectItem = new QTreeWidgetItem(PROJECTITEM);
		projectItem->setFirstColumnSpanned(true);
		projectItem->setText(0, "Project");
		projectItem->setIcon(0, CIconProvider::GetIcon("FEBioStudio"));
		projectItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
		qint64 size = 0;
		projectItem->setData(2, SIZE, size);
		fileTree->addTopLevelItem(projectItem);
		fileTree->setCurrentItem(projectItem);
	}

	bool edited()
	{
		bool edited = false;

		edited |= !name->text().isEmpty();
		edited |= !description->toPlainText().isEmpty();
		edited |= tags->Count() != 0;
		edited |= pubs->count() != 0;
		edited |= projectItem->childCount() !=0;

		return edited;
	}

};

CWzdUpload::CWzdUpload(QWidget* parent, int uploadPermissions, CModelDatabaseHandler* dbHandler, CModelRepoConnectionHandler* repoHandler, int modify)//, FEBioStudioProject* project)
	: QWizard(parent), ui(new Ui::CWzdUpload), dbHandler(dbHandler), repoHandler(repoHandler)
{
#ifdef WIN32
	// We need this style, since the default Aero style doesn't look right in dark mode.
	setWizardStyle(QWizard::ModernStyle);
#endif

	ui->setup(this, uploadPermissions, modify); //, project);

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

void CWzdUpload::setCategory(QString category)
{
	if(ui->categoryLabel)
	{
		ui->categoryLabel->setText(category);
	}
	else
	{
		for(int index = 0; index < ui->categoryBox->count(); index++)
		{
			if(ui->categoryBox->itemText(index) == category)
			{
				ui->categoryBox->setCurrentIndex(index);
				break;
			}
		}
	}
}

void CWzdUpload::setOwner(QString owner)
{
	ui->owner->setText(owner);
}

void CWzdUpload::setTags(QStringList& tags)
{
	ui->tags->SetTags(tags);
}

void CWzdUpload::setPublications(const std::vector<CPublicationWidget*>& pubs)
{
	for(auto pub : pubs)
	{
		ui->pubs->addPublicationCopy(*pub);
	}
}

void CWzdUpload::setFileInfo(QList<QList<QVariant>>& fileinfo)
{
	for(auto info : fileinfo)
	{
		QString filename = info[0].toString();
		QString desc = info[1].toString();
		qint64 size = info[2].toLongLong();
		QStringList tags = info[3].toStringList();


		ui->projectItem->addChild(ui->addRepoFile(filename, 0, desc, size, tags));
	}

	ui->fileTree->expandAll();

	ui->updateSizes();
}

void CWzdUpload::setTagCompleter(QStringList& tags)
{
    ui->tags->SetTagCompleter(tags);
    ui->fileTags->SetTagCompleter(tags);
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

QStringList CWzdUpload::getTags()
{
	return ui->tags->GetTags();
}

QList<QVariant> CWzdUpload::getPublicationInfo()
{
	return ui->pubs->getPublicationInfo();
}

QStringList CWzdUpload::GetFilePaths()
{
	QList<QTreeWidgetItem*> items;
	ui->getFileItems(items);

	QStringList paths;
	for(auto item : items)
	{
		QString path = item->text(1);

		// Skip this item if we're modifying and it was already in the project
		if(ui->m_modify && path.startsWith("{Repository}/"))
		{
			continue;
		}

		paths.push_back(path);
	}

	return paths;
}

QStringList CWzdUpload::GetZipFilePaths()
{
	QList<QTreeWidgetItem*> items;
	ui->getFileItems(items);

	QStringList paths;
	for(auto item : items)
	{
		// Skip this item if we're modifying and it was already in the project
		if(ui->m_modify && item->text(1).startsWith("{Repository}/"))
		{
			continue;
		}

		QString path;
		ui->getZipFilePath(path, item);
		paths.push_back(path);
	}

	return paths;
}

QList<QVariant> CWzdUpload::getFileInfo()
{
	QList<QVariant> info;
	QList<QTreeWidgetItem*> items;

	ui->getFileItems(items);

	for(auto item : items)
	{
		QVariantMap fileInfo;

		QString path;
		ui->getZipFilePath(path, item);

		fileInfo["filename"] = path;
		fileInfo["description"] = item->data(0, DESCRIPTION);
		fileInfo["tags"] = item->data(0, TAGS);

		// Add in the old name if we're modifying
		if(ui->m_modify)
		{
			QString repoString = "{Repository}/";
			QString oldPath = item->text(1);
			if(oldPath.startsWith(repoString))
			{
				fileInfo["oldFilename"] = oldPath.right(oldPath.length() - repoString.length());
			}
			else
			{
				fileInfo["oldFilename"] = "";
			}
		}

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

	QString name = getName();
	QString category = getCategory();
	if(!dbHandler->isValidUpload(name, category))
	{
		if(!ui->m_modify || dbHandler->ProjectNameFromID(ui->m_modify) != name)
		{
			QMessageBox::critical(this, "Upload", "A project with this name already exists in this category."
					"\n\nPlease choose a different project name.");

			// Go back to page 1
			while(currentId() != 0)
			{
				back();
			}

			return;
		}

	}

	std::string dissallowed = "<>:\"\\/|?*.";
	for(auto c : dissallowed)
	{
		if(name.contains(c))
		{
			QMessageBox::critical(this, "Upload", "Project names cannot contain any of the following symbols:\n\n"
					". < > : \" \\ / | ? *");

			// Go back to page 1
			while(currentId() != 0)
			{
				back();
			}

			return;
		}
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

	if(ui->tags->Count() == 0)
	{
		QMessageBox::critical(this, "Upload", "Please add at least one tag to your project.");

		while(currentId() != 0)
		{
			back();
		}

		return;
	}

	if(ui->invalidNames())
	{
		QMessageBox::critical(this, "Upload", "The following characters are not allowed in file or folder names:\n\n< > : \" \\ / | ? *");
		return;
	}

	if(ui->hasDuplicateNames())
	{
		QMessageBox::critical(this, "Upload", "You cannot have to files with the same name in the same folder.");
		return;
	}

	QStringList filePaths;

	ui->getFilePaths(filePaths);
	if(filePaths.isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please select at least one file to include in your project.");
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
	qint64 modifiedProjectSize = 0;

	if(ui->m_modify)
	{
		modifiedProjectSize = dbHandler->projectsSize(ui->m_modify);
	}

	if(totalSize + currentProjectsSize - modifiedProjectSize > sizeLimit)
	{
		QLocale locale = this->locale();

		QString message = QString("This upload would exceed your limit of %1 on the repository. Please remove some files "
				"or delete some projects from the repository.\n\n"
				"Current Project Size: %2\n"
				"Total on Repository: %3").arg(locale.formattedDataSize(sizeLimit))
									.arg(locale.formattedDataSize(totalSize))
									.arg(locale.formattedDataSize(currentProjectsSize));

		if(ui->m_modify)
		{
			message += QString("\nOld Project Size: %1").arg(locale.formattedDataSize(modifiedProjectSize, 2, QLocale::DataSizeTraditionalFormat));
		}

		QMessageBox::critical(this, "Upload", message);
		return;
	}

	QList<QTreeWidgetItem*> items;
	ui->getFileItems(items);
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

	if(ui->m_modify && ui->isDeleting())
	{
		QMessageBox::StandardButton reply = QMessageBox::question(this, "Upload", "You have unchecked some files that are currently part of "
				"this project. Continuing will PERMANENTLY DELETE these files from the repository."
				"\n\nWould you like to continue?");

		if(reply != QMessageBox::Yes)
		{
			return;
		}
	}

	QWizard::accept();
}

void CWzdUpload::reject()
{
	if(!ui->m_modify)
	{
		if(ui->edited())
		{
			QMessageBox box(QMessageBox::Warning, "Save Project", "Would you like to save this data to a file for later upload?");

			QPushButton* save = box.addButton("Save", QMessageBox::AcceptRole);
			box.addButton("Discard", QMessageBox::RejectRole);
			QPushButton* cancel = box.addButton(QMessageBox::Cancel);

			box.exec();

			if(box.clickedButton() == save)
			{
				on_saveJson_triggered();
			}
			else if(box.clickedButton() == cancel)
			{
				return;
			}
		}
	}

	QWizard::reject();
}

void CWzdUpload::keyPressEvent(QKeyEvent* e)
{
	// Prevent the enter key from finishing the wizard.

	if(e->key() != Qt::Key_Enter)
	{
		QWizard::keyPressEvent(e);
	}
}

void CWzdUpload::on_loadJson_triggered()
{
	QFileDialog filedlg(this);
	filedlg.setFileMode(QFileDialog::ExistingFile);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);
	filedlg.setNameFilter("Json Files (*.json)");

	if (filedlg.exec())
	{
		QFile file(filedlg.selectedFiles()[0]);
		file.open(QIODevice::ReadOnly);
		QByteArray ba = file.readAll();
		file.close();

		setProjectJson(&ba);
	}
}

void CWzdUpload::on_saveJson_triggered()
{
	QFileDialog filedlg(this);
	filedlg.setFileMode(QFileDialog::AnyFile);
	filedlg.setAcceptMode(QFileDialog::AcceptSave);
	filedlg.setNameFilter("Json Files (*.json)");

	if (filedlg.exec())
	{
		QByteArray projectInfo;
		getProjectJson(&projectInfo);

		QFile file(filedlg.selectedFiles()[0]);
		file.open(QIODeviceBase::WriteOnly);
		file.write(projectInfo);
		file.close();
	}
}

void CWzdUpload::on_addFolder_triggered()
{
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

	QTreeWidgetItem* child = ui->NewFolder("New Folder");

	parent->addChild(child);

	ui->fileTree->expandAll();

	ui->fileTree->setCurrentItem(child);
	ui->fileTree->editItem(child);

	ui->updateSizes();
}

void CWzdUpload::on_addFiles_triggered()
{
	QFileDialog dlg(this, "Add Files");
	dlg.setFileMode(QFileDialog::ExistingFiles);


	if (dlg.exec())
	{
		QStringList files = dlg.selectedFiles();

		for(auto file : files)
		{
			QTreeWidgetItem* child = ui->NewFile(file);

			QTreeWidgetItem* parent = ui->fileTree->currentItem();
			if(!parent)
			{
				parent = ui->projectItem;
			}
			else if(parent->type() == FILEITEM)
			{
				parent = parent->parent();
			}

			parent->addChild(child);

			ui->fileTree->setCurrentItem(child);
		}

		ui->updateSizes();

		ui->fileTree->expandAll();
	}
}

void CWzdUpload::on_rename_triggered()
{
	if(ui->fileTree->currentItem())
	{
		ui->fileTree->editItem(ui->fileTree->currentItem(), 0);
	}
}

void CWzdUpload::on_replaceFile_triggered()
{
	QTreeWidgetItem* oldItem = ui->fileTree->selectedItems()[0];

	QFileDialog dlg(this, "Replace/Update File");
	dlg.setFileMode(QFileDialog::ExistingFile);

	if (dlg.exec())
	{
		QString file = dlg.selectedFiles()[0];

		QTreeWidgetItem* newItem = ui->NewFile(file);

		// Add the new item to the old item's parent
		oldItem->parent()->addChild(newItem);

		// Copy the data from the old item to the new one
		newItem->setText(0, oldItem->text(0));
		newItem->setData(0, DESCRIPTION, oldItem->data(0, DESCRIPTION));
		newItem->setData(0, TAGS, oldItem->data(0, TAGS));

		// Rename and uncheck old item
		oldItem->setText(0, "(Old Version)" + oldItem->text(0));
		oldItem->setCheckState(0, Qt::Unchecked);

		ui->updateSizes();

		ui->fileTree->expandAll();

		ui->fileTree->setCurrentItem(newItem);
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

			ui->fileTags->Clear();
            QStringList tags = current->data(0, TAGS).toStringList();
            ui->fileTags->SetTags(tags);
		}
		else
		{
			ui->fileDescription->clear();
			ui->fileTags->Clear();

			ui->fileInfoEnabled(false);
		}

		if(ui->fileTree->selectedItems().count() == 1 && current->text(1).startsWith("{Repository}/"))	
		{
			ui->replaceFile->setEnabled(true);
		}
		else
		{
			ui->replaceFile->setEnabled(false);
		}
	}
	else
	{
		ui->replaceFile->setEnabled(false);
	}
}

void CWzdUpload::on_fileTree_itemChanged(QTreeWidgetItem *item, int column)
{
	QString temp = item->text(0);

	// Prevent files or folders from starting with a '.' to prevent hidden file shenanigans
	while(temp.startsWith("."))
	{
		temp.remove(0,1);
	}

	if(item->type() == FILEITEM)
	{
		// Ensure that renaming a file doesn't change its extension
		QString extension = QFileInfo(item->text(1)).completeSuffix();
		item->setText(0, QFileInfo(temp).baseName().append(".").append(extension));
	}
}

void CWzdUpload::on_fileTree_itemDoubleClicked(QTreeWidgetItem * item, int column)
{
	if(column == 0)
	{
		ui->fileTree->editItem(item, column);
	}
}

void CWzdUpload::on_fileTree_itemClicked(QTreeWidgetItem * item, int column)
{
	// I can't find a way to call these functions when a checkbox state is changed
	// this will do for now.
	ui->updateColor();

	// I can't find a way to update sizes after a user reorders the files.
	// This will sort of work for that sometimes
	ui->updateSizes();
}

void CWzdUpload::fileDescriptionChanged()
{
	for(auto item : ui->fileTree->selectedItems())
	{
		item->setData(0, DESCRIPTION, ui->fileDescription->toPlainText());
	}
}

void CWzdUpload::on_fileTags_TagAdded(QString& tag)
{
    for(auto item : ui->fileTree->selectedItems())
    {
        item->setData(0, TAGS, item->data(0, TAGS).toStringList() << tag);
    }
}

void CWzdUpload::on_fileTags_TagDeleted(QString& tag)
{
    for(auto treeItem : ui->fileTree->selectedItems())
    {
        QStringList tags = treeItem->data(0, TAGS).toStringList();

        tags.removeAll(tag);

        treeItem->setData(0, TAGS, tags);
    }
}

void CWzdUpload::setProjectJson(QByteArray* json)
{
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(*json, &err);

	if(err.error)
	{
		QByteArray leftSide = json->left(err.offset);
		int line = leftSide.count("\n") + 1;
		int position = err.offset - leftSide.lastIndexOf("\n") - 1;

		QString message = QString("There was an error while parsing this JSON file:\n\n%1\n\n"
			"This error was found on line %2 at position %3 in the file.")
			.arg(err.errorString()).arg(line).arg(position);

		QMessageBox::warning(this, "JSON Parse Error", message);

		return;
	}

	ui->clearUI();

	QJsonObject obj = doc.object();

	if(obj.contains("name"))
	{
		setName(obj["name"].toString());
	}

	if(obj.contains("description"))
	{
		setDescription(obj["description"].toString());
	}

	if(obj.contains("category"))
	{
		setCategory(obj["category"].toString());
	}

	if(obj.contains("tags"))
	{
		QStringList tags;

		QJsonArray jsonArray = obj["tags"].toArray();
		for(auto tag : jsonArray)
		{
			tags.append(tag.toString());
		}

		setTags(tags);
	}

	if(obj.contains("publications"))
	{
		QJsonArray pubs = obj["publications"].toArray();

		for(auto pub : pubs)
		{
			QJsonObject pubObject = pub.toObject();

			QString title, journal, year, volume, issue, pages, DOI;
			QStringList authorGiven, authorFamily;

			if(pubObject.contains("title"))
			{
				title = pubObject["title"].toString();
			}

			if(pubObject.contains("journal"))
			{
				journal = pubObject["journal"].toString();
			}

			if(pubObject.contains("year"))
			{
				year = pubObject["year"].toString();
			}

			if(pubObject.contains("volume"))
			{
				volume = pubObject["volume"].toString();
			}

			if(pubObject.contains("issue"))
			{
				issue = pubObject["issue"].toString();
			}

			if(pubObject.contains("pages"))
			{
				pages = pubObject["pages"].toString();
			}

			if(pubObject.contains("DOI"))
			{
				DOI = pubObject["DOI"].toString();
			}

			if(pubObject.contains("authors"))
			{
				QJsonArray authors = pubObject["authors"].toArray();
				for(auto author : authors)
				{
					QJsonObject auth = author.toObject();

					if(auth.contains("given"))
					{
						authorGiven.append(auth["given"].toString());
					}
					else
					{
						authorGiven.append("");
					}

					if(auth.contains("family"))
					{
						authorFamily.append(auth["family"].toString());
					}
					else
					{
						authorFamily.append("");
					}
				}
			}

			ui->pubs->addPublication(title, year, journal, volume, issue, pages, DOI, authorGiven, authorFamily);
		}
	}

	if(obj.contains("files"))
	{
		QJsonArray files = obj["files"].toArray();
		for(auto f : files)
		{
			QJsonObject fileObject = f.toObject();
			QTreeWidgetItem* child = addFileFromJson(fileObject);

			if(child) ui->projectItem->addChild(child);
		}
	}

	ui->fileTree->expandAll();
}

QTreeWidgetItem* CWzdUpload::addFileFromJson(QJsonObject& file)
{
	if(file.contains("files"))
	{
		QString name;

		if(file.contains("name"))
		{
			name = file["name"].toString();
		}
		else
		{
			return nullptr;
		}

		QTreeWidgetItem* folder = ui->NewFolder(name);

		QJsonArray files = file["files"].toArray();

		for(auto f : files)
		{
			QJsonObject fileObject = f.toObject();
			QTreeWidgetItem* child = addFileFromJson(fileObject);

			if(child) folder->addChild(child);
		}

		return folder;
	}
	else
	{
		bool active = true;
		QString name, description, location;
		QStringList tags;

		// A size of -1 will force the dialog to find the size on the disk
		qint64 size = -1;

		if(file.contains("active"))
		{
			active = file["active"].toBool();
		}

		if(file.contains("name"))
		{
			name = file["name"].toString();
		}

		if(file.contains("location"))
		{
			location = file["location"].toString();
		}
		else
		{
			return nullptr;
		}

		if(file.contains("description"))
		{
			description = file["description"].toString();
		}

		if(file.contains("tags"))
		{
			QJsonArray tagArray = file["tags"].toArray();
			for(auto tag : tagArray)
			{
				tags.append(tag.toString());
			}
		}

		QFileInfo info(location);
		if(!info.exists())
		{
			QMessageBox box(QMessageBox::Warning, "File Not Found", 
				QString("Could not locate file:\n\n%1\n\nWould you like to skip this file, or relocate it?").arg(location));

			box.addButton("Skip", QMessageBox::RejectRole);
			QPushButton* relocate = box.addButton("Relocate", QMessageBox::AcceptRole);

			box.exec();

			if(box.clickedButton() == relocate)
			{
				QString selection = QFileDialog::getOpenFileName(this, "Select File", info.filePath());
				
				if(selection.isEmpty())
				{
					return nullptr;
				}
				
				location = selection;
			}
			else
			{
				return nullptr;
			}
		}

		QTreeWidgetItem* fileItem = ui->NewFile(location, description, size, tags, name);

		if(!active)	fileItem->setCheckState(0, Qt::Unchecked);

		return fileItem;
	}
}

void CWzdUpload::getProjectJson(QByteArray* json)
{
	QVariantMap project;

	project["name"] = getName();
	project["description"] = getDescription();
	project["category"] = getCategory();
	project["tags"] = getTags();
	project["publications"] = ui->pubs->getPublicationInfo();

	QVariantList files;
	addFileToJson(ui->projectItem, files);
	project["files"] = files;

	*json = QJsonDocument::fromVariant(project).toJson();
}

void CWzdUpload::addFileToJson(QTreeWidgetItem* item, QVariantList& list)
{
	for(int index = 0; index < item->childCount(); index++)
	{
		QTreeWidgetItem* child = item->child(index);
		QVariantMap childInfo;
		childInfo["name"] = child->text(0);

		if(child->type() == FILEITEM)
		{
			childInfo["active"] = child->checkState(0) == Qt::Checked;
			childInfo["description"] = child->data(0, DESCRIPTION);
			childInfo["location"] = child->text(1);
			childInfo["tags"] = child->data(0, TAGS);
			
			list.append(childInfo);
		}
		else
		{
			QVariantList files;
			addFileToJson(child, files);

			childInfo["files"] = files;

			list.append(childInfo);
		}
	}
}