#include <vector>
#include <QApplication>
#include <QProcess>
#include <QDialog>
#include <QDialogButtonBox>
#include <QWizardPage>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QAbstractButton>
#include <QMessageBox>
#include <QListWidget>
#include <QTextEdit>
#include <QBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QProgressBar>
#include <QCheckBox>
#include <QDateTime>
#include <QSaveFile>
#include <QDebug>
#include <QLocale>
#include <QStringList>
#include <QXmlStreamReader>
#include "FEBioStudioUpdater.h"
#include "MainWindow.h"
#include <XML/XMLWriter.h>
#include <FEBioStudio/UpdateChecker.h>
#include <FEBioStudio/ServerSettings.h>
#include <FEBioStudio/PluginManager.h>
#include "ZipThread.h"

#include <iostream>

namespace Ui
{
class MyWizardPage : public QWizardPage
{
public:

	MyWizardPage()
	{
		complete = false;
		emit completeChanged();
	}

	bool isComplete() const
	{
		return complete;
	}

	void setComplete(bool comp)
	{
		complete = comp;

		emit completeChanged();
	}


private:
	bool complete;

};
}

class Ui::CMainWindow
{
public:
	QWizardPage* startPage;

	MyWizardPage* infoPage;
	QVBoxLayout* infoLayout;
	::CUpdateWidget* updateWidget;

	MyWizardPage* downloadPage;
	QLabel* downloadOverallLabel;
	QProgressBar* overallProgress;
	QLabel* downloadFileLabel;
	QProgressBar* fileProgress;
	QCheckBox* relaunch;

    CMainWindow() : currentIndex(0), currentZippedIndex(0), overallSize(0), downloadedSize(0) { }

	void setup(::CMainWindow* wnd, bool correctDir)
	{
		m_wnd = wnd;

		// Start page
		startPage = new QWizardPage;
		QVBoxLayout* startLayout = new QVBoxLayout;

		if(correctDir)
		{
			startLayout->addWidget(new QLabel("Welcome to the FEBio Studio Auto-Updater.\n\nClick Next to check for updates."));
		}
		else
		{
			startLayout->addWidget(new QLabel("This updater does not appear to be located in the 'bin' folder of an FEBio Studio "
					"installation.\n\nUpdate cannot proceed."));
		}

		startPage->setLayout(startLayout);
		wnd->addPage(startPage);

		if(correctDir)
		{
			// Info page
			infoPage = new MyWizardPage;
			infoLayout = new QVBoxLayout;

			infoLayout->addWidget(updateWidget = new ::CUpdateWidget);

			infoPage->setLayout(infoLayout);

			wnd->addPage(infoPage);

			downloadPage = new MyWizardPage;
			QVBoxLayout* downloadLayout = new QVBoxLayout;

			downloadLayout->addWidget(downloadFileLabel = new QLabel);
			downloadLayout->addWidget(fileProgress = new QProgressBar);

			downloadLayout->addWidget(downloadOverallLabel= new QLabel);
			downloadLayout->addWidget(overallProgress = new QProgressBar);

			downloadPage->setLayout(downloadLayout);

			wnd->addPage(downloadPage);
		}

		relaunch = nullptr;

		QObject::connect(updateWidget, &::CUpdateWidget::ready, m_wnd, &::CMainWindow::updateWidgetReady);
	}

	void downloadsFinished()
	{
		fileProgress->hide();
		downloadOverallLabel->hide();
		overallProgress->hide();

		if(updateWidget->doingUpdaterUpdate)
		{
			downloadFileLabel->setText("Auto-Updater update complete!\n\nThe updater must now restart to update FEBio Studio.\n\n"
					"Click Finish to restart the updater and start the update to FEBio Studio.");

			downloadFileLabel->setWordWrap(true);
		}
		else
		{
			downloadFileLabel->setText("Update Complete!");

			downloadPage->layout()->addWidget(relaunch = new QCheckBox("Relaunch FEBio Studio."));
			relaunch->setChecked(true);
		}

		downloadPage->setComplete(true);

		QList<QWizard::WizardButton> button_layout;
  		button_layout << QWizard::Stretch << QWizard::FinishButton;
  		m_wnd->setButtonLayout(button_layout);
	}

public:
	::CMainWindow* m_wnd;

    QStringList newFiles;
	QStringList newDirs;
	int currentIndex;
    int currentZippedIndex;
    std::vector<int> zippedFiles;
	qint64 overallSize;
	qint64 downloadedSize;

    CPluginManager pluginManager;
    int currentPluginID;
    std::vector<int> pluginIDs;
};


CMainWindow::CMainWindow(bool devChannel, bool updaterUpdateCheck, QString& branch) 
	: ui(new Ui::CMainWindow), restclient(new QNetworkAccessManager), m_devChannel(devChannel), 
    m_updaterUpdateCheck(updaterUpdateCheck), m_downloadingSDK(false), m_branch(branch)
{
	QString dir = QApplication::applicationDirPath();
	bool correctDir = false;

#ifdef WIN32
	if(dir.contains("FEBio") && dir.contains("Studio") && dir.contains("bin"))
	{
		correctDir = true;
	}
#elif __APPLE__
	if(dir.contains("FEBio") && dir.contains("Studio") && dir.contains("MacOS"))
	{
		correctDir = true;
	}
#else
	if(dir.contains("FEBio") && dir.contains("Studio") && dir.contains("bin"))
	{
		correctDir = true;
	}
#endif
	
	setWindowTitle("FEBio Studio Updater");

	setWizardStyle(QWizard::ModernStyle);

	setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/leftSide.png"));
	setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/leftSide.png"));
	setPixmap(QWizard::LogoPixmap, QPixmap(":/images/FEBioStudio.png"));

	ui->setup(this, correctDir);

	QList<QWizard::WizardButton> button_layout;
  	button_layout << QWizard::Stretch << QWizard::NextButton << QWizard::CancelButton;
  	setButtonLayout(button_layout);

    ui->pluginManager.SetDevelop(m_devChannel);

	connect(this->button(QWizard::FinishButton), &QPushButton::clicked, this, &CMainWindow::onFinish);
	connect(restclient, &QNetworkAccessManager::finished, this, &CMainWindow::connFinished);
	connect(restclient, &QNetworkAccessManager::sslErrors, this, &CMainWindow::sslErrorHandler);
    
    connect(&ui->pluginManager, &CPluginManager::downloadProgress, this, &CMainWindow::progress);
    connect(&ui->pluginManager, &CPluginManager::DownloadFinished, this, &CMainWindow::pluginDownloadFinished);
}

bool CMainWindow::checkBinaries()
{
#ifdef WIN32
    // Hack for febiostudio 3 branch
    if(!isFileWriteable(QApplication::applicationDirPath() + "\\FEBioStudio.exe", "FEBio Studio"))
    {
        return false;
    }

    if(!isFileWriteable(QApplication::applicationDirPath() + FBSBINARY, "FEBio Studio"))
    {
        return false;
    }

    if(!isFileWriteable(QApplication::applicationDirPath() + FEBIOBINARY, "FEBio"))
    {
        return false;
    }
#endif

    return true;
}
    
bool CMainWindow::isFileWriteable(QString filename, QString niceName)
{
    QDialog dlg;
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(new QLabel(QString("Cannot write to %1.\n\nIs %2 still running?").arg(filename).arg(niceName)));
    QDialogButtonBox* box = new QDialogButtonBox;
    box->addButton("Retry", QDialogButtonBox::AcceptRole);
    box->addButton(QDialogButtonBox::Cancel);
    layout->addWidget(box);
    dlg.setLayout(layout);

    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    QFile file(filename);
    
    if(!file.exists())
    {
        return true;
    }

    while(true)
    {
        if(file.open(QIODevice::Append))
        {
            file.close();
            return true;
        }
        else
        {
            if(!dlg.exec())
            {
                return false;
            }
        }
    }
}

void CMainWindow::getFile()
{
    ReleaseFile& currentFile = ui->updateWidget->updateFiles[ui->currentIndex];

	ui->downloadFileLabel->setText(QString("Downloading %1...").arg(currentFile.name));
	ui->downloadOverallLabel->setText(QString("Downloading File %1 of %2").arg(ui->currentIndex + 1).arg(ui->updateWidget->updateFiles.size()));

	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());

	myurl.setPath(currentFile.baseURL + "/" + currentFile.name);

    if(!m_branch.isEmpty())
    {
        QUrlQuery query;
        query.addQueryItem("branch", m_branch);

        myurl.setQuery(query);
    }

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

    QNetworkReply* reply = restclient->get(request);

    QObject::connect(reply, &QNetworkReply::downloadProgress, this, &CMainWindow::progress);
}

void CMainWindow::getFileReponse(QNetworkReply *r)
{
    ReleaseFile& currentFile = ui->updateWidget->updateFiles[ui->currentIndex];

	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode != 200)
	{
		QMessageBox::critical(this, "Update Failed", QString("Update Failed!\n\nUnable to download %1.").arg(currentFile.name));
		QApplication::quit();
		return;
	}

	QByteArray data = r->readAll();

	QFileInfo fileInfo(QApplication::applicationDirPath() + QString(REL_ROOT) + currentFile.name);

	// Ensure that the path to the file exists. Add any newly created directories to autoUpdate.xml
	// for deletion during uninstalltion
	makePath(QDir::fromNativeSeparators(fileInfo.absolutePath()));

	QString fileName = fileInfo.absoluteFilePath();
    addNewFile(fileName);

	// If we're downloading an updater file, add the suffix ".temp"
	if(ui->updateWidget->doingUpdaterUpdate)
	{
		if(!fileName.contains("mvUtil"))
		{
			fileName += ".temp";
		}
	}
	
	QSaveFile file(fileName);
	file.open(QIODevice::WriteOnly);

	file.write(data);
	file.commit();

	file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::WriteUser | QFileDevice::ExeUser | QFileDevice::ReadUser);

    if(currentFile.name.endsWith(".zip"))
    {
        ui->zippedFiles.push_back(ui->currentIndex);
    }

	ui->currentIndex++;
	ui->downloadedSize += data.size();

	if(ui->currentIndex < ui->updateWidget->updateFiles.size())
	{
		getFile();
	}
    else
	{
		unzipFiles();
	}
}

void CMainWindow::unzipFiles()
{
    if(ui->currentZippedIndex < ui->zippedFiles.size())
    {
        ReleaseFile& currentFile = ui->updateWidget->updateFiles[ui->zippedFiles[ui->currentZippedIndex]];

        ui->downloadOverallLabel->hide();
        ui->overallProgress->hide();

        ui->downloadFileLabel->setText(QString("Unzipping %1...").arg(currentFile.name));

        QFileInfo info(QApplication::applicationDirPath() + QString(REL_ROOT) + currentFile.name);

        QString zipFileName = info.absoluteFilePath();
        QString outDir = info.absolutePath();

        ZipThread* thread = new ZipThread(this, zipFileName, outDir);

        connect(thread, &ZipThread::progress, this, &CMainWindow::progress);
        connect(thread, &ZipThread::resultReady, this, &CMainWindow::unzipFinished);

        thread->run();
    }
    else
    {
        if(ui->updateWidget->m_getSDK && ui->updateWidget->m_getSDK->isChecked())
        {
            getSDK();
        }
        else
        {
            updatePlugins();
	    }
    }
}

void CMainWindow::unzipFinished()
{
    ReleaseFile& currentFile = ui->updateWidget->updateFiles[ui->zippedFiles[ui->currentZippedIndex]];

    QFile zip = QApplication::applicationDirPath() + QString(REL_ROOT) + currentFile.name;

    if(zip.exists())
    {
        zip.remove();
    }

    ui->currentZippedIndex++;

    unzipFiles();
}


void CMainWindow::getSDK()
{
    ui->downloadOverallLabel->hide();
    ui->overallProgress->hide();
    
    ui->downloadFileLabel->setText("Downloading SDK...");

    QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());

    myurl.setPath(ui->updateWidget->m_sdk.baseURL + "/" + ui->updateWidget->m_sdk.name);

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

    m_downloadingSDK = true;

    QNetworkReply* reply = restclient->get(request);

    QObject::connect(reply, &QNetworkReply::downloadProgress, this, &CMainWindow::progress);
}

void CMainWindow::getSDKResponse(QNetworkReply *r)
{
    int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode != 200)
	{
		QMessageBox::critical(this, "Update Failed", "Update Failed!\n\nUnable to download SDK.");
		QApplication::quit();
		return;
	}

	QByteArray data = r->readAll();

	QFileInfo zipFileInfo(QApplication::applicationDirPath() + QString(REL_ROOT) + "sdk.zip");
	QString zipFileName = zipFileInfo.absoluteFilePath();

    QFileInfo sdkDir(QApplication::applicationDirPath() + QString(REL_ROOT) + "sdk");
    QString sdkDirName = sdkDir.absoluteFilePath();

    // If the sdk dir doesn't already exist, create it and add it to autoUpdate.xml for deletion during uninstalltion.
    makePath(QDir::fromNativeSeparators(sdkDirName));

	QSaveFile file(zipFileName);
	file.open(QIODevice::WriteOnly);

	file.write(data);
	file.commit();

	file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::WriteUser | QFileDevice::ExeUser | QFileDevice::ReadUser);

    ui->downloadFileLabel->setText("Unzipping SDK...");

    ZipThread* thread = new ZipThread(this, zipFileName, sdkDirName);

    connect(thread, &ZipThread::progress, this, &CMainWindow::progress);
    connect(thread, &ZipThread::resultReady, this, &CMainWindow::unzipSDKFinished);

    thread->run();
}

void CMainWindow::unzipSDKFinished()
{
    QFile zip(QApplication::applicationDirPath() + QString(REL_ROOT) + "sdk.zip");

	if(zip.exists())
	{
		zip.remove();
	}

    updatePlugins();
}

void CMainWindow::pluginDownloadFinished()
{
    getNextPlugin();
}

void CMainWindow::initializePage(int id)
{
	switch(id)
	{
	case 1:
		ui->updateWidget->checkForUpdate(m_devChannel, true, m_updaterUpdateCheck, m_branch);
		break;
	case 2:
		deleteFiles();
		getFile();
		break;
	default:
		break;
	}
}

void CMainWindow::updateWidgetReady(bool update, bool terminal)
{
	if(!update || terminal)
	{
		this->removePage(2);

		QList<QWizard::WizardButton> button_layout;
  		button_layout << QWizard::Stretch << QWizard::FinishButton;
  		setButtonLayout(button_layout);
	}
    else
    {
        if(!checkBinaries())
        {
            QApplication::quit();
        }
    }

	ui->infoPage->setComplete(true);
    
    ui->overallSize = ui->updateWidget->overallSize;
}

void CMainWindow::connFinished(QNetworkReply *r)
{
    if(m_downloadingSDK)
    {
        getSDKResponse(r);
    }
    else
    {
        getFileReponse(r);
    }
}

void CMainWindow::sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors)
{
	for(QSslError error : errors)
	{
		qDebug() << error.errorString();
	}

	reply->ignoreSslErrors();
}

void CMainWindow::progress(qint64 bytesReceived, qint64 bytesTotal)
{
	ui->overallProgress->setValue((float)(bytesReceived + ui->downloadedSize)/(float)ui->overallSize*100);

	ui->fileProgress->setValue((float)bytesReceived/(float)bytesTotal*100);
}

void CMainWindow::deleteFiles()
{
	for(auto file : ui->updateWidget->deleteFiles)
	{
		QFile::remove(QApplication::applicationDirPath() + QString(REL_ROOT) + file);
	}
}

void CMainWindow::makePath(QString path)
{
	QDir dir(path);

	if(dir.exists())
	{
		return;
	}

	QString parentPath = path.left(path.lastIndexOf("/"));

	makePath(parentPath);

	dir.mkdir(dir.absolutePath());

	ui->newDirs.append(dir.absolutePath());
}

void CMainWindow::addNewFile(const QString filename)
{
    // If the file doesn't already exist, add it to autoUpdate.xml for deletion during uninstalltion.
	if(!QFile::exists(filename)) ui->newFiles.append(filename);
}

void CMainWindow::downloadsFinished()
{
    // Run install commands
    for(auto cmd : ui->updateWidget->installCmds)
    {
        std::system(cmd.toStdString().c_str());
    }

	QStringList oldFiles;
	QStringList oldDirs;
    QStringList oldCmds;

	readXML(oldFiles, oldDirs, oldCmds);

	XMLWriter writer;
	writer.open(QString(QApplication::applicationDirPath() + "/autoUpdate.xml").toStdString().c_str());

	XMLElement root("autoUpdate");
	writer.add_branch(root);

	// If we're doing an updater update, don't update the last update time in autoUpdate.xml
	if(m_updaterUpdateCheck && ui->updateWidget->doingUpdaterUpdate)
	{
		writer.add_leaf("lastUpdate", std::to_string(ui->updateWidget->lastUpdate));
	}
	else
	{
		writer.add_leaf("lastUpdate", std::to_string(ui->updateWidget->serverTime));
	}

	for(auto dir : oldDirs)
	{
		writer.add_leaf("dir", dir.toStdString().c_str());
	}

	for(auto dir : ui->newDirs)
	{
		writer.add_leaf("dir", dir.toStdString().c_str());
	}

	for(auto file : oldFiles)
	{
		writer.add_leaf("file", file.toStdString().c_str());
	}

	for(auto file : ui->newFiles)
	{
		writer.add_leaf("file", file.toStdString().c_str());
	}

    if(!oldCmds.isEmpty() || !ui->updateWidget->uninstallCmds.isEmpty())
    {
        XMLElement uninstallCmds("uninstallCmds");
        writer.add_branch(uninstallCmds);

        for(auto cmd : oldCmds)
        {
            writer.add_leaf("cmd", cmd.toStdString().c_str());
        }

        for(auto cmd : ui->updateWidget->uninstallCmds)
        {
            writer.add_leaf("cmd", cmd.toStdString().c_str());
        }

        writer.close_branch();
    }

	writer.close_branch();

	writer.close();

	ui->downloadsFinished();
}

void CMainWindow::updatePlugins()
{
    if(!ui->updateWidget->doingUpdaterUpdate)
    {
        ui->currentPluginID = 0;
        ui->pluginIDs.clear();

        ui->pluginManager.LoadXML();
        ui->pluginManager.Connect();

        for(auto& [id, plugin] : ui->pluginManager.GetPlugins())
        {
            if(plugin.localCopy && id > 0)
            {
                ui->pluginIDs.push_back(id);
            }
        }

        getNextPlugin();
    }
    else
    {
        downloadsFinished();
    }
}

void CMainWindow::getNextPlugin()
{
    if(ui->currentPluginID < ui->pluginIDs.size())
    {
        ui->downloadOverallLabel->hide();
        ui->overallProgress->hide();
        ui->fileProgress->setValue(0);

        ui->downloadFileLabel->setText(QString("Downloading Plugin %1 of %2").arg(ui->currentPluginID + 1).arg(ui->pluginIDs.size()));

        int pluginID = ui->pluginIDs[ui->currentPluginID];
        ui->pluginManager.DownloadPlugin(pluginID);
        ui->currentPluginID++;
    }
    else
    {
        downloadsFinished();
    }
}

void CMainWindow::onFinish()
{
	if(ui->updateWidget->doingUpdaterUpdate)
	{
		QStringList args;
		args.push_back(QApplication::applicationDirPath() + FBSUPDATERBINARY);
		if(m_devChannel) args.push_back("-d");
        if(!m_branch.isEmpty())
        {
            args.push_back("-b");
            args.push_back(m_branch);
        }

		for(auto file : ui->updateWidget->updateFiles)
		{
            QString& filename = file.name;
			if(filename.contains("mvUtil")) continue;

			QFileInfo fileInfo(QApplication::applicationDirPath() + QString(REL_ROOT) + filename);
			QString absName = fileInfo.absoluteFilePath();

			args.push_back(absName + ".temp");
			args.push_back(absName);
		}

		QProcess* mvUtil = new QProcess;
		mvUtil->startDetached(QApplication::applicationDirPath() + MVUTIL, args);
	}
	else
	{
		if(ui->relaunch && ui->relaunch->isChecked())
		{
			QProcess* fbs = new QProcess;
			fbs->startDetached(QApplication::applicationDirPath() + FBSBINARY, QStringList());
		}
	}
	

	QWizard::accept();
}

