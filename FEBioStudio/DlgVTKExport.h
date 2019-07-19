#pragma once
#include <QDialog>

namespace Ui {
	class CDlgVTKExport;
}

class CDlgVTKExport : public QDialog
{
public:
	CDlgVTKExport(QWidget* parent);

	void accept();

public:
	bool	m_bshell_thick;
	bool	m_bscalar_data;

private:
	Ui::CDlgVTKExport*	ui;
};
