#include "stdafx.h"
#include "DlgImportSTL.h"
#include <QCheckBox>
#include <QRadioButton>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QButtonGroup>

CDlgImportSTL::CDlgImportSTL(QWidget* parent) : QDialog(parent)
{
	m_nselect = 0;

	m_sel[0] = new QRadioButton("ASCII STL");	m_sel[0]->setChecked(true);
	m_sel[1] = new QRadioButton("Binary STL");

	QButtonGroup* bg = new QButtonGroup(this);
	bg->addButton(m_sel[0]);
	bg->addButton(m_sel[1]);

	QVBoxLayout* v = new QVBoxLayout;
	v->addWidget(m_sel[0]);
	v->addWidget(m_sel[1]);

	QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	v->addWidget(bb);

	setLayout(v);

	QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
}

void CDlgImportSTL::accept()
{
	m_nselect = (m_sel[0]->isChecked() ? 0 : 1);
	QDialog::accept();
}
