#pragma once
#include <QWidget>
#include <vector>
using namespace std;

namespace Ui {
	class CDatabasePanel;
}

class CMainWindow;
class CDatabaseHandler;
class QTreeWidgetItem;

class CDatabasePanel : public QWidget
{
	Q_OBJECT

public:
	CDatabasePanel(CMainWindow* pwnd, QWidget* parent = 0);
	~CDatabasePanel();

	void SetModelList(QJsonDocument& jsonDoc);

private slots:
	void on_loginButton_clicked();
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void displayProjectData();

private:
	CMainWindow*		m_wnd;
	CDatabaseHandler*	dbHandler;

	Ui::CDatabasePanel*	ui;
};
