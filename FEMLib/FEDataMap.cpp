#include "FEDataMap.h"

FEDataMap::FEDataMap(FEComponent* pc, const std::string& name, Param_Type type)
{
	m_parent = pc;
	m_type  = type;

	SetParamName(name);

	AddChoiceParam(0, "generator", "generator")->SetEnumNames("const\0math\0mesh data\0");
	AddDoubleParam(0.0, "value", "value");
	AddStringParam("", "f(X,Y,Z) =", "math");
}

FEComponent* FEDataMap::GetParent()
{ 
	return m_parent; 
}

int FEDataMap::GetGenerator() const
{
	return GetIntValue(0);
}

void FEDataMap::SetGenerator(int n)
{
	SetIntValue(0, n);
}

double FEDataMap::GetConstValue() const
{
	return GetFloatValue(1);
}

void FEDataMap::SetConstValue(double v)
{
	SetFloatValue(1, v);
}

std::string FEDataMap::GetMathString() const
{
	return GetStringValue(2);
}

void FEDataMap::SetMathString(const std::string& s)
{
	SetStringValue(2, s);
}

void FEDataMap::SetParamName(const std::string& s)
{
	m_param = s;
}

std::string FEDataMap::GetParamName()
{
	return m_param;
}
