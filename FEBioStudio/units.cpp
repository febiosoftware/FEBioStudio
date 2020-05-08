#include "stdafx.h"
#include "units.h"

#ifdef _DEBUG
static int unit_system = Units::SI;
#else
static int unit_system = Units::NONE;
#endif

void Units::SetUnitSystem(unsigned int us) { unit_system = us; }
unsigned int Units::GetUnitSystem() { return unit_system; }

QStringList Units::SupportedUnitSystems()
{
	QStringList s;
	s << "None" << "Dimensions only" << "SI" << "CGS";
	return s;
}

enum Unit_Symbol
{
	LENGTH,
	MASS,
	TIME,
	TEMPERATURE,
	FORCE,
	PRESSURE,
	ENERGY,
	POWER,
	CONCENTRATION
};

// define unit symbols in the order: length, mass, time, temperature, force, pressure, energy, power, concentration
static const char* unit_table[][9] = {
	// dimensions
	{ "L", "M", "t", "T", "F", "P", "J", "W", "C"},

	// SI units
	{ "m", "kg", "s", "K", "N", "Pa", "J", "W", "mol"},

	// CGS units
	{ "cm", "g", "s", "K", "dyn", "Ba", "erg", "erg/s", "mol" },
};

// turn this off to use non-unicode representation of symbols
#define USE_UNICODE

#ifdef USE_UNICODE
#define POW_2 QString(QChar(0x00B2))
#define POW_3 QString(QChar(0x00B3))
#define POW_4 QString(QChar(0x2074))
#define DEG QString(QChar(0x00B0))
#else
#define POW_2 QString("^2")
#define POW_3 QString("^3")
#define POW_4 QString("^4")
#define DEG QString("deg")
#endif

#define L QString(sz[LENGTH])
#define M QString(sz[MASS])
#define t QString(sz[TIME])
#define T QString(sz[TEMPERATURE])
#define F QString(sz[FORCE])
#define P QString(sz[PRESSURE])
#define J QString(sz[ENERGY])
#define W QString(sz[POWER])
#define C QString(sz[CONCENTRATION])
#define RAD QString("rad")

QString Units::GetUnitString(const char* szunit)
{
	if (szunit == 0) return "";
	if (unit_system == UNIT_SYSTEM::NONE) return "";

	const char* (*sz) = unit_table[unit_system - 1];

	// parse unit string
	QString s;
	const char* c = szunit;
	while (*c)
	{
		switch (*c)
		{
		case 'L': s += L; break;
		case 'M': s += M; break;
		case 't': s += t; break;
		case 'T': s += T; break;
		case 'F': s += F; break;
		case 'P': s += P; break;
		case 'J': s += J; break;
		case 'W': s += W; break;
		case 'C': s += C; break;
		case 'd': s += DEG; break;
		case 'r': s += RAD; break;
		case '/': s += '/'; break;
		case '.': s += '.'; break;
		case '^':
		{
			c++;
			if (*c == 0) { assert(false); return "?"; }
			int n = *c - '0';
			switch (n)
			{
			case 2: s += POW_2; break;
			case 3: s += POW_3; break;
			case 4: s += POW_4; break;
			default: 
				assert(false);
			}
		}
		break;
		default:
			assert(false);
		}

		c++;
	}

	if (unit_system == Units::DIMENSIONAL) s = QString("[") + s + QString("]");

	return s;
}
