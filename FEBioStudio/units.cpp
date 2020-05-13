#include "stdafx.h"
#include "units.h"
#include "UnitParser.h"
#include <FSCore/paramunit.h>

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
	s << "None" << "Dimensions only" << "SI" << "mm-Newton-second" << "CGS" << "µm-nN-second";

	return s;
}

static const char* unit_table[][12] = {
	// dimensions
	{"L","M","t","T","I","n","F","P","E","W","V","c"},

	// SI units
	{"m","kg","s","K","A","mol","N","Pa","J","W","V","mM"},

	// MMTS units
	{"mm","tonne","s","K","A","nmol","N","MPa","mJ","mW","mV","mM"},

	// CGS units
	{"cm","g","s","K","cA","µmol","dyn","[F/L^2]","erg","[E/t]","mV","mM"},
    
    // UMNNS units
    {"µm","g","s","K","pA","amol","nN","kPa","fJ","fW","mV","mM"}
};

QString Units::GetUnitString(const char* szunit)
{
	return GetUnitString(unit_system, szunit);
}

const char* Units::GetUnitSymbol(int unitSystem, Units::Unit_Symbol us)
{
	if (unitSystem == Units::NONE) return "";
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
