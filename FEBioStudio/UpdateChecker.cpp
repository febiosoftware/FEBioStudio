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

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QFile>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QCheckBox>
#include "UpdateChecker.h"
#include "version.h"

#include <iostream>

CUpdateWidget::CUpdateWidget(QWidget* parent)
    : QWidget(parent), restclient(new QNetworkAccessManager), currentIndex(0), overallSize(0), downloadedSize(0),
	devChannel(false), urlBase(URL_BASE)
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
	if(r->request().url().path() == urlBase + ".xml")
	{
		checkForUpdateResponse(r);
	}
}

void CUpdateWidget::checkForUpdate(bool dev)
{
	devChannel = dev;
	if(devChannel)
	{
		urlBase = DEV_BASE;
	}
	else
	{
		urlBase = URL_BASE;
	}

	QUrl myurl;
	myurl.setScheme(SCHEME);
	myurl.setHost(UPDATE_URL);
	myurl.setPort(PORT);
	myurl.setPath(urlBase + ".xml");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("version"), QString("%1.%2.%3").arg(VERSION).arg(SUBVERSION).arg(SUBSUBVERSION).toUtf8());
	request.setRawHeader(QByteArray("UUID"), UUID.toUtf8());
	

	if(NetworkAccessibleCheck())
	{
		restclient->get(request);
	}
}

bool CUpdateWidget::NetworkAccessibleCheck()
{
//	return restclient->networkAccessible() == QNetworkAccessManager::Accessible;
	return true;
}

void CUpdateWidget::checkForUpdateResponse(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	std::cout << statusCode << endl;

	if(statusCode != 200)
	{
		showError("Update Check Failed!\n\nUnable to receive response from server.");
	}

	serverTime = r->rawHeader("serverTime").toLongLong();

	QXmlStreamReader reader(r->readAll());

	if (reader.readNextStartElement())
	{
		if(reader.name() == "update")
		{
			while(reader.readNextStartElement())
			{
				if(reader.name() == "release")
				{
					Release release;

					while(reader.readNextStartElement())
					{
						if(reader.name() == "active")
						{
							release.active = reader.readElementText().toInt();
						}
						else if(reader.name() == "timestamp")
						{
							release.timestamp = reader.readElementText().toLongLong();
						}
						else if(reader.name() == "FEBioVersion")
						{
							release.FEBioVersion = reader.readElementText();
						}
						else if(reader.name() == "FBSVersion")
						{
							release.FBSVersion = reader.readElementText();
						}
						else if(reader.name() == "FEBioNotes")
						{
							release.FEBioNotes = reader.readElementText();
						}
						else if(reader.name() == "FBSNotes")
						{
							release.FBSNotes = reader.readElementText();
						}
						else if(reader.name() == "releaseMsg")
						{
							release.releaseMsg = reader.readElementText();
						}
						else if(reader.name() == "files")
						{
							while(reader.readNextStartElement())
							{
								if(reader.name() == "file")
								{
									ReleaseFile rfile;
									rfile.size = reader.attributes().value("size").toLongLong();
									rfile.name = reader.readElementText();
									

									release.files.push_back(rfile);
								}
								else
								{
									reader.skipCurrentElement();
								}
							}
						}
						else if(reader.name() == "deleteFiles")
						{
							while(reader.readNextStartElement())
							{
								if(reader.name() == "file")
								{
									release.deleteFiles.append(reader.readElementText());
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

					if(release.active) releases.push_back(release);
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

	lastUpdate = 0;

	QFile autoUpdateXML(QApplication::applicationDirPath() + "/autoUpdate.xml");
	autoUpdateXML.open(QIODevice::ReadOnly);

	reader.setDevice(&autoUpdateXML);

	if (reader.readNextStartElement())
	{
		if(reader.name() == "autoUpdate")
		{
			while(reader.readNextStartElement())
			{
				if(reader.name() == "lastUpdate")
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

	if(releases.size() > 0)
	{
		if(releases[0].timestamp > lastUpdate)
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

void CUpdateWidget::showUpdateInfo()
{
    // Find unique files that need to be downloaded or deleted
    for(auto release : releases)
    {
        if(release.timestamp > lastUpdate)
        {
            for(auto file : release.files)
            {
                if(!updateFiles.contains(file.name))
                {
                    updateFiles.append(file.name);
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
				layout->addWidget(new QLabel(releases[0].releaseMsg));
			}

			layout->addWidget(new QLabel("This update provides:"));

			if(newFEBio)
			{
				// QChar(0x22, 0x20) give us the unicode 'bullet' character
				QLabel* newFEBioLabel = new QLabel(QChar(0x22, 0x20) + QString(" An update to FEBio %1. Click <a href=\"FEBioNotes\">here</a> for release notes.").arg(releases[0].FEBioVersion));
				newFEBioLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
				QObject::connect(newFEBioLabel, &QLabel::linkActivated, this, &CUpdateWidget::linkActivated);

				layout->addWidget(newFEBioLabel);
			}

			if(newFBS)
			{
				// QChar(0x22, 0x20) give us the unicode 'bullet' character
				QLabel* newFBSLabel = new QLabel(QChar(0x22, 0x20) + QString(" An update to FEBio Studio %1. Click <a href=\"FBSNotes\">here</a> for release notes.").arg(releases[0].FBSVersion));
				newFBSLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
				QObject::connect(newFBSLabel, &QLabel::linkActivated, this, &CUpdateWidget::linkActivated);

				layout->addWidget(newFBSLabel);
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
    layout->addWidget(new QLabel(QString("The total download size is %1.").arg(locale().formattedDataSize(overallSize))));

    
    emit ready(true);
}

void CUpdateWidget::showUpToDate()
{
    infoLabel->setText("Software is up to date!");

    emit ready(false);
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
				timestamp.setTime_t(release.timestamp);

				notes += "   Released: ";
				notes += timestamp.toString(Qt::SystemLocaleShortDate);

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
				timestamp.setTime_t(release.timestamp);

				notes += "   Released: ";
				notes += timestamp.toString(Qt::SystemLocaleShortDate);

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

void CUpdateChecker::updateWidgetReady(bool update)
{
	if(update)
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