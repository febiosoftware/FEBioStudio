#include "stdafx.h"
#include "DlgUnitConverter.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QLineEdit>
#include <vector>
using namespace std;

enum Quantities {
	LENGTH,
	TEMPERATURE
};

enum LENGTH_UNITS {
	METER,
	CENTIMETER,
	MILLIMETER,
	INCH
};

enum TEMPERATURE_UNITS {
	CELSIUS,
	FAHRENHEIT,
	KELVIN
};

typedef double (*CONVERT_FUNC)(int from, int to, double fromVal);

static vector<CONVERT_FUNC> convert_table;

double convert_length(int nfrom, int nto, double v);
double convert_temperature(int nfrom, int nto, double v);

class Ui::CDlgUnitConverter
{
public:
	QComboBox*	quantity;
	QComboBox*	from;
	QComboBox*	to;

	QLineEdit*	fromVal;
	QLineEdit*	toVal;

public:
	void setup(QDialog* dlg)
	{
		QHBoxLayout* hfrom = new QHBoxLayout;
		hfrom->setMargin(0);
		hfrom->addWidget(fromVal = new QLineEdit);
		hfrom->addWidget(from = new QComboBox);

		from->addItem("a very long item");

		fromVal->setValidator(new QDoubleValidator);
		fromVal->setText(QString::number(0.0));

		QHBoxLayout* hto = new QHBoxLayout;
		hto->setMargin(0);
		hto->addWidget(toVal = new QLineEdit);
		hto->addWidget(to = new QComboBox);
		toVal->setReadOnly(true);

		to->addItem("a very long item");

		QFormLayout* fl = new QFormLayout;
		fl->addRow("Quantity:", quantity = new QComboBox);
		fl->addRow("From:", hfrom);
		fl->addRow("To:", hto);

		quantity->addItem("Length");
		quantity->addItem("Temperature");

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(fl);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(accept()));
		QObject::connect(quantity, SIGNAL(currentIndexChanged(int)), dlg, SLOT(on_quantity_changed()));
		QObject::connect(from, SIGNAL(currentIndexChanged(int)), dlg, SLOT(on_from_changed()));
		QObject::connect(to, SIGNAL(currentIndexChanged(int)), dlg, SLOT(on_to_changed()));
		QObject::connect(fromVal, SIGNAL(editingFinished()), dlg, SLOT(on_value_changed()));
	}
};

CDlgUnitConverter::CDlgUnitConverter(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgUnitConverter)
{
	convert_table.push_back(convert_length);
	convert_table.push_back(convert_temperature);

	setWindowTitle("Unit Converter");
	ui->setup(this);
	ui->quantity->setCurrentIndex(0);
	on_quantity_changed();
}

void CDlgUnitConverter::on_quantity_changed()
{
	int n = ui->quantity->currentIndex();

	QStringList units;

	switch (n)
	{
	case LENGTH: 
		units << "meter" << "cm" << "mm" << "inch";
		break;
	case TEMPERATURE:
		units << "Celsius" << "Fahrenheit" << "Kelvin";
		break;
	}

	ui->from->blockSignals(true);
	ui->from->clear();
	ui->from->addItems(units);
	ui->from->setCurrentIndex(0);
	ui->from->blockSignals(false);

	ui->to->blockSignals(true);
	ui->to->clear();
	ui->to->addItems(units);
	ui->to->setCurrentIndex(1);
	ui->to->blockSignals(false);

	ui->fromVal->setText(QString::number(0.0));

	update();
}

void CDlgUnitConverter::on_from_changed() 
{
	update();
}

void CDlgUnitConverter::on_to_changed()
{
	update();
}

void CDlgUnitConverter::on_value_changed()
{
	update();
}

void CDlgUnitConverter::update()
{
	int n = ui->quantity->currentIndex();

	CONVERT_FUNC f = convert_table[n];

	double fromVal = ui->fromVal->text().toDouble();

	int nfrom = ui->from->currentIndex();
	int nto = ui->to->currentIndex();

	double toVal = f(nfrom, nto, fromVal);

	ui->toVal->setText(QString::number(toVal));
}

double convert_length(int nfrom, int nto, double v)
{
	// convert to meter
	double m = 0.0;
	switch (nfrom)
	{
	case LENGTH_UNITS::METER     : m = v; break;
	case LENGTH_UNITS::CENTIMETER: m = v * 0.01; break;
	case LENGTH_UNITS::MILLIMETER: m = v * 0.001; break;
	case LENGTH_UNITS::INCH      : m = v * 0.0254; break;
	default:
		assert(false);
	}

	// convert meter to target unit
	double to = 0.0;
	switch (nto)
	{
	case LENGTH_UNITS::METER     : to = m; break;
	case LENGTH_UNITS::CENTIMETER: to = m / 0.01; break;
	case LENGTH_UNITS::MILLIMETER: to = m / 0.001; break;
	case LENGTH_UNITS::INCH      : to = m / 0.0254; break;
	default:
		assert(false);
	}

	return to;
}


double convert_temperature(int nfrom, int nto, double v)
{
	// convert to Celsius
	double C = 0.0;
	switch (nfrom)
	{
	case TEMPERATURE_UNITS::CELSIUS   : C = v; break;
	case TEMPERATURE_UNITS::FAHRENHEIT: C = (v - 32.0)/1.8; break;
	case TEMPERATURE_UNITS::KELVIN    : C = v - 273.15; break;
	default:
		assert(false);
	}

	// convert celsius to target unit
	double to = 0.0;
	switch (nto)
	{
	case TEMPERATURE_UNITS::CELSIUS   : to = C; break;
	case TEMPERATURE_UNITS::FAHRENHEIT: to = 1.8*C + 32.0; break;
	case TEMPERATURE_UNITS::KELVIN    : to = C + 273.15; break;
	default:
		assert(false);
	}

	return to;
}
