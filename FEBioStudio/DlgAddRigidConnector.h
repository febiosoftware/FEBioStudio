#pragma once

#include <QDialog>
#include <string>
#include <vector>
using namespace std;

class FEProject;

namespace Ui {
	class CDlgAddRigidConnector;
};

class CDlgAddRigidConnector : public QDialog
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

private:
	Ui::CDlgAddRigidConnector*		ui;
};
