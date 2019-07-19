#pragma once

#include <QDialog>
#include <string>

class FEProject;

namespace Ui {
	class CDlgAddBodyLoad;
};

class CDlgAddBodyLoad : public QDialog
{
public:
	CDlgAddBodyLoad(FEProject& fem, QWidget* parent);

	void accept();

public:
	std::string	m_name;
	int		m_nstep;
	int		m_ntype;

private:
	Ui::CDlgAddBodyLoad* ui;
};
