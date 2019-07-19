#pragma once
#include <QWizard>
#include "FEBioOpt.h"

class FEModel;

class CMainWindow;
class QLineEdit;
class QPushButton;

namespace Ui {
	class CDlgFEBioOptimize;
	class CDlgSelectParam;
}

class CDlgFEBioOptimize : public QWizard
{
	Q_OBJECT

public:
	CDlgFEBioOptimize(CMainWindow* parent);

	FEBioOpt GetFEBioOpt();

protected slots:
	void on_addParameter_clicked();
	void on_addData_clicked();
	void on_pasteData_clicked();

private:
	Ui::CDlgFEBioOptimize*	ui;
};

class CSelectParam : public QWidget
{
	Q_OBJECT

private:
	QLineEdit*		m_edit;
	QPushButton*	m_push;
	FEModel*		m_fem;
	int				m_paramOption;

public:
	CSelectParam(FEModel* fem, int paramOption = 0, QWidget* parent = nullptr);

	void clear();
	QString text();

private slots:
	void onSelectClicked();
};

class CDlgSelectParam : public QDialog
{
	Q_OBJECT

public:
	CDlgSelectParam(FEModel* fem, int paramOption, QWidget* parent = nullptr);

	void accept() override;

	QString text();

private:
	Ui::CDlgSelectParam*	ui;
};
