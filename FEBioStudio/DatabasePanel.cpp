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

#include "stdafx.h"
#include "DatabasePanel.h"

#ifdef MODEL_REPO
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <QApplication>
#include <QPalette>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QToolBar>
#include <QBoxLayout>
#include <QSplitter>
#include <QToolButton>
#include <QStackedLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QFont>
#include <QPushButton>
#include <QTreeWidget>
#include <QJsonDocument>
#include <QByteArray>
#include <QDir>
#include <QFileIconProvider>
#include <JlCompress.h>
#include "RepoConnectionHandler.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "DlgUpload.h"
#include "DlgSetRepoFolder.h"
#include "LocalDatabaseHandler.h"
#include "RepoProject.h"
#include "ToolBox.h"
#include "PublicationWidgetView.h"
#include "IconProvider.h"
#include "FSCore/FSDir.h"

enum ITEMTYPES {PROJECTITEM = 1001, FOLDERITEM = 1002, FILEITEM = 1003};

class CustomTreeWidgetItem : public QTreeWidgetItem
{
public:
	CustomTreeWidgetItem(QString name, int type)
		: QTreeWidgetItem(QStringList(name), type), localCopy(0), totalCopies(0)
	{

	}

	virtual CustomTreeWidgetItem* getProjectItem() = 0;

//	void addChild(CustomTreeWidgetItem* child)
//	{
//		localCopy += child->GetLocalCopy();
//		totalCopies += child->GetTotalCopies();
//		UpdateLocalCopyColor();
//
//		QTreeWidgetItem::addChild(child);
//	}

	bool LocalCopy() { return localCopy >= totalCopies; }

	int GetLocalCopy() { return localCopy; }
	int GetTotalCopies() { return totalCopies; }

	virtual void UpdateLocalCopyColor()
	{
		if(LocalCopy())
		{
			setForeground(0, qApp->palette().color(QPalette::Active, QPalette::Text));
		}
		else
		{
			setForeground(0, qApp->palette().color(QPalette::Disabled, QPalette::Text));
		}
	}


	void AddLocalCopy()
	{
		localCopy++;
		UpdateLocalCopyColor();

		if(type() != PROJECTITEM)
		{
			static_cast<CustomTreeWidgetItem*>(parent())->AddLocalCopy();

		}
	}

	void SubtractLocalCopy()
	{
		localCopy--;
		UpdateLocalCopyColor();

		if(type() != PROJECTITEM)
		{
			static_cast<CustomTreeWidgetItem*>(parent())->SubtractLocalCopy();

		}
	}

	void setLocalCopyRecursive(bool lc)
	{
		for(int index = 0; index < childCount(); index++)
		{
			static_cast<CustomTreeWidgetItem*>(child(index))->setLocalCopyRecursive(lc);
		}

		if(lc)
		{
			AddLocalCopy();
		}
		else
		{
			SubtractLocalCopy();
		}

	}

	void AddTotalCopy()
	{
		totalCopies++;
		UpdateLocalCopyColor();
	}

	void UpdateCopies()
	{
		int lc = 0;
		int tc = 0;

		for(int index = 0; index < childCount(); index++)
		{
			CustomTreeWidgetItem* current = static_cast<CustomTreeWidgetItem*>(child(index));
			current->UpdateCopies();

			lc += current->GetLocalCopy();
			tc += current->GetTotalCopies();
		}

		localCopy += lc;
		totalCopies += tc;
		UpdateLocalCopyColor();
	}

protected:
	int localCopy;
	int totalCopies;

};

class ProjectItem : public CustomTreeWidgetItem
{
public:
	ProjectItem(QString name, int projectID, bool owned)
		: CustomTreeWidgetItem(name, PROJECTITEM), m_projectID(projectID), m_ownedByUser(owned)
	{
		setIcon(0, QIcon(":/icons/folder.png"));
	}

	CustomTreeWidgetItem* getProjectItem()
	{
		return this;
	}

	void setProjectID(int project) {m_projectID = project;}
	int getProjectID() {return m_projectID;}
	bool ownedByUser() {return m_ownedByUser;}

private:
	int m_projectID;
	bool m_ownedByUser;
};

class FolderItem : public CustomTreeWidgetItem
{
public:
	FolderItem(QString name)
		: CustomTreeWidgetItem(name, FOLDERITEM)
	{
		setIcon(0, QIcon(":/icons/folder.png"));
	}

	CustomTreeWidgetItem* getProjectItem()
	{
		return ((CustomTreeWidgetItem*) parent())->getProjectItem();
	}
};

class FileItem : public CustomTreeWidgetItem
{
public:
	FileItem(QString name, int fileID, bool lc)
		: CustomTreeWidgetItem(name, FILEITEM), m_fileID(fileID)
	{
		if(name.endsWith(".fsprj"))
		{
			setIcon(0, QIcon(":/icons/FEBioStudio.png"));
		}
		else if(name.endsWith(".feb"))
		{
			setIcon(0, QIcon(":/icons/febio.png"));
		}
		else if(name.endsWith(".prv"))
		{
			setIcon(0, QIcon(":/icons/PreView.png"));
		}
		else if(name.endsWith(".xplt"))
		{
			setIcon(0, QIcon(":/icons/PostView.png"));
		}
		else
		{
			setIcon(0, QIcon(":/icons/new.png"));
		}


		localCopy = (lc ? 1 : 0);

		totalCopies = 1;

		UpdateLocalCopyColor();
	}

	CustomTreeWidgetItem* getProjectItem()
	{
		return ((CustomTreeWidgetItem*) parent())->getProjectItem();
	}

	int getFileID()
	{
		return m_fileID;
	}

private:
	int m_fileID;

};

class Ui::CDatabasePanel
{
public:
	QStackedLayout* stack;

	QWidget* loginPage;
	QLabel* loginLabel;
	QFormLayout* loginForm;
	QLineEdit*	userName;
	QLineEdit*	password;
	QPushButton* loginButton;

	QWidget* modelPage;
	QTreeWidget* treeWidget;

	CToolBox* projectInfoBox;

	QLabel* projectName;
	QLabel* projectDesc;
	QLabel* projectOwner;
	QLabel* projectVersion;
	QLabel* projectTags;

	QLabel* filenameLabel;
	QLabel* fileDescLabel;

	::CPublicationWidgetView* projectPubs;

	QToolBar* toolbar;

	QAction* actionDownload;
	QAction* actionOpen;
	QAction* actionOpenFileLocation;
	QAction* actionDelete;
	QAction* actionUpload;

	QAction* actionDeleteRemote;
	QAction* actionModify;

	QLineEdit* searchLineEdit;
	QAction* actionSearch;
	QAction* actionClearSearch;



public:
	CDatabasePanel() : currentProject(nullptr), openAfterDownload(nullptr){}

	void setupUi(::CDatabasePanel* parent)
	{
		stack = new QStackedLayout(parent);

		// Login Page
		QVBoxLayout* loginVBLayout = new QVBoxLayout;
		loginVBLayout->setAlignment(Qt::AlignCenter);

		loginLabel = new QLabel("To access the model database, please login with your FEBio.org username and password.");
		loginLabel->setWordWrap(true);
		loginLabel->setAlignment(Qt::AlignHCenter);
		loginVBLayout->addWidget(loginLabel);

		loginForm = new QFormLayout;
		loginForm->addRow("Username:", userName = new QLineEdit);
		loginForm->addRow("Password:", password = new QLineEdit);
		password->setEchoMode(QLineEdit::Password);
		loginVBLayout->addLayout(loginForm);

		QHBoxLayout* loginButtonLayout = new QHBoxLayout;
		loginButtonLayout->addStretch();
		loginButton = new QPushButton("Login");
		loginButton->setObjectName("loginButton");
		loginButtonLayout->addWidget(loginButton);
		loginButtonLayout->addStretch();
		loginVBLayout->addLayout(loginButtonLayout);

		QObject::connect(password, &QLineEdit::returnPressed, loginButton, &QPushButton::click);

		loginPage = new QWidget;
		loginPage->setLayout(loginVBLayout);

		stack->addWidget(loginPage);

		// Model view page
		QVBoxLayout* modelVBLayout = new QVBoxLayout;

		toolbar = new QToolBar();

		actionDownload = new QAction(CIconProvider::GetIcon("download"), "Download", parent);
		actionDownload->setObjectName("actionDownload");
		actionDownload->setIconVisibleInMenu(false);
		toolbar->addAction(actionDownload);

		actionOpen = new QAction(CIconProvider::GetIcon("open"), "Open Local Copy", parent);
		actionOpen->setObjectName("actionOpen");
		actionOpen->setIconVisibleInMenu(false);
		toolbar->addAction(actionOpen);

		actionOpenFileLocation = new QAction(CIconProvider::GetIcon("openContaining"), "Open File Location", parent);
		actionOpenFileLocation->setObjectName("actionOpenFileLocation");
		actionOpenFileLocation->setIconVisibleInMenu(false);
		toolbar->addAction(actionOpenFileLocation);

		actionDelete = new QAction(CIconProvider::GetIcon("delete"), "Delete Local Copy", parent);
		actionDelete->setObjectName("actionDelete");
		actionDelete->setIconVisibleInMenu(false);
		toolbar->addAction(actionDelete);

		actionDeleteRemote = new QAction(CIconProvider::GetIcon("deleteRemote"), "Delete From Repository", parent);
		actionDeleteRemote->setObjectName("actionDeleteRemote");
		actionDeleteRemote->setIconVisibleInMenu(false);
		toolbar->addAction(actionDeleteRemote);

		actionModify = new QAction(CIconProvider::GetIcon("edit"), "Modify Project Metadata", parent);
		actionModify->setObjectName("actionModify");
		actionModify->setIconVisibleInMenu(false);
		toolbar->addAction(actionModify);

		toolbar->addSeparator();
		QWidget* empty = new QWidget();
		empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
		toolbar->addWidget(empty);

		actionUpload = new QAction(CIconProvider::GetIcon("upload"), "Upload", parent);
		actionUpload->setObjectName("actionUpload");
		actionUpload->setIconVisibleInMenu(false);
		toolbar->addAction(actionUpload);

		modelVBLayout->addWidget(toolbar);

		QToolBar* searchBar = new QToolBar;
		searchBar->addWidget(searchLineEdit = new QLineEdit);
		actionSearch = new QAction(CIconProvider::GetIcon("search"), "Search", parent);
		actionSearch->setObjectName("actionSearch");
		searchBar->addAction(actionSearch);
		actionClearSearch = new QAction(CIconProvider::GetIcon("clear"), "Clear", parent);
		actionClearSearch->setObjectName("actionClearSearch");
		searchBar->addAction(actionClearSearch);

		modelVBLayout->addWidget(searchBar);

		QSplitter* splitter = new QSplitter;
		splitter->setOrientation(Qt::Vertical);

		treeWidget = new QTreeWidget;
		treeWidget->setObjectName("treeWidget");
		treeWidget->setColumnCount(1);
		treeWidget->setHeaderLabel("Project Database");
		treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
		treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		splitter->addWidget(treeWidget);

		projectInfoBox = new CToolBox;
		QWidget* projectDummy = new QWidget;
		QVBoxLayout* modelInfoLayout = new QVBoxLayout;
		projectDummy->setLayout(modelInfoLayout);

		QHBoxLayout* centerName = new QHBoxLayout;
		centerName->addStretch();
		centerName->addWidget(projectName = new QLabel);
		centerName->addStretch();

		modelInfoLayout->addLayout(centerName);

		QFont font = projectName->font();
		font.setBold(true);
		font.setPointSize(14);
		projectName->setFont(font);

		modelInfoLayout->addWidget(projectDesc = new QLabel);
		projectDesc->setWordWrap(true);

		QFormLayout* modelInfoForm = new QFormLayout;
		modelInfoForm->setHorizontalSpacing(10);
		modelInfoForm->addRow("Owner:", projectOwner = new QLabel);
		modelInfoForm->addRow("Version:", projectVersion = new QLabel);
		modelInfoForm->addRow("Tags:", projectTags = new QLabel);

		modelInfoLayout->addLayout(modelInfoForm);

		projectInfoBox->addTool("Project Info", projectDummy);

		projectInfoBox->addTool("Publications", projectPubs = new ::CPublicationWidgetView);
		projectInfoBox->getToolItem(1)->hide();

		QWidget* fileDummy = new QWidget;
		QVBoxLayout* fileInfoLayout = new QVBoxLayout;
		fileDummy->setLayout(fileInfoLayout);

		QFormLayout* fileInfoForm = new QFormLayout;
		fileInfoForm->setHorizontalSpacing(10);
		fileInfoForm->addRow("Filename:", filenameLabel = new QLabel);
		fileInfoForm->addRow("Description:", fileDescLabel = new QLabel);

		fileInfoLayout->addLayout(fileInfoForm);

		projectInfoBox->addTool("File Info", fileDummy);
		projectInfoBox->getToolItem(2)->hide();

		splitter->addWidget(projectInfoBox);

		modelVBLayout->addWidget(splitter);


		modelPage = new QWidget;
		modelPage->setLayout(modelVBLayout);

		stack->addWidget(modelPage);

	}

	CustomTreeWidgetItem* addFile(QString &path, int index, int fileID, bool localCopy, std::unordered_map<std::string, CustomTreeWidgetItem*>* folders)
	{
		int pos = path.right(index).lastIndexOf("/");

		if(pos == -1)
		{
			FileItem* child = new FileItem(path.right(index), fileID, localCopy);

			fileItemsByID[fileID] = child;

			return child;
		}

		CustomTreeWidgetItem* child = addFile(path, index - (pos + 1), fileID, localCopy, folders);
		CustomTreeWidgetItem* parent;

		try
		{
			parent = folders->at(path.left(pos).toStdString());
		}
		catch(out_of_range& e)
		{
			parent = new FolderItem(path.left(pos));

			(*folders)[path.left(pos).toStdString()] = parent;
		}

		parent->addChild(child);

		return parent;
	}

	void setLoginDisabled(bool disabled)
	{
		userName->setDisabled(disabled);
		password->setDisabled(disabled);
		loginButton->setDisabled(disabled);
	}

public:
	ProjectItem* currentProject;
	std::unordered_map<std::string, CustomTreeWidgetItem*> currentProjectFolders;
	QStringList currentTags;
	std::unordered_map<int, ProjectItem*> projectItemsByID;
	std::unordered_map<int, FileItem*> fileItemsByID;

	CustomTreeWidgetItem* openAfterDownload;


};

CDatabasePanel::CDatabasePanel(CMainWindow* pwnd, QWidget* parent)
	: QWidget(parent), m_wnd(pwnd), ui(new Ui::CDatabasePanel)
{
	// build Ui
	ui->setupUi(this);

	dbHandler = new CLocalDatabaseHandler(this);
	repoHandler = new CRepoConnectionHandler(this, dbHandler, m_wnd);

	QMetaObject::connectSlotsByName(this);
}

CDatabasePanel::~CDatabasePanel()
{
	delete repoHandler;
	delete dbHandler;
	delete ui;
}

void CDatabasePanel::SetModelList()
{
	ui->treeWidget->blockSignals(true);
	ui->treeWidget->clear();
	ui->treeWidget->blockSignals(false);

	dbHandler->GetCategories();

	QString category("My Projects");
	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(category));
	item->setIcon(0, QIcon(":/icons/folder.png"));
	ui->treeWidget->addTopLevelItem(item);

	dbHandler->GetProjects();

	if(ui->treeWidget->topLevelItemCount() > 0)
	{
		ui->treeWidget->topLevelItem(0)->setSelected(true);
	}

	ui->stack->setCurrentIndex(1);
}

void CDatabasePanel::ShowMessage(QString message)
{
	QDialog *dlg = new QDialog(this);
	QVBoxLayout* l = new QVBoxLayout;
	dlg->setLayout(l);
	QLabel *msg = new QLabel(message);
	msg->setOpenExternalLinks(true);
	QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
	l->addWidget(msg);
	l->addWidget(bb);

	QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));

	dlg->exec();

	ui->setLoginDisabled(false);
}

void CDatabasePanel::LoginTimeout()
{
	QDialog *dlg = new QDialog(this);
	QVBoxLayout* l = new QVBoxLayout;
	dlg->setLayout(l);
	QLabel *msg = new QLabel("Your login to the model repository has timed out.");
	QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
	l->addWidget(msg);
	l->addWidget(bb);

	QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));

	dlg->exec();

	ui->stack->setCurrentIndex(0);
	ui->setLoginDisabled(false);
}

void CDatabasePanel::NetworkInaccessible()
{
	QDialog *dlg = new QDialog(this);
	QVBoxLayout* l = new QVBoxLayout;
	dlg->setLayout(l);
	QLabel *msg = new QLabel("FEBio Studio cannot connect to the network.");
	QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
	l->addWidget(msg);
	l->addWidget(bb);

	QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));

	dlg->exec();
}

void CDatabasePanel::DownloadFinished(int fileID, int fileType)
{
	if(fileType == FULL)
	{
		// Extract the files from the archive
		QString filename = dbHandler->FullFileNameFromID(fileID, fileType);
		QFileInfo fileInfo(filename);
		QString dir = fileInfo.path() + "/";
		dir += fileInfo.baseName();

//		m_wnd->ShowIndeterminateProgress(true, "Unzipping...");
		JlCompress::extractFiles(filename, JlCompress::getFileList(filename), dir);
//		m_wnd->ShowIndeterminateProgress(false);

		// Set the appropriate local copy flags
		ui->projectItemsByID[fileID]->setLocalCopyRecursive(true);

	}
	else
	{
		ui->fileItemsByID[fileID]->AddLocalCopy();
	}

	if(ui->openAfterDownload)
	{
		OpenItem(ui->openAfterDownload);
		ui->openAfterDownload = nullptr;
	}

	on_treeWidget_itemSelectionChanged();
}

void CDatabasePanel::AddCategory(char **data)
{
	QString category(data[0]);
	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(category));
	item->setIcon(0, QIcon(":/icons/folder.png"));

	ui->treeWidget->addTopLevelItem(item);

}

void CDatabasePanel::AddProject(char **data)
{
	int ID = stoi(data[0]);
	QString name(data[1]);
	QString owner(data[2]);
	QString category(data[3]);

	bool owned = false;

	if(repoHandler->getUsername().compare(owner) == 0)
	{
		category = "My Projects";
		owned = true;
	}

	ProjectItem* projectItem = new ProjectItem(name, ID, owned);
	ui->projectItemsByID[ID] = projectItem;


	QTreeWidgetItem* categoryItem = nullptr;
	for(int item = 0; item < ui->treeWidget->topLevelItemCount(); item++)
	{
		QTreeWidgetItem* current = ui->treeWidget->topLevelItem(item);
		if(current->text(0).compare(category) == 0)
		{
			categoryItem = current;
			break;
		}
	}
	assert(categoryItem);
	categoryItem->addChild(projectItem);

	ui->currentProject = projectItem;
	ui->currentProjectFolders.clear();

	dbHandler->GetProjectFiles(ID);

	ui->currentProject->UpdateCopies();
}

void CDatabasePanel::AddProjectFile(char **data)
{
	int ID = std::stoi(data[0]);
	QString filename(data[1]);
	bool localCopy = std::stoi(data[2]);

	ui->currentProject->addChild(ui->addFile(filename, filename.size(), ID, localCopy, &ui->currentProjectFolders));
}


void CDatabasePanel::on_loginButton_clicked()
{
	if(m_repositoryFolder.isEmpty())
	{
		QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
		defaultPath += "/FEBio Studio Repo Files";

		// set proper separators.
		std::string sPath = FSDir::filePath(defaultPath.toStdString());

		CDlgSetRepoFolder dlg(sPath.c_str(), this);

		if(dlg.exec())
		{
			SetRepositoryFolder(dlg.GetRepoFolder());
		}
		else
		{
			return;
		}
	}

	ui->setLoginDisabled(true);
	repoHandler->authenticate(ui->userName->text(), ui->password->text());
}

void CDatabasePanel::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	CustomTreeWidgetItem* customItem = static_cast<CustomTreeWidgetItem*>(item);

	if(!customItem->LocalCopy())
	{
		DownloadItem(customItem);

		ui->openAfterDownload = customItem;
	}
	else
	{
		OpenItem(customItem);
	}
}

void CDatabasePanel::on_actionDownload_triggered()
{
	DownloadItem(static_cast<CustomTreeWidgetItem*>(ui->treeWidget->selectedItems()[0]));
}

void CDatabasePanel::on_actionOpen_triggered()
{
	OpenItem(static_cast<CustomTreeWidgetItem*>(ui->treeWidget->selectedItems()[0]));
}

void CDatabasePanel::on_actionOpenFileLocation_triggered()
{
	ShowItemInBrowser(static_cast<CustomTreeWidgetItem*>(ui->treeWidget->selectedItems()[0]));
}

void CDatabasePanel::on_actionDelete_triggered()
{
	DeleteItem(static_cast<CustomTreeWidgetItem*>(ui->treeWidget->selectedItems()[0]));

	on_treeWidget_itemSelectionChanged();
}

void CDatabasePanel::on_actionUpload_triggered()
{
	if(repoHandler->getUploadPermission())
	{
		CDlgUpload dlg(this);
		dlg.setName(m_wnd->GetDocument()->GetDocFileBase().c_str());
		dlg.setOwner(repoHandler->getUsername());
		dlg.setVersion("1");
		QStringList tags = dbHandler->GetTags();
		dlg.setTagList(tags);


		if (dlg.exec())
		{
			QVariantMap projectInfo;
			projectInfo.insert("name", dlg.getName());
			projectInfo.insert("description", dlg.getDescription());
			projectInfo.insert("version", dlg.getVersion());

			QList<QVariant> tags;
			for(QString tag : dlg.getTags())
			{
				tags.append(tag);
			}
			projectInfo.insert("tags", tags);

			projectInfo.insert("publications", dlg.getPublicationInfo());

			QByteArray payload=QJsonDocument::fromVariant(projectInfo).toJson();

			repoHandler->uploadFileRequest(payload);
		}
	}
	else
	{
		QDialog *dlg = new QDialog(this);
		QVBoxLayout* l = new QVBoxLayout;
		dlg->setLayout(l);
		QLabel *msg = new QLabel("You do not have permission to upload models to the model repository.");
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
		l->addWidget(msg);
		l->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));

		dlg->exec();
	}

}

void CDatabasePanel::on_actionSearch_triggered()
{
	std::unordered_set<int> projIDs = dbHandler->FullTextSearch(ui->searchLineEdit->text());

	ui->treeWidget->blockSignals(true);

	for(std::pair<int, ProjectItem*> current : ui->projectItemsByID)
	{
		if(projIDs.count(current.second->getProjectID()) > 0)
		{
			current.second->setHidden(false);
		}
		else
		{
			current.second->setHidden(true);
		}
	}

	ui->treeWidget->blockSignals(false);

}

void CDatabasePanel::on_actionClearSearch_triggered()
{
	ui->searchLineEdit->clear();

	for(std::pair<int, ProjectItem*> current : ui->projectItemsByID)
	{
		current.second->setHidden(false);
	}
}

void CDatabasePanel::on_actionDeleteRemote_triggered()
{
	if(repoHandler->getUploadPermission())
	{
		int projID = static_cast<ProjectItem*>(ui->treeWidget->selectedItems()[0])->getProjectID();

		repoHandler->deleteProject(projID);
	}
	else
	{
		QMessageBox box;
		box.setText("You do not have permission to modify the repository.");
	}

}

void CDatabasePanel::on_actionModify_triggered()
{
	if(repoHandler->getUploadPermission())
	{
		CDlgUpload dlg(this);
		dlg.setName(ui->projectName->text());
		dlg.setOwner(repoHandler->getUsername());
		dlg.setVersion(QString("%1").arg(stoi(ui->projectVersion->text().toStdString()) + 1));
		dlg.setDescription(ui->projectDesc->text());
		dlg.setTags(ui->currentTags);
		dlg.setPublications(ui->projectPubs->getPublications());

		QStringList tags = dbHandler->GetTags();
		dlg.setTagList(tags);


		int projID = static_cast<ProjectItem*>(ui->treeWidget->selectedItems()[0])->getProjectID();

		if (dlg.exec())
		{
			QVariantMap projectInfo;
			projectInfo.insert("name", dlg.getName());
			projectInfo.insert("description", dlg.getDescription());

			QList<QVariant> tags;
			for(QString tag : dlg.getTags())
			{
				tags.append(tag);
			}
			projectInfo.insert("tags", tags);

			projectInfo.insert("publications", dlg.getPublicationInfo());

			QByteArray payload=QJsonDocument::fromVariant(projectInfo).toJson();

			repoHandler->modifyProject(projID, payload);
		}
	}
	else
	{
		QMessageBox box;
		box.setText("You do not have permission to modify the repository.");
	}

}

void CDatabasePanel::DownloadItem(CustomTreeWidgetItem *item)
{
	int ID;
	int type;

	if(item->type() == PROJECTITEM)
	{
		ProjectItem* projItem = static_cast<ProjectItem*>(item);
		ID = projItem->getProjectID();
		type = FULL;
	}
	else if (item->type() == FILEITEM)
	{
		FileItem* fileItem = static_cast<FileItem*>(item);
		ID = fileItem->getFileID();
		type = PART;
	}
	else
	{
		return;
	}

	QString filename = dbHandler->FullFileNameFromID(ID, type);

	repoHandler->getFile(ID, type);
}

void CDatabasePanel::OpenItem(CustomTreeWidgetItem *item)
{
	int ID;
	int type;

	if(item->type() == PROJECTITEM)
	{
		ProjectItem* projItem = static_cast<ProjectItem*>(item);
		ID = projItem->getProjectID();
		type = FULL;
	}
	else if (item->type() == FILEITEM)
	{
		FileItem* fileItem = static_cast<FileItem*>(item);
		ID = fileItem->getFileID();
		type = PART;
	}
	else
	{
		return;
	}

	QString filename = dbHandler->FullFileNameFromID(ID, type);

	m_wnd->OpenFile(filename);
}

void CDatabasePanel::DeleteItem(CustomTreeWidgetItem *item)
{
	int children = item->childCount();
	for(int child = 0; child < children; child++)
	{
		DeleteItem(static_cast<CustomTreeWidgetItem*>(item->child(child)));
	}

	int ID;
	int type;

	if(item->type() == PROJECTITEM)
	{
		ProjectItem* projItem = static_cast<ProjectItem*>(item);
		ID = projItem->getProjectID();
		type = FULL;
	}
	else if (item->type() == FILEITEM)
	{
		FileItem* fileItem = static_cast<FileItem*>(item);
		ID = fileItem->getFileID();
		type = PART;

		item->SubtractLocalCopy();
	}
	else
	{
		return;
	}

	QString filename = dbHandler->FullFileNameFromID(ID, type);

	QFile::remove(filename);

}

void CDatabasePanel::ShowItemInBrowser(CustomTreeWidgetItem *item)
{
	int ID;
	int type;

	if(item->type() == PROJECTITEM)
	{
		ProjectItem* projItem = static_cast<ProjectItem*>(item);
		ID = projItem->getProjectID();
		type = FULL;
	}
	else if (item->type() == FILEITEM)
	{
		FileItem* fileItem = static_cast<FileItem*>(item);
		ID = fileItem->getFileID();
		type = PART;
	}
	else
	{
		return;
	}

	QFileInfo fileInfo(dbHandler->FullFileNameFromID(ID, type));

	m_wnd->OpenFile(fileInfo.absolutePath());
}

void CDatabasePanel::on_treeWidget_itemSelectionChanged()
{
	if(ui->treeWidget->selectedItems()[0]->type() == 0)
	{
		ui->projectInfoBox->getToolItem(0)->hide();
		ui->projectInfoBox->getToolItem(1)->hide();
		ui->projectInfoBox->getToolItem(2)->hide();

		ui->actionDownload->setVisible(false);
		ui->actionOpen->setVisible(false);
		ui->actionOpenFileLocation->setVisible(false);
		ui->actionDelete->setVisible(false);
		ui->actionDeleteRemote->setVisible(false);
		ui->actionModify->setVisible(false);

		return;
	}

	// Find the project item
	CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(ui->treeWidget->selectedItems()[0]);
	ProjectItem* projItem = static_cast<ProjectItem*>(item->getProjectItem());

	// Display the project info
	dbHandler->GetProjectData(projItem->getProjectID());

	// Get the project tags
	ui->currentTags.clear();
	dbHandler->GetProjectTags(projItem->getProjectID());

	QString tagString;
	if(ui->currentTags.size() > 0)
	{
		tagString = ui->currentTags[0];

		for(int tag = 1; tag < ui->currentTags.size(); tag++)
		{
			tagString += ", ";
			tagString += ui->currentTags[tag];
		}
	}
	ui->projectTags->setText(tagString);

	ui->projectInfoBox->getToolItem(0)->show();

	//Get the project publications
	ui->projectPubs->clear();
	dbHandler->GetProjectPubs(projItem->getProjectID());

	if(ui->projectPubs->count() == 0)
	{
		ui->projectInfoBox->getToolItem(1)->hide();
	}
	else
	{
		ui->projectInfoBox->getToolItem(1)->show();
	}


	// If a file was selected, show update the file info, otherwise hide it
	if(item->type() == FILEITEM)
	{
		dbHandler->GetFileData(static_cast<FileItem*>(item)->getFileID());
		ui->projectInfoBox->getToolItem(2)->show();
	}
	else
	{
		ui->projectInfoBox->getToolItem(2)->hide();
	}


	ui->actionDownload->setVisible(true);
	if(item->LocalCopy())
	{
		ui->actionOpen->setVisible(true);
		ui->actionOpenFileLocation->setVisible(true);
		ui->actionDelete->setVisible(true);
	}
	else
	{
		ui->actionOpen->setVisible(false);
		ui->actionOpenFileLocation->setVisible(false);
		ui->actionDelete->setVisible(false);
	}

	ui->actionDeleteRemote->setVisible(false);
	ui->actionModify->setVisible(false);

	if(item->type() == PROJECTITEM)
	{
		if(static_cast<ProjectItem*>(item)->ownedByUser())
		{
			ui->actionDeleteRemote->setVisible(true);
			ui->actionModify->setVisible(true);
		}
	}


}

void CDatabasePanel::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
	if(ui->treeWidget->itemAt(pos)->type() == 0) return;

	CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(ui->treeWidget->itemAt(pos));
	item->setSelected(true);

	QMenu menu(this);

	menu.addAction(ui->actionDownload);

	if(item->LocalCopy())
	{
		menu.addAction(ui->actionDelete);
		menu.addAction(ui->actionOpen);
		menu.addAction(ui->actionOpenFileLocation);
	}

	if(item->type() == PROJECTITEM)
	{
		if(static_cast<ProjectItem*>(item)->ownedByUser())
			{
				menu.addSeparator();
				menu.addAction(ui->actionDeleteRemote);
				menu.addAction(ui->actionModify);
			}
	}


//	switch(item->type())
//	{
//	case FILEITEM:
//	case PROJECTITEM:
//		if(static_cast<ProjectItem*>(item)->ownedByUser())
//		{
//			menu.addSeparator();
//			menu.addAction(ui->actionDeleteRemote);
//			menu.addAction(ui->actionModify);
//		}
//		break;
//	default:
//		menu.addAction(ui->actionDelete);
//	}

	menu.exec(ui->treeWidget->viewport()->mapToGlobal(pos));
}

void CDatabasePanel::SetProjectData(char **data)
{
	ui->projectName->setText(data[0]);
	ui->projectDesc->setText(data[1]);
	ui->projectOwner->setText(data[2]);
	ui->projectVersion->setText(data[3]);
}

void CDatabasePanel::SetFileData(char **data)
{
	ui->filenameLabel->setText(data[0]);
	ui->fileDescLabel->setText(data[1]);
}

void CDatabasePanel::AddCurrentTag(char **data)
{
	ui->currentTags.append(data[0]);
}

void CDatabasePanel::AddPublication(QVariantMap data)
{
	ui->projectPubs->addPublication(data);
}

QString CDatabasePanel::GetRepositoryFolder()
{
	return m_repositoryFolder;
}

void CDatabasePanel::SetRepositoryFolder(QString folder)
{
	m_repositoryFolder = folder;
}

#else

CDatabasePanel::CDatabasePanel(CMainWindow* pwnd, QWidget* parent){}
CDatabasePanel::~CDatabasePanel(){}
void CDatabasePanel::SetModelList(){}
void CDatabasePanel::ShowMessage(QString message) {}
void CDatabasePanel::LoginTimeout() {}
void CDatabasePanel::NetworkInaccessible() {}
void CDatabasePanel::DownloadFinished(int fileID, int fileType) {}
void CDatabasePanel::AddCategory(char **argv) {}
void CDatabasePanel::AddProject(char **argv) {}
void CDatabasePanel::AddProjectFile(char **argv) {}
void CDatabasePanel::SetProjectData(char **argv) {}
void CDatabasePanel::SetFileData(char **argv) {}
void CDatabasePanel::AddCurrentTag(char **argv) {}
void CDatabasePanel::AddPublication(QVariantMap data) {}
QString CDatabasePanel::GetRepositoryFolder() { return QString(); }
void CDatabasePanel::SetRepositoryFolder(QString folder) {}
void CDatabasePanel::on_loginButton_clicked() {}
void CDatabasePanel::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column){}
void CDatabasePanel::on_actionDownload_triggered() {}
void CDatabasePanel::on_actionOpen_triggered() {}
void CDatabasePanel::on_actionOpenFileLocation_triggered() {}
void CDatabasePanel::on_actionDelete_triggered() {}
void CDatabasePanel::on_actionUpload_triggered() {}
void CDatabasePanel::on_actionSearch_triggered() {}
void CDatabasePanel::on_actionClearSearch_triggered() {}
void CDatabasePanel::on_actionDeleteRemote_triggered() {}
void CDatabasePanel::on_actionModify_triggered() {}
void CDatabasePanel::on_treeWidget_itemSelectionChanged() {}
void CDatabasePanel::on_treeWidget_customContextMenuRequested(const QPoint &pos) {}
void CDatabasePanel::DownloadItem(CustomTreeWidgetItem *item) {}
void CDatabasePanel::OpenItem(CustomTreeWidgetItem *item) {}
void CDatabasePanel::DeleteItem(CustomTreeWidgetItem *item) {}
void CDatabasePanel::ShowItemInBrowser(CustomTreeWidgetItem *item) {}
#endif
