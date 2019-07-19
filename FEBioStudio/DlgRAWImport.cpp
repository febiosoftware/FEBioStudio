#include "stdafx.h"
#include "DlgRAWImport.h"
#include <QLineEdit>
#include <QBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QValidator>

class Ui::CDlgRAWImport
{
public:
	QLineEdit*	nx;
	QLineEdit*	ny;
	QLineEdit*	nz;

	QLineEdit*	x0;
	QLineEdit*	y0;
	QLineEdit*	z0;

	QLineEdit*	w;
	QLineEdit*	h;
	QLineEdit*	d;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* lo = new QVBoxLayout;
		nx = new QLineEdit; nx->setValidator(new QIntValidator(1, 4096));
		ny = new QLineEdit; ny->setValidator(new QIntValidator(1, 4096));
		nz = new QLineEdit; nz->setValidator(new QIntValidator(1, 4096));

		x0 = new QLineEdit; x0->setValidator(new QDoubleValidator);
		y0 = new QLineEdit; y0->setValidator(new QDoubleValidator);
		z0 = new QLineEdit; z0->setValidator(new QDoubleValidator);

		w = new QLineEdit; w->setValidator(new QDoubleValidator);
		h = new QLineEdit; h->setValidator(new QDoubleValidator);
		d = new QLineEdit; d->setValidator(new QDoubleValidator);

		QFormLayout* form = new QFormLayout;
		form->addRow("nx", nx);
		form->addRow("ny", ny);
		form->addRow("nz", nz);
		form->addRow("x0", x0);
		form->addRow("y0", y0);
		form->addRow("z0", z0);
		form->addRow("width", w);
		form->addRow("heght", h);
		form->addRow("depth", d);

		lo->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		lo->addWidget(bb);

		parent->setLayout(lo);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgRAWImport::CDlgRAWImport(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgRAWImport)
{
	ui->setupUi(this);
}

void CDlgRAWImport::accept()
{
	m_nx = ui->nx->text().toInt();	
	m_ny = ui->ny->text().toInt();
	m_nz = ui->nz->text().toInt();

	m_x0 = ui->x0->text().toDouble();
	m_y0 = ui->y0->text().toDouble();
	m_z0 = ui->z0->text().toDouble();

	m_w = ui->w->text().toDouble();
	m_h = ui->h->text().toDouble();
	m_d = ui->d->text().toDouble();

	QDialog::accept();
}
