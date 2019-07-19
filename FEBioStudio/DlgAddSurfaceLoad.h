#pragma once

#include <QDialog>
#include <string>

class FEProject;

namespace Ui {
	class CDlgAddSurfaceLoad;
};

class CDlgAddSurfaceLoad : public QDialog
{
public:
	CDlgAddSurfaceLoad(FEProject& prj, QWidget* parent);

	void accept();

public:
	std::string	m_name;
	int		m_nstep;
	int		m_ntype;

private:
	Ui::CDlgAddSurfaceLoad* ui;
};
