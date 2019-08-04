#include "stdafx.h"
#include "DlgAddEquation.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>

class CDlgAddEquation_UI
{
public:
	QLineEdit*	name;
	QLineEdit*	eq[3];
	QComboBox*	type;
	QLabel*	lbl[3];

public:
	void setup(QDialog* dlg)
	{
		QFormLayout* f = new QFormLayout;

		f->addRow("Name:", name = new QLineEdit);
		f->addRow("Type:", type = new QComboBox);
		f->addRow(lbl[0] = new QLabel("Equation:"), eq[0] = new QLineEdit);
		f->addRow(lbl[1] = new QLabel("Y:"), eq[1] = new QLineEdit);
		f->addRow(lbl[2] = new QLabel("Z:"), eq[2] = new QLineEdit);
		f->setLabelAlignment(Qt::AlignRight);

		makeFloatEdit();

		type->addItems(QStringList() << "FLOAT" << "VEC3F");
		
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(f);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(type, SIGNAL(currentIndexChanged(int)), dlg, SLOT(typeChanged(int)));
	}

	void makeFloatEdit()
	{
		lbl[0]->setText("Equation:");
		lbl[1]->hide();
		lbl[2]->hide();

		eq[1]->hide();
		eq[2]->hide();
	}

	void makeVec3Edit()
	{
		lbl[0]->setText("X:");
		lbl[1]->show();
		lbl[2]->show();

		eq[1]->show();
		eq[2]->show();
	}
};

CDlgAddEquation::CDlgAddEquation(QWidget* parent) : QDialog(parent), ui(new CDlgAddEquation_UI)
{
	ui->setup(this);
}

void CDlgAddEquation::typeChanged(int n)
{
	if (n == 0) ui->makeFloatEdit();
	else ui->makeVec3Edit();
}

QString CDlgAddEquation::GetDataName()
{
	return ui->name->text();
}

QString CDlgAddEquation::GetEquation(int n)
{
	return ui->eq[n]->text();
}

int CDlgAddEquation::GetDataType()
{
	return ui->type->currentIndex();
}
