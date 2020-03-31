#pragma once
#include <QDialog>

namespace Ui {
	class CDlgExportXPLT;
};

class CMainWindow;

class CDlgExportXPLT : public QDialog
{
public:
	CDlgExportXPLT(CMainWindow* pwnd);

	void accept() override;

public:
	bool	m_bcompress;

protected:
	Ui::CDlgExportXPLT*	ui;
};
