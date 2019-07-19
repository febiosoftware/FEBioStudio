#pragma once
#include <QDialog>
#include <string>

class FEProject;

namespace Ui {
	class CDlgAddBC;
};

class CDlgAddBC : public QDialog
{
public:
	CDlgAddBC(FEProject& fem, QWidget* parent);

	void accept();

public:
	std::string	m_name;
	int		m_nstep;
	int		m_bctype;

private:
	Ui::CDlgAddBC* ui;
};
