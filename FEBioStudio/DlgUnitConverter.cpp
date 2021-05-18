/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "DlgUnitConverter.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QLineEdit>
#include <vector>
//using namespace std;

using std::vector;

enum Quantities {
	ANGLE,
	ENERGY,
	FORCE,
	LENGTH,
	MASS,
	PRESSURE,
	TEMPERATURE,
	TIME
};

enum ANGLE_UNITS {
	DEGREE,
	RADIAN
};

enum ENERGY_UNITS {
	JOULE,
	ERG
};

enum FORCE_UNITS {
	NEWTON,
	KILONEWTON,
	DYNE
};

enum LENGTH_UNITS {
	METER,
	CENTIMETER,
	MILLIMETER,
	INCH,
	FOOT
};

enum MASS_UNITS {
	KILOGRAM,
	POUND,
	STONE
};

enum PRESSURE_UNITS {
	PASCAL,
	KILOPASCAL,
	MEGAPASCAL,
	ATMOSPHERE,
	MMHG,
	PSI,
	BARYE
};

enum TEMPERATURE_UNITS {
	CELSIUS,
	FAHRENHEIT,
	KELVIN
};

enum TIME_UNITS {
	SECOND,
	MINUTE,
	HOUR,
	DAY
};

typedef double (*CONVERT_FUNC)(int from, int to, double fromVal);

static vector<CONVERT_FUNC> convert_table;

double convert_angle(int nfrom, int nto, double v);
double convert_energy(int nfrom, int nto, double v);
double convert_force(int nfrom, int nto, double v);
double convert_length(int nfrom, int nto, double v);
double convert_mass(int nfrom, int nto, double v);
double convert_pressure(int nfrom, int nto, double v);
double convert_temperature(int nfrom, int nto, double v);
double convert_time(int nfrom, int nto, double v);

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
		hfrom->setContentsMargins(0,0,0,0);
		hfrom->addWidget(fromVal = new QLineEdit);
		hfrom->addWidget(from = new QComboBox);

		fromVal->setValidator(new QDoubleValidator);
		fromVal->setText(QString::number(0.0));

		QHBoxLayout* hto = new QHBoxLayout;
		hto->setContentsMargins(0,0,0,0);
		hto->addWidget(toVal = new QLineEdit);
		hto->addWidget(to = new QComboBox);
		toVal->setReadOnly(true);

		QFormLayout* fl = new QFormLayout;
		fl->setLabelAlignment(Qt::AlignRight);
		fl->addRow("Quantity:", quantity = new QComboBox);
		fl->addRow("From:", hfrom);
		fl->addRow("To:", hto);

		// NOTE: Make sure the order here is the same as in enum Quantities
		//       (should be alphabetical)
		quantity->addItem("Angle");
		quantity->addItem("Energy");
		quantity->addItem("Force");
		quantity->addItem("Length");
		quantity->addItem("Mass");
		quantity->addItem("Pressure");
		quantity->addItem("Temperature");
		quantity->addItem("Time");

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
	convert_table.push_back(convert_angle);
	convert_table.push_back(convert_energy);
	convert_table.push_back(convert_force);
	convert_table.push_back(convert_length);
	convert_table.push_back(convert_mass);
	convert_table.push_back(convert_pressure);
	convert_table.push_back(convert_temperature);
	convert_table.push_back(convert_time);

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
	case ANGLE:
		// NOTE: the extra spaces is to make sure
		//       that the combo box is large enough to fit the other units.
		units << "Degree         " << "Radian";
		break;
	case ENERGY:
		units << "Joule" << "Erg";
		break;
	case FORCE:
		units << "Newton" << "Kilonewton" << "Dyne";
		break;
	case LENGTH:
		units << "Meter" << "Centimeter" << "Millimeter" << "Inch" << "Foot";
		break;
	case MASS:
		units << "Kilogram" << "Pound" << "Stone";
		break;
	case PRESSURE:
		units << "Pascal" << "Kilopascal" << "Megapascal" << "Atmosphere" << "mmHg" << "PSI" << "Barye";
		break;
	case TEMPERATURE:
		units << "Celsius" << "Fahrenheit" << "Kelvin";
		break;
	case TIME:
		units << "Second" << "Minute" << "Hour" << "Day";
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

	if (n == TEMPERATURE)
		ui->fromVal->setText(QString::number(0.0));
	else 
		ui->fromVal->setText(QString::number(1.0));

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


double convert_angle(int nfrom, int nto, double v)
{
	// convert to degree
	double d = 0.0;
	switch (nfrom)
	{
	case ANGLE_UNITS::DEGREE : d = v; break;
	case ANGLE_UNITS::RADIAN : d = v * 57.2957795; break;
	default:
		assert(false);
	}

	// convert to target unit
	double to = 0.0;
	switch (nto)
	{
	case ANGLE_UNITS::DEGREE: to = d; break;
	case ANGLE_UNITS::RADIAN: to = d / 57.2957795; break;
	default:
		assert(false);
	}

	return to;
}


double convert_energy(int nfrom, int nto, double v)
{
	// convert to joule
	double J = 0.0;
	switch (nfrom)
	{
	case ENERGY_UNITS::JOULE: J = v; break;
	case ENERGY_UNITS::ERG  : J = v * 1.0e-7; break;
	default:
		assert(false);
	}

	// convert to target unit
	double to = 0.0;
	switch (nto)
	{
	case ENERGY_UNITS::JOULE: to = J; break;
	case ENERGY_UNITS::ERG  : to = J / 1.0e-7; break;
	default:
		assert(false);
	}

	return to;
}


double convert_force(int nfrom, int nto, double v)
{
	// convert to Newton
	double N = 0.0;
	switch (nfrom)
	{
	case FORCE_UNITS::NEWTON    : N = v; break;
	case FORCE_UNITS::KILONEWTON: N = v * 1000.0; break;
	case FORCE_UNITS::DYNE      : N = v * 1e-5; break;
	default:
		assert(false);
	}

	// convert to target unit
	double to = 0.0;
	switch (nto)
	{
	case FORCE_UNITS::NEWTON    : to = N; break;
	case FORCE_UNITS::KILONEWTON: to = N / 1000.0; break;
	case FORCE_UNITS::DYNE      : to = N / 1e-5; break;
	default:
		assert(false);
	}

	return to;
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
	case LENGTH_UNITS::FOOT      : m = v * 0.3048; break;
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
	case LENGTH_UNITS::FOOT      : to = v / 0.3048; break;
	default:
		assert(false);
	}

	return to;
}


double convert_mass(int nfrom, int nto, double v)
{
	// convert to kilogram
	double kg = 0.0;
	switch (nfrom)
	{
	case MASS_UNITS::KILOGRAM: kg = v; break;
	case MASS_UNITS::POUND   : kg = v * 0.45359237; break;
	case MASS_UNITS::STONE   : kg = v * 6.35029317; break;
	default:
		assert(false);
	}

	// convert kg to target unit
	double to = 0.0;
	switch (nto)
	{
	case MASS_UNITS::KILOGRAM: to = kg; break;
	case MASS_UNITS::POUND   : to = kg / 0.45359237; break;
	case MASS_UNITS::STONE   : to = kg / 6.35029317; break;
	default:
		assert(false);
	}

	return to;
}

double convert_pressure(int nfrom, int nto, double v)
{
	// convert to Pascal
	double P = 0.0;
	switch (nfrom)
	{
	case PRESSURE_UNITS::PASCAL    : P = v; break;
	case PRESSURE_UNITS::KILOPASCAL: P = v * 1.0e3; break;
	case PRESSURE_UNITS::MEGAPASCAL: P = v * 1.0e6; break;
	case PRESSURE_UNITS::ATMOSPHERE: P = v * 101325.0; break;
	case PRESSURE_UNITS::MMHG      : P = v * 133.322368; break;
	case PRESSURE_UNITS::PSI       : P = v * 6894.757; break;
	case PRESSURE_UNITS::BARYE     : P = v * 0.1; break;
	default: assert(false);
	}

	// convert Pa to target unit
	double to = 0.0;
	switch (nto)
	{
	case PRESSURE_UNITS::PASCAL    : to = P; break;
	case PRESSURE_UNITS::KILOPASCAL: to = P / 1.0e3; break;
	case PRESSURE_UNITS::MEGAPASCAL: to = P / 1.0e6; break;
	case PRESSURE_UNITS::ATMOSPHERE: to = P / 101325.0; break;
	case PRESSURE_UNITS::MMHG      : to = P / 133.322368; break;
	case PRESSURE_UNITS::PSI       : to = P / 6894.757; break;
	case PRESSURE_UNITS::BARYE     : to = P / 0.1; break;
	default: assert(false);
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

double convert_time(int nfrom, int nto, double v)
{
	// convert to sec
	double s = 0.0;
	switch (nfrom)
	{
	case TIME_UNITS::SECOND: s = v; break;
	case TIME_UNITS::MINUTE: s = v * 60.0; break;
	case TIME_UNITS::HOUR  : s = v * 3600.0; break;
	case TIME_UNITS::DAY   : s = v * 86400.0; break;
	default: assert(false);
	}

	// convert second to target unit
	double to = 0.0;
	switch (nto)
	{
	case TIME_UNITS::SECOND: to = s; break;
	case TIME_UNITS::MINUTE: to = s / 60.0; break;
	case TIME_UNITS::HOUR  : to = s / 3600.0; break;
	case TIME_UNITS::DAY   : to = s / 86400.0; break;
	default: assert(false);
	}
	return to;
}
