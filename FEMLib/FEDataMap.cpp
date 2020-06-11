/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
