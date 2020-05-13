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

	QString unitSymbol(int us , Unit_Symbol sym);
}
