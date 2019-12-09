#include "DatabaseHandler.h"

#ifdef MODEL_REPO
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QBuffer>
#include <QJsonDocument>
#include <QVariantMap>
#include <QUrl>
#include <QDir>
#include <QList>
#include <QJsonObject>
#include <QSaveFile>
#include <QFileDialog>
#include <QTcpSocket>
#include "DatabasePanel.h"
#include "MainWindow.h"
#include "Document.h"

#include <quazip5/quazip.h>
#include "ZipFiles.h"
#include <iostream>

class CDatabaseHandler::Imp
{
public:
	Imp(CDatabasePanel* dbPanel, CDatabaseHandler* handler, CMainWindow* wnd)
		: dbPanel(dbPanel), m_wnd(wnd)
	{
		restclient = new QNetworkAccessManager(handler);
	}

	CDatabasePanel* dbPanel;
	QNetworkAccessManager* restclient;
	CMainWindow* m_wnd;


	QString username;
	QString token;


};


CDatabaseHandler::CDatabaseHandler(CDatabasePanel* dbPanel, CMainWindow* wnd)
{
	imp = new Imp(dbPanel, this, wnd);

	connect(imp->restclient, SIGNAL(finished(QNetworkReply*)), this, SLOT(connFinished(QNetworkReply*)));
}

CDatabaseHandler::~CDatabaseHandler()
{
	delete imp;
}

void CDatabaseHandler::authenticate(QString userName, QString password)
{
	imp->username = userName;

	QVariantMap feed;
	feed.insert("username", userName);
	feed.insert("password", password);
	QByteArray payload=QJsonDocument::fromVariant(feed).toJson();

	QUrl myurl;
	myurl.setScheme("http");
	myurl.setHost("omen.sci.utah.edu");
	myurl.setPort(5000);
	myurl.setPath("/modelRepo/api/v1.0/authenticate");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	imp->restclient->setNetworkAccessible(QNetworkAccessManager::Accessible);

//	if(imp->restclient->networkAccessible() == QNetworkAccessManager::Accessible)
//	{
//		qDebug() << "Network accessible";
//	}
//	else
//	{
//		qDebug() << "Network is not accessible";
//	}

	imp->restclient->post(request, payload);
}

void CDatabaseHandler::getModelList()
{
	QUrl myurl;
	myurl.setScheme("http");
	myurl.setHost("omen.sci.utah.edu");
	myurl.setPort(5000);
	myurl.setPath("/modelRepo/api/v1.0/models");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setRawHeader(QByteArray("userName"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());

//	if(imp->restclient->networkAccessible() == QNetworkAccessManager::Accessible)
//	{
//		qDebug() << "Network accessible";
//	}
//	else
//	{
//		qDebug() << "Network is not accessible";
//	}

	imp->restclient->get(request);

}

void CDatabaseHandler::getFile(int id)
{
	QUrl myurl;
	myurl.setScheme("http");
	myurl.setHost("omen.sci.utah.edu");
	myurl.setPort(5000);
	myurl.setPath(QString("/modelRepo/api/v1.0/files/%1").arg(id));

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setRawHeader(QByteArray("userName"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());

//	if(imp->restclient->networkAccessible() == QNetworkAccessManager::Accessible)
//	{
//		qDebug() << "Network accessible";
//	}
//	else
//	{
//		qDebug() << "Network is not accessible";
//	}

	imp->restclient->get(request);
}

void CDatabaseHandler::upload(QByteArray projectInfo)
{
	QUrl myurl;
	myurl.setScheme("http");
	myurl.setHost("omen.sci.utah.edu");
	myurl.setPort(5000);
	myurl.setPath("/modelRepo/api/v1.0/upload");

	QNetworkRequest request;
	request.setUrl(myurl);
	request.setRawHeader(QByteArray("userName"), imp->username.toUtf8());
	request.setRawHeader(QByteArray("token"), imp->token.toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

//	if(imp->restclient->networkAccessible() == QNetworkAccessManager::Accessible)
//	{
//		qDebug() << "Network accessible";
//	}
//	else
//	{
//		qDebug() << "Network is not accessible";
//	}

	imp->restclient->post(request, projectInfo);
}

void CDatabaseHandler::connFinished(QNetworkReply *r)
{
	QString URL = r->request().url().toString();

	if(URL.contains("authenticate"))
	{
		authReply(r);
	}
	else if(URL.contains("models"))
	{
		modelListReply(r);
	}
	else if(URL.contains("files/"))
	{
		getFileReply(r);
	}
	else if(URL.contains("upload"))
	{
		uploadReply(r);
	}

}

void CDatabaseHandler::authReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	imp->token = (QString)r->readAll();

	std::cout << statusCode << std::endl;
	std::cout << imp->token.toStdString() << std::endl;

	getModelList();
//	TCPUpload("test");

//	QVariantMap feed;
//	feed.insert("name", "testprojectFBS");
//	feed.insert("description", "Test to see if this works in FBS");
//	feed.insert("version", 1);
//	QList<QVariant> files;
//	files.append("testfile.feb");
//	feed.insert("files", files);
//	QByteArray payload=QJsonDocument::fromVariant(feed).toJson();
//
//	upload(payload);
}


void CDatabaseHandler::modelListReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	std::cout << statusCode << std::endl;
//	std::cout << ((QString)r->readAll()).toStdString() << std::endl;
//	std::cout << ((QString)r->readAll()).toStdString() << std::endl;

	QJsonDocument jsonDoc = QJsonDocument::fromJson(r->readAll());

	imp->dbPanel->SetModelList(jsonDoc);

}

void CDatabaseHandler::getFileReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	std::cout << statusCode << std::endl;

	QString fileName = QFileDialog::getSaveFileName(NULL, "Export", ".", "FEBio Studio Project (*.prj)");
	if (fileName.isEmpty() == false)
	{
		// make sure the file has an extension
		std::string sfile = fileName.toStdString();
		std::size_t found = sfile.rfind(".");
		if (found == std::string::npos) sfile.append(".prj");


		QByteArray data = r->readAll();

		QSaveFile file(sfile.c_str());
		file.open(QIODevice::WriteOnly);

		file.write(data);
		file.commit();

//		imp->m_wnd->OpenFEModel("/home/sci/mherron/Desktop/test.zip");

		imp->m_wnd->ImportProject(sfile.c_str());

	}

}

void CDatabaseHandler::uploadReply(QNetworkReply *r)
{
	int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	std::cout << statusCode << std::endl;

	QString fileToken = r->readAll();

	cout << fileToken.toStdString() << endl;

	TCPUpload(fileToken);

//	imp->dbPanel->SetModelList(jsonDoc);

}

void CDatabaseHandler::TCPUpload(QString fileToken)
{
	QTcpSocket socket;
	socket.connectToHost("omen.sci.utah.edu", 50000);

	socket.waitForReadyRead(5000);

	QByteArray message = socket.readAll();

//	cout << message.toStdString() << endl;

	socket.write(imp->token.toUtf8());

	socket.waitForReadyRead(5000);

	message = socket.readAll();

//	cout << message.toStdString() << endl;

	socket.write(fileToken.toUtf8());

	socket.waitForReadyRead(5000);

	message = socket.readAll();

//	cout << message.toStdString() << endl;

	string fileName = imp->m_wnd->GetDocument()->GetDocFolder() + "/.projOutForUpload.prj";

//	cout << "FileName: " << fileName << endl;

	archive(fileName.c_str(), QDir(imp->m_wnd->GetDocument()->GetDocFolder().c_str()));

	QFile arch(fileName.c_str());
	arch.open(QIODevice::ReadOnly);

	while(socket.write(arch.read(1024)))
	{
		socket.flush();
	}

	arch.close();

	arch.remove();


//	QBuffer buff;
//	buff.open(QBuffer::ReadWrite);

//	QuaZip zip(&socket);
//	if (zip.getZipError() != 0) {
//			cout << "Zip error: " << zip.getZipError() << endl;
//		}
//
//	zip.setFileNameCodec("IBM866");
//	if (!zip.open(QuaZip::mdCreate)) {
//		cout << "Open Failed." << endl;
//		return;
//	}
//
//	QFile inFile;
//	QuaZipFile outFile(&zip);
//
//	QStringList sl;
//	QDir dir(imp->m_wnd->GetDocument()->GetDocFolder().c_str());
//	cout << "Dir: "<< imp->m_wnd->GetDocument()->GetDocFolder().c_str() << endl;
//
//	recurseAddDir(dir, sl);
//
//	QFileInfoList files;
//	foreach (QString fn, sl) files << QFileInfo(fn);
//
//	foreach(QFileInfo fileInfo, files) {
//
//		if (!fileInfo.isFile())
//			continue;
//
//		QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);
//		cout << fileNameWithRelativePath.toStdString() << endl;
//
//		inFile.setFileName(fileInfo.filePath());
//
//		if (!inFile.open(QIODevice::ReadOnly)) {
//			cout << "Failed to open inFile" << endl;
//			return;
//		}
//
//		if(fileInfo.isSymLink())
//		{
//			if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithRelativePath, fileInfo.symLinkTarget()))) {
//				return;
//			}
//		}
//		else
//		{
//			if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithRelativePath, fileInfo.filePath()))) {
//				return;
//			}
//		}
//
//		while(outFile.write(inFile.read(1024)))
//		{
//				socket.flush();
//		}
//
//
//
////		while (inFile.getChar(&c) && outFile.putChar(c));
//
////		socket.write(buff.readAll());
////		socket.flush();
//
//		if (outFile.getZipError() != UNZ_OK) {
//			cout << "Out error: " << outFile.getZipError() << endl;
//		}
////
////		if (outFile.getZipError() != UNZ_OK) {
////			return;
////		}
//
//		outFile.close();
//
//		inFile.close();
//	}
//
////	while(socket.write(buff.read(1024)))
////	{
////		socket.flush();
////	}
//
//	zip.close();
//
//	if (zip.getZipError() != 0) {
//		cout << "Zip error: " << zip.getZipError() << endl;
//	}


}
#else

CDatabaseHandler::CDatabaseHandler(CDatabasePanel* dbPanel, CMainWindow* wnd){}
CDatabaseHandler::~CDatabaseHandler(){}
void CDatabaseHandler::authenticate(QString userName, QString password){}
void CDatabaseHandler::getModelList(){}
void CDatabaseHandler::getFile(int id){}
void CDatabaseHandler::upload(QByteArray projectInfo){}
void CDatabaseHandler::connFinished(QNetworkReply *r){}
void CDatabaseHandler::authReply(QNetworkReply *r){}
void CDatabaseHandler::modelListReply(QNetworkReply *r){}
void CDatabaseHandler::getFileReply(QNetworkReply *r){}
void CDatabaseHandler::uploadReply(QNetworkReply *r){}
void CDatabaseHandler::TCPUpload(QString fileToken){}

#endif







