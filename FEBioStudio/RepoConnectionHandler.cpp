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
#include <QJsonDocument>
#include "RepoConnectionHandler.h"
#include "MainWindow.h"
#include "ServerSettings.h"
#include "LocalDatabaseHandler.h"

#include <iostream>

CRepoConnectionHandler::CRepoConnectionHandler(QString apiURL, CLocalDatabaseHandler* dbHandler, CMainWindow* wnd)
    : apiURL(apiURL), dbHandler(dbHandler), wnd(wnd)
{
	restclient = new QNetworkAccessManager(this);

	connect(restclient, SIGNAL(finished(QNetworkReply*)), this, SLOT(connFinished(QNetworkReply*)));
	connect(restclient, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError>&)));
}

CRepoConnectionHandler::~CRepoConnectionHandler()
{
	delete restclient;
}

void CRepoConnectionHandler::getSchema()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + "schema");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

	if(NetworkAccessibleCheck())
	{
		restclient->get(request);
	}
}

void CRepoConnectionHandler::getTables()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + "tables");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

	if(NetworkAccessibleCheck())
	{
		restclient->get(request);
	}

}

void CRepoConnectionHandler::connFinished(QNetworkReply *r)
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
    else
    {
        handleReply(r);
    }
}

void CRepoConnectionHandler::sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors)
{
	for(QSslError error : errors)
	{
		std::cout << error.errorString().toStdString() << std::endl;
	}

	reply->ignoreSslErrors();

}

void CRepoConnectionHandler::progress(qint64 bytesReceived, qint64 bytesTotal)
{
	wnd->UpdateProgress((float)bytesReceived/(float)bytesTotal*100);
}

bool CRepoConnectionHandler::NetworkAccessibleCheck()
{
	// if(imp->restclient->networkAccessible() == QNetworkAccessManager::Accessible)
	// {
	// 	return true;
	// }
	// else
	// {
	// 	imp->dbPanel->NetworkInaccessible();

	// 	return false;
	// }

	return true;
}

void CRepoConnectionHandler::getSchemaReply(QNetworkReply *r)
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
		handleReply(r);
	}
}

void CRepoConnectionHandler::getTablesReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		QJsonDocument jsonDoc = QJsonDocument::fromJson(r->readAll());

		dbHandler->update(jsonDoc);

        tablesReady();
	}
	else
	{
		handleReply(r);
	}
}