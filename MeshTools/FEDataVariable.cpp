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

#include "FEDataVariable.h"

//-----------------------------------------------------------------------------
FEDataVariable::FEDataVariable()
{
	static int n = 1;
	m_nID = n++;
	m_v[0] = "";
	m_v[1] = "";
	m_v[2] = "";

	m_mth.set_variable("x", 0);
	m_mth.set_variable("y", 0);
	m_mth.set_variable("z", 0);
}

//-----------------------------------------------------------------------------
void FEDataVariable::SetString(int n, const char* sz)
{
	m_v[n] = string(sz);
}

//-----------------------------------------------------------------------------
vec3d FEDataVariable::Value(vec3d &r)
{
	m_mth.set_variable("x", r.x);
	m_mth.set_variable("y", r.y);
	m_mth.set_variable("z", r.z);

	vec3d v(0,0,0);
	int ierr;
	v.x = m_mth.eval(m_v[0].c_str(), ierr);
	v.y = m_mth.eval(m_v[1].c_str(), ierr);
	v.z = m_mth.eval(m_v[2].c_str(), ierr);
	return v;
}
