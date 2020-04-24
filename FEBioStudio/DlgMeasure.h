#pragma once
#include <QDialog>

namespace Ui {
	class CDlgMeasure;
}

class CMainWindow;

class CDlgMeasure : public QDialog
{
	Q_OBJECT

public:
	CDlgMeasure(CMainWindow* wnd);

	void Update();

	void showEvent(QShowEvent* ev) override;
	void closeEvent(QCloseEvent* ev) override;

public slots:
	void onMeasureChanged(int n);

private:
	Ui::CDlgMeasure*	ui;
};
