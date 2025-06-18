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

#pragma once

#include <QObject>
#include <QList>
#include <QSslError>

class QNetworkReply;
//class QSslError;
class CRepositoryPanel;
class CModelDatabaseHandler;
class CMainWindow;

#define API_URL "/modelRepo/api/v1.03/"

class CModelRepoConnectionHandler : public QObject
{
	Q_OBJECT

	class Imp;

public:
	CModelRepoConnectionHandler(CRepositoryPanel* dbPanel, CModelDatabaseHandler* dbHandler, CMainWindow* wnd);
	~CModelRepoConnectionHandler();

	void authenticate(QString userName, QString password);
	void getSchema();
	void getTables();
	void getFile(int id, int type);
	void uploadFileRequest(QByteArray projectInfo);
	void uploadFile();
	void requestUploadPermissions(QByteArray userInfo);

	void modifyProject(int id, QByteArray projectInfo);
	void modifyProjectUpload();
	void deleteProject(int id);

	void cancelUpload();

	QString getUsername();
	int getUploadPermission();
	qint64 getSizeLimit();
	bool isAuthenticated();

	void setUploadReady(bool ready);
	bool isUploadReady();

	void loggedOut();

private slots:
	void connFinished(QNetworkReply *r);
	void sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors);
	void progress(qint64 bytesReceived, qint64 bytesTotal);

private:
	bool NetworkAccessibleCheck();
	void getMessages();

	void authReply(QNetworkReply *r);
	void getSchemaReply(QNetworkReply *r);
	void getTablesReply(QNetworkReply *r);
	void getFileReply(QNetworkReply *r);
	void uploadFileRequestReply(QNetworkReply *r);
	void uploadFileReply(QNetworkReply *r);
	void requestUploadPermissionsReply(QNetworkReply *r);
	void getMessagesReply(QNetworkReply *r);

	void modifyProjectRepy(QNetworkReply *r);
	void modifyProjectUploadReply(QNetworkReply *r);
	void deleteProjectRepy(QNetworkReply *r);

private:
	Imp* imp;

};
