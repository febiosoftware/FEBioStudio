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

#pragma once
#include "FEMeshData_T.h"

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

class FEMathDataField : public ModelDataField
{
public:
	FEMathDataField(Post::FEPostModel* fem, unsigned int flag = 0) : ModelDataField(fem, DATA_SCALAR, DATA_NODE, CLASS_NODE, flag)
	{
		m_eq = "";
	}

	//! Create a copy
	ModelDataField* Clone() const override
	{
		FEMathDataField* pd = new FEMathDataField(m_fem);
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

class FEMathVec3DataField : public ModelDataField
{
public:
	FEMathVec3DataField(Post::FEPostModel* fem, unsigned int flag = 0) : ModelDataField(fem, DATA_VEC3, DATA_NODE, CLASS_NODE, flag)
	{
		m_eq[0] = "";
		m_eq[1] = "";
		m_eq[2] = "";
	}

	//! Create a copy
	ModelDataField* Clone() const override
	{
		FEMathVec3DataField* pd = new FEMathVec3DataField(m_fem);
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

class FEMathMat3DataField : public ModelDataField
{
public:
	FEMathMat3DataField(Post::FEPostModel* fem, unsigned int flag = 0) : ModelDataField(fem, DATA_MAT3, DATA_NODE, CLASS_NODE, flag)
	{
	}

	//! Create a copy
	ModelDataField* Clone() const override
	{
		FEMathMat3DataField* pd = new FEMathMat3DataField(m_fem);
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
