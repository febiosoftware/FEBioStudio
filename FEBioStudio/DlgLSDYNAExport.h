#pragma once
#include <QDialog>

namespace Ui {
	class CDlgLSDYNAExport;
}

class CDlgLSDYNAExport : public QDialog
{
public:
	CDlgLSDYNAExport(QWidget* parent);

	void accept();

public:
	bool	m_bselonly;
	bool	m_bshell_thick;

private:
	Ui::CDlgLSDYNAExport*	ui;
};
