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

#include "FEDataVariable.h"

//-----------------------------------------------------------------------------
FEDataVariable::FEDataVariable()
{
	static int n = 1;
	m_nID = n++;
	m_v[0] = "";
	m_v[1] = "";
	m_v[2] = "";

	m_mth[0].AddVariable("x"); m_mth[0].AddVariable("y"); m_mth[0].AddVariable("z");
	m_mth[1].AddVariable("x"); m_mth[1].AddVariable("y"); m_mth[1].AddVariable("z");
	m_mth[2].AddVariable("x"); m_mth[2].AddVariable("y"); m_mth[2].AddVariable("z");
}

FEDataVariable::FEDataVariable(const FEDataVariable& v) 
{ 
	m_nID = -1; 
}

//-----------------------------------------------------------------------------
void FEDataVariable::SetString(int n, const char* sz)
{
	m_v[n] = std::string(sz);
}

//-----------------------------------------------------------------------------
vec3d FEDataVariable::Value(vec3d &r)
{
	m_mth[0].Clear(); m_mth[0].Create(m_v[0]);
	m_mth[1].Clear(); m_mth[1].Create(m_v[1]);
	m_mth[2].Clear(); m_mth[2].Create(m_v[2]);

	std::vector<double> xyz = { r.x, r.y, r.z };

	vec3d v;
	v.x = m_mth[0].value_s(xyz);
	v.y = m_mth[1].value_s(xyz);
	v.z = m_mth[2].value_s(xyz);
	return v;
}
