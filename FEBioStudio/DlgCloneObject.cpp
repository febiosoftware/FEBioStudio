#include "stdafx.h"
#include "DlgCloneObject.h"
#include <QFormLayout>
#include <QBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>

class Ui::CDlgCloneObject
{
public:
	QString	_name;
	QLineEdit* name;

	static int count;

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

int Ui::CDlgCloneObject::count = 1;

CDlgCloneObject::CDlgCloneObject(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgCloneObject)
{
	ui->setup(this);

	ui->name->setText(QString("Clone%1").arg(ui->count));
}

void CDlgCloneObject::accept()
{
	ui->_name = ui->name->text();
	if (ui->_name.isEmpty())
	{
		QMessageBox::critical(this, "Clone Object", "Invalid name for cloned object");
	}
	else 
	{
		ui->count++;
		QDialog::accept();
	}
}

QString CDlgCloneObject::GetNewObjectName()
{
	return ui->_name;
}
