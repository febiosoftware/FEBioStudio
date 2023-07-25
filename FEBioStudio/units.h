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

#pragma once
#include <QString>
#include <QStringList>
#include <FSCore/ParamBlock.h>
#include <FECore/units.h>

namespace Units {

	struct UnitSymbol
	{
		const char* s;	// symbol string
		double		f;	// SI scale factor
	};

	enum UNIT_SYSTEM
	{
		NONE,
		DIMENSIONAL,
		SI,
		MMTS,
        MMKS,
        UMNNS,
        CGS,
        MMGS,
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
        CONCENTRATION,
        RELATIVE_TEMPERATURE
	};

	QStringList SupportedUnitSystems();

	void SetUnitSystem(unsigned int us);
	unsigned int GetUnitSystem();

	int FindUnitSytemFromName(const char* sz);

	QString GetUnitString(const char* sz);
	QString GetUnitString(int unit_system, const char* sz);
	QString GetUnitString(int unit_system, Unit_Symbol us);

	UnitSymbol GetUnitSymbol(int unit_system, Unit_Symbol us);

	// convert a value 
	double Convert(double val, const char* szunit, int src, int dst);
}
