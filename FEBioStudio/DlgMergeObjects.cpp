#include "stdafx.h"
#include "DlgMergeObjects.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QValidator>

class Ui::CDlgMergeObjects
{
public:
	QLineEdit*	name;
	QCheckBox*	check;
	QLineEdit*	tol;

public:
	void setupUi(QWidget* parent)
	{
		QFormLayout* form = new	QFormLayout;
		
		form->addRow("Name:", name = new QLineEdit);
		form->addRow("", check = new QCheckBox("Weld"));
		form->addRow("Tolerance:", tol = new QLineEdit); tol->setValidator(new QDoubleValidator);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(bb);

		parent->setLayout(mainLayout);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgMergeObjects::CDlgMergeObjects(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgMergeObjects)
{
	ui->setupUi(this);

	static int ncount = 1;
	char sz[256] = {0};
	sprintf(sz, "MergedObject%d", ncount++);
	ui->name->setText(sz);
	ui->check->setChecked(true);
	ui->tol->setText(QString::number(1e-6));
}

void CDlgMergeObjects::accept()
{
	m_name = ui->name->text().toStdString();
	m_weld = ui->check->isChecked();
	m_tol  = ui->tol->text().toDouble();

	QDialog::accept();
}
