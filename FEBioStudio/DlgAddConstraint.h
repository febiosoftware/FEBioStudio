#pragma once

#include <QDialog>
#include <string>

class FEProject;

namespace Ui {
	class CDlgAddConstraint;
};

class CDlgAddConstraint : public QDialog
{
public:
	CDlgAddConstraint(FEProject& prj, QWidget* parent);

	void accept();

public:
	std::string	m_name;
	int		m_nstep;
	int		m_ntype;

private:
	Ui::CDlgAddConstraint* ui;
};
