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

private slots:
	void on_fileList_itemDoubleClicked(QTreeWidgetItem* item, int column);

private:
	Ui::CFileViewer*	ui;
};
