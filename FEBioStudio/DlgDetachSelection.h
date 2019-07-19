#pragma once
#include <QDialog>

namespace Ui {
	class CDlgDetachSelection;
	class CDlgExtractSelection;
}

class CDlgDetachSelection : public QDialog
{
public:
	CDlgDetachSelection(QWidget* parent);

	void accept();

	QString getName();

protected:
	Ui::CDlgDetachSelection*	ui;
};

class CDlgExtractSelection : public QDialog
{
public:
	CDlgExtractSelection(QWidget* parent);

	void accept();

	QString getName();

protected:
	Ui::CDlgExtractSelection*	ui;
};
