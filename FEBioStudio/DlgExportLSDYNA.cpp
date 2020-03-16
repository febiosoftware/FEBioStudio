#include "stdafx.h"
#include "DlgExportLSDYNA.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QMessageBox>
#include "DataFieldSelector.h"
using namespace Post;

class CDlgExportLSDYNA_UI
{
public:
	QCheckBox*	selOnly;
	QCheckBox*	expSurf;
	QCheckBox*	expResu;

public:
	void setup(QDialog* dlg)
	{
		selOnly = new QCheckBox("Selection only");
		expSurf = new QCheckBox("Export surface");
		expResu = new QCheckBox("Export nodal results");

		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(selOnly);
		l->addWidget(expSurf);
		l->addWidget(expResu);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgExportLSDYNA::CDlgExportLSDYNA(QWidget* parent) : QDialog(parent), ui(new CDlgExportLSDYNA_UI)
{
	m_bsel = false;
	m_bsurf = false;
	m_bnode = false;

	ui->setup(this);
}

void CDlgExportLSDYNA::accept()
{
	m_bsel = ui->selOnly->isChecked();
	m_bsurf = ui->expSurf->isChecked();
	m_bnode = ui->expResu->isChecked();

	QDialog::accept();
}

class CDlgExportLSDYNAPlot_UI
{
public:
	QCheckBox*			m_flag[6];
	CDataFieldSelector*	m_code[6];
	FEModel*			m_fem;

public:
	void setup(QDialog* dlg)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow(m_flag[0] = new QCheckBox("stress"        ), m_code[0] = new CDataFieldSelector); m_code[0]->BuildMenu(m_fem, Data_Tensor_Type::DATA_TENSOR2);
		form->addRow(m_flag[1] = new QCheckBox("plastic strain"), m_code[1] = new CDataFieldSelector); m_code[1]->BuildMenu(m_fem, Data_Tensor_Type::DATA_SCALAR);
		form->addRow(m_flag[2] = new QCheckBox("displacement"  ), m_code[2] = new CDataFieldSelector); m_code[2]->BuildMenu(m_fem, Data_Tensor_Type::DATA_VECTOR);
		form->addRow(m_flag[3] = new QCheckBox("velocity"      ), m_code[3] = new CDataFieldSelector); m_code[3]->BuildMenu(m_fem, Data_Tensor_Type::DATA_VECTOR);
		form->addRow(m_flag[4] = new QCheckBox("acceleration"  ), m_code[4] = new CDataFieldSelector); m_code[4]->BuildMenu(m_fem, Data_Tensor_Type::DATA_VECTOR);
		form->addRow(m_flag[5] = new QCheckBox("temperature"   ), m_code[5] = new CDataFieldSelector); m_code[5]->BuildMenu(m_fem, Data_Tensor_Type::DATA_SCALAR);

		if (m_code[0]->setCurrentValue("stress"        )) m_flag[0]->setChecked(true);
		if (m_code[1]->setCurrentValue("plastic strain")) m_flag[1]->setChecked(true);
		if (m_code[2]->setCurrentValue("displacement"  )) m_flag[2]->setChecked(true);
		if (m_code[3]->setCurrentValue("velocity"      )) m_flag[3]->setChecked(true);
		if (m_code[4]->setCurrentValue("acceleration"  )) m_flag[4]->setChecked(true);
		if (m_code[5]->setCurrentValue("temperature"   )) m_flag[5]->setChecked(true);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgExportLSDYNAPlot::CDlgExportLSDYNAPlot(FEModel* fem, QWidget* parent) : QDialog(parent), ui(new CDlgExportLSDYNAPlot_UI)
{
	for (int i = 0; i < 6; ++i)
	{
		m_flag[i] = false;
		m_code[i] = -1;
	}

	ui->m_fem = fem;
	ui->setup(this);
}

void CDlgExportLSDYNAPlot::accept()
{
	for (int i = 0; i < 6; ++i)
	{
		m_flag[i] = ui->m_flag[i]->isChecked();
		m_code[i] = ui->m_code[i]->currentValue();
		if (m_flag[i] && (m_code[i] == -1))
		{
			QMessageBox::critical(this, "PostView2", QString("Please select a valid field for \"%1\" or uncheck this item").arg(ui->m_flag[i]->text()));
			return;
		}
	}

	QDialog::accept();
}
