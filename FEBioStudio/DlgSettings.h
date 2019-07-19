#pragma once
#include <QDialog>

class CMainWindow;
class QAbstractButton;

namespace Ui {
	class CDlgSettings;
};

class CDlgSettings : public QDialog
{
	Q_OBJECT

public:
	CDlgSettings(CMainWindow* pwnd);

	void showEvent(QShowEvent* ev);

	void apply();

public slots:
	void accept();
	void onClicked(QAbstractButton*);
	void onTabChanged(int n);

protected:
	CMainWindow*		m_pwnd;
	Ui::CDlgSettings*	ui;
};
