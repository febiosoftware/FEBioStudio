#pragma once
#include <FSCore/Object.h>
#include "MathLib/math3d.h"
#include "MathParser.h"
#include <string>
using namespace std;

//-----------------------------------------------------------------------------
// This class defines a data variable over the volume of a domain.
class FEDataVariable : public CObject
{
public:
	FEDataVariable(void);
	virtual ~FEDataVariable(void) {}

	void SetString(int n, const char* sz);
	const char* GetString(int n) { return m_v[n].c_str(); }

	//! return the value at point r
	virtual vec3d Value(vec3d& r);

	int GetID() { return m_nID; }
 
private:
	FEDataVariable(const FEDataVariable& v) {}

protected:
	string		m_v[3];
	int			m_nID;
	CMathParser	m_mth;
};
