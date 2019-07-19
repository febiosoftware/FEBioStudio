#include "stdafx.h"
#include "DlgPurge.h"
#include <QLayout>
#include <QDialogButtonBox>
#include <QRadioButton>

CDlgPurge::CDlgPurge(QWidget* w) : QDialog(w)
{
	QVBoxLayout* l = new QVBoxLayout;

	m_b[0] = new QRadioButton("Remove all materials, boundary conditions, loads, etc."); m_b[0]->setChecked(true);
	m_b[1] = new QRadioButton("Clear all selections of boundary conditions, loads, etc.");

	QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	l->addWidget(m_b[0]);
	l->addWidget(m_b[1]);
	l->addWidget(bb);

	setLayout(l);

	QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
}

void CDlgPurge::accept()
{
	if (m_b[0]->isChecked()) m_option = 0;
	else m_option = 1;

	QDialog::accept();
}
