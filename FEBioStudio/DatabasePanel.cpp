#include "DatabasePanel.h"

#ifdef MODEL_REPO
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QToolBar>
#include <QBoxLayout>
#include <QToolButton>
#include <QStackedLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QFont>
#include <QPushButton>
#include <QTreeWidget>
#include <QJsonDocument>
#include <QVariantMap>
#include <QByteArray>
#include <QDir>
#include <QFileIconProvider>
#include <quazip5/JlCompress.h>
#include "RepoConnectionHandler.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "DlgUpload.h"
#include "LocalDatabaseHandler.h"
#include "RepoProject.h"
#include "ToolBox.h"

#include <iostream>

//class StackedWidget : public QStackedWidget
//{
//public:
//	StackedWidget(QWidget* parent) : QStackedWidget(parent) {}
//private:
//	QSize sizeHint() const override
//	{
//		return parentWidget()->sizeHint();
//	}
//
//	QSize minimumSizeHint() const override
//	{
//		return parentWidget()->minimumSizeHint();
//	}
//
//};

enum ITEMTYPES {PROJECT = 1001, FOLDER = 1002, FILEITEM = 1003};

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
			setForeground(0, Qt::black);
		}
		else
		{
			setForeground(0, Qt::gray);
		}
	}


	void AddLocalCopy()
	{
		localCopy++;
		UpdateLocalCopyColor();

		if(type() != PROJECT)
		{
			static_cast<CustomTreeWidgetItem*>(parent())->AddLocalCopy();

		}
	}

	void SubtractLocalCopy()
	{
		localCopy--;
		UpdateLocalCopyColor();

		if(type() != PROJECT)
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
	ProjectItem(QString name, int projectID)
		: CustomTreeWidgetItem(name, PROJECT), m_projectID(projectID)
	{
		setIcon(0, QIcon(":/icons/FEBioStudio.png"));
	}

	CustomTreeWidgetItem* getProjectItem()
	{
		return this;
	}

	void setProjectID(int project) {m_projectID = project;}
	int getProjectID() {return m_projectID;}

private:
	int m_projectID;
};

class FolderItem : public CustomTreeWidgetItem
{
public:
	FolderItem(QString name)
		: CustomTreeWidgetItem(name, FOLDER)
	{
		setIcon(0, QIcon::fromTheme("folder"));
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
			setIcon(0, QIcon(":/icons/FEBio.png"));
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

class CInfoItem : public QWidget
{
public:
	CInfoItem(QString label, QString data = "", QWidget* parent = 0)
	{
		m_label = new QLabel(label + ":");
		QFont font = m_label->font();
		font.setItalic(true);
		m_label->setFont(font);

		m_data = new QLabel(data);

		QHBoxLayout* layout = new QHBoxLayout;

		layout->addWidget(m_label);

		layout->addSpacing(100 - m_label->fontMetrics().boundingRect(label).width());
		layout->addWidget(m_data);
		layout->addStretch();

		layout->setMargin(0);

		setLayout(layout);
	}

	~CInfoItem(){};

	QLabel* m_data;
	QLabel* m_label;
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
	CInfoItem* ownerInfoItem;
	CInfoItem* versionInfoItem;
	CInfoItem* tagsInfoItem;

	CInfoItem* filenameInfoItem;
	CInfoItem* fileDescInfoItem;

	QToolBar* toolbar;

	QAction* actionDownload;
	QAction* actionOpen;
	QAction* actionOpenFileLocation;
	QAction* actionDelete;
	QAction* actionUpload;

	QLineEdit* searchLineEdit;
	QAction* actionSearch;



public:
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

		loginPage = new QWidget;
		loginPage->setLayout(loginVBLayout);

		stack->addWidget(loginPage);

		// Model view page
		QVBoxLayout* modelVBLayout = new QVBoxLayout;

		toolbar = new QToolBar();

		actionDownload = new QAction(QIcon(":/icons/download.png"), "Download", parent);
		actionDownload->setObjectName("actionDownload");
		actionDownload->setIconVisibleInMenu(false);
		toolbar->addAction(actionDownload);

		actionOpen = new QAction(QIcon(":/icons/open.png"), "Open Local Copy", parent);
		actionOpen->setObjectName("actionOpen");
		actionOpen->setIconVisibleInMenu(false);
		toolbar->addAction(actionOpen);

		actionOpenFileLocation = new QAction(QIcon(":/icons/openContaining.png"), "Open File Location", parent);
		actionOpenFileLocation->setObjectName("actionOpenFileLocation");
		actionOpenFileLocation->setIconVisibleInMenu(false);
		toolbar->addAction(actionOpenFileLocation);

		actionDelete = new QAction(QIcon(":/icons/delete.png"), "Delete Local Copy", parent);
		actionDelete->setObjectName("actionDelete");
		actionDelete->setIconVisibleInMenu(false);
		toolbar->addAction(actionDelete);

		toolbar->addSeparator();
		QWidget* empty = new QWidget();
		empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
		toolbar->addWidget(empty);

		actionUpload = new QAction(QIcon(":/icons/upload.png"), "Upload", parent);
		actionUpload->setObjectName("actionUpload");
		actionUpload->setIconVisibleInMenu(false);
		toolbar->addAction(actionUpload);

		modelVBLayout->addWidget(toolbar);

		QToolBar* searchBar = new QToolBar;
		searchBar->addWidget(searchLineEdit = new QLineEdit);
		actionSearch = new QAction(QIcon(":/icons/search.png"), "Search", parent);
		actionSearch->setObjectName("actionSearch");
		searchBar->addAction(actionSearch);

		modelVBLayout->addWidget(searchBar);

		treeWidget = new QTreeWidget;
		treeWidget->setObjectName("treeWidget");
		treeWidget->setColumnCount(1);
		treeWidget->setHeaderLabel("Project Database");
		treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
		treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		modelVBLayout->addWidget(treeWidget);

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

		modelInfoLayout->addWidget(ownerInfoItem = new CInfoItem("Owner"));
		modelInfoLayout->addWidget(versionInfoItem = new CInfoItem("Version"));
		modelInfoLayout->addWidget(tagsInfoItem = new CInfoItem("Tags"));


		projectInfoBox->addTool("Project Info", projectDummy);

		QWidget* fileDummy = new QWidget;
		QVBoxLayout* fileInfoLayout = new QVBoxLayout;
		fileDummy->setLayout(fileInfoLayout);

		fileInfoLayout->addWidget(filenameInfoItem = new CInfoItem("Filename"));
		fileInfoLayout->addWidget(fileDescInfoItem = new CInfoItem("Description"));

		projectInfoBox->addTool("File Info", fileDummy);
		projectInfoBox->getToolItem(1)->hide();

		modelVBLayout->addWidget(projectInfoBox);

		modelPage = new QWidget;
		modelPage->setLayout(modelVBLayout);

		stack->addWidget(modelPage);

//		QFileIconProvider provider;
//		folderIcon = provider.icon(QFileIconProvider::Folder);

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
	std::unordered_map<int, FileItem*> fileItemsByID;
};

CDatabasePanel::CDatabasePanel(CMainWindow* pwnd, QWidget* parent)
	: QWidget(parent), m_wnd(pwnd), ui(new Ui::CDatabasePanel)
{
	// build Ui
	ui->setupUi(this);

	// Init must be called after the main window has read in the settings
	dbHandler = NULL;
	repoHandler = NULL;

	QMetaObject::connectSlotsByName(this);

//	QObject::connect(ui->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(getProjectData()));
//	QObject::connect(ui->treeWidget, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(contextMenu(const QPoint)));
}

void CDatabasePanel::Init(QString repositoryFolder)
{
	m_repositoryFolder = repositoryFolder;

	QDir dir;
	dir.mkpath(m_repositoryFolder);

	dbHandler = new CLocalDatabaseHandler(m_repositoryFolder.toStdString() + "/localdb.db ", this);
	repoHandler = new CRepoConnectionHandler(this, dbHandler, m_wnd);
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

	dbHandler->GetProjects();

	if(ui->treeWidget->topLevelItemCount() > 0)
	{
		ui->treeWidget->topLevelItem(0)->setSelected(true);
	}

	ui->stack->setCurrentIndex(1);
}

void CDatabasePanel::FailedLogin(QString message)
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

		JlCompress::extractFiles(filename, JlCompress::getFileList(filename), dir);


		// Find the corresponding project item
		int projID = dbHandler->ProjectIDFromFileID(fileID, fileType);

		ProjectItem* projItem = nullptr;

		for(int item = 0; item < ui->treeWidget->topLevelItemCount(); item++)
		{
			ProjectItem* current = static_cast<ProjectItem*>(ui->treeWidget->topLevelItem(item));

			if(current->getProjectID() == projID)
			{
				projItem = current;
				break;
			}
		}

		// Once found, set the appropriate local copy flag
		if(projItem)
		{
			projItem->setLocalCopyRecursive(true);
		}

	}
	else
	{
		ui->fileItemsByID[fileID]->AddLocalCopy();
	}

}

void CDatabasePanel::AddProject(char **data)
{
	int ID = stoi(data[0]);
	QString name = data[1];

	ProjectItem* projectItem = new ProjectItem(name, ID);
	ui->treeWidget->addTopLevelItem(projectItem);

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
	ui->setLoginDisabled(true);
	repoHandler->authenticate(ui->userName->text(), ui->password->text());
}

void CDatabasePanel::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	CustomTreeWidgetItem* customItem = static_cast<CustomTreeWidgetItem*>(item);

	if(!customItem->LocalCopy())
	{
		DownloadItem(customItem);
	}

	OpenItem(customItem);
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
}

void CDatabasePanel::on_actionUpload_triggered()
{
	CDlgUpload dlg(this);
	dlg.setName(m_wnd->GetDocument()->GetDocFileBase().c_str());
	dlg.setOwner(repoHandler->getUsername());
	dlg.setVersion("1");


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

		QByteArray payload=QJsonDocument::fromVariant(projectInfo).toJson();

		repoHandler->upload(payload);
	}

}

void CDatabasePanel::on_actionSearch_triggered()
{
	std::unordered_set<int> projIDs = dbHandler->FullTextSearch(ui->searchLineEdit->text());

	ui->treeWidget->blockSignals(true);

	for(int item = 0; item < ui->treeWidget->topLevelItemCount(); item++)
	{
		ProjectItem* current = static_cast<ProjectItem*>(ui->treeWidget->topLevelItem(item));

		if(projIDs.count(current->getProjectID()) > 0)
		{
			current->setHidden(false);
		}
		else
		{
			current->setHidden(true);
		}
	}

	ui->treeWidget->blockSignals(false);

}

void CDatabasePanel::DownloadItem(CustomTreeWidgetItem *item)
{
	int ID;
	int type;

	if(item->type() == PROJECT)
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

	if(item->type() == PROJECT)
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

	if(item->type() == PROJECT)
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

	if(item->type() == PROJECT)
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
	ui->tagsInfoItem->m_data->setText(tagString);

	// If a file was selected, show update the file info, otherwise hide it
	if(item->type() == FILEITEM)
	{
		dbHandler->GetFileData(static_cast<FileItem*>(item)->getFileID());
		ui->projectInfoBox->getToolItem(1)->show();
	}
	else
	{
		ui->projectInfoBox->getToolItem(1)->hide();
	}

	if(item->LocalCopy())
	{
		ui->actionOpen->setEnabled(true);
		ui->actionOpenFileLocation->setEnabled(true);
		ui->actionDelete->setEnabled(true);
	}
	else
	{
		ui->actionOpen->setEnabled(false);
		ui->actionOpenFileLocation->setEnabled(false);
		ui->actionDelete->setEnabled(false);
	}
}

void CDatabasePanel::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
	CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(ui->treeWidget->itemAt(pos));
	item->setSelected(true);

	QMenu menu(this);

	switch(item->type())
	{
	case PROJECT:
	case FILEITEM:
	{
//		menu.addAction("Download", this, SLOT(OnDownloadClicked()));

		menu.addAction(ui->actionDownload);

		if(item->LocalCopy())
		{
			menu.addAction(ui->actionDelete);
			menu.addAction(ui->actionOpen);
			menu.addAction(ui->actionOpenFileLocation);
		}
		break;
	}
	default:
		menu.addAction(ui->actionDelete);
	}

	menu.exec(ui->treeWidget->viewport()->mapToGlobal(pos));
}

void CDatabasePanel::SetProjectData(char **data)
{
	ui->projectName->setText(data[0]);
	ui->projectDesc->setText(data[1]);
	ui->ownerInfoItem->m_data->setText(data[2]);
	ui->versionInfoItem->m_data->setText(data[3]);
}

void CDatabasePanel::SetFileData(char **data)
{
	ui->filenameInfoItem->m_data->setText(data[0]);
	ui->fileDescInfoItem->m_data->setText(data[1]);
}

void CDatabasePanel::AddCurrentTag(char **data)
{
	ui->currentTags.append(data[0]);
}

void CDatabasePanel::PrintModel(char **argv)
{
	cout << argv[0] << endl;
	cout << argv[1] << endl;
	cout << argv[2] << endl;
	cout << argv[3] << endl;
}

QString CDatabasePanel::RepositoryFolder()
{
	return m_repositoryFolder;
}

#else

CDatabasePanel::CDatabasePanel(CMainWindow* pwnd, QWidget* parent){}
CDatabasePanel::~CDatabasePanel(){}
void CDatabasePanel::Init(QString repositoryFolder) {}
void CDatabasePanel::SetModelList(){}
void CDatabasePanel::on_loginButton_clicked(){}
void CDatabasePanel::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column){}
void CDatabasePanel::on_actionDownload_triggered() {}
void CDatabasePanel::on_actionOpen_triggered() {}
void CDatabasePanel::on_actionOpenFileLocation_triggered() {}
void CDatabasePanel::on_actionDelete_triggered() {}
void CDatabasePanel::on_actionUpload_triggered() {}
void CDatabasePanel::on_treeWidget_itemSelectionChanged() {}
void CDatabasePanel::on_treeWidget_customContextMenuRequested(const QPoint &pos) {}

#endif
