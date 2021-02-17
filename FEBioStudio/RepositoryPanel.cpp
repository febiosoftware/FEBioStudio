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

#include "RepositoryPanel.h"
#include "ui_repositorypanel.h"
#include "stdafx.h"

#ifdef MODEL_REPO
#include <vector>
#include <unordered_map>
#include <JlCompress.h>
#include <QStandardPaths>
#include <QDateTime>
#include <QXmlStreamReader>
#include "RepoConnectionHandler.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "WzdUpload.h"
#include "DlgRequestUploadPerm.h"
#include "DlgSetRepoFolder.h"
#include "LocalDatabaseHandler.h"
#include "ToolBox.h"
#include "IconProvider.h"
#include "FSCore/FSDir.h"
#include "DlgLogin.h"
#include "WrapLabel.h"
#include "TagLabel.h"
#include "ZipFiles.h"

#include <iostream>
#include <QDebug>

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

void CRepositoryPanel::ShowWelcomeMessage(QByteArray messages)
{
	QString message;

	qDebug() << lastMessageTime;
	
	if(lastMessageTime == -1)
	{
	 	message += "<h2>Welcome to the FEBio Project Repository!</h2><p>This repository is a way for FEBio users to easily access models "
            "created by the FEBio Team or by other users, and to share models of their own from within FEBio Studio itself.<br><br>"
            "Using this panel, you can view or search for projects and download the associated files.<br><br>"
            "To share a project of your own, click on the Upload button in the upper right corner of the panel.<br><br>"
			"For more information, please see the section entitled \"The Repository Panel\" in the FEBio Studio user manual.</p>";
	}

	qint64 newMessageTime = 0;

	QXmlStreamReader reader(messages);

	if (reader.readNextStartElement())
	{
		if(reader.name() == "messages")
		{
			while(reader.readNextStartElement())
			{
				if(reader.name() == "message")
				{
					qint64 time = reader.attributes().value("time").toLongLong();

					if(time > lastMessageTime)
					{
						if(time > newMessageTime) newMessageTime = time;

						QDateTime dateTime;
						dateTime.setSecsSinceEpoch(time);

						if(!message.isEmpty()) message += "<br>";

						message += "<h2>Message from ";
						message += dateTime.toString("ddd MMMM d yyyy:");
						message += "</h2><p>";

						message += reader.readElementText().replace("\n", "<br>");
						message += "</p>";

					}
				}
				else
				{
					reader.skipCurrentElement();
				}
			}
		}
		else
		{

		}
	}

	if(!message.isEmpty())
	{
		QDialog *dlg = new QDialog(this);
		QVBoxLayout* l = new QVBoxLayout;
		dlg->setLayout(l);
		QTextBrowser* msg = new QTextBrowser();

		msg->setFrameStyle(QFrame::Plain|QFrame::NoFrame);
		QPalette qpalette = palette();
		qpalette.setColor(QPalette::Base, qApp->palette().color(QPalette::Window));
		msg->setPalette(qpalette);

		msg->setText(message);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
		l->addWidget(msg);
		l->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));

		int width = 600;
		msg->document()->setTextWidth(width);
		int height = msg->document()->size().height() + 100;

		if(height > 500) height = 500;

		dlg->resize(width,height);
		dlg->exec();

		lastMessageTime = newMessageTime;
	}
}

void CRepositoryPanel::LoginTimeout()
{
	ShowMessage("Your login to the model repository has timed out.");

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
	int ID = std::stoi(data[0]);
	QString name(data[1]);
	QString owner(data[2]);
	QString category(data[3]);
	bool authorized = std::stoi(data[4]);
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
	if(!repoHandler->isAuthenticated())
	{
		CDlgLogin dlg;

		if(dlg.exec())
		{
			repoHandler->authenticate(dlg.username(), dlg.password());

			ui->showLoadingPage("Logging in...");
		}

		return;
	}

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

		CWzdUpload dlg(this, repoHandler->getUploadPermission(), dbHandler, repoHandler, projID);
		dlg.setName(ui->projectName->text());
		dlg.setOwner(repoHandler->getUsername());

		QStringList categories = GetCategories();
		dlg.setCategories(categories);

		dlg.setCategory(dbHandler->CategoryFromID(projID));

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

qint64 CRepositoryPanel::GetLastMessageTime()
{
	return lastMessageTime;
}

void CRepositoryPanel::SetLastMessageTime(qint64 time)
{
	lastMessageTime = time;
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
void CRepositoryPanel::ShowWelcomeMessage(QByteArray messages) {}
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
qint64 CRepositoryPanel::GetLastMessageTime() { return -1; }
void CRepositoryPanel::SetLastMessageTime(qint64 time) {}
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
