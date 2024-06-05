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

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QCheckBox>
#include "UpdateChecker.h"
#include "ServerSettings.h"
#include "version.h"

#include <iostream>

CUpdateWidget::CUpdateWidget(QWidget* parent)
    : QWidget(parent), restclient(new QNetworkAccessManager), overallSize(0), devChannel(false), devAlreadyParsed(false),
    updaterUpdateCheck(false), doingUpdaterUpdate(false), m_askSDK(false), m_getSDK(nullptr)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    layout->addWidget(infoLabel = new QLabel("Checking for updates..."));
    layout->addStretch(0);

    setLayout(layout);

    connect(restclient, &QNetworkAccessManager::finished, this, &CUpdateWidget::connFinished);
	connect(restclient, &QNetworkAccessManager::sslErrors, this, &CUpdateWidget::sslErrorHandler);
}


void CUpdateWidget::sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors)
{
	for(QSslError error : errors)
	{
		qDebug() << error.errorString();
	}

	reply->ignoreSslErrors();
}

void CUpdateWidget::connFinished(QNetworkReply *r)
{
	if(r->request().url().path() == QString(UPDATER_BASE) + ".xml")
	{
        checkForUpdaterUpdateResponse(r);
	}
	else
	{
		checkForAppUpdateResponse(r);
	}
}

void CUpdateWidget::checkForUpdate(bool dev, bool checkSDK, bool upCheck)
{
	updaterUpdateCheck = upCheck;
	devChannel = dev;
    m_askSDK = checkSDK;

	if(updaterUpdateCheck)
	{
		checkForUpdaterUpdate();
	}
	else
	{
		checkForAppUpdate();
	}
}

void CUpdateWidget::checkForAppUpdate()
{
    QString urlBase;

	if(devChannel && !devAlreadyParsed)
	{
		urlBase = DEV_BASE;
	}
	else
	{
		urlBase = URL_BASE;
	}

	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(urlBase + ".xml");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("version"), QString("%1.%2.%3").arg(FBS_VERSION).arg(FBS_SUBVERSION).arg(FBS_SUBSUBVERSION).toUtf8());
	
	request.setRawHeader(QByteArray("UUID"), UUID.toUtf8());

	restclient->get(request);
}

void CUpdateWidget::checkForAppUpdateResponse(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(r->hasRawHeader("message"))
    {
        serverMessage = r->rawHeader("message");
    }

	if(statusCode != 200)
	{
		showError("Update Check Failed!\n\nUnable to receive response from server.");
	}

	QXmlStreamReader reader(r->readAll());

    if(devChannel && !devAlreadyParsed)
	{
        parseAppXML(reader, true);
        devAlreadyParsed = true;
		checkForAppUpdate();
        return;
	}
	else
	{
		parseAppXML(reader, false);
	}

	ReadLastUpdateInfo();

	if(releases.size() > 0)
	{
        serverTime = releases[0].timestamp;

		if(releases[0].terminal)
		{
			showTerminal();
		}
		else if(releases[0].timestamp > lastUpdate)
		{
			showUpdateInfo();
		}
		else
		{
			showUpToDate();
		}
	}
	else
	{
		showError("Failed to read release information from server.\nPlease try again later.");
	}
}

void CUpdateWidget::parseAppXML(QXmlStreamReader& reader, bool dev)
{


    if (reader.readNextStartElement())
	{
		if(reader.name() == UPDATE) 
		{
			while(reader.readNextStartElement())
			{
				if(reader.name() == RELEASE)
				{
					Release release;

					while(reader.readNextStartElement())
					{
						if(reader.name() == ACTIVE)
						{
							release.active = reader.readElementText().toInt();
						}
						else if(reader.name() == TERMINAL)
						{
							release.terminal = reader.readElementText().toInt();
						}
						else if(reader.name() == TIMESTAMP)
						{
							release.timestamp = reader.readElementText().toLongLong();
						}
						else if(reader.name() == FEBIOVERSION)
						{
							release.FEBioVersion = reader.readElementText();
						}
						else if(reader.name() == FBSVERSION)
						{
							release.FBSVersion = reader.readElementText();
						}
						else if(reader.name() == FEBIONOTES)
						{
							release.FEBioNotes = reader.readElementText();
						}
						else if(reader.name() == FBSNOTES)
						{
							release.FBSNotes = reader.readElementText();
						}
						else if(reader.name() == RELEASEMSG)
						{
							release.releaseMsg = reader.readElementText();
						}
                        else if(reader.name() == SDK)
						{
                            release.hasSDK = true;
                            
                            release.sdk.size = reader.attributes().value("size").toLongLong();
							release.sdk.name = reader.readElementText();
                            
                            if(dev) release.sdk.baseURL = DEV_BASE;
                            else release.sdk.baseURL = URL_BASE;
						}
						else if(reader.name() == FEBFILES)
						{
							while(reader.readNextStartElement())
							{
								if(reader.name() == FEBFILE)
								{
									ReleaseFile rfile;
									rfile.size = reader.attributes().value("size").toLongLong();
									rfile.name = reader.readElementText();

                                    if(dev) rfile.baseURL = DEV_BASE;
                                    else rfile.baseURL = URL_BASE;
                                
									release.files.push_back(rfile);
								}
								else
								{
									reader.skipCurrentElement();
								}
							}
						}
						else if(reader.name() == DELETEFILES)
						{
							while(reader.readNextStartElement())
							{
								if(reader.name() == FEBFILE)
								{
									release.deleteFiles.append(reader.readElementText());
								}
								else
								{
									reader.skipCurrentElement();
								}
							}
						}
                        else if(reader.name() == INSTALLCMD)
						{
							while(reader.readNextStartElement())
							{
								if(reader.name() == COMMAND)
								{
									release.installCommands.append(reader.readElementText());
								}
								else
								{
									reader.skipCurrentElement();
								}
							}
						}
                        else if(reader.name() == UNINSTALLCMD)
						{
							while(reader.readNextStartElement())
							{
								if(reader.name() == COMMAND)
								{
									release.uninstallCommands.append(reader.readElementText());
								}
								else
								{
									reader.skipCurrentElement();
								}
							}
						}
						else
						{
							reader.skipCurrentElement();
						}
					}

					if(release.active)
                    {
                        // Insertion sort based on timestamp in releases. This is only necessary
                        // when doing a dev update, during which, the dev XML is parsed, and then
                        // the release xml is parsed, and those releases get interleaved into the
                        // dev releases according to their timestamp
                        bool inserted = false;
                        for(auto it = releases.begin(); it != releases.end(); it++)
                        {
                            if(release.timestamp > it->timestamp)
                            {
                                releases.insert(it, release);
                                inserted = true;
                                break;
                            }
                        }

                        if(!inserted) releases.push_back(release);
                    }
                    
				}
				else
				{
					reader.skipCurrentElement();
				}
			}

		}
	}
}

void CUpdateWidget::checkForUpdaterUpdate()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(UPDATER_BASE) + ".xml");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

	restclient->get(request);
}

void CUpdateWidget::checkForUpdaterUpdateResponse(QNetworkReply *r)
{

	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode != 200)
	{
		showError("Update Check Failed!\n\nUnable to receive response from server.");
        return;
	}

	QXmlStreamReader reader(r->readAll());

	if (reader.readNextStartElement())
	{
		if(reader.name() == UPDATE)
		{
			while(reader.readNextStartElement())
			{
				if(reader.name() == RELEASE)
				{
					Release release;
					release.terminal = false;

					while(reader.readNextStartElement())
					{
						if(reader.name() == ACTIVE)
						{
							release.active = reader.readElementText().toInt();
						}
						else if(reader.name() == TIMESTAMP)
						{
							release.timestamp = reader.readElementText().toLongLong();
						}
						else if(reader.name() == FEBFILES)
						{
							while(reader.readNextStartElement())
							{
								if(reader.name() == FEBFILE)
								{
									ReleaseFile rfile;
									rfile.size = reader.attributes().value("size").toLongLong();
									rfile.name = reader.readElementText();
                                    rfile.baseURL = UPDATER_BASE;

									release.files.push_back(rfile);
								}
								else
								{
									reader.skipCurrentElement();
								}
							}
						}
						else
						{
							reader.skipCurrentElement();
						}
					}

					if(release.active) updaterReleases.push_back(release);
				}
				else
				{
					reader.skipCurrentElement();
				}
			}
			
		}
	}

	ReadLastUpdateInfo();

	if(updaterReleases.size() > 0)
	{
		serverTime = updaterReleases[0].timestamp;

		if(updaterReleases[0].timestamp > lastUpdate)
		{
			showUpdaterUpdateInfo();
		}
		else
		{
			checkForAppUpdate();
		}

	}
	else
	{
		checkForAppUpdate();
	}

}

void CUpdateWidget::ReadLastUpdateInfo()
{
	QXmlStreamReader reader;

	lastUpdate = 0;

	QFile autoUpdateXML(QApplication::applicationDirPath() + "/autoUpdate.xml");
	autoUpdateXML.open(QIODevice::ReadOnly);

	reader.setDevice(&autoUpdateXML);

	if (reader.readNextStartElement())
	{
		if(reader.name() == AUTOUPDATE)
		{
			while(reader.readNextStartElement())
			{
				if(reader.name() == LASTUPDATE)
				{
					lastUpdate = reader.readElementText().toLongLong();
				}
				else
				{
					reader.skipCurrentElement();
				}
			}
		}
	}
	autoUpdateXML.close();

}

void CUpdateWidget::showUpdateInfo()
{
    bool newSDK = false;
    
    // Find unique files that need to be downloaded or deleted
    for(auto release : releases)
    {
        if(release.timestamp > lastUpdate)
        {
            for(auto file : release.files)
            {
                bool duplicate = false;
                for(auto updateFile : updateFiles)
                {
                    if(updateFile.name == file.name)
                    {
                        duplicate = true;
                        break;
                    }
                }

                if(!duplicate)
                {
                    updateFiles.push_back(file);
                    overallSize += file.size;
                }
            }

			for(auto file : release.deleteFiles)
            {
                if(!deleteFiles.contains(file))
                {
                    deleteFiles.append(file);
                }
            }

            // replace $INSTALLDIR variable in the commands
            QString installDir = QFileInfo(QApplication::applicationDirPath() + QString(REL_ROOT)).absoluteFilePath();

            for(auto cmd : release.installCommands)
            {
                installCmds.append(cmd.replace("$INSTALLDIR", installDir));
            }

            for(auto cmd : release.uninstallCommands)
            {
                uninstallCmds.append(cmd.replace("$INSTALLDIR", installDir));
            }

            // Only grab the latest sdk
            if(release.hasSDK && !newSDK)
            {
                newSDK = true;
                m_sdk = release.sdk;
            }
        }
    }

	if(!devChannel)
	{
		// Find last installed update
		int lastUpdateIndex;
		for(lastUpdateIndex = 0; lastUpdateIndex < releases.size(); lastUpdateIndex++)
		{
			if(releases[lastUpdateIndex].timestamp <= lastUpdate)
			{
				break;
			}
		}

		bool newFEBio = false;
		bool newFBS = false;

		if(lastUpdateIndex == releases.size())
		{
			newFEBio = true;
			newFBS = true;
		}
		else
		{
			if(releases[0].FEBioVersion != releases[lastUpdateIndex].FEBioVersion) newFEBio = true;
			if(releases[0].FBSVersion != releases[lastUpdateIndex].FBSVersion) newFBS = true;
		}
		

		if(!newFEBio && !newFBS)
		{
			if(!releases[0].releaseMsg.isEmpty())
			{
				infoLabel->setText(releases[0].releaseMsg);
			}
			else
			{
				infoLabel->setText("This update does not include new versions of FEBio or FEBioStudio.\n\nIt provides updates to FEBio dependencies for added stability.");
			}
		}
		else
		{
			infoLabel->setText("There is a new update available!");

			if(!releases[0].releaseMsg.isEmpty())
			{
				// Show all release messages since last update.
				QStringList messages;
				for(auto release : releases)
				{
					if(release.timestamp > lastUpdate)
					{
						QLabel* label = new QLabel(release.releaseMsg);
						label->setWordWrap(true);

						layout->addWidget(label);
					}
				}
			}

			layout->addWidget(new QLabel("This update provides:"));

			if(newFEBio)
			{
				// QChar(0x22, 0x20) give us the unicode 'bullet' character
				QLabel* newFEBioLabel = new QLabel(QChar(0x22, 0x20) + QString(" An update to FEBio %1. Click <a href=\"FEBioNotes\">here</a> for release notes.").arg(releases[0].FEBioVersion));
				newFEBioLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
				QObject::connect(newFEBioLabel, &QLabel::linkActivated, this, &CUpdateWidget::linkActivated);

				layout->addWidget(newFEBioLabel);

				// if(newFEBioLabel->sizeHint().width() > sizeHint().width()) setMinimumWidth(newFEBioLabel->sizeHint().width());
				newFEBioLabel->setWordWrap(true);
			}

			if(newFBS)
			{
				// QChar(0x22, 0x20) give us the unicode 'bullet' character
				QLabel* newFBSLabel = new QLabel(QChar(0x22, 0x20) + QString(" An update to FEBio Studio %1. Click <a href=\"FBSNotes\">here</a> for release notes.").arg(releases[0].FBSVersion));
				newFBSLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
				QObject::connect(newFBSLabel, &QLabel::linkActivated, this, &CUpdateWidget::linkActivated);
				
				layout->addWidget(newFBSLabel);

				// if(newFBSLabel->sizeHint().width() > sizeHint().width()) setMinimumWidth(newFBSLabel->sizeHint().width());

				newFBSLabel->setWordWrap(true);
			}
		}
	}
	else
	{
		infoLabel->setText("This will update FEBio and FEBio Studio to the latest development versions. "
			"These versions contain the latest bugfixes and features but are potentially unstable. "
			"Please proceed with caution.\n\n"
			"There are no patch notes for development releases.");

		infoLabel->setWordWrap(true);
	}

    layout->addStretch(10);

    if(m_askSDK && newSDK)
    {
        bool hasSDK = QFileInfo::exists(QApplication::applicationDirPath() + QString(REL_ROOT) + "sdk");

        QString msg("Download FEBio SDK (%1)");
        if(hasSDK) msg = "Update FEBio SDK (%1)";

        layout->addWidget(m_getSDK = new QCheckBox(msg.arg(locale().formattedDataSize(m_sdk.size))));
        m_getSDK->setChecked(hasSDK);
    }

    layout->addWidget(new QLabel(QString("The total download size is %1.").arg(locale().formattedDataSize(overallSize))));
    
    emit ready(true);
}

void CUpdateWidget::showUpdaterUpdateInfo()
{
	doingUpdaterUpdate = true;

	// Find unique files that need to be downloaded or deleted
	for(auto release : updaterReleases)
	{
		if(release.timestamp > lastUpdate)
		{
			for(auto file : release.files)
			{
                bool duplicate = false;
                for(auto updateFile : updateFiles)
                {
                    if(updateFile.name == file.name)
                    {
                        duplicate = true;
                        break;
                    }
                }

                if(!duplicate)
                {
                    updateFiles.push_back(file);
                    overallSize += file.size;
                }
			}
		}
	}

	infoLabel->setText("Before this update can be downloaded, the auto-updater needs to download an update for itself.");
	infoLabel->setWordWrap(true);

    layout->addStretch(10);
    layout->addWidget(new QLabel(QString("The total download size is %1.").arg(locale().formattedDataSize(overallSize))));
    
    emit ready(true);
}

void CUpdateWidget::showUpToDate()
{
    infoLabel->setText("Software is up to date!");

    emit ready(false);
}

void CUpdateWidget::showTerminal()
{
    infoLabel->setText("There is a new update available, but you cannot update to it automatically.<br>"
		"You must download a new installer from <a href=\"https://febio.org\">febio.org</a>");
	infoLabel->setOpenExternalLinks(true);

    emit ready(true, true);
}

void CUpdateWidget::showError(const QString& error)
{
    infoLabel->setText(error);

    emit ready(false);
}

void CUpdateWidget::linkActivated(const QString& link)
{
	QDialog dlg;
	QVBoxLayout* layout = new QVBoxLayout;

	QTextEdit* edit = new QTextEdit;
	edit->setReadOnly(true);
	layout->addWidget(edit);

	QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok);
	layout->addWidget(box);
	QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

	QString notes = "";

	for(auto release : releases)
	{
		if(release.timestamp > lastUpdate)
		{
			if(link == "FEBioNotes")
			{
				if(release.FEBioNotes.isEmpty())
				{
					continue;
				}
				
				if(!notes.isEmpty())
				{
					notes += "\n\n";
				}

				notes += "------------------------------------------------------\n";
				notes += "FEBio Version: ";
				notes += release.FEBioVersion;

				QDateTime timestamp;
				timestamp.setSecsSinceEpoch(release.timestamp);

				notes += "   Released: ";
				notes += timestamp.toString("M/d/yy");

				notes += "\n------------------------------------------------------\n";

				notes  += release.FEBioNotes;
			}

			else if(link == "FBSNotes")
			{
				if(release.FBSNotes.isEmpty())
				{
					continue;
				}

				if(!notes.isEmpty())
				{
					notes += "\n\n";
				}

				notes += "------------------------------------------------------\n";
				notes += "FEBio Version: ";
				notes += release.FBSVersion;

				QDateTime timestamp;
				timestamp.setSecsSinceEpoch(release.timestamp);

				notes += "   Released: ";
				notes += timestamp.toString("M/d/yy");

				notes += "\n------------------------------------------------------\n";

				notes += release.FBSNotes;
			}
		}
	}

	edit->setText(notes);

	dlg.setLayout(layout);

	dlg.resize(QSize(600,500));

	dlg.exec();
}

QString CUpdateWidget::getServerMessage()
{
    return serverMessage;
}

QString CUpdateWidget::getUpdaterPath() const
{
	return QApplication::applicationDirPath() + UPDATER;
}

///// Update Checker Dialog

CUpdateChecker::CUpdateChecker(bool dev, QWidget* parent) 
	: QDialog(parent), update(false), updateAvailable(false)
{
	layout = new QVBoxLayout;
	
	CUpdateWidget* widget = new  CUpdateWidget;
	layout->addWidget(widget);

	box = new QDialogButtonBox(QDialogButtonBox::Cancel);

	layout->addWidget(box);

	setLayout(layout);

	QObject::connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
	QObject::connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
	QObject::connect(widget, &CUpdateWidget::ready, this, &CUpdateChecker::updateWidgetReady);

	widget->checkForUpdate(dev);
}

void CUpdateChecker::updateWidgetReady(bool update, bool terminal)
{
	if(update && !terminal)
	{
		updateAvailable = true;

		layout->insertWidget(1, new QLabel("Click \"Close and Update\" to close FEBio Studio and run the updater."));
		
		box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		box->button(QDialogButtonBox::Ok)->setText("Close and Update");
	}
	else
	{
		box->setStandardButtons(QDialogButtonBox::Ok);
	}
}

void CUpdateChecker::accept()
{
	if(updateAvailable)
	{
		update = true;
	}

	QDialog::accept();
}