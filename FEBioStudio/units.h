#pragma once
#include <QString>
#include <QStringList>
#include <FSCore/ParamBlock.h>

namespace Units {

	enum UNIT_SYSTEM
	{
		NONE,
		DIMENSIONAL,
		SI,
		MMTS,
		CGS,
        UMNNS,
	};

	enum Unit_Symbol
	{
		LENGTH,
		MASS,
		TIME,
		TEMPERATURE,
		CURRENT,
		SUBSTANCE,
		FORCE,
		PRESSURE,
		ENERGY,
		POWER,
        VOLTAGE,
        CONCENTRATION
	};

	QStringList SupportedUnitSystems();

	void SetUnitSystem(unsigned int us);
	unsigned int GetUnitSystem();

	QString GetUnitString(const char* sz);
	QString GetUnitString(int unit_system, const char* sz);
	QString GetUnitString(int unit_system, Unit_Symbol us);

	const char* GetUnitSymbol(int unit_system, Unit_Symbol us);
}
