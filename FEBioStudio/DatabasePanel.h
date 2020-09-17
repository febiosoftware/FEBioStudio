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
#include <QVariantMap>
#include <vector>
using namespace std;

namespace Ui {
	class CDatabasePanel;
	class CMainWindow;
}

class CMainWindow;
class CRepoConnectionHandler;
class CLocalDatabaseHandler;
class QTreeWidgetItem;
class QLabel;
class QProgressBar;
class CustomTreeWidgetItem;
class ZipThread;

enum FILETYPE {FULL=0, PART=1};

class CDatabasePanel : public QWidget
{
	Q_OBJECT

public:
	CDatabasePanel(CMainWindow* pwnd, QWidget* parent = 0);
	~CDatabasePanel();

	void SetModelList();
	void ShowMessage(QString message);
	void LoginTimeout();
	void NetworkInaccessible();
	void DownloadFinished(int fileID, int fileType);

	// SQLite callbacks
	void AddCategory(char **argv);
	void AddProject(char **argv);
	void AddProjectFile(char **argv);
	void SetProjectData(char **argv);
	void SetFileData(char **argv);
	void AddCurrentTag(char **argv);
	void AddPublication(QVariantMap data);
	void AddCurrentFileTag(char **argv);

	QString GetRepositoryFolder();
	void SetRepositoryFolder(QString folder);

	void showMainPage();
	void showLoadingPage(QString message, bool progress = false);

public slots:
	void updateUploadReady(bool ready);
	void updateModifyReady(bool ready);
	void loadingPageProgress(qint64 bytesSent, qint64 bytesTotal);

signals:
	void cancelClicked();

private slots:
	void on_connectButton_clicked();
	void on_loginButton_clicked();
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_actionRefresh_triggered();
	void on_actionDownload_triggered();
	void on_actionOpen_triggered();
	void on_actionOpenFileLocation_triggered();
	void on_actionDelete_triggered();
	void on_actionUpload_triggered();
	void on_actionSearch_triggered();
	void on_actionClearSearch_triggered();
	void on_actionDeleteRemote_triggered();
	void on_actionModify_triggered();
	void on_treeWidget_itemSelectionChanged();
	void on_treeWidget_customContextMenuRequested(const QPoint &pos);
	void on_projectTags_linkActivated(const QString& link);
	void on_fileTags_linkActivated(const QString& link);

private:
	void DownloadItem(CustomTreeWidgetItem *item);
	void OpenItem(CustomTreeWidgetItem *item);
	void DeleteItem(CustomTreeWidgetItem *item);
	void ShowItemInBrowser(CustomTreeWidgetItem *item);

	QStringList GetCategories();

private:
	CMainWindow*		m_wnd;
	CRepoConnectionHandler*	repoHandler;
	CLocalDatabaseHandler* dbHandler;
	QString m_repositoryFolder;

	Ui::CDatabasePanel*	ui;
};
