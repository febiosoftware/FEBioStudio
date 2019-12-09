#include <QtCore/QObject>

class QNetworkReply;
class CDatabasePanel;
class CMainWindow;

class CDatabaseHandler : QObject
{
	Q_OBJECT

	class Imp;

public:
	CDatabaseHandler(CDatabasePanel* dbPanel, CMainWindow* wnd);
	~CDatabaseHandler();

	void authenticate(QString userName, QString password);
	void getModelList();
	void getFile(int id);
	void upload(QByteArray projectInfo);

private slots:
	void connFinished(QNetworkReply *r);

private:
	void authReply(QNetworkReply *r);
	void modelListReply(QNetworkReply *r);
	void getFileReply(QNetworkReply *r);
	void uploadReply(QNetworkReply *r);

	void TCPUpload(QString fileToken);

	Imp* imp;

};
