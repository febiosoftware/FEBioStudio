#pragma once

#include <QDialog>
#include <string>

class FEProject;

namespace Ui {
	class CDlgAddStep;
};

class CDlgAddStep : public QDialog
{
public:
	CDlgAddStep(FEProject& prj, QWidget* parent);

	void accept();

	int insertPosition() const;

public:
	std::string	m_name;
	int		m_ntype;

private:
	Ui::CDlgAddStep* ui;
};
