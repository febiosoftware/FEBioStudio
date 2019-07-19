#include "stdafx.h"
#include "DlgAddBC.h"
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
#include <FEMLib/FEMKernel.h>

class Ui::CDlgAddBC
{
public:
	QLineEdit* name;
	QComboBox* step;
	QListWidget* list;

public:
	void setupUi(QWidget* parent)
	{
		name = new QLineEdit;
		name->setPlaceholderText("(leave blank for default)");
		step = new QComboBox;

		list = new QListWidget;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);
		form->addRow("Step:", step);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(list);
		mainLayout->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), parent, SLOT(accept()));

		parent->setLayout(mainLayout);
	}

	void addBC(const std::string& name, int nid)
	{
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(name));
		item->setData(Qt::UserRole, nid);
		list->addItem(item);
	}
};

CDlgAddBC::CDlgAddBC(FEProject& prj, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddBC)
{
	FEModel& fem = prj.GetFEModel();

	setWindowTitle("Add BC");

	ui->setupUi(this);

	// add the steps
	for (int i = 0; i<fem.Steps(); ++i)
	{
		ui->step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	// add the essential BCs
	vector<FEClassFactory*> bc = FEMKernel::FindAllClasses(prj.GetModule(), FE_ESSENTIAL_BC);
	int N = (int) bc.size();
	for (int i=0; i<N; ++i)
	{
		FEClassFactory* fac = bc[i];
		ui->addBC(fac->GetTypeStr(), fac->GetClassID());
	}
}

void CDlgAddBC::accept()
{
	// get the name
	QString name = ui->name->text();
	m_name = name.toStdString();

	// get the step
	m_nstep = ui->step->currentIndex();

	QListWidgetItem* item = ui->list->currentItem();
	if (item == 0)
	{
		QMessageBox::critical(this, windowTitle(), "Please select a boundary condition.");
		return;
	}

	// store the BC type
	m_bctype = item->data(Qt::UserRole).toInt();

	QDialog::accept();
}
