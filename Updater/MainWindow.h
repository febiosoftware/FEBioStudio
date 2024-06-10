#pragma once
#include <QWizard>
#include <QSslError>

#ifdef WIN32
#define FEBIOBINARY "\\febio4.exe"
#define FBSBINARY "\\FEBioStudio2.exe"
#define FBSUPDATERBINARY "\\FEBioStudioUpdater.exe"
#define MVUTIL "\\mvUtil.exe"
#elif __APPLE__
#define FEBIOBINARY "/febio4"
#define FBSBINARY "/FEBioStudio"
#define FBSUPDATERBINARY "/FEBioStudioUpdater"
#define MVUTIL "/mvUtil"
#else
#define FEBIOBINARY "/febio4"
#define FBSBINARY "/FEBioStudio"
#define FBSUPDATERBINARY "/FEBioStudioUpdater"
#define MVUTIL "/mvUtil"
#endif

namespace Ui{
class CMainWindow;
}

class QNetworkAccessManager;
class QNetworkReply;

class CMainWindow : public QWizard
{
	Q_OBJECT

public:
	CMainWindow(bool devChannel, bool updaterUpdateCheck);

    void makePath(QString path);
    void addNewFile(const QString filename);
protected:
	void initializePage(int id) override;

public slots:
	void updateWidgetReady(bool update, bool terminal);
	void onFinish();

private slots:
	void connFinished(QNetworkReply *r);
	void sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors);
	void progress(qint64 bytesReceived, qint64 bytesTotal);
    void unzipFinished();

private:
    bool checkBinaries();
    bool isFileWriteable(QString filename, QString niceName);

	void getFile();
	void getFileReponse(QNetworkReply *r);

    void getSDK();
    void getSDKResponse(QNetworkReply *r);

	void deleteFiles();
	void downloadsFinished();

private:
	Ui::CMainWindow* ui;
	QNetworkAccessManager* restclient;

	bool m_devChannel;
	bool m_updaterUpdateCheck;
    bool m_downloadingSDK;
};
