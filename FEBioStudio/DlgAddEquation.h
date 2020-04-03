#pragma once
#include <QDialog>

class CDlgAddEquation_UI;

class CDlgAddEquation : public QDialog
{
	Q_OBJECT

public:
	CDlgAddEquation(QWidget* parent = 0);

	QString GetDataName();

	int GetDataType();

	QString GetScalarEquation();
	QStringList GetVectorEquations();
	QStringList GetMatrixEquations();

private:
	CDlgAddEquation_UI*	ui;
};
