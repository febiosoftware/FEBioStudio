#pragma once
#include <QDialog>
#include <Abaqus/AbaqusImport.h>

namespace Ui {
	class CDlgImportAbaqus;
}

class CDlgImportAbaqus : public QDialog
{
public:
	CDlgImportAbaqus(AbaqusImport* fileReader, QWidget* parent);

	void accept();

private:
	AbaqusImport*	m_fileReader;
	Ui::CDlgImportAbaqus*	ui;
};
