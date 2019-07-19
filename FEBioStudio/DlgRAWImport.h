#pragma once
#include <QDialog>

namespace Ui {
	class CDlgRAWImport;
}

class CDlgRAWImport : public QDialog
{
public:
	CDlgRAWImport(QWidget* parent);

	void accept();

public:
	int	m_nx, m_ny, m_nz;
	double	m_x0, m_y0, m_z0;
	double	m_w, m_h, m_d;

private:
	Ui::CDlgRAWImport*	ui;
};
