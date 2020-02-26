#include "stdafx.h"
#include "DlgSelectRange.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include "CIntInput.h"

class Ui::CDlgSelectRange
{
public:
	CFloatInput *pmin, *pmax;
	QCheckBox* prange;

public:
	void setupUi(QDialog* parent)
	{
		QVBoxLayout* pv = new QVBoxLayout;

		QFormLayout* pform = new QFormLayout;
		pform->addRow("min:", pmin = new CFloatInput);
		pform->addRow("max:", pmax = new CFloatInput);
		pv->addLayout(pform);

		prange = new QCheckBox("Apply to current selection");
		pv->addWidget(prange);

		QDialogButtonBox* pb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(pb);

		parent->setLayout(pv);

		QObject::connect(pb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(pb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgSelectRange::CDlgSelectRange(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgSelectRange)
{
	ui->setupUi(this);

	ui->pmin->setValue(0);
	ui->pmax->setValue(0);
}

int CDlgSelectRange::exec()
{
	ui->pmin->setValue(m_min);
	ui->pmax->setValue(m_max);

	return QDialog::exec();
}

void CDlgSelectRange::accept()
{
	m_min = ui->pmin->value();
	m_max = ui->pmax->value();
	m_brange = ui->prange->isChecked();

	QDialog::accept();
}
