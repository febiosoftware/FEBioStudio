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
