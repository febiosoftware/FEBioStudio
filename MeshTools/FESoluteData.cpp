#include "stdafx.h"
#include "FESoluteData.h"

FESoluteData::FESoluteData()
{
	AddIntParam   (  0, "charge_number", "Charge Number");
	AddDoubleParam(0.0, "molar_mass", "Molar Mass");
	AddDoubleParam(0.0, "density", "Density");
}
