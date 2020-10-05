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

#include <RepositoryPanel.h>
#include "stdafx.h"

#ifdef MODEL_REPO
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <QApplication>
#include <QLocale>
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
#include <QTextBrowser>
#include <QProgressBar>
#include <QTextBrowser>
#include <QLabel>
#include <QFont>
#include <QPushButton>
#include <QTreeWidget>
#include <QJsonDocument>
#include <QByteArray>
#include <QDir>
#include <QFileIconProvider>
#include <JlCompress.h>
#include <QStandardPaths>
#include "RepoConnectionHandler.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "WzdUpload.h"
#include "DlgRequestUploadPerm.h"
//#include "ExportProjectWidget.h"
#include "DlgSetRepoFolder.h"
#include "LocalDatabaseHandler.h"
#include "RepoProject.h"
#include "ToolBox.h"
#include "PublicationWidgetView.h"
#include "IconProvider.h"
#include "FSCore/FSDir.h"
#include "DlgLogin.h"
#include "WrapLabel.h"
#include "TagLabel.h"
#include "ZipFiles.h"

#include <iostream>
#include <QDebug>

enum ITEMTYPES {PROJECTITEM = 1001, FOLDERITEM = 1002, FILEITEM = 1003};

class CustomTreeWidgetItem : public QTreeWidgetItem
{
public:
	CustomTreeWidgetItem(QString name, int type)
		: QTreeWidgetItem(QStringList(name), type), localCopy(0), totalCopies(0), m_size(0)
	{

	}

	virtual CustomTreeWidgetItem* getProjectItem() = 0;

	bool LocalCopy() { return localCopy >= totalCopies; }

	int GetLocalCopy() { return localCopy; }
	int GetTotalCopies() { return totalCopies; }

	virtual void UpdateLocalCopyColor()
	{
		if(LocalCopy())
		{
			setForeground(0, qApp->palette().color(QPalette::Active, QPalette::Text));
			setForeground(1, qApp->palette().color(QPalette::Active, QPalette::Text));
		}
		else
		{
			setForeground(0, qApp->palette().color(QPalette::Disabled, QPalette::Text));
			setForeground(1, qApp->palette().color(QPalette::Disabled, QPalette::Text));
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

	void UpdateSize()
	{
		int currentSize = 0;

		for(int index = 0; index < childCount(); index++)
		{
			CustomTreeWidgetItem* current = static_cast<CustomTreeWidgetItem*>(child(index));
			current->UpdateSize();

			currentSize += current->m_size;
		}

		m_size += currentSize;

		setText(1, qApp->topLevelWidgets()[0]->locale().formattedDataSize(m_size));
	}

protected:
	int localCopy;
	int totalCopies;

	qint64 m_size;

};

class ProjectItem : public CustomTreeWidgetItem
{
public:
	ProjectItem(QString name, int projectID, bool owned, bool authorized)
		: CustomTreeWidgetItem(name, PROJECTITEM), m_projectID(projectID), m_ownedByUser(owned), m_authorized(authorized)
	{
		setIcon(0, CIconProvider::GetIcon("FEBioStudio"));
	}

	CustomTreeWidgetItem* getProjectItem()
	{
		return this;
	}

	void setProjectID(int project) {m_projectID = project;}
	int getProjectID() {return m_projectID;}
	bool ownedByUser() {return m_ownedByUser;}
	bool isAuthorized() {return m_authorized;}

private:
	int m_projectID;
	bool m_ownedByUser;
	bool m_authorized;
};

class FolderItem : public CustomTreeWidgetItem
{
public:
	FolderItem(QString name)
		: CustomTreeWidgetItem(name, FOLDERITEM)
	{
		setIcon(0, CIconProvider::GetIcon("folder"));
	}

	CustomTreeWidgetItem* getProjectItem()
	{
		return ((CustomTreeWidgetItem*) parent())->getProjectItem();
	}

};

class FileItem : public CustomTreeWidgetItem
{
public:
	FileItem(QString name, int fileID, bool lc, qint64 size)
		: CustomTreeWidgetItem(name, FILEITEM), m_fileID(fileID)
	{
		if(name.endsWith(".fsp"))
		{
			setIcon(0, CIconProvider::GetIcon("FEBioStudio"));
		}
		else if(name.endsWith(".fsm") || name.endsWith(".fsprj") || name.endsWith(".prv"))
		{
			setIcon(0, CIconProvider::GetIcon("PreView"));
		}
		else if(name.endsWith(".feb"))
		{
			setIcon(0, CIconProvider::GetIcon("febio"));
		}
		else if(name.endsWith(".xplt"))
		{
			setIcon(0, CIconProvider::GetIcon("PostView"));
		}
		else
		{
			setIcon(0, CIconProvider::GetIcon("new"));
		}


		localCopy = (lc ? 1 : 0);

		totalCopies = 1;

		UpdateLocalCopyColor();

		m_size = size;
	}

	virtual CustomTreeWidgetItem* getProjectItem()
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

class FileSearchItem : public QTreeWidgetItem
{
public:
	FileSearchItem(FileItem* item)
		: QTreeWidgetItem(), realItem(item)
	{
		setText(0, realItem->text(0));
		setText(1, realItem->text(1));
		UpdateColor();
		setIcon(0,realItem->icon(0));
	}

	void UpdateColor()
	{
		setForeground(0, realItem->foreground(0));
		setForeground(1, realItem->foreground(1));
	}

	FileItem* getRealItem()
	{
		return realItem;
	}

private:
	FileItem* realItem;

};

class DisplayTextEdit : public QTextBrowser
{
public:
	DisplayTextEdit(QWidget *parent = nullptr) : QTextBrowser(parent)
	{
		init();
	}

	DisplayTextEdit(const QString &text, QWidget *parent = nullptr) : QTextBrowser(parent)
	{
		init();

		setText(text);
	}

	void setText(const QString &text)
	{
		QTextBrowser::setText(text);

		document()->setTextWidth(size().width());

		setMinimumHeight(document()->size().height());
		setMaximumHeight(document()->size().height());
	}

	void resizeEvent(QResizeEvent *event) override
	{
		document()->setTextWidth(event->size().width());

		setMinimumHeight(document()->size().height());
		setMaximumHeight(document()->size().height());
	}

private:
	void init()
		{
			// Make this look like a QLabel
			setFrameStyle(QFrame::Plain|QFrame::NoFrame);
			QPalette qpalette = palette();
			qpalette.setColor(QPalette::Base, qApp->palette().color(QPalette::Window));
			setPalette(qpalette);

			// Make this only take up the necessary size
			setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

			// Allow hyperlinks to open in a browser
			setOpenExternalLinks(true);
			setTextInteractionFlags(Qt::LinksAccessibleByMouse);
		}
};

class Ui::CRepositoryPanel
{
public:
	QStackedLayout* stack;

	QWidget* welcomePage;
	QPushButton* connectButton;

	QPushButton* loginButton;
	QAction* loginAction;

	QWidget* modelPage;
	QStackedWidget* treeStack;
	QTreeWidget* projectTree;
	QTreeWidget* fileSearchTree;

	CToolBox* projectInfoBox;

	QLabel* unauthorized;

	QFormLayout* projectInfoForm;
	QLabel* projectName;
	DisplayTextEdit* projectDesc;
	QLabel* projectOwner;
	TagLabel* projectTags;

	QFormLayout* fileInfoForm;
	DisplayTextEdit* filenameLabel;
	DisplayTextEdit* fileDescLabel;
	TagLabel* fileTags;

	::CPublicationWidgetView* projectPubs;

	QToolBar* toolbar;

	QAction* actionRefresh;
	QAction* actionDownload;
	QAction* actionOpen;
	QAction* actionOpenFileLocation;
	QAction* actionDelete;

	QAction* actionUpload;

	QAction* actionDeleteRemote;
	QAction* actionModify;

	QAction* actionFindInTree;

	QLineEdit* searchLineEdit;
	QAction* actionSearch;
	QAction* actionClearSearch;

	QWidget* loadingPage;
	QLabel* loadingLabel;
	QProgressBar* loadingBar;
	QPushButton* loadingCancel;

public:
	CRepositoryPanel() : currentProject(nullptr), openAfterDownload(nullptr){}

	void setupUi(::CRepositoryPanel* parent)
	{
		stack = new QStackedLayout(parent);


		// Weclome Page
		QVBoxLayout* welcomeVBLayout = new QVBoxLayout;
		welcomeVBLayout->setAlignment(Qt::AlignCenter);
		QLabel* welcomeLabel = new QLabel("To access the project repository, please click the Connect button below.");
		welcomeLabel->setWordWrap(true);
		welcomeLabel->setAlignment(Qt::AlignCenter);
		welcomeVBLayout->addWidget(welcomeLabel);

		QHBoxLayout* connectButtonLayout = new QHBoxLayout;
		connectButtonLayout->addStretch();
		connectButton = new QPushButton("Connect");
		connectButton->setObjectName("connectButton");
		connectButtonLayout->addWidget(connectButton);
		connectButtonLayout->addStretch();
		welcomeVBLayout->addLayout(connectButtonLayout);

		welcomePage = new QWidget;
		welcomePage->setLayout(welcomeVBLayout);

		stack->addWidget(welcomePage);

		// Model view page
		QVBoxLayout* modelVBLayout = new QVBoxLayout;

		toolbar = new QToolBar();

		actionRefresh = new QAction(CIconProvider::GetIcon("refresh"), "Refresh", parent);
		actionRefresh->setObjectName("actionRefresh");
		actionRefresh->setIconVisibleInMenu(false);
		toolbar->addAction(actionRefresh);

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

		toolbar->addSeparator();
		QWidget* empty = new QWidget();
		empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
		toolbar->addWidget(empty);

		actionDeleteRemote = new QAction(CIconProvider::GetIcon("deleteRemote"), "Delete From Repository", parent);
		actionDeleteRemote->setObjectName("actionDeleteRemote");
		actionDeleteRemote->setIconVisibleInMenu(false);
		toolbar->addAction(actionDeleteRemote);

		actionModify = new QAction(CIconProvider::GetIcon("edit"), "Modify Project", parent);
		actionModify->setObjectName("actionModify");
		actionModify->setIconVisibleInMenu(false);
		toolbar->addAction(actionModify);

		loginButton = new QPushButton("Login");
		loginButton->setObjectName("loginButton");
		loginAction = toolbar->addWidget(loginButton);

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

		actionFindInTree = new QAction("Show in Project Tree", parent);
		actionFindInTree->setObjectName("actionFindInTree");

		QSplitter* splitter = new QSplitter;
		splitter->setOrientation(Qt::Vertical);

		treeStack = new QStackedWidget;

		projectTree = new QTreeWidget;
		projectTree->setObjectName("treeWidget");
		projectTree->setColumnCount(2);
		projectTree->setHeaderLabels(QStringList() << "Projects" << "Size");
		projectTree->setSelectionMode(QAbstractItemView::SingleSelection);
		projectTree->setContextMenuPolicy(Qt::CustomContextMenu);
		treeStack->addWidget(projectTree);

		fileSearchTree = new QTreeWidget;
		fileSearchTree->setObjectName("fileSearchTree");
		fileSearchTree->setColumnCount(2);
		fileSearchTree->setHeaderLabels(QStringList() << "Files" << "Size");
		fileSearchTree->setSelectionMode(QAbstractItemView::SingleSelection);
		fileSearchTree->setContextMenuPolicy(Qt::CustomContextMenu);
		treeStack->addWidget(fileSearchTree);

		splitter->addWidget(treeStack);

		projectInfoBox = new CToolBox;
		QWidget* projectDummy = new QWidget;
		QVBoxLayout* modelInfoLayout = new QVBoxLayout;
		projectDummy->setLayout(modelInfoLayout);

		modelInfoLayout->addWidget(unauthorized = new QLabel("<font color='red'>This project has not yet been approved by our "
				"reviewers. It is visible only to you. For now, you may only modify the metadata or delete the project. Once "
				"approved, it will available for all users.</font>"));
		unauthorized->setWordWrap(true);
		unauthorized->hide();

		QHBoxLayout* centerName = new QHBoxLayout;
		centerName->addStretch();
		centerName->addWidget(projectName = new QLabel);
		centerName->addStretch();

		modelInfoLayout->addLayout(centerName);

		QFont font = projectName->font();
		font.setBold(true);
		font.setPointSize(14);
		projectName->setFont(font);

		modelInfoLayout->addWidget(projectDesc = new DisplayTextEdit);

		QFrame* line = new QFrame();
		line->setFrameShape(QFrame::HLine);
		modelInfoLayout->addWidget(line);

		projectInfoForm = new QFormLayout;
		projectInfoForm->setHorizontalSpacing(10);
		projectInfoForm->addRow("Owner:", projectOwner = new QLabel);

		modelInfoLayout->addLayout(projectInfoForm);

		modelInfoLayout->addWidget(projectTags = new TagLabel);
		projectTags->setObjectName("projectTags");

		projectInfoBox->addTool("Project Info", projectDummy);

		projectInfoBox->addTool("Publications", projectPubs = new ::CPublicationWidgetView(::CPublicationWidgetView::LIST, false));
		projectInfoBox->getToolItem(1)->hide();

		QWidget* fileDummy = new QWidget;
		QVBoxLayout* fileInfoLayout = new QVBoxLayout;
		fileDummy->setLayout(fileInfoLayout);

		fileInfoForm = new QFormLayout;
		fileInfoForm->setHorizontalSpacing(10);
		fileInfoForm->addRow("Filename:", filenameLabel = new DisplayTextEdit);
		fileInfoForm->addRow("Description:", fileDescLabel = new DisplayTextEdit);
		fileInfoLayout->addLayout(fileInfoForm);

		fileInfoLayout->addWidget(fileTags = new TagLabel);
		fileTags->setObjectName("fileTags");

		projectInfoBox->addTool("File Info", fileDummy);
		projectInfoBox->getToolItem(2)->hide();

		splitter->addWidget(projectInfoBox);

		modelVBLayout->addWidget(splitter);

		modelPage = new QWidget;
		modelPage->setLayout(modelVBLayout);

		stack->addWidget(modelPage);

		// Loading Page
		loadingPage = new QWidget;
		QVBoxLayout* loadingLayout = new QVBoxLayout;
		loadingLayout->setAlignment(Qt::AlignCenter);

		loadingLayout->addWidget(loadingLabel = new QLabel);
		loadingLayout->setAlignment(loadingLabel, Qt::AlignCenter);
		loadingLayout->addWidget(loadingBar = new QProgressBar);
		loadingLayout->addWidget(loadingCancel = new QPushButton("Cancel"));
		loadingCancel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
		loadingLayout->setAlignment(loadingCancel, Qt::AlignCenter);

		loadingPage->setLayout(loadingLayout);
		stack->addWidget(loadingPage);

		setLoginVisible(true);
	}

	CustomTreeWidgetItem* addFile(QString &path, int index, int fileID, bool localCopy, qint64 size)
	{
		int pos = path.right(path.length() - index).indexOf("/");

		if(pos == -1)
		{
			FileItem* child = new FileItem(path.right(path.length() - index), fileID, localCopy, size);

			fileItemsByID[fileID] = child;

			return child;
		}

		CustomTreeWidgetItem* child = addFile(path, index + (pos + 1), fileID, localCopy, size);
		CustomTreeWidgetItem* parent;

		try
		{
			parent = currentProjectFolders.at(path.left(pos + index).toStdString());
		}
		catch(out_of_range& e)
		{
			parent = new FolderItem(path.right(path.length() - index).left(pos));

			currentProjectFolders[path.left(pos + index).toStdString()] = parent;
		}

		parent->addChild(child);

		return parent;
	}

	void unhideAll()
	{
		for(auto current : projectItemsByID)
		{
			current.second->setHidden(false);
		}

		for(auto current : fileItemsByID)
		{
			current.second->setHidden(false);
		}
	}

	void setLoginVisible(bool visible)
	{
		loginAction->setVisible(visible);
		actionUpload->setVisible(!visible);

		actionDeleteRemote->setVisible(!visible);
		actionModify->setVisible(!visible);
	}

	void showLoadingPage(QString message, bool progress = false)
	{
		loadingLabel->setText(message);

		loadingBar->setVisible(progress);
		loadingBar->setValue(0);
		loadingCancel->setVisible(progress);

		stack->setCurrentIndex(2);
	}

	void setProjectTags()
	{
		if(currentTags.isEmpty())
		{
			projectTags->hide();
		}
		else
		{
			projectTags->show();
			projectTags->setTagList(currentTags);
		}
	}

	void setFileDescription(QString description)
	{
		fileInfoForm->removeRow(fileDescLabel);

		if(!description.isEmpty())
		{
			fileInfoForm->insertRow(1, "Description:", fileDescLabel = new DisplayTextEdit(description));
		}

	}

	void setFileTags()
	{
		if(currentFileTags.isEmpty())
		{
			fileTags->hide();
		}
		else
		{
			fileTags->show();
			fileTags->setTagList(currentFileTags);
		}
	}


public:
	ProjectItem* currentProject;
	std::unordered_map<std::string, CustomTreeWidgetItem*> currentProjectFolders;
	QStringList currentTags;
	QStringList currentFileTags;
	std::unordered_map<int, ProjectItem*> projectItemsByID;
	std::unordered_map<int, FileItem*> fileItemsByID;

	CustomTreeWidgetItem* openAfterDownload;
};

CRepositoryPanel::CRepositoryPanel(CMainWindow* pwnd, QWidget* parent)
	: QWidget(parent), m_wnd(pwnd), ui(new Ui::CRepositoryPanel)
{
	// build Ui
	ui->setupUi(this);
	QObject::connect(ui->searchLineEdit, &QLineEdit::returnPressed, this, &CRepositoryPanel::on_actionSearch_triggered);
	QObject::connect(ui->loadingCancel, SIGNAL(clicked(bool)), this, SIGNAL(cancelClicked()));

	dbHandler = new CLocalDatabaseHandler(this);
	repoHandler = new CRepoConnectionHandler(this, dbHandler, m_wnd);

	QMetaObject::connectSlotsByName(this);
}

CRepositoryPanel::~CRepositoryPanel()
{
	delete repoHandler;
	delete dbHandler;
	delete ui;
}

void CRepositoryPanel::SetModelList()
{
	ui->projectTree->blockSignals(true);
	ui->projectTree->clear();
	ui->projectTree->blockSignals(false);

	dbHandler->GetCategories();

	if(repoHandler->isAuthenticated())
	{
		ui->setLoginVisible(false);

		QString category("My Projects");
		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(category));
		item->setIcon(0, QIcon(":/icons/folder.png"));
		ui->projectTree->addTopLevelItem(item);
	}

	dbHandler->GetProjects();

	// Delete empty categories.
	vector<int> empty;
	for(int item = 0; item < ui->projectTree->topLevelItemCount(); item++)
	{
		if(ui->projectTree->topLevelItem(item)->childCount() == 0)
		{
			empty.push_back(item);
		}
	}

	for(int item : empty)
	{
		delete ui->projectTree->topLevelItem(item);
	}

	// Resize columns
	ui->projectTree->resizeColumnToContents(0);
	ui->projectTree->setColumnWidth(0, ui->projectTree->columnWidth(0)*2.2);

	// Select the first category
	if(ui->projectTree->topLevelItemCount() > 0)
	{
		ui->projectTree->topLevelItem(0)->setSelected(true);
	}

	ui->stack->setCurrentIndex(1);
}

void CRepositoryPanel::ShowMessage(QString message)
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
}

void CRepositoryPanel::LoginTimeout()
{
	ShowMessage("Your login to the model repository has timed out.");

	ui->setLoginVisible(true);
	ui->stack->setCurrentIndex(1);
}

void CRepositoryPanel::NetworkInaccessible()
{
	ShowMessage("FEBio Studio cannot connect to the network.");
}

void CRepositoryPanel::DownloadFinished(int fileID, int fileType)
{
	if(fileType == FULL)
	{
		// Extract the files from the archive
		QString filename = dbHandler->FullFileNameFromID(fileID, fileType);
		QFileInfo fileInfo(filename);
		QString dir = fileInfo.path() + "/";
		dir += fileInfo.baseName();

		m_wnd->ShowIndeterminateProgress(true, "Unzipping...");
		JlCompress::extractFiles(filename, JlCompress::getFileList(filename), dir);
		m_wnd->ShowIndeterminateProgress(false);

		// Set the appropriate local copy flags
		ui->projectItemsByID[fileID]->setLocalCopyRecursive(true);

	}
	else
	{
		ui->fileItemsByID[fileID]->AddLocalCopy();
	}

	// Update fileSearchItem's color if there's a current file search
	if(ui->treeStack->currentIndex() == 1)
	{
		if(ui->fileSearchTree->selectedItems().count() > 0)
		{
			static_cast<FileSearchItem*>(ui->fileSearchTree->selectedItems()[0])->UpdateColor();
		}
	}

	if(ui->openAfterDownload)
	{
		OpenItem(ui->openAfterDownload);
		ui->openAfterDownload = nullptr;
	}

	on_treeWidget_itemSelectionChanged();
}

void CRepositoryPanel::AddCategory(char **data)
{
	QString category(data[0]);
	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(category));
	item->setIcon(0, QIcon(":/icons/folder.png"));

	ui->projectTree->addTopLevelItem(item);
}

void CRepositoryPanel::AddProject(char **data)
{
	int ID = stoi(data[0]);
	QString name(data[1]);
	QString owner(data[2]);
	QString category(data[3]);
	bool authorized = stoi(data[4]);
	bool cont = authorized;

	bool owned = false;

	if(repoHandler->isAuthenticated())
	{
		if(repoHandler->getUsername().compare(owner) == 0)
		{
			category = "My Projects";
			owned = true;

			cont = true;
		}
	}

	if(cont)
	{
		ProjectItem* projectItem = new ProjectItem(name, ID, owned, authorized);
		ui->projectItemsByID[ID] = projectItem;


		QTreeWidgetItem* categoryItem = nullptr;
		for(int item = 0; item < ui->projectTree->topLevelItemCount(); item++)
		{
			QTreeWidgetItem* current = ui->projectTree->topLevelItem(item);
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
		ui->currentProject->UpdateSize();
	}
}

void CRepositoryPanel::AddProjectFile(char **data)
{
	int ID = std::stoi(data[0]);
	QString filename(data[1]);
	bool localCopy = std::stoi(data[2]);
	qint64 size = QString(data[3]).toLongLong();

	ui->currentProject->addChild(ui->addFile(filename, 0, ID, localCopy, size));
}

void CRepositoryPanel::on_connectButton_clicked()
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

	repoHandler->getSchema();

	ui->showLoadingPage("Connecting...");
}

void CRepositoryPanel::on_loginButton_clicked()
{
	CDlgLogin dlg;

	if(dlg.exec())
	{
		repoHandler->authenticate(dlg.username(), dlg.password());

		ui->showLoadingPage("Logging in...");
	}
}

void CRepositoryPanel::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
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

void CRepositoryPanel::on_fileSearchTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	FileSearchItem* searchItem = static_cast<FileSearchItem*>(item);

	on_treeWidget_itemDoubleClicked(searchItem->getRealItem(),0);
}

void CRepositoryPanel::on_actionRefresh_triggered()
{
	ui->showLoadingPage("Refreshing...");
	repoHandler->getSchema();
}

void CRepositoryPanel::on_actionDownload_triggered()
{
	if(ui->treeStack->currentIndex() == 0)
	{
		DownloadItem(static_cast<CustomTreeWidgetItem*>(ui->projectTree->selectedItems()[0]));
	}
	else
	{
		DownloadItem(static_cast<FileSearchItem*>(ui->fileSearchTree->selectedItems()[0])->getRealItem());
	}

}

void CRepositoryPanel::on_actionOpen_triggered()
{
	if(ui->treeStack->currentIndex() == 0)
	{
		OpenItem(static_cast<CustomTreeWidgetItem*>(ui->projectTree->selectedItems()[0]));
	}
	else
	{
		OpenItem(static_cast<FileSearchItem*>(ui->fileSearchTree->selectedItems()[0])->getRealItem());
	}
}

void CRepositoryPanel::on_actionOpenFileLocation_triggered()
{
	if(ui->treeStack->currentIndex() == 0)
	{
		ShowItemInBrowser(static_cast<CustomTreeWidgetItem*>(ui->projectTree->selectedItems()[0]));
	}
	else
	{
		ShowItemInBrowser(static_cast<FileSearchItem*>(ui->fileSearchTree->selectedItems()[0])->getRealItem());
	}
}

void CRepositoryPanel::on_actionDelete_triggered()
{
	if(ui->treeStack->currentIndex() == 0)
	{
		DeleteItem(static_cast<CustomTreeWidgetItem*>(ui->projectTree->selectedItems()[0]));
	}
	else
	{
		DeleteItem(static_cast<FileSearchItem*>(ui->fileSearchTree->selectedItems()[0])->getRealItem());
	}

	on_treeWidget_itemSelectionChanged();
}

void CRepositoryPanel::on_actionUpload_triggered()
{
	if(repoHandler->getUploadPermission())
	{
		CWzdUpload dlg(this,repoHandler->getUploadPermission(), dbHandler, repoHandler);
		dlg.setOwner(repoHandler->getUsername());

		QStringList categories = GetCategories();
		dlg.setCategories(categories);

		QStringList tags = dbHandler->GetTags();
		dlg.setTagList(tags);


		if (dlg.exec())
		{
			QVariantMap projectInfo;
			projectInfo.insert("name", dlg.getName());
			projectInfo.insert("description", dlg.getDescription());
			projectInfo.insert("category", dbHandler->CategoryIDFromName(dlg.getCategory().toStdString()));

			QList<QVariant> tags;
			for(QString tag : dlg.getTags())
			{
				tags.append(tag);
			}
			projectInfo.insert("tags", tags);

			projectInfo.insert("publications", dlg.getPublicationInfo());

			QStringList filePaths = dlg.GetFilePaths();
			QStringList zipFilePaths = dlg.GetZipFilePaths();

			projectInfo.insert("files", dlg.getFileInfo());

			QByteArray payload=QJsonDocument::fromVariant(projectInfo).toJson();

			qDebug() << payload;

			repoHandler->setUploadReady(false);

			repoHandler->uploadFileRequest(payload);

			QString archiveName = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/.projOutForUpload.prj";

			ui->showLoadingPage("Compressing Files...", true);

			ZipThread* zip = new ZipThread(archiveName, filePaths, zipFilePaths);
			QObject::connect(zip, &ZipThread::resultReady, this, &CRepositoryPanel::updateUploadReady);
			QObject::connect(zip, &ZipThread::finished, zip, &ZipThread::deleteLater);
			QObject::connect(ui->loadingCancel, &QPushButton::clicked, zip, &ZipThread::abort);
			QObject::connect(zip, &ZipThread::progress, this, &CRepositoryPanel::loadingPageProgress);
			zip->start();
		}
	}
	else
	{
		CDlgRequestUploadPerm dlg;

		if(dlg.exec())
		{
			QVariantMap userInfo;

			userInfo.insert("email", dlg.getEmail());
			userInfo.insert("org", dlg.getOrg());
			userInfo.insert("description", dlg.getDescription());

			QByteArray payload=QJsonDocument::fromVariant(userInfo).toJson();

			repoHandler->requestUploadPermissions(payload);
		}

	}

}

void CRepositoryPanel::on_actionSearch_triggered()
{
	ui->unhideAll();

	QString searchTerm = ui->searchLineEdit->text();
	if(searchTerm.isEmpty()) return;

	QString projectSearch;
	QString fileSearch;
	if(searchTerm.contains("files:"))
	{
		QStringList parts = searchTerm.split("files:");
		projectSearch = parts[0];
		fileSearch = parts[1];

		if(!fileSearch.isEmpty())
		{
			std::unordered_set<int> fileIDs = dbHandler->FileSearch(fileSearch);

			ui->fileSearchTree->blockSignals(true);

			ui->fileSearchTree->clear();
			ui->fileSearchTree->setColumnWidth(0, ui->projectTree->columnWidth(0));

			for(int id : fileIDs)
			{
				FileSearchItem* item = new FileSearchItem(ui->fileItemsByID[id]);

				ui->fileSearchTree->addTopLevelItem(item);
			}

			ui->fileSearchTree->blockSignals(false);

			ui->treeStack->setCurrentIndex(1);
		}
	}
	else
	{
		projectSearch = searchTerm;

		std::unordered_set<int> projIDs = dbHandler->FullTextSearch(projectSearch);

		ui->projectTree->blockSignals(true);

		for(auto current : ui->projectItemsByID)
		{
			if(projIDs.count(current.first) == 0)
			{
				current.second->setHidden(true);
			}
		}

		ui->projectTree->blockSignals(false);
		ui->treeStack->setCurrentIndex(0);
	}


}

void CRepositoryPanel::on_actionClearSearch_triggered()
{
	ui->searchLineEdit->clear();

	ui->unhideAll();

	ui->treeStack->setCurrentIndex(0);
}

void CRepositoryPanel::on_actionDeleteRemote_triggered()
{
	if(repoHandler->getUploadPermission())
	{
		int response = QMessageBox::question(this, "Delete Project", "Are you sure that you want to permanently "
				"delete this project from the repository?\n\nThis action cannot be undone.", QMessageBox::Yes | QMessageBox::No);

		if(response == QMessageBox::Yes)
		{
			int projID = static_cast<ProjectItem*>(ui->projectTree->selectedItems()[0])->getProjectID();

			repoHandler->deleteProject(projID);
		}
	}
	else
	{
		QMessageBox::information(this, "Permission Denied", "You do not have permission to delete this project.", QMessageBox::Ok);
	}

}

void CRepositoryPanel::on_actionModify_triggered()
{
	if(repoHandler->getUploadPermission())
	{
		int projID = static_cast<ProjectItem*>(ui->projectTree->selectedItems()[0])->getProjectID();

		CWzdUpload dlg(this, repoHandler->getUploadPermission(), dbHandler, repoHandler, projID); //, m_wnd->GetProject());
		dlg.setName(ui->projectName->text());
		dlg.setOwner(repoHandler->getUsername());

		QStringList categories = GetCategories();
		dlg.setCategories(categories);

		dlg.setDescription(ui->projectDesc->toPlainText());
		dlg.setTags(ui->currentTags);
		dlg.setPublications(ui->projectPubs->getPublications());

		QStringList tags = dbHandler->GetTags();
		dlg.setTagList(tags);

		QList<QList<QVariant>> fileInfo = dbHandler->GetProjectFileInfo(projID);
		dlg.setFileInfo(fileInfo);

		if (dlg.exec())
		{
			QVariantMap projectInfo;
			projectInfo.insert("name", dlg.getName());
			projectInfo.insert("description", dlg.getDescription());
			projectInfo.insert("category", dbHandler->CategoryIDFromName(dlg.getCategory().toStdString()));

			QList<QVariant> tags;
			for(QString tag : dlg.getTags())
			{
				tags.append(tag);
			}
			projectInfo.insert("tags", tags);

			projectInfo.insert("publications", dlg.getPublicationInfo());

			projectInfo.insert("files", dlg.getFileInfo());

			QStringList filePaths = dlg.GetFilePaths();
			QStringList zipFilePaths = dlg.GetZipFilePaths();

			if(filePaths.size() > 0)
			{
				projectInfo.insert("uploadRequired", true);
			}
			else
			{
				projectInfo.insert("uploadRequired", false);
			}

			QByteArray payload=QJsonDocument::fromVariant(projectInfo).toJson();

			ui->showLoadingPage("Modifying Project...");
			repoHandler->setUploadReady(false);

			repoHandler->modifyProject(projID, payload);

			if(filePaths.size() > 0)
			{
				QString archiveName = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/.projOutForUpload.prj";

				ui->showLoadingPage("Compressing Files...", true);

				ZipThread* zip = new ZipThread(archiveName, filePaths, zipFilePaths);
				QObject::connect(zip, &ZipThread::resultReady, this, &CRepositoryPanel::updateModifyReady);
				QObject::connect(zip, &ZipThread::finished, zip, &ZipThread::deleteLater);
				QObject::connect(ui->loadingCancel, &QPushButton::clicked, zip, &ZipThread::abort);
				QObject::connect(zip, &ZipThread::progress, this, &CRepositoryPanel::loadingPageProgress);
				zip->start();
			}
		}
	}
	else
	{
		QMessageBox box;
		box.setText("You do not have permission to modify the repository.");
	}

}

void CRepositoryPanel::on_actionFindInTree_triggered()
{
	FileSearchItem* item = static_cast<FileSearchItem*>(ui->fileSearchTree->selectedItems()[0]);

	ui->projectTree->setCurrentItem(item->getRealItem());

	ui->treeStack->setCurrentIndex(0);

	ui->searchLineEdit->clear();
}

void CRepositoryPanel::UpdateInfo(CustomTreeWidgetItem *item)
{
	ProjectItem* projItem = static_cast<ProjectItem*>(item->getProjectItem());

	ui->unauthorized->setHidden(projItem->isAuthorized());

	// Display the project info
	dbHandler->GetProjectData(projItem->getProjectID());

	// Get the project tags
	ui->currentTags.clear();
	dbHandler->GetProjectTags(projItem->getProjectID());
	ui->setProjectTags();

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

		// Get the file tags
		ui->currentFileTags.clear();
		dbHandler->GetFileTags(static_cast<FileItem*>(item)->getFileID());
		ui->setFileTags();

		ui->projectInfoBox->getToolItem(2)->show();
	}
	else
	{
		ui->projectInfoBox->getToolItem(2)->hide();
	}


	ui->actionDownload->setDisabled(false);
	if(item->LocalCopy())
	{
		ui->actionOpen->setDisabled(false);
		ui->actionOpenFileLocation->setDisabled(false);
		ui->actionDelete->setDisabled(false);
	}
	else
	{
		ui->actionOpen->setDisabled(true);
		ui->actionOpenFileLocation->setDisabled(true);
		ui->actionDelete->setDisabled(true);
	}

	ui->actionDeleteRemote->setDisabled(true);
	ui->actionModify->setDisabled(true);

	if(item->type() == PROJECTITEM)
	{
		if(static_cast<ProjectItem*>(item)->ownedByUser())
		{
			ui->actionDeleteRemote->setDisabled(false);
			ui->actionModify->setDisabled(false);
		}
	}
}

void CRepositoryPanel::DownloadItem(CustomTreeWidgetItem *item)
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
	QFileInfo info(filename);

	ui->showLoadingPage("Downloading " + info.fileName() + "...", true);

	repoHandler->getFile(ID, type);
}

void CRepositoryPanel::OpenItem(CustomTreeWidgetItem *item)
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

void CRepositoryPanel::DeleteItem(CustomTreeWidgetItem *item)
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

	// Update fileSearchItem's color if there's a current file search
	if(ui->treeStack->currentIndex() == 1)
	{
		if(ui->fileSearchTree->selectedItems().count() > 0)
		{
			static_cast<FileSearchItem*>(ui->fileSearchTree->selectedItems()[0])->UpdateColor();
		}
	}

	QString filename = dbHandler->FullFileNameFromID(ID, type);

	QFile::remove(filename);

}

void CRepositoryPanel::ShowItemInBrowser(CustomTreeWidgetItem *item)
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

void CRepositoryPanel::on_treeWidget_itemSelectionChanged()
{
	if(ui->projectTree->selectedItems()[0]->type() == 0)
	{
		ui->projectInfoBox->getToolItem(0)->hide();
		ui->projectInfoBox->getToolItem(1)->hide();
		ui->projectInfoBox->getToolItem(2)->hide();

		ui->actionDownload->setDisabled(true);
		ui->actionOpen->setDisabled(true);
		ui->actionOpenFileLocation->setDisabled(true);
		ui->actionDelete->setDisabled(true);
		ui->actionDeleteRemote->setDisabled(true);
		ui->actionModify->setDisabled(true);

		return;
	}

	// Find the project item
	CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(ui->projectTree->selectedItems()[0]);

	UpdateInfo(item);
}

void CRepositoryPanel::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
	if(ui->projectTree->itemAt(pos)->type() == 0) return;

	CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(ui->projectTree->itemAt(pos));
	item->setSelected(true);

	QMenu menu(this);

	menu.addAction(ui->actionRefresh);

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

	menu.exec(ui->projectTree->viewport()->mapToGlobal(pos));
}

void CRepositoryPanel::on_fileSearchTree_itemSelectionChanged()
{
	FileSearchItem* item = static_cast<FileSearchItem*>(ui->fileSearchTree->selectedItems()[0]);

	UpdateInfo(item->getRealItem());
}

void CRepositoryPanel::on_fileSearchTree_customContextMenuRequested(const QPoint &pos)
{
	FileSearchItem* item = static_cast<FileSearchItem*>(ui->fileSearchTree->itemAt(pos));
	item->setSelected(true);

	QMenu menu(this);

	menu.addAction(ui->actionFindInTree);

	menu.addAction(ui->actionDownload);

	if(item->getRealItem()->LocalCopy())
	{
		menu.addAction(ui->actionDelete);
		menu.addAction(ui->actionOpen);
		menu.addAction(ui->actionOpenFileLocation);
	}

	menu.exec(ui->fileSearchTree->viewport()->mapToGlobal(pos));
}

void CRepositoryPanel::on_projectTags_linkActivated(const QString& link)
{
	ui->searchLineEdit->setText(link);
	on_actionSearch_triggered();
}

void CRepositoryPanel::on_fileTags_linkActivated(const QString& link)
{
	ui->searchLineEdit->setText(QString("files:") + link);
	on_actionSearch_triggered();
}

void CRepositoryPanel::updateUploadReady(bool ready)
{
	if(ready)
	{
		if(repoHandler->isUploadReady()) repoHandler->uploadFile();

		repoHandler->setUploadReady(true);
	}
	else
	{
		repoHandler->cancelUpload();
		ui->stack->setCurrentIndex(1);
	}
}

void CRepositoryPanel::updateModifyReady(bool ready)
{
	if(ready)
	{
		if(repoHandler->isUploadReady()) repoHandler->modifyProjectUpload();

		repoHandler->setUploadReady(true);
	}
	else
	{
		repoHandler->cancelUpload();
		ui->stack->setCurrentIndex(1);
	}
}

QStringList CRepositoryPanel::GetCategories()
{
	int permission = repoHandler->getUploadPermission();

	std::map<int, std::string> categoryMap;

	dbHandler->GetCategoryMap(categoryMap);

	QStringList categories;


	for(auto it : categoryMap)
	{
		if(permission & it.first)
		{
			categories.append(it.second.c_str());
		}
	}

	return categories;
}

void CRepositoryPanel::SetProjectData(char **data)
{
	ui->projectName->setText(data[0]);
	ui->projectOwner->setText(data[2]);

	// Fix new lines
	QString desc(data[1]);
	desc.replace("\\n", "\n");
	ui->projectDesc->setText(desc);
}

void CRepositoryPanel::SetFileData(char **data)
{
	ui->filenameLabel->setText(data[0]);
	ui->setFileDescription(data[1]);
}

void CRepositoryPanel::AddCurrentTag(char **data)
{
	ui->currentTags.append(data[0]);
}

void CRepositoryPanel::AddPublication(QVariantMap data)
{
	ui->projectPubs->addPublication(data);
}

void CRepositoryPanel::AddCurrentFileTag(char **data)
{
	ui->currentFileTags.append(data[0]);
}

QString CRepositoryPanel::GetRepositoryFolder()
{
	return m_repositoryFolder;

	QDir dir(m_repositoryFolder);
	if(!dir.exists()) dir.mkpath(m_repositoryFolder);
}

void CRepositoryPanel::SetRepositoryFolder(QString folder)
{
	m_repositoryFolder = folder;
}

void CRepositoryPanel::showMainPage()
{
	ui->stack->setCurrentIndex(1);
}

void CRepositoryPanel::showLoadingPage(QString message, bool progress)
{
	ui->showLoadingPage(message, progress);
}

void CRepositoryPanel::loadingPageProgress(qint64 bytesSent, qint64 bytesTotal)
{
	ui->loadingBar->setRange(0, bytesTotal);

	ui->loadingBar->setValue(bytesSent);
}

#else

CRepositoryPanel::CRepositoryPanel(CMainWindow* pwnd, QWidget* parent){}
CRepositoryPanel::~CRepositoryPanel(){}
void CRepositoryPanel::SetModelList(){}
void CRepositoryPanel::ShowMessage(QString message) {}
void CRepositoryPanel::LoginTimeout() {}
void CRepositoryPanel::NetworkInaccessible() {}
void CRepositoryPanel::DownloadFinished(int fileID, int fileType) {}
void CRepositoryPanel::AddCategory(char **argv) {}
void CRepositoryPanel::AddProject(char **argv) {}
void CRepositoryPanel::AddProjectFile(char **argv) {}
void CRepositoryPanel::SetProjectData(char **argv) {}
void CRepositoryPanel::SetFileData(char **argv) {}
void CRepositoryPanel::AddCurrentTag(char **argv) {}
void CRepositoryPanel::AddPublication(QVariantMap data) {}
QString CRepositoryPanel::GetRepositoryFolder() { return QString(); }
void CRepositoryPanel::SetRepositoryFolder(QString folder) {}
void CRepositoryPanel::on_loginButton_clicked() {}
void CRepositoryPanel::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column){}
void CRepositoryPanel::on_actionDownload_triggered() {}
void CRepositoryPanel::on_actionOpen_triggered() {}
void CRepositoryPanel::on_actionOpenFileLocation_triggered() {}
void CRepositoryPanel::on_actionDelete_triggered() {}
void CRepositoryPanel::on_actionUpload_triggered() {}
void CRepositoryPanel::on_actionSearch_triggered() {}
void CRepositoryPanel::on_actionClearSearch_triggered() {}
void CRepositoryPanel::on_actionDeleteRemote_triggered() {}
void CRepositoryPanel::on_actionModify_triggered() {}
void CRepositoryPanel::on_treeWidget_itemSelectionChanged() {}
void CRepositoryPanel::on_treeWidget_customContextMenuRequested(const QPoint &pos) {}
void CRepositoryPanel::DownloadItem(CustomTreeWidgetItem *item) {}
void CRepositoryPanel::OpenItem(CustomTreeWidgetItem *item) {}
void CRepositoryPanel::DeleteItem(CustomTreeWidgetItem *item) {}
void CRepositoryPanel::ShowItemInBrowser(CustomTreeWidgetItem *item) {}
void CRepositoryPanel::on_connectButton_clicked() {}
void CRepositoryPanel::on_actionRefresh_triggered() {}
void CRepositoryPanel::on_projectTags_linkActivated(const QString& link) {}
void CRepositoryPanel::on_fileTags_linkActivated(const QString& link) {}
void CRepositoryPanel::updateUploadReady(bool ready) {}
void CRepositoryPanel::updateModifyReady(bool ready) {}
void CRepositoryPanel::loadingPageProgress(qint64 bytesSent, qint64 bytesTotal) {}
void CRepositoryPanel::on_fileSearchTree_itemDoubleClicked(QTreeWidgetItem *item, int column) {}
void CRepositoryPanel::on_actionFindInTree_triggered() {}
void CRepositoryPanel::on_fileSearchTree_itemSelectionChanged() {}
void CRepositoryPanel::on_fileSearchTree_customContextMenuRequested(const QPoint &pos) {}
#endif
