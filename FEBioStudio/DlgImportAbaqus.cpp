#include "stdafx.h"
#include "DlgImportAbaqus.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QBoxLayout>

class Ui::CDlgImportAbaqus
{
public:
	QCheckBox* pc[5];

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* layout = new QVBoxLayout;

		pc[0] = new QCheckBox("Import node sets");
		pc[1] = new QCheckBox("Import element sets");
		pc[2] = new QCheckBox("Import surface");
		pc[3] = new QCheckBox("Auto-partition from element sets");
		pc[4] = new QCheckBox("Auto-partition surface");

		layout->addWidget(pc[0]);
		layout->addWidget(pc[1]);
		layout->addWidget(pc[2]);
		layout->addWidget(pc[3]);
		layout->addWidget(pc[4]);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(bb);

		parent->setLayout(layout);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgImportAbaqus::CDlgImportAbaqus(AbaqusImport* fileReader, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgImportAbaqus)
{
	m_fileReader = fileReader;
	ui->setupUi(this);

	ui->pc[0]->setChecked(m_fileReader->m_bnodesets);
	ui->pc[1]->setChecked(m_fileReader->m_belemsets);
	ui->pc[2]->setChecked(m_fileReader->m_bfacesets);
	ui->pc[3]->setChecked(m_fileReader->m_bautopart);
	ui->pc[4]->setChecked(m_fileReader->m_bautosurf);
}

void CDlgImportAbaqus::accept()
{
	m_fileReader->m_bnodesets = ui->pc[0]->isChecked();
	m_fileReader->m_belemsets = ui->pc[1]->isChecked();
	m_fileReader->m_bfacesets = ui->pc[2]->isChecked();
	m_fileReader->m_bautopart = ui->pc[3]->isChecked();
	m_fileReader->m_bautosurf = ui->pc[4]->isChecked();
	QDialog::accept();
}
