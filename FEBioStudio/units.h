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
		CGS
	};

	QStringList SupportedUnitSystems();

	void SetUnitSystem(unsigned int us);
	unsigned int GetUnitSystem();

	QString GetUnitString(const char* sz);
}
