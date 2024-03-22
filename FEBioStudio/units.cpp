/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include "units.h"
#include "UnitParser.h"
#include <FECore/units.h>

static int unit_system = Units::SI;

void Units::SetUnitSystem(unsigned int us) { unit_system = us; }
unsigned int Units::GetUnitSystem() { return unit_system; }

QStringList Units::SupportedUnitSystems()
{
	QStringList s;
	s << "None" << "Dimensions only" << "SI" << "mm-N-s" << "mm-kg-s" << "um-nN-s" << "CGS" << "mm-g-s";

	return s;
}

int Units::FindUnitSytemFromName(const char* sz)
{
	if (sz == nullptr) return -1;
	QStringList sl = SupportedUnitSystems();
	int n = sl.indexOf(sz); assert((n >= 0) && (n < sl.size()));
	return n;
}

static Units::UnitSymbol unit_table[][14] = {
	// dimensions
    {{"L", 1},{"M", 1},{"t", 1},{"T", 1},{"I", 1},{"n", 1},{"F", 1},{"P", 1},{"E", 1},{"W",1},{"V",1},{"c", 1},{"R", 1}},

	// SI units
    {{"m", 1}, {"kg",1},{"s",1},{"K",1},{"A",1},{"mol",1},{"N",1},{"Pa",1},{"J",1},{"W",1},{"V",1},{"mM",1}, {"℃", 1}},

	// MMTS units
    {{"mm",1e-3},{"tonne",1e3},{"s",1},{"K",1},{"A",1},{"nmol",1e-9},{"N",1},{"MPa",1e6},{"mJ",1e-3},{"mW",1e-3},{"mV",1e-3},{"mM",1}, {"℃", 1}},

	// MMKS units
    {{"mm",1e-3},{"kg",1},{"s",1},{"K",1},{"mA",1e-3},{"nmol",1e-9},{"mN",1e-3},{"kPa",1e3},{"µJ",1e-6},{"µW",1e-6}, {"mV",1e-3}, {"mM",1}, {"℃", 1}},

	// UMNNS units
    {{"µm", 1e-6},{"g",1e-3},{"s",1},{"K",1},{"pA",1e-12},{"amol",1e-18},{"nN",1e-9},{"kPa",1e3},{"fJ",1e-15},{"fW",1e-15}, {"mV",1e-3},{"mM",1}, {"℃", 1}},

	// CGS units
    {{"cm",1e-2},{"g",1e-3},{"s",1},{"K",1},{"cA",1e-2},{"µmol",1e-6},{"dyn",1e-5},{"[F/L^2]",0.1}, {"erg",1e-7},{"[E/t]",1e-7},{"mV",1e-3},{"mM", 1}, {"℃", 1}},

    // MMGS units
    {{"mm",1e-3},{"g",1e-3},{"s",1},{"K",1},{"µA",1e-6},{"nmol",1e-9},{"µN",1e-6},{"Pa",1.0}, {"nJ",1e-9},{"nW",1e-9},{"mV",1e-3},{"mM", 1}, {"℃", 1}}
};

QString Units::GetUnitString(const char* szunit)
{
	return GetUnitString(unit_system, szunit);
}

Units::UnitSymbol Units::GetUnitSymbol(int unitSystem, Units::Unit_Symbol us)
{
	if (unitSystem == Units::NONE) return Units::UnitSymbol{ "",1 };
	return unit_table[unitSystem - 1][us];
}

QString Units::GetUnitString(int unit_system, Unit_Symbol us)
{
	switch (us)
	{
	case LENGTH       : return GetUnitString(unit_system, UNIT_LENGTH       ); break;
	case MASS         : return GetUnitString(unit_system, UNIT_MASS         ); break;
	case TIME         : return GetUnitString(unit_system, UNIT_TIME         ); break;
	case TEMPERATURE  : return GetUnitString(unit_system, UNIT_TEMPERATURE  ); break;
	case CURRENT      : return GetUnitString(unit_system, UNIT_CURRENT      ); break;
	case SUBSTANCE    : return GetUnitString(unit_system, UNIT_SUBSTANCE    ); break;
	case FORCE        : return GetUnitString(unit_system, UNIT_FORCE        ); break;
	case PRESSURE     : return GetUnitString(unit_system, UNIT_PRESSURE     ); break;
	case ENERGY       : return GetUnitString(unit_system, UNIT_ENERGY       ); break;
	case POWER        : return GetUnitString(unit_system, UNIT_POWER        ); break;
	case VOLTAGE      : return GetUnitString(unit_system, UNIT_VOLTAGE      ); break;
	case CONCENTRATION: return GetUnitString(unit_system, UNIT_CONCENTRATION); break;
    case RELATIVE_TEMPERATURE: return GetUnitString(unit_system, UNIT_RELATIVE_TEMPERATURE); break;
	default:
		assert(false);
	}
	return "";
}

QString Units::GetUnitString(int unit_system, const char* szunit)
{
	if (szunit == 0) return "";
	if (szunit[0] == 0) return "";
	if (unit_system == UNIT_SYSTEM::NONE) return "";

	// convert string to UNIT
	UnitParser parser(unit_system);
	Unit unit = parser.parseUnitString(szunit);

	// convert UNIT back to string
	QString s = unit.toString();

	// add square brackets for dimensional units
	if (unit_system == Units::DIMENSIONAL) s = QString("[") + s + QString("]");

	return s;
}

double Units::Convert(double val, const char* szunit, int src, int dst)
{
	if (szunit == nullptr) return 0.0;

	UnitParser parser_src(src);
	Unit unit_src = parser_src.parseUnitString(szunit);
	double s = unit_src.TotalScaleFactor();

	UnitParser parser_dst(dst);
	Unit unit_dst = parser_dst.parseUnitString(szunit);
	double d = unit_dst.TotalScaleFactor();
	if (d == 0.0) return 0.0;

	return val * s / d;
}
