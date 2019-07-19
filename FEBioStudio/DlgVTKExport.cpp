#include "stdafx.h"
#include "DlgVTKExport.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>

class Ui::CDlgVTKExport
{
public:
	QCheckBox* shellThick;
	QCheckBox* scalarData;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* lo = new QVBoxLayout;

		lo->addWidget(shellThick = new QCheckBox("Shell thickness"));
		lo->addWidget(scalarData = new QCheckBox("Scalar data"));

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		lo->addWidget(bb);

		parent->setLayout(lo);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgVTKExport::CDlgVTKExport(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgVTKExport)
{
	ui->setupUi(this);
}

void CDlgVTKExport::accept()
{
	m_bshell_thick = ui->shellThick->isChecked();
	m_bscalar_data = ui->scalarData->isChecked();

	QDialog::accept();
}
