#pragma once
#include <QDialog>

class CDlgKinematUI;
class CMainWindow;

class CDlgKinemat : public QDialog
{
	Q_OBJECT

public:
	CDlgKinemat(CMainWindow* parent);

private slots:
	void OnBrowse1();
	void OnBrowse2();
	void OnApply();

private:
	CDlgKinematUI*	ui;
};
