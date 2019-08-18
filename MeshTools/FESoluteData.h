#pragma once
#include <FSCore/FSObject.h>

// This class is used for both solutes and sbms. 
// This was done to simplify the dialog box for editing solutes/sbms.
// Perhaps a flag cam be added to identify this molecule as a solute or sbm but I have not found a need for this.
class FESoluteData : public FSObject
{
	enum { Z, MM, DENS};

public:
	FESoluteData();

	int GetChargeNumber() const { return GetIntValue(Z); }
	void SetChargeNumber(int n) { SetIntValue(Z, n); }

	double GetMolarMass() const { return GetFloatValue(MM); }
	void SetMolarMass(double m) { SetFloatValue(MM, m); }

	double GetDensity() const { return GetFloatValue(DENS); }
	void SetDensity(double m) { SetFloatValue(DENS, m); }
};
