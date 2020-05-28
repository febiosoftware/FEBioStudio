#pragma once
#include <string>
#include <vector>

#include "HelpDialog.h"

using namespace std;

class FEProject;
class GMaterial;

namespace Ui {
	class CDlgAddRigidConstraint;
};

class CDlgAddRigidConstraint : public CHelpDialog
{
public:
	CDlgAddRigidConstraint(FEProject& prj, QWidget* parent);

	void accept();

	GMaterial* GetMaterial() { return m_selMat; }

public:
	std::string	m_name;
	int		m_nstep;
	int		m_type;

protected:
	void SetURL();

private:
	Ui::CDlgAddRigidConstraint* ui;
	vector<GMaterial*>	m_mat;
	GMaterial*			m_selMat;
};
