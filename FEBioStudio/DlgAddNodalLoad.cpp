#include "stdafx.h"
#include "DlgAddNodalLoad.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <PreViewLib/FEModel.h>
#include "CIntInput.h"

class Ui::CDlgAddNodalLoad
{
public:
	QLineEdit* name;
	QComboBox* step;
	QComboBox* var;
	CFloatInput* value;

public:
	void setupUi(QWidget* parent)
	{
		name = new QLineEdit; name->setPlaceholderText("(leave empty for default)");
		step = new QComboBox;

		var = new QComboBox;

		value = new CFloatInput;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);
		form->addRow("Step:", step);
		form->addRow("Variable:", var);
		form->addRow("Load:", value);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));

		parent->setLayout(mainLayout);
	}
};

CDlgAddNodalLoad::CDlgAddNodalLoad(FEModel& fem, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddNodalLoad)
{
	setWindowTitle("Add Nodal Load");

	ui->setupUi(this);

	// add the steps
	for (int i = 0; i<fem.Steps(); ++i)
	{
		ui->step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	// set the variables
	ui->var->addItem("x-force");
	ui->var->addItem("y-force");
	ui->var->addItem("z-force");
	ui->var->addItem("fluid volumetric flow rate");

	ui->value->setValue(1.0);
}

void CDlgAddNodalLoad::accept()
{
	// get the name
	QString name = ui->name->text();
	m_name = name.toStdString();

	// get the step
	m_nstep = ui->step->currentIndex();

	// get the variable
	m_nvar = ui->var->currentIndex();

	// get the value
	m_val = ui->value->value();

	QDialog::accept();
}
