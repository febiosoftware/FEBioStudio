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

private slots:
	void on_fileList_itemDoubleClicked(QTreeWidgetItem* item, int column);
	void onCreateFolder();
	void onMoveToFolder(int i);
	void onRemoveFolder();
	void onRenameFolder();

private:
	Ui::CFileViewer*	ui;
};
