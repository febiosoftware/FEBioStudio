#include "stdafx.h"
#include "DlgLSDYNAExport.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>

class Ui::CDlgLSDYNAExport
{
public:
	QCheckBox* selectOnly;
	QCheckBox* shellThick;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* lo = new QVBoxLayout;

		lo->addWidget(selectOnly = new QCheckBox("Selection only"));
		lo->addWidget(shellThick = new QCheckBox("Shell thickness"));

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		lo->addWidget(bb);

		parent->setLayout(lo);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgLSDYNAExport::CDlgLSDYNAExport(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgLSDYNAExport)
{
	ui->setupUi(this);
}

void CDlgLSDYNAExport::accept()
{
	m_bselonly = ui->selectOnly->isChecked();
	m_bshell_thick = ui->shellThick->isChecked();

	QDialog::accept();
}
