#pragma once

#include <string>
#include <vector>
#include "HelpDialog.h"

using namespace std;

class FEProject;

namespace Ui {
	class CDlgAddRigidConnector;
};

class CDlgAddRigidConnector : public CHelpDialog
{
public:
	CDlgAddRigidConnector(FEProject& prj, QWidget* parent);

	int GetType();
	std::string GetName();
	int GetStep();

	int GetMaterialA();
	int GetMaterialB();

protected:
	void accept();
	void SetURL();

private:
	Ui::CDlgAddRigidConnector*		ui;
};
