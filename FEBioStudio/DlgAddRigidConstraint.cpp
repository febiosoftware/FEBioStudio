#include "stdafx.h"
#include "DlgAddRigidConstraint.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <PreViewLib/FEModel.h>
#include <PreViewLib/FEProject.h>
#include <PreViewLib/FERigidConstraint.h>
#include <PreViewLib/FEMultiMaterial.h>
#include <FEMLib/FEMKernel.h>

class Ui::CDlgAddRigidConstraint
{
public:
	QLineEdit* name;
	QComboBox* step;
	QComboBox* mat;
	QListWidget* list;

public:
	void setupUi(QWidget* parent)
	{
		name = new QLineEdit;
		name->setPlaceholderText("(leave blank for default)");
		step = new QComboBox;

		mat = new QComboBox;

		list = new QListWidget;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);
		form->addRow("Step:", step);
		form->addRow("Rigid material:", mat);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(list);
		mainLayout->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), parent, SLOT(accept()));

		parent->setLayout(mainLayout);
	}
};

CDlgAddRigidConstraint::CDlgAddRigidConstraint(FEProject& prj, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddRigidConstraint)
{
	setWindowTitle("Add Rigid Constraint");

	ui->setupUi(this);

	// add the steps
	FEModel& fem = prj.GetFEModel();
	for (int i = 0; i<fem.Steps(); ++i)
	{
		ui->step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	// set the materials
	m_selMat = 0;
	for (int i=0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (dynamic_cast<FERigidMaterial*>(pm->GetMaterialProperties()))
		{
			ui->mat->addItem(QString::fromStdString(pm->GetName()));
			m_mat.push_back(pm);
		}
	}
	if (m_mat.empty() == false) m_selMat = m_mat[0];

	// add the DOFs
	int mod = prj.GetModule();
	vector<FEClassFactory*> v = FEMKernel::FindAllClasses(mod, FE_RIGID_CONSTRAINT);
	for (int i=0; i<(int)v.size(); ++i)
	{
		FEClassFactory* fac = v[i];

		QListWidgetItem* item = new QListWidgetItem(ui->list);
		item->setText(QString::fromStdString(fac->GetTypeStr()));
		item->setData(Qt::UserRole, fac->GetClassID());
	}
}

void CDlgAddRigidConstraint::accept()
{
	// get the name
	QString name = ui->name->text();
	m_name = name.toStdString();

	// get the step
	m_nstep = ui->step->currentIndex();

	if (m_mat.empty() == false)
	{
		m_selMat = m_mat[ui->mat->currentIndex()];
	}

	QListWidgetItem* item = ui->list->currentItem();
	if (item == 0)
	{
		QMessageBox::critical(this, windowTitle(), "Please select a rigid constraint");
		return;
	}

	m_type = item->data(Qt::UserRole).toInt();

	QDialog::accept();
}
