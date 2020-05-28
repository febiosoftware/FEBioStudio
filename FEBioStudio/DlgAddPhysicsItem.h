#pragma once
#include "HelpDialog.h"

class QListWidget;

class CDlgAddPhysicsItem : public CHelpDialog
{
public:
	CDlgAddPhysicsItem(QString windowName, int superID, FEProject& prj, QWidget* parent);

	std::string GetName();
	int GetStep();
	int GetClassID();

protected:
	void SetURL();

private:
	QListWidget* type;
	QLineEdit* name;
	QComboBox* step;

	int m_superID;

public:
//	QString	m_name;
//	int		m_nstep;
//	int		m_ntype;
//	int		m_module;
};
