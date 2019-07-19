#pragma once

#include <QDialog>
#include <string>

class FEModel;

namespace Ui {
	class CDlgAddNodalLoad;
};

class CDlgAddNodalLoad : public QDialog
{
public:
	CDlgAddNodalLoad(FEModel& fem, QWidget* parent);

	void accept();

public:
	std::string	m_name;
	int		m_nstep;
	int		m_nvar;
	double	m_val;

private:
	Ui::CDlgAddNodalLoad* ui;
};
