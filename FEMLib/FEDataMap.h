#pragma once
#include "FEBase.h"

class FEComponent;

//-------------------------------------------------------------------
// Class that represents a data map, which is mapped to a model parameter
class FEDataMap : public FEBase
{
public:
	FEDataMap(FEComponent* pc, const std::string& name, Param_Type type);

	FEComponent* GetParent();

	int GetGenerator() const;
	void SetGenerator(int n);

	double GetConstValue() const;
	void SetConstValue(double v);

	std::string GetMathString() const;
	void SetMathString(const std::string& s);

	void SetParamName(const std::string& s);
	std::string GetParamName();

private:
	std::string		m_param;	// name of parameter
	Param_Type		m_type;		// parameter type
	FEComponent*	m_parent;	// parent class that uses the map
};
