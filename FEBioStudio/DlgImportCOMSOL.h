#pragma once
#include <QDialog>
#include <Comsol/COMSOLImport.h>

namespace Ui {
	class CDlgImportCOMSOL;
}

class CDlgImportCOMSOL : public QDialog
{
public:
	CDlgImportCOMSOL(COMSOLimport* fileReader, QWidget* parent);

	void accept();

private:
	COMSOLimport*	m_fileReader;
	Ui::CDlgImportCOMSOL*	ui;
};
