#pragma once
#include "CommandPanel.h"
#include <QDialog>

class CPostDocument;

namespace Ui{
	class CStatePanel;
	class CDlgAddState;
}

class CMainWindow;

class CStatePanel : public CCommandPanel
{
	Q_OBJECT

public:
	CStatePanel(CMainWindow* pwnd, QWidget* parent = 0);

	void Update(bool breset) override;

	CPostDocument* GetActiveDocument();

private slots:
	void on_stateList_doubleClicked(const QModelIndex& index);
	void on_addButton_clicked();
	void on_editButton_clicked();
	void on_deleteButton_clicked();

private:
	Ui::CStatePanel* ui;
};

class CDlgAddState : public QDialog
{
	Q_OBJECT

public:
	CDlgAddState(QWidget* parent);

private slots:
	void accept();

public:
	int		m_nstates;
	double	m_minTime;
	double	m_maxTime;

private:
	Ui::CDlgAddState* ui;
};
