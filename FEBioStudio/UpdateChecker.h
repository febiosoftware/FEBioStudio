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

#include <QWidget>
#include <QDialog>
#include <QSslError>
#include <QStringList>
#include <unordered_map>

class QNetworkReply;
class QVBoxLayout;
class QLabel;
class QCheckBox;
class QNetworkAccessManager;
class QDialogButtonBox;
class QXmlStreamReader;

#ifdef WIN32
	#define URL_BASE "/update2/FEBioStudio2/Windows"
	#define DEV_BASE "/update2/FEBioStudio2Dev/Windows"
	#define UPDATER_BASE "/update2/Updater2/Windows"
	#define REL_ROOT "\\..\\"
	#define UPDATER "/FEBioStudioUpdater.exe"
#elif __APPLE__
	#define URL_BASE "/update2/FEBioStudio2/macOS"
	#define DEV_BASE "/update2/FEBioStudio2Dev/macOS"
	#define UPDATER_BASE "/update2/Updater2/macOS"
	#define REL_ROOT "/../../../"
	#define UPDATER "/FEBioStudioUpdater"
#else
	#define URL_BASE "/update2/FEBioStudio2/Linux"
	#define DEV_BASE "/update2/FEBioStudio2Dev/Linux"
	#define UPDATER_BASE "/update2/Updater2/Linux"
	#define REL_ROOT "/../"
	#define UPDATER "/FEBioStudioUpdater"
#endif

struct ReleaseFile
{
	QString baseURL;
    QString name;
	qint64 size;
};

struct Release
{
	bool active;
	bool terminal;
	qint64 timestamp;
	QString FEBioVersion;
	QString FBSVersion;
	QString FEBioNotes;
	QString FBSNotes;
	QString releaseMsg;
    bool hasSDK = false;
    ReleaseFile sdk;
	std::vector<ReleaseFile> files;
	QStringList deleteFiles;
	std::vector<ReleaseFile> updaterFiles;
    QStringList installCommands;
    QStringList uninstallCommands;
};

class CUpdateWidget : public QWidget
{
    Q_OBJECT

public:
    CUpdateWidget(QWidget* parent = nullptr);

    void checkForUpdate(bool dev = false, bool checkSDK = false, bool updaterUpdateCheck = false, QString branch = "");

    QString getServerMessage();

	QString getUpdaterPath() const;

public slots:
	void linkActivated(const QString& link);

signals:
    void ready(bool update, bool terminal=false);

private slots:
	void connFinished(QNetworkReply *r);
	void sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors);

private:
	void checkForAppUpdate();
    void checkForAppUpdateResponse(QNetworkReply *r);
    void parseAppXML(QXmlStreamReader& reader, bool dev);

	void checkForUpdaterUpdate();
	void checkForUpdaterUpdateResponse(QNetworkReply *r);

    void showUpdateInfo();
	void showUpdaterUpdateInfo();
    void showUpToDate();
	void showTerminal();
    void showError(const QString& error);

	void ReadLastUpdateInfo();

public:
	QVBoxLayout* layout;
	QLabel* infoLabel;

    QNetworkAccessManager* restclient;

    std::vector<ReleaseFile> updateFiles;
	QStringList deleteFiles;
    QStringList installCmds;
    QStringList uninstallCmds;
	qint64 overallSize;

    std::vector<Release> releases;
	std::vector<Release> updaterReleases;
	qint64 lastUpdate;
	qint64 serverTime;

	bool devChannel;
    bool devAlreadyParsed;
    QString m_branch;
	bool updaterUpdateCheck;
	bool doingUpdaterUpdate;

    bool m_askSDK;
    QCheckBox* m_getSDK;
    ReleaseFile m_sdk;

	QString UUID;

    QString serverMessage;

	//Made this so that QStringView can look up without making a copy.
	const QString UPDATE       = "update";
	const QString RELEASE      = "release";
	const QString ACTIVE       = "active";
    const QString TERMINAL     = "terminal";
	const QString TIMESTAMP    = "timestamp";
	const QString FEBIOVERSION = "FEBioVersion";
	const QString FBSVERSION   = "FBSVersion";
	const QString FEBIONOTES   = "FEBioNotes";
	const QString FBSNOTES     = "FBSNotes";
	const QString RELEASEMSG   = "releaseMsg";
    const QString SDK          = "sdk";
	const QString FEBFILES     = "files";
	const QString FEBFILE      = "file";
	const QString DELETEFILES  = "deleteFiles";
	const QString AUTOUPDATE   = "autoUpdate";
	const QString LASTUPDATE   = "lastUpdate";
    const QString INSTALLCMD   = "installCmds";
    const QString UNINSTALLCMD = "uninstallCmds";
    const QString COMMAND      = "cmd";

};

class CUpdateChecker : public QDialog
{
	Q_OBJECT

public:
	CUpdateChecker(bool dev, QWidget *parent = nullptr);

	bool doUpdate() { return update; }

private:
	void accept() override;

private slots:
	void updateWidgetReady(bool update, bool terminal);

private:
	QVBoxLayout* layout;
	QDialogButtonBox* box;
	bool update;
	bool updateAvailable;
};