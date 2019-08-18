#pragma once
#include "FEMeshData_T.h"
#include "MathParser.h"

namespace Post {

class FEMathDataField;
class FEMathVec3DataField;

class FEMathData : public FENodeData_T<float>
{
public:
	FEMathData(FEState* state, FEMathDataField* pdf);

	// evaluate the nodal data for this state
	void eval(int n, float* pv) override;

private:
	FEMathDataField*	m_pdf;
};

class FEMathVec3Data : public FENodeData_T<vec3f>
{
public:
	FEMathVec3Data(FEState* state, FEMathVec3DataField* pdf);

	// evaluate the nodal data for this state
	void eval(int n, vec3f* pv) override;

private:
	FEMathVec3DataField*	m_pdf;
};

class FEMathDataField : public FEDataField
{
public:
	FEMathDataField(const std::string& name, unsigned int flag = 0) : FEDataField(name, DATA_FLOAT, DATA_NODE, CLASS_NODE, flag)
	{
		m_eq = "";
	}

	//! Create a copy
	FEDataField* Clone() const override
	{
		FEMathDataField* pd = new FEMathDataField(GetName());
		pd->m_eq = m_eq;
		return pd;
	}

	//! FEMeshData constructor
	FEMeshData* CreateData(FEState* pstate) override
	{
		return new FEMathData(pstate, this);
	}

	void SetEquationString(const std::string& eq) { m_eq = eq; }

	const std::string& EquationString() const { return m_eq; }

private:
	std::string	m_eq;		//!< equation string
};

class FEMathVec3DataField : public FEDataField
{
public:
	FEMathVec3DataField(const std::string& name, unsigned int flag = 0) : FEDataField(name, DATA_VEC3F, DATA_NODE, CLASS_NODE, flag)
	{
		m_eq[0] = "";
		m_eq[1] = "";
		m_eq[2] = "";
	}

	//! Create a copy
	FEDataField* Clone() const override
	{
		FEMathVec3DataField* pd = new FEMathVec3DataField(GetName());
		pd->m_eq[0] = m_eq[0];
		pd->m_eq[1] = m_eq[1];
		pd->m_eq[2] = m_eq[2];
		return pd;
	}

	//! FEMeshData constructor
	FEMeshData* CreateData(FEState* pstate) override
	{
		return new FEMathVec3Data(pstate, this);
	}

	void SetEquationStrings(const std::string& x, const std::string& y, const std::string& z)
	{
		m_eq[0] = x; 
		m_eq[1] = y;
		m_eq[2] = z;
	}

	void SetEquationString(int n, const std::string& eq) { m_eq[n] = eq; }

	const std::string& EquationString(int n) const { return m_eq[n]; }

private:
	std::string	m_eq[3];		//!< equation string
};
}
