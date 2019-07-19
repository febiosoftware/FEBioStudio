#include "stdafx.h"
#include "DlgDetachSelection.h"
#include <QLineEdit>
#include <QFormLayout>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

class Ui::CDlgDetachSelection
{
public:
	QLineEdit	*name;

public:
	void setup(QWidget* w)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow("Name:", name = new QLineEdit);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		w->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), w, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), w, SLOT(reject()));
	}
};

CDlgDetachSelection::CDlgDetachSelection(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgDetachSelection)
{
	ui->setup(this);

	static int n = 1;
	
	QString newName = QString("Detached%1").arg(n++);
	ui->name->setText(newName);
}

void CDlgDetachSelection::accept()
{
	QString name = getName();
	if (name.isEmpty())
	{
		QMessageBox::critical(this, "Detach Selection", "Please enter a valid name.");
	}
	else QDialog::accept();
}

QString CDlgDetachSelection::getName()
{
	return ui->name->text();
}


class Ui::CDlgExtractSelection
{
public:
	QLineEdit	*name;

public:
	void setup(QWidget* w)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow("Name:", name = new QLineEdit);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		w->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), w, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), w, SLOT(reject()));
	}
};

CDlgExtractSelection::CDlgExtractSelection(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgExtractSelection)
{
	ui->setup(this);

	static int n = 1;

	QString newName = QString("Extracted%1").arg(n++);
	ui->name->setText(newName);
}

void CDlgExtractSelection::accept()
{
	QString name = getName();
	if (name.isEmpty())
	{
		QMessageBox::critical(this, "Extract Selection", "Please enter a valid name.");
	}
	else QDialog::accept();
}

QString CDlgExtractSelection::getName()
{
	return ui->name->text();
}
