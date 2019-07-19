#pragma once
#include <QDialog>
#include <string>
#include <vector>
using namespace std;

class FEProject;
class GMaterial;

namespace Ui {
	class CDlgAddRigidConstraint;
};

class CDlgAddRigidConstraint : public QDialog
{
public:
	CDlgAddRigidConstraint(FEProject& fem, QWidget* parent);

	void accept();

	GMaterial* GetMaterial() { return m_selMat; }

public:
	std::string	m_name;
	int		m_nstep;
	int		m_type;

private:
	Ui::CDlgAddRigidConstraint* ui;
	vector<GMaterial*>	m_mat;
	GMaterial*			m_selMat;
};
