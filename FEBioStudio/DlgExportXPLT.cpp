#include "stdafx.h"
#include "DlgExportXPLT.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QCheckBox>

//-----------------------------------------------------------------------------
class Ui::CDlgExportXPLT
{
public:
	QCheckBox*	pc;

public:
	void setupUi(::CDlgExportXPLT* pwnd)
	{
		QVBoxLayout* pg = new QVBoxLayout(pwnd);

		pc = new QCheckBox("Data compression");

		pg->addWidget(pc);
		pg->addStretch();
	
		QHBoxLayout* ph = new QHBoxLayout;
		QPushButton* pb1 = new QPushButton("OK");
		QPushButton* pb2 = new QPushButton("Cancel");

		ph->addStretch();
		ph->addWidget(pb1);
		ph->addWidget(pb2);
		ph->addStretch();
		pg->addLayout(ph);

		QObject::connect(pb1, SIGNAL(clicked()), pwnd, SLOT(accept()));
		QObject::connect(pb2, SIGNAL(clicked()), pwnd, SLOT(reject()));
	}
};


CDlgExportXPLT::CDlgExportXPLT(CMainWindow* pwnd) : ui(new Ui::CDlgExportXPLT)
{
	m_bcompress = false;
	ui->setupUi(this);
}

void CDlgExportXPLT::accept()
{
	m_bcompress = ui->pc->isChecked();
	QDialog::accept();
}
