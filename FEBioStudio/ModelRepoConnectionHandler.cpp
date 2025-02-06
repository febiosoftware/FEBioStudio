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

#include "stdafx.h"
#include "ModelRepoConnectionHandler.h"

#ifdef MODEL_REPO
#include <QEventLoop>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QByteArray>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <QList>
#include <QSaveFile>
#include <QFileDialog>
#include <QTcpSocket>
#include "RepositoryPanel.h"
#include "MainWindow.h"
#include "Document.h"
#include "LocalDatabaseHandler.h"
#include "ZipFiles.h"
#include "ServerSettings.h"
#include <iostream>

#define API_URL "/modelRepo/api/v1.03/"

using std::cout;
using std::endl;

class CModelRepoConnectionHandler::Imp
{
public:
	Imp(CRepositoryPanel* dbPanel, CModelRepoConnectionHandler* handler)
		: dbPanel(dbPanel), uploadPermission(0), sizeLimit(0), authenticated(false), uploadReady(false)
	{
	}

	void loggedOut()
	{
		username = "";
		token = "";
		uploadPermission = 0;
		sizeLimit = 0;
		authenticated = false;

		fileToken = "";
		uploadReady = false;

		dbPanel->LoginTimeout();
	}

	CRepositoryPanel* dbPanel;

	QString username;
	QString token;
	int uploadPermission;
	qint64 sizeLimit;
	bool authenticated;

	QString fileToken;
	bool uploadReady;
};


CModelRepoConnectionHandler::CModelRepoConnectionHandler(CRepositoryPanel* dbPanel, CLocalDatabaseHandler* dbHandler, CMainWindow* wnd)
    : CRepoConnectionHandler(API_URL, dbHandler, wnd), imp(new Imp(dbPanel, this))
{

}

CModelRepoConnectionHandler::~CModelRepoConnectionHandler()
{
	delete imp;
}

void CModelRepoConnectionHandler::authenticate(QString username, QString password)
{
	imp->username = username;

	QVariantMap feed;
	feed.insert("username", username);
	feed.insert("password", password);
	QByteArray payload=QJsonDocument::fromVariant(feed).toJson();

	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + "authenticate");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	if(NetworkAccessibleCheck())
	{
		restclient->post(request, payload);
	}

}

void CModelRepoConnectionHandler::getFile(int id, int type)
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + QString("files/%1/%2").arg(type).arg(id));

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

	if(NetworkAccessibleCheck())
	{
		QNetworkReply* reply = restclient->get(request);

		QObject::connect(reply, &QNetworkReply::downloadProgress, imp->dbPanel, &CRepositoryPanel::loadingPageProgress);
		QObject::connect(imp->dbPanel, &CRepositoryPanel::cancelClicked, reply, &QNetworkReply::abort);
	}

}

void CModelRepoConnectionHandler::uploadFileRequest(QByteArray projectInfo)
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + "uploadFileRequest");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	if(NetworkAccessibleCheck())
	{
		QNetworkReply* reply = restclient->post(request, projectInfo);
		QObject::connect(imp->dbPanel, &CRepositoryPanel::cancelClicked, reply, &QNetworkReply::abort);
	}
}

void CModelRepoConnectionHandler::uploadFile()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + "uploadFile");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setRawHeader(QByteArray("fileToken"), imp->fileToken.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/zip");

	QString fileName = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/.projOutForUpload.prj";

	if(NetworkAccessibleCheck())
	{
		imp->dbPanel->showLoadingPage("Uploading...", true);

		QFile* arch = new QFile(fileName);
		arch->open(QIODevice::ReadOnly);

		QNetworkReply* reply = restclient->post(request, arch);
		arch->setParent(reply);

		QObject::connect(reply, &QNetworkReply::uploadProgress, imp->dbPanel, &CRepositoryPanel::loadingPageProgress);
		QObject::connect(imp->dbPanel, &CRepositoryPanel::cancelClicked, reply, &QNetworkReply::abort);
	}
}

void CModelRepoConnectionHandler::requestUploadPermissions(QByteArray userInfo)
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + "requestUploaderPermissions");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	if(NetworkAccessibleCheck())
	{
		restclient->post(request, userInfo);
	}
}

void CModelRepoConnectionHandler::getMessages()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + "messages");

	QNetworkRequest request;
	request.setUrl(myurl);

	if(NetworkAccessibleCheck())
	{
		restclient->get(request);
	}
}

void CModelRepoConnectionHandler::modifyProject(int id, QByteArray projectInfo)
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + QString("projects/%1").arg(id));

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	if(NetworkAccessibleCheck())
	{
		QNetworkReply* reply = restclient->put(request, projectInfo);
		QObject::connect(imp->dbPanel, &CRepositoryPanel::cancelClicked, reply, &QNetworkReply::abort);
	}
}

void CModelRepoConnectionHandler::modifyProjectUpload()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + "modifyProjectUpload");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setRawHeader(QByteArray("fileToken"), imp->fileToken.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/zip");

	QString fileName = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/.projOutForUpload.prj";

	if(NetworkAccessibleCheck())
	{
		imp->dbPanel->showLoadingPage("Uploading...", true);

		QFile* arch = new QFile(fileName);
		arch->open(QIODevice::ReadOnly);

		QNetworkReply* reply = restclient->post(request, arch);
		arch->setParent(reply);

		QObject::connect(reply, &QNetworkReply::uploadProgress, imp->dbPanel, &CRepositoryPanel::loadingPageProgress);
		QObject::connect(imp->dbPanel, &CRepositoryPanel::cancelClicked, reply, &QNetworkReply::abort);
	}
}

void CModelRepoConnectionHandler::deleteProject(int id)
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + QString("projects/%1").arg(id));

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	if(NetworkAccessibleCheck())
	{
		restclient->deleteResource(request);
	}
}

void CModelRepoConnectionHandler::cancelUpload()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(apiURL + QString("cancelUpload"));

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setRawHeader(QByteArray("fileToken"), imp->fileToken.toUtf8());

	if(NetworkAccessibleCheck() && !imp->fileToken.isEmpty())
	{
		restclient->deleteResource(request);
	}

	imp->fileToken = "";
	imp->uploadReady = false;
}

void CModelRepoConnectionHandler::handleReply(QNetworkReply *r)
{
	QString URL = r->request().url().toString();

	if(URL.contains("authenticate"))
	{
		authReply(r);
	}
	else if(URL.contains("schema"))
	{
		unhandledSchemaReply(r);
	}
	else if(URL.contains("files/"))
	{
		getFileReply(r);
	}
	else if(URL.contains("uploadFileRequest"))
	{
		uploadFileRequestReply(r);
	}
	else if(URL.contains("uploadFile"))
	{
		uploadFileReply(r);
	}
	else if(URL.contains("requestUploaderPermissions"))
	{
		requestUploadPermissionsReply(r);
	}
	else if(URL.contains("tables"))
	{
		unhandledTablesReply(r);
	}
	else if(URL.contains("projects"))
	{
		if(r->operation() == QNetworkAccessManager::PutOperation)
		{
			modifyProjectRepy(r);
		}
		else if(r->operation() == QNetworkAccessManager::DeleteOperation)
		{
			deleteProjectRepy(r);
		}
	}
	else if(URL.contains("modifyProjectUpload"))
	{
		modifyProjectUploadReply(r);
	}
	else if(URL.contains("messages"))
	{
		getMessagesReply(r);
	}

}

void CModelRepoConnectionHandler::tablesReady()
{
    imp->dbPanel->SetModelList();

    getMessages();
}

void CModelRepoConnectionHandler::outOfDate()
{
    imp->dbPanel->ShowMessage("This version of FEBio Studio is too old to connect to the project "
            "repository.\n\nPlease update FEBio Studio in order to connect to the repository.");
}

void CModelRepoConnectionHandler::authReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		QJsonDocument jsonDoc = QJsonDocument::fromJson(r->readAll());
		QJsonObject obj = jsonDoc.object();

		imp->authenticated = true;
		imp->token = obj.value("authToken").toString();
		imp->uploadPermission = obj.value("uploader").toInt();
		imp->sizeLimit = obj.value("sizeLimit").toString().toLongLong();

		// Send a request to get the repository schema
		getSchema();
	}
	else if(statusCode == 403)
	{
		QJsonDocument jsonDoc = QJsonDocument::fromJson(r->readAll());
		QJsonObject obj = jsonDoc.object();
		QString message = obj.value("message").toString();

		imp->authenticated = false;
		imp->token = "";
		imp->uploadPermission = 0;
		imp->sizeLimit = 0;

		getSchema();

		imp->dbPanel->ShowMessage(message);
	}
	else
	{
		QString message = "An unknown server error has occurred.\nHTTP Staus Code: ";
		message += std::to_string(statusCode).c_str();

		imp->authenticated = false;
		imp->token = "";
		imp->uploadPermission = 0;

		getSchema();

		imp->dbPanel->ShowMessage(message);
	}

}

// Handle the non 200 status codes for the schema request
void CModelRepoConnectionHandler::unhandledSchemaReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 403)
	{
		imp->loggedOut();
	}
	else
	{
		QString message = "An unknown server error has occurred.\nHTTP Staus Code: ";
		message += std::to_string(statusCode).c_str();

		imp->dbPanel->ShowMessage(message, true);
	}
}

// Handle the non 200 status codes for the tables request
void CModelRepoConnectionHandler::unhandledTablesReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(statusCode == 403)
	{
		imp->loggedOut();
	}
	else
	{
		QString message = "An unknown server error has occurred.\nHTTP Status Code: ";
		message += std::to_string(statusCode).c_str();

		imp->dbPanel->ShowMessage(message, true);
	}
}

void CModelRepoConnectionHandler::getFileReply(QNetworkReply *r)
{
	wnd->ShowProgress(false);

	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(r->error() == QNetworkReply::OperationCanceledError)
	{

	}
	else if(statusCode == 200)
	{
		int fileID = r->rawHeader(QByteArray("fileID")).toInt();
		int IDType = r->rawHeader(QByteArray("IDType")).toInt();

		QString filename = dbHandler->FullFileNameFromID(fileID, IDType);

		QFileInfo info(filename);

		QDir dir;
		dir.mkpath(info.path());

		QByteArray data = r->readAll();

		QSaveFile file(filename);
		file.open(QIODevice::WriteOnly);

		file.write(data);
		file.commit();

		imp->dbPanel->DownloadFinished(fileID, IDType);
	}
	else if(statusCode == 403)
	{
		imp->loggedOut();
	}
	else if(statusCode == 404)
	{
		imp->dbPanel->ShowMessage(r->readAll());
	}
	else
	{
		QString message = "An unknown server error has occurred.\nHTTP Staus Code: ";
		message += std::to_string(statusCode).c_str();

		imp->dbPanel->ShowMessage(message);
	}

	imp->dbPanel->showMainPage();
}

void CModelRepoConnectionHandler::uploadFileRequestReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		imp->fileToken = r->readAll();

		imp->dbPanel->updateUploadReady(true);

	}
	else if(statusCode == 403)
	{
		imp->loggedOut();
	}
	else if(statusCode == 0)
	{
		imp->fileToken = "";
	}
	else
	{
		QString message = "An unknown server error has occurred.\nHTTP Staus Code: ";
		message += std::to_string(statusCode).c_str();

		imp->dbPanel->updateUploadReady(false);
	}
}

void CModelRepoConnectionHandler::uploadFileReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	imp->dbPanel->showLoadingPage("Loading...");

	getSchema();

	// HTTP status code will be 0 if the upload was aborted
	if(statusCode == 0)
	{
		cancelUpload();
	}

	QString fileName = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/.projOutForUpload.prj";
	QFile::remove(fileName);

	imp->uploadReady = false;
	imp->fileToken = "";

	if(statusCode)
	{
		if(statusCode == 200)
		{
			imp->dbPanel->UploadFinished(true, r->readAll());
		}
        else if(statusCode == 403)
        {
            imp->loggedOut();
        }
		else
		{
			imp->dbPanel->UploadFinished(false, r->readAll());
		}
		
	}
	else
	{
		imp->dbPanel->UploadFinished(false, "Upload cancelled.");
	}
}

void CModelRepoConnectionHandler::modifyProjectRepy(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 202)
	{
		imp->fileToken = r->readAll();

		imp->dbPanel->updateModifyReady(true);

		return;
	}
    else if(statusCode == 403)
	{
		imp->dbPanel->updateModifyReady(false);
		
		imp->loggedOut();
	}
	else if(statusCode == 200)
	{
		getSchema();
	}

	imp->dbPanel->ShowMessage(r->readAll());

}

void CModelRepoConnectionHandler::modifyProjectUploadReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	imp->dbPanel->showLoadingPage("Loading...");

	getSchema();

	// HTTP status code will be 0 if the upload was aborted
	if(statusCode == 0)
	{
		cancelUpload();
	}

	QString fileName = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/.projOutForUpload.prj";
	QFile::remove(fileName);

	imp->uploadReady = false;
	imp->fileToken = "";

	
	if(statusCode == 0)
	{
		imp->dbPanel->ShowMessage("Upload cancelled.");
	}
    else if(statusCode == 403)
	{
		imp->dbPanel->updateModifyReady(false);
		
		imp->loggedOut();
	}
    else
	{
		imp->dbPanel->ShowMessage(r->readAll());
	}
}

void CModelRepoConnectionHandler::deleteProjectRepy(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		getSchema();
	}

	imp->dbPanel->ShowMessage(r->readAll());

}

void CModelRepoConnectionHandler::requestUploadPermissionsReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		imp->dbPanel->ShowMessage("Your request for uploader permissions will be reviewed.\nYou should "
				"receive an email when your request has been granted.");
	}
	else
	{
		QString message = "An unknown server error has occurred.\nHTTP Staus Code: ";
		message += std::to_string(statusCode).c_str();

		imp->dbPanel->ShowMessage(message);
	}
}

void CModelRepoConnectionHandler::getMessagesReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		imp->dbPanel->ShowWelcomeMessage(r->readAll());
	}
}


QString CModelRepoConnectionHandler::getUsername()
{
	return imp->username;
}

int CModelRepoConnectionHandler::getUploadPermission()
{
	return imp->uploadPermission;
}

qint64 CModelRepoConnectionHandler::getSizeLimit()
{
	return imp->sizeLimit;
}

bool CModelRepoConnectionHandler::isAuthenticated()
{
	return imp->authenticated;
}

void CModelRepoConnectionHandler::setUploadReady(bool ready)
{
	imp->uploadReady = ready;
}

bool CModelRepoConnectionHandler::isUploadReady()
{
	return imp->uploadReady;
}

void CModelRepoConnectionHandler::loggedOut()
{
	imp->loggedOut();
}

#else

CRepoConnectionHandler::CRepoConnectionHandler(CRepositoryPanel* dbPanel, CLocalDatabaseHandler* dbHandler, CMainWindow* wnd){}
CRepoConnectionHandler::~CRepoConnectionHandler(){}
void CRepoConnectionHandler::authenticate(QString userName, QString password){}
void CRepoConnectionHandler::getFile(int id, int type){}
//void CRepoConnectionHandler::upload(QByteArray projectInfo){}
void CRepoConnectionHandler::connFinished(QNetworkReply *r){}
void CRepoConnectionHandler::authReply(QNetworkReply *r){}
void CRepoConnectionHandler::getFileReply(QNetworkReply *r){}
//void CRepoConnectionHandler::uploadReply(QNetworkReply *r){}
//void CRepoConnectionHandler::TCPUpload(QString fileToken){}
void CRepoConnectionHandler::sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors) {}
void CRepoConnectionHandler::progress(qint64 bytesReceived, qint64 bytesTotal) {}
QString CRepoConnectionHandler::getUsername() {	return ""; }
qint64 CRepoConnectionHandler::getSizeLimit() { return 0; }

#endif
