#pragma once
#include <QDialog>

class CDlgAddEquation_UI;

class CDlgAddEquation : public QDialog
{
	Q_OBJECT

public:
	CDlgAddEquation(QWidget* parent = 0);

	QString GetDataName();
	QString GetEquation(int n = 0);
	int GetDataType();

protected slots:
	void typeChanged(int n);

private:
	CDlgAddEquation_UI*	ui;
};
