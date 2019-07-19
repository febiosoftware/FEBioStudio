#pragma once

#include <QDialog>
#include <string>

class FEProject;

namespace Ui {
	class CDlgAddContact;
};

class CDlgAddContact : public QDialog
{
public:
	CDlgAddContact(FEProject& prj, QWidget* parent);

	void accept();

public:
	std::string	m_name;
	int		m_nstep;
	int		m_ntype;

private:
	Ui::CDlgAddContact* ui;
};
