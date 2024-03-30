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

#include "RepositoryPanel.h"
#include "ui_repositorypanel.h"
#include "stdafx.h"

#ifdef MODEL_REPO
#include <vector>
#include <map>
// #include <JlCompress.h>
#include <QStandardPaths>
#include <QDockWidget>
#include <QDesktopServices>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QUrl>
#include <QUrlQuery>
#include <QClipboard>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
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
#include "ModelTypeInfoReader.h"
#include "ServerSettings.h"

#include <iostream>
#include <QDebug>

using std::vector;
using std::pair;
using std::map;
using std::set;

CRepositoryPanel::CRepositoryPanel(CMainWindow* pwnd, QDockWidget* parent)
	: QWidget(parent), m_wnd(pwnd), dock(parent), ui(new Ui::CRepositoryPanel)
{
	// build Ui
	ui->setupUi(this);
	connect(ui->searchLineEdit, &QLineEdit::returnPressed, this, &CRepositoryPanel::on_actionSearch_triggered);
	connect(ui->loadingCancel, SIGNAL(clicked(bool)), this, SIGNAL(cancelClicked()));

    connect(ui->advancedSearch->actionSearch, &QAction::triggered, this, &::CRepositoryPanel::on_actionAdvnacedSearch_triggered);
    connect(ui->advancedSearch->actionClear, &QAction::triggered, this, &::CRepositoryPanel::on_actionAdvancedClear_triggered);
    connect(ui->advancedSearch->actionHide, &QAction::triggered, this, &::CRepositoryPanel::on_actionAdvnacedHide_triggered);

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

void CRepositoryPanel::OpenLink(const QString& link)
{
	// Connect to the repository if we haven't already
	if(ui->stack->currentIndex() == 0)
	{
		linkToOpen = link;
		on_connectButton_clicked();
		return;
	}

	QUrl url(link);
	QString type(url.path());
	QUrlQuery query(url.query());
	if(type == "/project")

	{
		ui->selectProjectByID(query.queryItemValue("ID").toInt());
	}

	linkToOpen.clear();
}

void CRepositoryPanel::Raise()
{
	dock->raise();
}

void CRepositoryPanel::SetModelList()
{
	ui->projectTree->blockSignals(true);
	ui->projectTree->clear();
	ui->projectTree->blockSignals(false);

	dbHandler->GetCategories();

	dbHandler->GetProjects();

	// Select the first category
	if(ui->projectTree->topLevelItemCount() > 0)
	{
		ui->projectTree->topLevelItem(0)->setSelected(true);
	}

    // Set Fields in the advanced search box
    ui->advancedSearch->Reset(dbHandler->GetAdvancedSearchInfo());

	ui->stack->setCurrentIndex(1);

	if(!linkToOpen.isEmpty())
	{
		OpenLink(linkToOpen);
	}
}

void CRepositoryPanel::ShowMessage(QString message, bool logout)
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

    if(logout)
    {
        ui->stack->setCurrentIndex(0);
    }
}

void CRepositoryPanel::ShowWelcomeMessage(QByteArray messages)
{
	QString message;

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
		if(reader.name() == MESSAGES)
		{
			while(reader.readNextStartElement())
			{
				if(reader.name() == MESSAGE)
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
    // This will stop files from being zipped if that's currently happening
    emit cancelClicked();

	ShowMessage("Your login to the model repository has timed out.", true);

	// ui->stack->setCurrentIndex(1);

    // Prompt for login again
    on_actionUpload_triggered();
}

void CRepositoryPanel::NetworkInaccessible()
{
	ShowMessage("FEBio Studio cannot connect to the network.");
}

void CRepositoryPanel::DownloadFinished(int fileID, int fileType)
{
    qint64 now;

	if(fileType == FULL)
	{
		// Extract the files from the archive
		QString filename = dbHandler->FullFileNameFromID(fileID, fileType);
		QFileInfo fileInfo(filename);
		QString dir = fileInfo.path() + "/";
		dir += fileInfo.baseName();

		m_wnd->ShowIndeterminateProgress(true, "Unzipping...");
		// JlCompress::extractFiles(filename, JlCompress::getFileList(filename), dir);
        extractAllFiles(filename, dir);
		m_wnd->ShowIndeterminateProgress(false);

        // Delete archive
        QFile::remove(filename);

        // Get the current time and update the tree items
        now = QDateTime::currentSecsSinceEpoch();
		ui->projectItemsByID[fileID]->justDownloaded(now);
	}
	else
	{
        // Get the current time and update the tree item
        now = QDateTime::currentSecsSinceEpoch();
		ui->fileItemsByID[fileID]->justDownloaded(now);
	}

    // Update the download time(s) in the database
    dbHandler->setDownloadTime(fileID, fileType, now);

	// Update fileSearchItem's color and icon if there's a current file search
	if(ui->treeStack->currentIndex() == 1)
	{
		if(ui->searchTree->selectedItems().count() > 0)
		{
			static_cast<SearchItem*>(ui->searchTree->selectedItems()[0])->Update();
		}
	}

	if(ui->openAfterDownload)
	{
		OpenItem(ui->openAfterDownload);
		ui->openAfterDownload = nullptr;
	}

	on_treeWidget_itemSelectionChanged();
}

void CRepositoryPanel::UploadFinished(bool ok, QString message)
{
	if(!message.isEmpty())
	{
		ShowMessage(message);
	}
	
	if(ui->projectInfo)
	{
		if(!ok)
		{
			on_actionUpload_triggered();
		}
		else
		{
			delete ui->projectInfo;
			ui->projectInfo = nullptr;
		}
	}
	
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
    qint64 uploadTime = QString(data[4]).toLongLong();
    qint64 downloadTime = QString(data[5]).toLongLong();

	ui->currentProject->addChild(ui->addFile(filename, 0, ID, localCopy, size, uploadTime, downloadTime));
}

void CRepositoryPanel::on_connectButton_clicked()
{
	bool getNewFolder = false;

	getNewFolder = m_repositoryFolder.isEmpty();

	if(!getNewFolder)
	{
		getNewFolder = !QFile::exists(m_repositoryFolder);
	}

	if(getNewFolder)
	{
		QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
		defaultPath += "/FEBio Studio Repo Files";

		// set proper separators.
		string sPath = FSDir::filePath(defaultPath.toStdString());

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
		ui->openAfterDownload = customItem;
        DownloadItem(customItem);
	}
	else
	{
		OpenItem(customItem);
	}
}

void CRepositoryPanel::on_searchTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	SearchItem* searchItem = static_cast<SearchItem*>(item);

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
		DownloadItem(static_cast<SearchItem*>(ui->searchTree->selectedItems()[0])->getRealItem());
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
		OpenItem(static_cast<SearchItem*>(ui->searchTree->selectedItems()[0])->getRealItem());
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
		ShowItemInBrowser(static_cast<SearchItem*>(ui->searchTree->selectedItems()[0])->getRealItem());
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
		DeleteItem(static_cast<SearchItem*>(ui->searchTree->selectedItems()[0])->getRealItem());
	}

	on_treeWidget_itemSelectionChanged();
}

void CRepositoryPanel::on_actionCopyPermalink_triggered()
{
	ProjectItem* item = static_cast<ProjectItem*>(ui->projectTree->selectedItems()[0]);

	QString permalink = QString("https://%1:%2/permalink/project/%3").arg(ServerSettings::URL()).arg(ServerSettings::Port()).arg(item->getProjectID());

	QGuiApplication::clipboard()->setText(permalink);
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
	}
	else if(repoHandler->getUploadPermission())
	{
		CWzdUpload dlg(this,repoHandler->getUploadPermission(), dbHandler, repoHandler);
		dlg.setOwner(repoHandler->getUsername());

		QStringList categories = GetCategories();
		dlg.setCategories(categories);

		QStringList tags = dbHandler->GetTags();
		dlg.setTagList(tags);

		if(ui->projectInfo) dlg.setProjectJson(ui->projectInfo);

		if (dlg.exec())
		{
			ui->projectInfo = new QByteArray;
			dlg.getProjectJson(ui->projectInfo);

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

            QVariantList fileInfo = dlg.getFileInfo();

            GetFileMetaDataForUpload(fileInfo, filePaths, zipFilePaths);

			projectInfo.insert("files", fileInfo);

			QByteArray payload=QJsonDocument::fromVariant(projectInfo).toJson();

			repoHandler->setUploadReady(false);

			repoHandler->uploadFileRequest(payload);

			QString archiveName = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/.projOutForUpload.prj";

			ui->showLoadingPage("Compressing Files...", true);

			ZipThread* zip = new ZipThread(archiveName, filePaths, zipFilePaths);
			QObject::connect(zip, &ZipThread::resultReady, this, &CRepositoryPanel::updateUploadReady);
			QObject::connect(zip, &ZipThread::finished, zip, &ZipThread::deleteLater);
			QObject::connect(this, &CRepositoryPanel::cancelClicked, zip, &ZipThread::abort);
			QObject::connect(zip, &ZipThread::progress, this, &CRepositoryPanel::loadingPageProgress);
			zip->start();
		}
		else
		{
			if(ui->projectInfo)
			{
				delete ui->projectInfo;
				ui->projectInfo = nullptr;
			}
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
	QString searchTerm = ui->searchLineEdit->text();
	if(searchTerm.isEmpty()) return;

    searchTerm = "all:" + searchTerm;

    SearchDatabase(searchTerm);
}

void CRepositoryPanel::on_actionClearSearch_triggered()
{
	ui->searchLineEdit->clear();
    ui->advancedSearch->Clear();

	ui->treeStack->setCurrentIndex(0);
}

void CRepositoryPanel::on_actionShowAdvanced_triggered()
{
    ui->searchBar->hide();
    ui->advancedSearch->show();
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
        ProjectItem* item;
        if(ui->treeStack->currentIndex() == 0)
        {
            item = static_cast<ProjectItem*>(ui->projectTree->selectedItems()[0]);
        }
        else
        {
            item = static_cast<ProjectItem*>(static_cast<SearchItem*>(ui->searchTree->selectedItems()[0])->getRealItem());
        }

        int projID = item->getProjectID();

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

            QStringList filePaths = dlg.GetFilePaths();
			QStringList zipFilePaths = dlg.GetZipFilePaths();

            QVariantList fileInfo = dlg.getFileInfo();

            GetFileMetaDataForUpload(fileInfo, filePaths, zipFilePaths);

			projectInfo.insert("files", fileInfo);

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
	SearchItem* item = static_cast<SearchItem*>(ui->searchTree->selectedItems()[0]);

	ui->projectTree->setCurrentItem(item->getRealItem());

	ui->treeStack->setCurrentIndex(0);

	ui->searchLineEdit->clear();
}

void CRepositoryPanel::GetFileMetaDataForUpload(QVariantList& fileInfoList, QStringList& localPaths, QStringList& zipPaths)
{
    for(QVariant& fileInfo : fileInfoList)
    {  
        QString filename = fileInfo.toMap()["filename"].toString();

        // find full file path
        int index; bool found = false;
        for(index = 0; index < zipPaths.size(); index++)
        {
            if(zipPaths[index] == filename)
            {
                found = true;
                break;
            }
        }

        if(!found) continue;

        QString fullFilePath = localPaths[index];

        if(QFileInfo::exists(fullFilePath))
        {
            ModelTypeInfoReader reader;
            if(!reader.ReadTypeInfo(fullFilePath.toStdString())) continue;

            string module = reader.GetModule();
            unordered_map<string, unordered_set<string>> typeInfo = reader.GetTypeInfo();

            QVariantMap metadata;
            metadata["Module"] = QStringList(module.c_str());

            for(auto item : typeInfo)
            {
                if(item.second.size() > 0)
                {
                    QString section = item.first.c_str();
                    
                    QStringList typeNames;

                    for(auto typeName : item.second)
                    {
                        typeNames.append(typeName.c_str());
                    }

                    metadata[section] = typeNames;
                }
            }

            ((QVariantMap&)fileInfo)["metaData"] = metadata;
        }
    }
}

void CRepositoryPanel::SearchDatabase(QString searchTerm)
{
    vector<pair<QString, QStringList>> termList;

    QRegularExpression regex("\\w*:.*?(?=(?:\\w*:)|$)");

    QRegularExpressionMatchIterator i = regex.globalMatch(searchTerm);
    while(i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        QStringList splitResult = match.captured().split(":");
        QString key = splitResult[0];

        // Loop over characters to split terms at spaces, unless the space occurs in a quote
        QStringList values;
        QString value;
        bool inQuote = false;
        for(QChar chr : splitResult[1].trimmed())
        {
            if(chr == '"')
            {
                if(inQuote)
                {
                    inQuote = false;
                    if(!value.isEmpty())
                    {
                        values.push_back(value);
                        value.clear();
                    }
                }
                else
                {
                    inQuote = true;
                    if(!value.isEmpty())
                    {
                        values.push_back(value);
                        value.clear();
                    }
                }
            }
            else if(chr == ' ')
            {
                if(inQuote)
                {
                    value.push_back(chr);
                }
                else
                {
                    if(!value.isEmpty())
                    {
                        values.push_back(value);
                        value.clear();
                    }
                }
            }
            else
            {
                value.push_back(chr);
            }
        }
        
        // Add the last term
        if(!value.isEmpty())
        {
            values.push_back(value);
        }

        if(!values.empty())
        {
            termList.emplace_back(key,values);
        }
    }

    map<pair<int, bool>,int> IDs;

    ui->searchTree->blockSignals(true);
    ui->searchTree->clear();
	ui->searchTree->setColumnWidth(0, ui->projectTree->columnWidth(0));

    bool firstTerm = true;
    for(auto& item : termList)
    {
        QString dataType = item.first;
        QStringList terms = item.second;

        map<int,int> itemIDs;

        for(auto term : terms)
        {
            set<int> tempIDs = dbHandler->ProjectSearch(dataType, term);

            for(int ID : tempIDs)
            {
                try
                {
                    int val = itemIDs.at(ID);

                    itemIDs[ID] = val + 1;
                }
                catch(const std::out_of_range& e)
                {
                    itemIDs[ID] = 1;
                }
            }
        }

        if(firstTerm)
        {
            for(auto& item : itemIDs)
            {
                pair<int,bool> pair(item.first, true);

                IDs[pair] = item.second;
            }
        }
        else
        {
            vector<pair<int, bool>> remove;

            for(auto& item : IDs)
            {
                if(item.first.second)
                {
                    if(itemIDs.count(item.first.first) == 0)
                    {
                        remove.push_back(item.first);
                    }
                    else
                    {
                        IDs[item.first] += itemIDs[item.first.first];
                    }
                }
            }

            for(auto item : remove)
            {
                IDs.erase(item);
            }
        }

        itemIDs.clear();

        for(auto term : terms)
        {
            set<int> tempIDs = dbHandler->FileSearch(dataType, term);

            for(int ID : tempIDs)
            {
                try
                {
                    int val = itemIDs.at(ID);

                    itemIDs[ID] = val + 1;
                }
                catch(const std::out_of_range& e)
                {
                    itemIDs[ID] = 1;
                }
            }
        }

        if(firstTerm)
        {
            for(auto& item : itemIDs)
            {
                pair<int,bool> pair(item.first, false);

                IDs[pair] = item.second;
            }
        }
        else
        {
            vector<pair<int, bool>> remove;

            for(auto& item : IDs)
            {
                if(!item.first.second)
                {
                    if(itemIDs.count(item.first.first) == 0)
                    {
                        remove.push_back(item.first);
                    }
                    else
                    {
                        IDs[item.first] += itemIDs[item.first.first];
                    }
                }
            }

            for(auto item : remove)
            {
                IDs.erase(item);
            }
        }
        
        firstTerm = false;
    }

    vector<pair<pair<int,bool>,int>> results;

    for(auto item : IDs)
    {
        results.push_back(item);
    }

    sort(results.begin(), results.end(), 
        [](pair<pair<int,bool>,int> a,
        pair<pair<int,bool>,int> b)
        {
            return a.second > b.second;
        }
    );

    SearchItem* searchItem;
    for(auto item : results)
    {
        if(item.first.second)
        {
            searchItem = new SearchItem(ui->projectItemsByID[item.first.first]);
        }
        else
        {
            searchItem = new SearchItem(ui->fileItemsByID[item.first.first]);
        }

        ui->searchTree->addTopLevelItem(searchItem);
    }

    ui->searchTree->blockSignals(false);
    ui->treeStack->setCurrentIndex(1);
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

    if(item->type() == FOLDERITEM)
    {
        ui->actionDownload->setDisabled(true);
    }
    else
    {
        ui->actionDownload->setDisabled(false);
    }
	
    // We don't just use item->LocalCopy() because this allows these to work 
    // for partially downloaded projects or folders
	if(item->GetLocalCopy() > 0)
	{
        if(item->type() == FILEITEM)
        {
            ui->actionOpen->setDisabled(false);
        }
		
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

        if(projectModified(projItem))
        {
            int ret = QMessageBox::question(this, "Overwrite File", "You have modifed at least one file in this project after downloading it.\n"
                "Downloading again will overwrite these changes.\n\nWould you like to continue?");

            if(ret == QMessageBox::No)
            {
                ui->openAfterDownload = nullptr;
                return;
            }
        }

	}
	else if (item->type() == FILEITEM)
	{
		FileItem* fileItem = static_cast<FileItem*>(item);
		ID = fileItem->getFileID();
		type = PART;

        if(fileModified(fileItem))
        {
            int ret = QMessageBox::question(this, "Overwrite File", "You have modifed this file after downloading it.\n"
                "Downloading again will overwrite these changes.\n\nWould you like to continue?");

            if(ret == QMessageBox::No)
            {
                ui->openAfterDownload = nullptr;
                return;
            }
        }

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
    if (item->type() != FILEITEM)
    {
        ui->openAfterDownload = nullptr;
        return;
    }

    FileItem* fileItem = static_cast<FileItem*>(item);
    int ID = fileItem->getFileID();

    if(fileItem->outOfDate())
    {
        int answer = QMessageBox::question(this, "File Out of Date", "There is a newer version of this file "
            "available on the repository.\n\nWould you like to download it first?");
        
        if(answer == QMessageBox::Yes)
        {
            ui->openAfterDownload = fileItem;
            DownloadItem(fileItem);
            return;
        }

    }

	QString filename = dbHandler->FullFileNameFromID(ID, PART);

	m_wnd->OpenFile(filename);
}

void CRepositoryPanel::DeleteItem(CustomTreeWidgetItem *item)
{
    // This prevents file items from lowering their parents copy count
    // when they don't already exist
    if(item->GetLocalCopy() <= 0) return;

	int children = item->childCount();
	for(int child = 0; child < children; child++)
	{
		DeleteItem(static_cast<CustomTreeWidgetItem*>(item->child(child)));
	}

    if(item->type() != FILEITEM)
    {
        QDir dir;

        // Removes the directory, only if it is empty. 
        dir.rmdir(getLocalPath(item));

        return;
    }

    FileItem* fileItem = static_cast<FileItem*>(item);
    int ID = fileItem->getFileID();

    QString filename = dbHandler->FullFileNameFromID(ID, PART);

    QFile::remove(filename);

    // Update FileItem in tree
    fileItem->justDeleted();

    // Update fileSearchItem if there's a current file search
    if(ui->treeStack->currentIndex() == 1)
    {
        if(ui->searchTree->selectedItems().count() > 0)
        {
            static_cast<SearchItem*>(ui->searchTree->selectedItems()[0])->Update();
        }
    }
}

void CRepositoryPanel::ShowItemInBrowser(CustomTreeWidgetItem *item)
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(getLocalPath(item)));
}

bool CRepositoryPanel::projectModified(ProjectItem* item)
{
    QList<CustomTreeWidgetItem*> fileItems = item->getFileItems();

    for(auto item : fileItems)
    {
        FileItem* fileItem = static_cast<FileItem*>(item);
        if(fileModified(fileItem)) return true;
    }

    return false;
}

bool CRepositoryPanel::fileModified(FileItem* item)
{
    if(!item->LocalCopy())
    {
        return false;
    }

    QFileInfo info(dbHandler->FullFileNameFromID(item->getFileID(), PART));

    return info.lastModified().toSecsSinceEpoch() > item->downloadTime();
}

QString CRepositoryPanel::getLocalPath(CustomTreeWidgetItem* item)
{
	if(item->type() == PROJECTITEM)
	{
		ProjectItem* projItem = static_cast<ProjectItem*>(item);
		int ID = projItem->getProjectID();

        QFileInfo fileInfo(dbHandler->FullFileNameFromID(ID, FULL));

        return fileInfo.absolutePath() + "/" + fileInfo.baseName();
	}
    else if(item->type() == FOLDERITEM)
	{
        int depth = 0;

        CustomTreeWidgetItem* current = item;
        while(current->childCount() > 0)
        {
            current = static_cast<CustomTreeWidgetItem*>(current->child(0));
            depth++;
        }

        FileItem* fileItem = static_cast<FileItem*>(current);
        int ID = fileItem->getFileID();

        QFileInfo fileInfo(dbHandler->FullFileNameFromID(ID, PART));

        QDir dir = fileInfo.dir();

        while(depth > 1)
        {
            dir.cdUp();
            depth--;
        }

        return dir.absolutePath();
	}
	else if (item->type() == FILEITEM)
	{
		FileItem* fileItem = static_cast<FileItem*>(item);
		int ID = fileItem->getFileID();

        QFileInfo fileInfo(dbHandler->FullFileNameFromID(ID, PART));

        return fileInfo.absolutePath();
	}
	else
	{
		return "";
	}
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
    menu.addSeparator();

    if(item->type() != FOLDERITEM)
    {
        menu.addAction(ui->actionDownload);
    }

    // We don't just use item->LocalCopy() because this allows these to work 
    // for partially downloaded projects or folders
	if(item->GetLocalCopy() > 0)
	{
        if(item->type() == FILEITEM)
        {
            menu.addAction(ui->actionOpen);
        }

		menu.addAction(ui->actionOpenFileLocation);
        menu.addAction(ui->actionDelete);
	}

	if(item->type() == PROJECTITEM)
	{
		menu.addAction(ui->actionCopyPermalink);

		if(static_cast<ProjectItem*>(item)->ownedByUser())
			{
				menu.addSeparator();
				menu.addAction(ui->actionDeleteRemote);
				menu.addAction(ui->actionModify);
			}
	}

	menu.exec(ui->projectTree->viewport()->mapToGlobal(pos));
}

void CRepositoryPanel::on_searchTree_itemSelectionChanged()
{
	SearchItem* item = static_cast<SearchItem*>(ui->searchTree->selectedItems()[0]);

	UpdateInfo(item->getRealItem());
}

void CRepositoryPanel::on_searchTree_customContextMenuRequested(const QPoint &pos)
{
	SearchItem* item = static_cast<SearchItem*>(ui->searchTree->itemAt(pos));
	item->setSelected(true);

	QMenu menu(this);

	menu.addAction(ui->actionFindInTree);

	menu.addAction(ui->actionDownload);

	if(item->getRealItem()->LocalCopy())
	{
		menu.addAction(ui->actionOpen);
		menu.addAction(ui->actionOpenFileLocation);
        menu.addAction(ui->actionDelete);
	}

	menu.exec(ui->searchTree->viewport()->mapToGlobal(pos));
}

void CRepositoryPanel::on_showProjectsCB_stateChanged(int state)
{
    for(int child = 0; child < ui->searchTree->topLevelItemCount(); child++)
    {
        SearchItem* item = static_cast<SearchItem*>(ui->searchTree->topLevelItem(child));

        if(dynamic_cast<ProjectItem*>(item->getRealItem()))
        {
            item->setHidden(state == Qt::Unchecked);
        }
    }
}

void CRepositoryPanel::on_showFilesCB_stateChanged(int state)
{
    for(int child = 0; child < ui->searchTree->topLevelItemCount(); child++)
    {
        SearchItem* item = static_cast<SearchItem*>(ui->searchTree->topLevelItem(child));

        if(dynamic_cast<FileItem*>(item->getRealItem()))
        {
            item->setHidden(state == Qt::Unchecked);
        }
    }
}

void CRepositoryPanel::on_projectTags_linkActivated(const QString& link)
{
	ui->searchLineEdit->setText(link);
	on_actionSearch_triggered();
}

void CRepositoryPanel::on_fileTags_linkActivated(const QString& link)
{
	ui->searchLineEdit->setText(link);
	on_actionSearch_triggered();
}

void CRepositoryPanel::on_actionAdvnacedSearch_triggered()
{
    QString searchTerm = ui->advancedSearch->GetSearchTerm();
    SearchDatabase(searchTerm);
    ui->searchLineEdit->setText(searchTerm);
    on_actionAdvnacedHide_triggered();
}

void CRepositoryPanel::on_actionAdvancedClear_triggered()
{
    ui->advancedSearch->Clear();
    ui->searchLineEdit->clear();
    ui->treeStack->setCurrentIndex(0);
}

void CRepositoryPanel::on_actionAdvnacedHide_triggered()
{
    ui->searchBar->show();
    ui->advancedSearch->hide();
}

void CRepositoryPanel::updateUploadReady(bool ready, QString message)
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

		UploadFinished(false, message);
	}
}

void CRepositoryPanel::updateModifyReady(bool ready, QString message)
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

	map<int, string> categoryMap;

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
	desc.replace("\\n", "<br>");
	ui->projectDesc->setText(desc);
}

void CRepositoryPanel::SetFileData(char **data)
{
	ui->filenameLabel->setText(data[0]);

    QString desc(data[1]);
	desc.replace("\\n", "<br>");
	ui->setFileDescription(desc);
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

QString CRepositoryPanel::GetFilePathFromID(int fileID)
{
    return dbHandler->FullFileNameFromID(fileID, PART);
}

void CRepositoryPanel::loadingPageProgress(qint64 bytesSent, qint64 bytesTotal)
{
	ui->loadingBar->setRange(0, bytesTotal);

	ui->loadingBar->setValue(bytesSent);
}

#else

CRepositoryPanel::CRepositoryPanel(CMainWindow* pwnd, QDockWidget* parent){}
CRepositoryPanel::~CRepositoryPanel(){}
void CRepositoryPanel::OpenLink(const QString& link) {}
// void CRepositoryPanel::Raise() {}
void CRepositoryPanel::SetModelList(){}
void CRepositoryPanel::ShowMessage(QString message, bool logout) {}
void CRepositoryPanel::ShowWelcomeMessage(QByteArray messages) {}
void CRepositoryPanel::LoginTimeout() {}
void CRepositoryPanel::NetworkInaccessible() {}
void CRepositoryPanel::DownloadFinished(int fileID, int fileType) {}
void CRepositoryPanel::UploadFinished(bool error, QString message) {}
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
void CRepositoryPanel::on_actionShowAdvanced_triggered() {}
void CRepositoryPanel::on_actionDeleteRemote_triggered() {}
void CRepositoryPanel::on_actionModify_triggered() {}
void CRepositoryPanel::on_treeWidget_itemSelectionChanged() {}
void CRepositoryPanel::on_treeWidget_customContextMenuRequested(const QPoint &pos) {}
void CRepositoryPanel::on_actionCopyPermalink_triggered() {}
void CRepositoryPanel::Raise() {}
void CRepositoryPanel::DownloadItem(CustomTreeWidgetItem *item) {}
void CRepositoryPanel::OpenItem(CustomTreeWidgetItem *item) {}
void CRepositoryPanel::DeleteItem(CustomTreeWidgetItem *item) {}
void CRepositoryPanel::ShowItemInBrowser(CustomTreeWidgetItem *item) {}
bool CRepositoryPanel::fileModified(FileItem* item) {return false;}
QString CRepositoryPanel::getLocalPath(CustomTreeWidgetItem* item) {return "";}
bool CRepositoryPanel::projectModified(ProjectItem* item) {return false;}
void CRepositoryPanel::on_connectButton_clicked() {}
void CRepositoryPanel::on_actionRefresh_triggered() {}
void CRepositoryPanel::on_projectTags_linkActivated(const QString& link) {}
void CRepositoryPanel::on_fileTags_linkActivated(const QString& link) {}
void CRepositoryPanel::updateUploadReady(bool ready, QString message) {}
void CRepositoryPanel::updateModifyReady(bool ready, QString message) {}
void CRepositoryPanel::loadingPageProgress(qint64 bytesSent, qint64 bytesTotal) {}
void CRepositoryPanel::on_searchTree_itemDoubleClicked(QTreeWidgetItem *item, int column) {}
void CRepositoryPanel::on_actionFindInTree_triggered() {}
void CRepositoryPanel::on_searchTree_itemSelectionChanged() {}
void CRepositoryPanel::on_searchTree_customContextMenuRequested(const QPoint &pos) {}
void CRepositoryPanel::on_showProjectsCB_stateChanged(int state) {}
void CRepositoryPanel::on_showFilesCB_stateChanged(int state) {}
void CRepositoryPanel::on_actionAdvnacedSearch_triggered() {}
void CRepositoryPanel::on_actionAdvancedClear_triggered() {}
void CRepositoryPanel::on_actionAdvnacedHide_triggered() {}
void CRepositoryPanel::SearchDatabase(QString searchTerm) {}
void CRepositoryPanel::GetFileMetaDataForUpload(QVariantList& fileInfoList, QStringList& localPaths, QStringList& zipPaths) {}
QString CRepositoryPanel::GetFilePathFromID(int fileID) {return "";}
#endif
