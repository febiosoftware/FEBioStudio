/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

class QNetworkReply;
class QSslError;
class QVBoxLayout;
class QLabel;
class QNetworkAccessManager;
class QDialogButtonBox;
class QCheckBox;

// #define UPDATE_URL "repo.febio.org"
// #define PORT 4433
// #define SCHEME "https"

#define UPDATE_URL "localhost"
#define PORT 5236
#define SCHEME "http"

#ifdef WIN32
	#define URL_BASE "/update/FEBioStudio/Windows"
	#define REL_ROOT "\\..\\"
	#define UPDATER "\\FEBioStudioUpdater.exe"
#elif __APPLE__
	#define URL_BASE "/update/FEBioStudio/macOS"
	#define REL_ROOT "/../../../"
	#define UPDATER "/FEBioStudioUpdater"
#else
	#define URL_BASE "/update/FEBioStudio/Linux"
	#define REL_ROOT "/../"
	#define UPDATER "/FEBioStudioUpdater"
#endif

struct ReleaseFile
{
	QString name;
	qint64 size;
};

struct Release
{
	bool active;
	qint64 timestamp;
	QString FEBioVersion;
	QString FBSVersion;
	QString FEBioNotes;
	QString FBSNotes;
	QString releaseMsg;
	std::vector<ReleaseFile> files;
	QStringList deleteFiles;
};


class CUpdateWidget : public QWidget
{
    Q_OBJECT

public:
    CUpdateWidget(QWidget* parent = nullptr);

    void checkForUpdate();

public slots:
	void linkActivated(const QString& link);

signals:
    void ready(bool update);

private slots:
	void connFinished(QNetworkReply *r);
	void sslErrorHandler(QNetworkReply *reply, const QList<QSslError> &errors);

private:
	bool NetworkAccessibleCheck();

    void checkForUpdateResponse(QNetworkReply *r);

    void showUpdateInfo();
    void showUpToDate();
    void showError(const QString& error);

public:
	QVBoxLayout* layout;
	QLabel* infoLabel;

    QNetworkAccessManager* restclient;

    QStringList updateFiles;
	QStringList deleteFiles;
	QStringList newFiles;
	QStringList newDirs;
	int currentIndex;
	qint64 overallSize;
	qint64 downloadedSize;

    std::vector<Release> releases;
	qint64 lastUpdate;
	qint64 serverTime;

};

class CUpdateChecker : public QDialog
{
	Q_OBJECT

public:
	CUpdateChecker(bool autoUpdate, QWidget *parent = nullptr);

	bool doUpdate() { return update; }
	bool autoUpdateCheck();

private:
	void accept() override;

private slots:
	void updateWidgetReady(bool update);

private:
	QVBoxLayout* layout;
	QDialogButtonBox* box;
	QCheckBox* autoUpdateCB;
	bool update;
	bool updateAvailable;
};