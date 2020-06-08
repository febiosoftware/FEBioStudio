#include "stdafx.h"
#include "DlgAddStep.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <MeshTools/FEModel.h>
#include "CIntInput.h"
#include <FEMLib/FEMKernel.h>
#include <MeshTools/FEProject.h>

class Ui::CDlgAddStep
{
public:
	QLineEdit* name;
	QListWidget* type;
	QComboBox*	steps;

public:
	void setupUi(QWidget* parent)
	{
		name = new QLineEdit;
		name->setPlaceholderText("(Leave blank for default)");
		type = new QListWidget;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);

		steps = new QComboBox;
		form->addRow("Insert after:", steps);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(type);
		mainLayout->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(type, SIGNAL(itemDoubleClicked(QListWidgetItem*)), parent, SLOT(accept()));

		parent->setLayout(mainLayout);
	}
};

CDlgAddStep::CDlgAddStep(FEProject& prj, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddStep)
{
	setWindowTitle("Add Analysis Step");

	ui->setupUi(this);

	FEModel& fem = prj.GetFEModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		ui->steps->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}
	ui->steps->setCurrentIndex(fem.Steps() - 1);

	// set the types
	vector<FEClassFactory*> l = FEMKernel::FindAllClasses(prj.GetModule(), FE_ANALYSIS);
	for (int i=0; i<l.size(); ++i)
	{
		QListWidgetItem* w = new QListWidgetItem;
		w->setText(l[i]->GetTypeStr());
		w->setData(Qt::UserRole, l[i]->GetClassID());
		ui->type->addItem(w);
	}
}

void CDlgAddStep::accept()
{
	// get the name
	QString name = ui->name->text();
	m_name = name.toStdString();

	// get the variable
	QListWidgetItem* item = ui->type->currentItem();
	if (item)
	{
		m_ntype = item->data(Qt::UserRole).toInt();
	}
	else
	{
		QMessageBox::critical(this, windowTitle(), "Please select a step type.");
		return;
	}

	QDialog::accept();
}

int CDlgAddStep::insertPosition() const
{
	return ui->steps->currentIndex();
}
