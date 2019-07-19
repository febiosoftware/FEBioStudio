#pragma once
#include <QDialog>

class CMainWindow;
class QTableWidgetItem;

namespace Ui {
	class CDlgSoluteTable;
};

class CDlgSoluteTable : public QDialog
{
	Q_OBJECT

public:
	enum Mode { ShowSolutes, ShowSBMs };

public:
	CDlgSoluteTable(int mode, CMainWindow* wnd);
	void Update();

private slots:
	void on_addButton_clicked();
	void on_removeButton_clicked();
	void on_table_itemChanged(QTableWidgetItem* item);

private:
	Ui::CDlgSoluteTable*	ui;
	CMainWindow*			m_wnd;
	int						m_mode;
};
