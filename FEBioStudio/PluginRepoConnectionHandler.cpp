/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include "PluginRepoConnectionHandler.h"
#include "MainWindow.h"
#include "ServerSettings.h"
#include "PluginDatabaseHandler.h"
#include "PluginManager.h"

#ifndef UPDATER
#include <FECore/version.h>
#endif

#include <iostream>

#define API_URL "/pluginRepo/api/v1.00/"

#ifdef WIN32
    #define OS_ID "1"
#elif defined(__APPLE__)
    #define OS_ID "2"
#else
    #define OS_ID "3"
#endif

CPluginRepoConnectionHandler::CPluginRepoConnectionHandler(CPluginManager* manager, CPluginDatabaseHandler* dbHandler)
    : manager(manager), dbHandler(dbHandler), restclient(new QNetworkAccessManager(this))
{
	connect(restclient, &QNetworkAccessManager::finished, this, &CPluginRepoConnectionHandler::connFinished);
	connect(restclient, &QNetworkAccessManager::sslErrors, this, &CPluginRepoConnectionHandler::sslErrorHandler);
}

void CPluginRepoConnectionHandler::getSchema()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + "schema");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

	if(NetworkAccessibleCheck())
	{
		restclient->get(request);
	}
}

void CPluginRepoConnectionHandler::getTables()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + "tables");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

    request.setRawHeader(QByteArray("os"), OS_ID);

	if(NetworkAccessibleCheck())
	{
		restclient->get(request);
	}

}

void CPluginRepoConnectionHandler::connFinished(QNetworkReply *r)
{
    int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // If this version of FEBioStudio is too old, send message and prevent further interaction.
	if(statusCode == 426)
	{
		outOfDate();
        return;
	}

	QString URL = r->request().url().toString();

	if(URL.contains("schema"))
	{
		getSchemaReply(r);
	}
	else if(URL.contains("tables"))
	{
		getTablesReply(r);
	}
    else if(URL.contains("/plugin/"))
    {
        getPluginFilesReply(r);
    }
}

void CPluginRepoConnectionHandler::sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors)
{
	for(QSslError error : errors)
	{
		std::cout << error.errorString().toStdString() << std::endl;
	}

	reply->ignoreSslErrors();
}

bool CPluginRepoConnectionHandler::NetworkAccessibleCheck()
{
	// if(restclient->networkAccessible() == QNetworkAccessManager::Accessible)
	// {
	// 	return true;
	// }
	// else
	// {
	// 	dbPanel->NetworkInaccessible();

	// 	return false;
	// }

	return true;
}

void CPluginRepoConnectionHandler::getPluginFiles(int pluginID, int fileNumber)
{
    QUrl myurl;
    myurl.setScheme(ServerSettings::Scheme());
    myurl.setHost(ServerSettings::URL());
    myurl.setPort(ServerSettings::Port());
    myurl.setPath(QString(API_URL) + "plugin/" + QString::number(pluginID));

    QNetworkRequest request;
    request.setUrl(myurl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

// The updater just get the latest version of the plugin, so it does not need the SDK version
#ifndef UPDATER
    request.setRawHeader(QByteArray("sdk"), QString("%1.%2.%3").arg(FE_SDK_MAJOR_VERSION).arg(FE_SDK_SUB_VERSION).arg(FE_SDK_SUBSUB_VERSION).toUtf8());
#endif

    request.setRawHeader(QByteArray("os"), OS_ID);
    request.setRawHeader(QByteArray("fileID"), QString::number(fileNumber).toUtf8());

    if(NetworkAccessibleCheck())
    {
        QNetworkReply* reply = restclient->get(request);

        QObject::connect(reply, &QNetworkReply::downloadProgress, this, 
            [pluginID, this](qint64 bytesSent, qint64 bytesTotal) 
            {
                emit manager->downloadProgress(bytesSent, bytesTotal, pluginID);
            });
    }
}

void CPluginRepoConnectionHandler::outOfDate()
{
    
}

void CPluginRepoConnectionHandler::getSchemaReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		std::string schema = QString(r->readAll()).toStdString();

		dbHandler->init(schema);

		getTables();
	}
	else
	{
        QString msg = "Unable to connect to the plugin repository."
            "\nPlease check your connection or try again later";
		
        emit manager->HTMLError(msg, true);
	}
}

void CPluginRepoConnectionHandler::getTablesReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		QJsonDocument jsonDoc = QJsonDocument::fromJson(r->readAll());

		dbHandler->update(jsonDoc);

        manager->ReadDatabase();
	}
	else
	{
		QString msg = "Unable to connect to the plugin repository."
            "\nPlease check your connection or try again later";
		
        emit manager->HTMLError(msg, true);
	}
}

void CPluginRepoConnectionHandler::getPluginFilesReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(r->error() == QNetworkReply::OperationCanceledError)
	{

	}
	else if(statusCode == 200)
	{
        // Grab a system path for the plugin
        QString pluginPath = (QStandardPaths::writableLocation(
            QStandardPaths::AppLocalDataLocation) + "/plugins/");

        QString filename = pluginPath + r->rawHeader("filename");
        int pluginID = r->rawHeader("pluginID").toInt();
		int nextFileID = r->rawHeader("nextFileID").toInt();
        QString version = r->rawHeader("version");
        QString sdk = r->rawHeader("sdk");
        int main = r->rawHeader("main").toInt();

		QFileInfo info(filename);

		QDir dir;
		dir.mkpath(info.path());

		QByteArray data = r->readAll();

		QSaveFile file(filename);
		file.open(QIODevice::WriteOnly);

		file.write(data);
		file.commit();

        manager->AddPluginFile(pluginID, filename.toStdString(), main, version.toStdString(), sdk.toStdString());

        if(nextFileID > 0)
        {
            getPluginFiles(pluginID, nextFileID);
        }
        else
        {
            manager->OnDownloadFinished(pluginID);
        }
	}
	else if(statusCode == 404)
	{
        QString msg = "Unable to download file.\nPlease check your connection or try again later";

		emit manager->HTMLError(msg);
	}
	else
	{
		QString msg = "An unknown server error has occurred.\nHTTP Staus Code: ";
		msg += std::to_string(statusCode).c_str();

		emit manager->HTMLError(msg);
	}
}