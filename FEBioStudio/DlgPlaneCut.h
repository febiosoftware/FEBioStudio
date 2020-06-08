#pragma once
#include <QDialog>

class UIDlgPlaneCut;

class CMainWindow;

class CDlgPlaneCut : public QDialog
{
	Q_OBJECT

public:
	CDlgPlaneCut(CMainWindow* wnd);
	~CDlgPlaneCut();

	void Update();

	void showEvent(QShowEvent* ev) override;
	void closeEvent(QCloseEvent* ev) override;
	void reject() override;

public slots:
	void onDataChanged();

private:
	UIDlgPlaneCut*	ui;
};
