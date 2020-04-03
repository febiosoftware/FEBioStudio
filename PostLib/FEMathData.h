#pragma once
#include "FEMeshData_T.h"
#include <MathLib/MathParser.h>

namespace Post {

class FEMathDataField;
class FEMathVec3DataField;
class FEMathMat3DataField;

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

class FEMathMat3Data : public FENodeData_T<mat3f>
{
public:
	FEMathMat3Data(FEState* state, FEMathMat3DataField* pdf);

	// evaluate the nodal data for this state
	void eval(int n, mat3f* pv) override;

private:
	FEMathMat3DataField*	m_pdf;
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

class FEMathMat3DataField : public FEDataField
{
public:
	FEMathMat3DataField(const std::string& name, unsigned int flag = 0) : FEDataField(name, DATA_MAT3F, DATA_NODE, CLASS_NODE, flag)
	{
	}

	//! Create a copy
	FEDataField* Clone() const override
	{
		FEMathMat3DataField* pd = new FEMathMat3DataField(GetName());
		for (int i = 0; i < 9; ++i) pd->m_eq[i] = m_eq[i];
		return pd;
	}

	//! FEMeshData constructor
	FEMeshData* CreateData(FEState* pstate) override
	{
		return new FEMathMat3Data(pstate, this);
	}

	void SetEquationStrings(
		const std::string& m00, const std::string& m01, const std::string& m02,
		const std::string& m10, const std::string& m11, const std::string& m12,
		const std::string& m20, const std::string& m21, const std::string& m22)
	{
		m_eq[0] = m00; m_eq[1] = m01; m_eq[2] = m02;
		m_eq[3] = m10; m_eq[4] = m11; m_eq[5] = m12;
		m_eq[6] = m20; m_eq[7] = m21; m_eq[8] = m22;
	}

	void SetEquationString(int n, const std::string& eq) { m_eq[n] = eq; }

	const std::string& EquationString(int n) const { return m_eq[n]; }

private:
	std::string	m_eq[9];		//!< equation string
};
}
