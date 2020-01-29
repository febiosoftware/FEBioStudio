#pragma once
#include <QWidget>
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
class CustomTreeWidgetItem;

enum FILETYPE {FULL=0, PART=1};

class CDatabasePanel : public QWidget
{
	Q_OBJECT

public:
	CDatabasePanel(CMainWindow* pwnd, QWidget* parent = 0);
	~CDatabasePanel();

	void Init(QString repositoryFolder);

	void SetModelList();
	void FailedLogin(QString message);
	void LoginTimeout();
	void NetworkInaccessible();
	void DownloadFinished(int fileID, int fileType);

	// SQLite callbacks
	void AddProject(char **argv);
	void AddProjectFile(char **argv);
	void SetProjectData(char **argv);
	void SetFileData(char **argv);
	void AddCurrentTag(char **argv);
	void PrintModel(char **argv);

	QString RepositoryFolder();

private slots:
	void on_loginButton_clicked();
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_actionDownload_triggered();
	void on_actionOpen_triggered();
	void on_actionOpenFileLocation_triggered();
	void on_actionDelete_triggered();
	void on_actionUpload_triggered();
	void on_actionSearch_triggered();
	void on_actionClearSearch_triggered();
	void on_treeWidget_itemSelectionChanged();
	void on_treeWidget_customContextMenuRequested(const QPoint &pos);

private:
	void DownloadItem(CustomTreeWidgetItem *item);
	void OpenItem(CustomTreeWidgetItem *item);
	void DeleteItem(CustomTreeWidgetItem *item);
	void ShowItemInBrowser(CustomTreeWidgetItem *item);

private:
	CMainWindow*		m_wnd;
	CRepoConnectionHandler*	repoHandler;
	CLocalDatabaseHandler* dbHandler;
	QString m_repositoryFolder;

	Ui::CDatabasePanel*	ui;
};
