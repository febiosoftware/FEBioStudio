#include "stdafx.h"
#include "DlgEditProject.h"
#include <MeshTools/FEProject.h>
#include <QListWidget>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>

class Ui::CDlgEditProject
{
public:
	QListWidget* list;
	
public:
	void setup(QDialog* dlg)
	{
		list = new QListWidget;

		QPushButton* all  = new QPushButton("All");
		QPushButton* none = new QPushButton("None");

		QVBoxLayout* l = new QVBoxLayout;
		l->setMargin(0);
		l->addWidget(all);
		l->addWidget(none);
		l->addStretch();

		QHBoxLayout* h = new QHBoxLayout;
		h->setMargin(0);
		h->addWidget(list);
		h->addLayout(l);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QLabel* m = new QLabel("Modules:");
		m->setAlignment(Qt::AlignLeft);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(m);
		mainLayout->addLayout(h);
		mainLayout->addWidget(bb);

		dlg->setLayout(mainLayout);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(all, SIGNAL(clicked(bool)), dlg, SLOT(onAllClicked()));
		QObject::connect(none, SIGNAL(clicked(bool)), dlg, SLOT(onNoneClicked()));
	}

	void addModule(const QString& name, int id, bool checked)
	{
		QListWidgetItem* item = new QListWidgetItem;
		item->setText(name);
		item->setData(Qt::UserRole, id);
		item->setCheckState(checked? Qt::Checked : Qt::Unchecked);

		list->addItem(item);
	}

	bool isRowChecked(int nrow)
	{
		QListWidgetItem* item = list->item(nrow);
		if (item)
		{
			return (item->checkState() == Qt::Checked);
		}
		else
		{
			assert(false);
			return false;
		}
	}

	unsigned int getModule()
	{
		unsigned int mod = 0;
		int N = list->count();
		for (int i=0; i<N; ++i)
		{
			QListWidgetItem* item = list->item(i);
			if (item->checkState() == Qt::Checked)
			{
				mod |= item->data(Qt::UserRole).toInt();
			}
		}
		return mod;
	}

	void checkAll(bool checked)
	{
		int N = list->count();
		for (int i = 0; i<N; ++i)
		{
			QListWidgetItem* item = list->item(i);
			item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
		}
	}
};

CDlgEditProject::CDlgEditProject(FEProject& prj, QWidget* parent): QDialog(parent), ui(new Ui::CDlgEditProject), m_prj(prj)
{
	ui->setup(this);

	unsigned int mod = prj.GetModule();

	ui->addModule("Mechanics"			, MODULE_MECH				, (mod & MODULE_MECH));
	ui->addModule("Heat Transer"		, MODULE_HEAT				, (mod & MODULE_HEAT));
	ui->addModule("Biphasic"			, MODULE_BIPHASIC			, (mod & MODULE_BIPHASIC));
	ui->addModule("Solutes"				, MODULE_SOLUTES			, (mod & MODULE_SOLUTES));
	ui->addModule("Multiphasic"			, MODULE_MULTIPHASIC		, (mod & MODULE_MULTIPHASIC));
	ui->addModule("Fluid"				, MODULE_FLUID				, (mod & MODULE_FLUID));
	ui->addModule("Reactions"			, MODULE_REACTIONS			, (mod & MODULE_REACTIONS));
	ui->addModule("Reaction-Diffusion"	, MODULE_REACTION_DIFFUSION	, (mod & MODULE_REACTION_DIFFUSION));
    ui->addModule("Fluid-FSI"			, MODULE_FLUID_FSI  , (mod & MODULE_FLUID_FSI));
}

void CDlgEditProject::accept()
{
	unsigned int module = ui->getModule();

	if (module == 0)
	{
		QMessageBox::critical(this, "Edit Properties", "You must select at least one module.");
		return;
	}
	
	m_prj.SetModule(module);

	QDialog::accept();
}

void CDlgEditProject::onAllClicked()
{
	ui->checkAll(true);
}

void CDlgEditProject::onNoneClicked()
{
	ui->checkAll(false);
}

