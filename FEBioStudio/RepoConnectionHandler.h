#include <QtCore/QObject>
#include <QList>

class QNetworkReply;
class QSslError;
class CDatabasePanel;
class CLocalDatabaseHandler;
class CMainWindow;

class CRepoConnectionHandler : public QObject
{
	Q_OBJECT

	class Imp;

public:
	CRepoConnectionHandler(CDatabasePanel* dbPanel, CLocalDatabaseHandler* dbHandler, CMainWindow* wnd);
	~CRepoConnectionHandler();

	void authenticate(QString userName, QString password);
//	void getModelList();
	void getTables();
	void getFile(int id, int type);
//	void upload(QByteArray projectInfo);
	void uploadFileRequest(QByteArray projectInfo);
	void uploadFile(QString fileToken);

	QString getUsername();
	int getUploadPermission();

private slots:
	void connFinished(QNetworkReply *r);
	void sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors);
	void progress(qint64 bytesReceived, qint64 bytesTotal);

private:
	bool NetworkAccessibleCheck();
//	bool AuthCheck();

	void authReply(QNetworkReply *r);
//	void authCheckReply(QNetworkReply *r);
//	void modelListReply(QNetworkReply *r);
	void getTablesReply(QNetworkReply *r);
	void getFileReply(QNetworkReply *r);
//	void uploadReply(QNetworkReply *r);
	void uploadFileRequestReply(QNetworkReply *r);
	void uploadFileReply(QNetworkReply *r);

//	void TCPUpload(QString fileToken);

	Imp* imp;

};
