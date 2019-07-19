#pragma once

#include <QDialog>
#include <string>

class FEProject;

namespace Ui {
	class CDlgAddIC;
};

class CDlgAddIC : public QDialog
{
public:
	CDlgAddIC(FEProject& fem, QWidget* parent);

	void accept();

public:
	std::string	m_name;
	int		m_nstep;
	int		m_ntype;

private:
	Ui::CDlgAddIC* ui;
};
