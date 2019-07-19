#pragma once
#include <QDialog>

class FEProject;
class FEPlotVariable;
class QListWidgetItem;

namespace Ui {
	class CDlgAddDomain;
	class CDlgEditOutput;
}

class CDlgAddDomain : public QDialog
{
	Q_OBJECT

public:
	CDlgAddDomain(QWidget* parent);

	void setVariable(const FEPlotVariable& var);
	void setDomains(const QStringList& l);

	int selectedDomain();

private:
	Ui::CDlgAddDomain*	ui;
};

class CDlgEditOutput : public QDialog
{
	Q_OBJECT

public:
	CDlgEditOutput(FEProject& prj, QWidget* parent = 0, int tab = 0);

	void showEvent(QShowEvent* ev) override;

private:
	void UpdateVariables(const QString& flt);
	void UpdateLogTable();

protected slots:
	void OnAddDomain();
	void OnRemoveDomain();
	void OnNewVariable();
	void OnVariable(int nrow);
	void OnItemClicked(QListWidgetItem* item);
	void onFilterChanged(const QString& txt);
	void onLogAdd();
	void onLogRemove();
	void UpdateLogItemList();

private:
	FEProject&	m_prj;
	Ui::CDlgEditOutput*	ui;
};
