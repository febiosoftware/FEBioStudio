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
#include "RepoConnectionHandler.h"

#ifdef MODEL_REPO
#include <QEventLoop>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslSocket>
//#include <QSslError>
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

using std::cout;
using std::endl;

class CRepoConnectionHandler::Imp
{
public:
	Imp(CRepositoryPanel* dbPanel, CLocalDatabaseHandler* dbHandler, CRepoConnectionHandler* handler, CMainWindow* wnd)
		: dbPanel(dbPanel), dbHandler(dbHandler), m_wnd(wnd), uploadPermission(0), sizeLimit(0), authenticated(false), uploadReady(false)
	{
		restclient = new QNetworkAccessManager(handler);
	}

	~Imp()
	{
		delete restclient;
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
	CLocalDatabaseHandler* dbHandler;
	CMainWindow* m_wnd;

	QNetworkAccessManager* restclient;

	QString username;
	QString token;
	int uploadPermission;
	qint64 sizeLimit;
	bool authenticated;

	QString fileToken;
	bool uploadReady;
};


CRepoConnectionHandler::CRepoConnectionHandler(CRepositoryPanel* dbPanel, CLocalDatabaseHandler* dbHandler, CMainWindow* wnd)
{
	imp = new Imp(dbPanel, dbHandler, this, wnd);

	connect(imp->restclient, SIGNAL(finished(QNetworkReply*)), this, SLOT(connFinished(QNetworkReply*)));
	connect(imp->restclient, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError>&)));
}

CRepoConnectionHandler::~CRepoConnectionHandler()
{
	delete imp;
}

void CRepoConnectionHandler::authenticate(QString username, QString password)
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
	myurl.setPath(QString(API_URL) + "authenticate");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	//imp->restclient->setNetworkAccessible(QNetworkAccessManager::Accessible);

	if(NetworkAccessibleCheck())
	{
		imp->restclient->post(request, payload);
	}

}

void CRepoConnectionHandler::getSchema()
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
		imp->restclient->get(request);
	}
}

void CRepoConnectionHandler::getTables()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + "tables");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

	if(NetworkAccessibleCheck())
	{
		imp->restclient->get(request);
	}

}

void CRepoConnectionHandler::getFile(int id, int type)
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + QString("files/%1/%2").arg(type).arg(id));

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);

	if(NetworkAccessibleCheck())
	{
		QNetworkReply* reply = imp->restclient->get(request);

		QObject::connect(reply, &QNetworkReply::downloadProgress, imp->dbPanel, &CRepositoryPanel::loadingPageProgress);
		QObject::connect(imp->dbPanel, &CRepositoryPanel::cancelClicked, reply, &QNetworkReply::abort);
	}

}

void CRepoConnectionHandler::uploadFileRequest(QByteArray projectInfo)
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + "uploadFileRequest");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	if(NetworkAccessibleCheck())
	{
		QNetworkReply* reply = imp->restclient->post(request, projectInfo);
		QObject::connect(imp->dbPanel, &CRepositoryPanel::cancelClicked, reply, &QNetworkReply::abort);
	}
}

void CRepoConnectionHandler::uploadFile()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + "uploadFile");

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

		QNetworkReply* reply = imp->restclient->post(request, arch);
		arch->setParent(reply);

		QObject::connect(reply, &QNetworkReply::uploadProgress, imp->dbPanel, &CRepositoryPanel::loadingPageProgress);
		QObject::connect(imp->dbPanel, &CRepositoryPanel::cancelClicked, reply, &QNetworkReply::abort);
	}
}

void CRepoConnectionHandler::requestUploadPermissions(QByteArray userInfo)
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + "requestUploaderPermissions");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	if(NetworkAccessibleCheck())
	{
		imp->restclient->post(request, userInfo);
	}
}

void CRepoConnectionHandler::getMessages()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + "messages");

	QNetworkRequest request;
	request.setUrl(myurl);

	if(NetworkAccessibleCheck())
	{
		imp->restclient->get(request);
	}
}

void CRepoConnectionHandler::modifyProject(int id, QByteArray projectInfo)
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + QString("projects/%1").arg(id));

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	if(NetworkAccessibleCheck())
	{
		QNetworkReply* reply = imp->restclient->put(request, projectInfo);
		QObject::connect(imp->dbPanel, &CRepositoryPanel::cancelClicked, reply, &QNetworkReply::abort);
	}
}

void CRepoConnectionHandler::modifyProjectUpload()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + "modifyProjectUpload");

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

		QNetworkReply* reply = imp->restclient->post(request, arch);
		arch->setParent(reply);

		QObject::connect(reply, &QNetworkReply::uploadProgress, imp->dbPanel, &CRepositoryPanel::loadingPageProgress);
		QObject::connect(imp->dbPanel, &CRepositoryPanel::cancelClicked, reply, &QNetworkReply::abort);
	}
}

void CRepoConnectionHandler::deleteProject(int id)
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + QString("projects/%1").arg(id));

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	if(NetworkAccessibleCheck())
	{
		imp->restclient->deleteResource(request);
	}
}

void CRepoConnectionHandler::cancelUpload()
{
	QUrl myurl;
	myurl.setScheme(ServerSettings::Scheme());
	myurl.setHost(ServerSettings::URL());
	myurl.setPort(ServerSettings::Port());
	myurl.setPath(QString(API_URL) + QString("cancelUpload"));

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::SameOriginRedirectPolicy);
	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setRawHeader(QByteArray("fileToken"), imp->fileToken.toUtf8());

	if(NetworkAccessibleCheck() && !imp->fileToken.isEmpty())
	{
		imp->restclient->deleteResource(request);
	}

	imp->fileToken = "";
	imp->uploadReady = false;
}

void CRepoConnectionHandler::connFinished(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	// If this version of FEBioStudio is too old, send message and prevent further interaction.
	if(statusCode == 426)
	{
		imp->dbPanel->ShowMessage("This version of FEBio Studio is too old to connect to the project "
				"repository.\n\nPlease update FEBio Studio in order to connect to the repository.");
		return;
	}

	QString URL = r->request().url().toString();

	if(URL.contains("authenticate"))
	{
		authReply(r);
	}
	else if(URL.contains("schema"))
	{
		getSchemaReply(r);
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
		getTablesReply(r);
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

void CRepoConnectionHandler::sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors)
{
	for(QSslError error : errors)
	{
		cout << error.errorString().toStdString() << endl;
	}

	reply->ignoreSslErrors();

}

void CRepoConnectionHandler::progress(qint64 bytesReceived, qint64 bytesTotal)
{
	imp->m_wnd->UpdateProgress((float)bytesReceived/(float)bytesTotal*100);
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

//bool CRepoConnectionHandler::AuthCheck()
//{
//	QUrl myurl;
//	myurl.setScheme(ServerSettings::Scheme());
//	myurl.setHost(ServerSettings::URL());
//	myurl.setPort(ServerSettings::Port());
//	myurl.setPath(QString(API_URL) + "authCheck");
//
//	QNetworkRequest request;
//	request.setUrl(myurl);
//	request.setRawHeader(QByteArray("username"), imp->username.toUtf8());
//	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
//	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
//
//	imp->restclient->get(request);
//
//	QEventLoop loop;
//	connect( this, &CRepoConnectionHandler::authCheckDone, &loop, &QEventLoop::quit );
//	loop.exec();
//
//	return imp->authorized;
//}

void CRepoConnectionHandler::authReply(QNetworkReply *r)
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

//void CRepoConnectionHandler::authCheckReply(QNetworkReply *r)
//{
//	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//
//	imp->authorized = (statusCode == 200);
//
//	cout << "authCheck response code: " << statusCode << endl;
//
//	emit authCheckDone();
//}

void CRepoConnectionHandler::getSchemaReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		std::string schema = QString(r->readAll()).toStdString();

		imp->dbHandler->init(schema);

		getTables();
	}
	else if(statusCode == 403)
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

void CRepoConnectionHandler::getTablesReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		QJsonDocument jsonDoc = QJsonDocument::fromJson(r->readAll());

		imp->dbHandler->update(jsonDoc);

		imp->dbPanel->SetModelList();
	}
	else if(statusCode == 403)
	{
		imp->loggedOut();
	}
	else
	{
		QString message = "An unknown server error has occurred.\nHTTP Status Code: ";
		message += std::to_string(statusCode).c_str();

		imp->dbPanel->ShowMessage(message, true);
	}

	getMessages();
}

void CRepoConnectionHandler::getFileReply(QNetworkReply *r)
{
	imp->m_wnd->ShowProgress(false);

	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(r->error() == QNetworkReply::OperationCanceledError)
	{

	}
	else if(statusCode == 200)
	{
		int fileID = r->rawHeader(QByteArray("fileID")).toInt();
		int IDType = r->rawHeader(QByteArray("IDType")).toInt();

//		QString path = imp->dbPanel->GetRepositoryFolder() + "/";
//		path += imp->dbHandler->FilePathFromID(fileID, IDType);
//
//		QString filename = path + "/";
//		filename += imp->dbHandler->FileNameFromID(fileID, IDType);

		QString filename = imp->dbHandler->FullFileNameFromID(fileID, IDType);

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

void CRepoConnectionHandler::uploadFileRequestReply(QNetworkReply *r)
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

void CRepoConnectionHandler::uploadFileReply(QNetworkReply *r)
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

void CRepoConnectionHandler::modifyProjectRepy(QNetworkReply *r)
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

void CRepoConnectionHandler::modifyProjectUploadReply(QNetworkReply *r)
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

void CRepoConnectionHandler::deleteProjectRepy(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		getSchema();
	}

	imp->dbPanel->ShowMessage(r->readAll());

}

void CRepoConnectionHandler::requestUploadPermissionsReply(QNetworkReply *r)
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

void CRepoConnectionHandler::getMessagesReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(statusCode == 200)
	{
		imp->dbPanel->ShowWelcomeMessage(r->readAll());
	}
}


QString CRepoConnectionHandler::getUsername()
{
	return imp->username;
}

int CRepoConnectionHandler::getUploadPermission()
{
	return imp->uploadPermission;
}

qint64 CRepoConnectionHandler::getSizeLimit()
{
	return imp->sizeLimit;
}

bool CRepoConnectionHandler::isAuthenticated()
{
	return imp->authenticated;
}

void CRepoConnectionHandler::setUploadReady(bool ready)
{
	imp->uploadReady = ready;
}

bool CRepoConnectionHandler::isUploadReady()
{
	return imp->uploadReady;
}

void CRepoConnectionHandler::loggedOut()
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
