#pragma once
#include <QWidget>
#include <vector>
using namespace std;

class QTreeWidgetItem;
class CMainWindow;

namespace Ui {
	class CFileViewer;
}

class CFileViewer : public QWidget
{
	Q_OBJECT

public:
	CFileViewer(CMainWindow* pwnd, QWidget* parent = 0);

	void Update();

	void contextMenuEvent(QContextMenuEvent* ev) override;

private:
	QTreeWidgetItem* currentItem();

private slots:
	void on_fileList_itemDoubleClicked(QTreeWidgetItem* item, int column);
	void onCreateGroup();
	void onMoveToGroup(int i);
	void onRemoveGroup();
	void onRenameGroup();
	void onShowInExplorer();
	void onRemoveFromProject();

private:
	Ui::CFileViewer*	ui;
};