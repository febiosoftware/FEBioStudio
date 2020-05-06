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
	};

	QStringList SupportedUnitSystems();

	void SetUnitSystem(unsigned int us);
	unsigned int GetUnitSystem();

	QString GetUnitString(const char* sz);
}
