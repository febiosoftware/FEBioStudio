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

#include "stdafx.h"
#include "FEMathData.h"
#include "FEPostModel.h"
#include "constants.h"
using namespace Post;

FEScalarMathDataField::FEScalarMathDataField(Post::FEPostModel* fem, DATA_CLASS dataClass, unsigned int flag) : ModelDataField(fem, DATA_SCALAR, DATA_ITEM, dataClass, flag)
{

}

double FEScalarMathDataField::value(const std::vector<double>& vars)
{
	if (!m_math.IsValid()) return 0.0;
	double v = m_math.value_s(vars);
	return v;
}

FEMathNodeDataField::FEMathNodeDataField(Post::FEPostModel* fem, unsigned int flag) : FEScalarMathDataField(fem, NODE_DATA, flag)
{
	m_eq = "";
	m_math.AddVariable("x", 0.0);
	m_math.AddVariable("y", 0.0);
	m_math.AddVariable("z", 0.0);
	m_math.AddVariable("t", 0.0);
	BuildMath(true);
}

FEMeshData* FEMathNodeDataField::CreateData(FEState* pstate)
{
	FEMathNodeData* data = new FEMathNodeData(pstate, this);

	FEPostModel* fem = m_fem;
	for (int i = 0; i < m_var.size(); ++i)
	{
		int nfield = m_var[i].second->GetFieldID();
		int n = FIELD_CODE(nfield);
		FEMeshData& d = pstate->m_Data[n];

		FENodeData_T<float>* ped = dynamic_cast<FENodeData_T<float>*>(&d);
		assert(ped);
		data->AddVariable(ped);
	}

	return data;
}

bool FEMathNodeDataField::BuildMath(bool updateVars)
{
	string expr = m_eq;
	if (expr.empty()) expr = "0";

	if (updateVars)
	{
		m_var.clear();
		m_math.Clear();
		m_math.AddVariable("x", 0.0);
		m_math.AddVariable("y", 0.0);
		m_math.AddVariable("z", 0.0);
		m_math.AddVariable("t", 0.0);
		bool b = m_math.Create(expr, true); assert(b);

		FEPostModel* fem = m_fem;
		Post::FEDataManager* pDM = fem->GetDataManager();

		int nvar = m_math.Variables();
		for (int i = 4; i < nvar; ++i)
		{
			MVariable* pv = m_math.Variable(i);
			const std::string& varName = pv->Name();
			int nfield = pDM->FindDataField(varName); assert(nfield >= 0);
			ModelDataField* pdf = *pDM->DataField(nfield);
			if (pdf->DataClass() != NODE_DATA) return false;
			if (pdf->Format() != DATA_ITEM) return false;
			m_var.push_back(std::make_pair(varName, pdf));
		}
	}
	else
	{
		bool b = m_math.Create(expr, false); assert(b);
	}

	return true;
}

FEMathNodeData::FEMathNodeData(FEState* state, FEMathNodeDataField* pdf) : FENodeData_T<float>(state, pdf)
{
	m_pdf = pdf;
}

// evaluate all the nodal data for this state
void FEMathNodeData::eval(int n, float* pv)
{
	if ((m_pdf == nullptr) || (pv == nullptr)) return;

	double time = m_state->m_time;
	FEPostModel& fem = *GetFSModel();
	vec3f r = fem.NodePosition(n, m_state->GetID());
	std::vector<double> vars = { r.x, r.y, r.z, time };

	for (int i = 0; i < m_vars.size(); ++i)
	{
		Post::FENodeData_T<float>* vari = m_vars[i];
		float vi = 0.f;
		if (vari && vari->active(n)) vari->eval(n, &vi);
		vars.push_back(vi);
	}

	double v = m_pdf->value(vars);
	*pv = (float)v;
}

FEMathElemDataField::FEMathElemDataField(Post::FEPostModel* fem, unsigned int flag) : FEScalarMathDataField(fem, ELEM_DATA, flag)
{
	m_eq = "";
	BuildMath(false);
}

bool FEMathElemDataField::BuildMath(bool updateVars)
{
	string expr = m_eq;
	if (expr.empty()) expr = "0";

	if (updateVars)
	{
		m_var.clear();
		m_math.Clear();
		m_math.AddVariable("x", 0.0);
		m_math.AddVariable("y", 0.0);
		m_math.AddVariable("z", 0.0);
		m_math.AddVariable("t", 0.0);
		bool b = m_math.Create(expr, true); assert(b);

		FEPostModel* fem = m_fem;
		Post::FEDataManager* pDM = fem->GetDataManager();

		int nvar = m_math.Variables();
		for (int i = 4; i < nvar; ++i)
		{
			MVariable* pv = m_math.Variable(i);
			const std::string& varName = pv->Name();
			int nfield = pDM->FindDataField(varName); assert(nfield >= 0);
			ModelDataField* pdf = *pDM->DataField(nfield);
			if (pdf->DataClass() != ELEM_DATA) return false;
			if (pdf->Format() != DATA_ITEM) return false;
			m_var.push_back(std::make_pair(varName, pdf));
		}
	}
	else
	{
		bool b = m_math.Create(expr, false); assert(b);
	}

	return true;
}

FEMeshData* FEMathElemDataField::CreateData(FEState* pstate)
{
	FEMathElemData* data = new FEMathElemData(pstate, this);

	FEPostModel* fem = m_fem;
	for (int i = 0; i < m_var.size(); ++i)
	{
		int nfield = m_var[i].second->GetFieldID();
		int n = FIELD_CODE(nfield);
		FEMeshData& d = pstate->m_Data[n];

		FEElemData_T<float, DATA_ITEM>* ped = dynamic_cast<FEElemData_T<float, DATA_ITEM>*>(&d);
		assert(ped);
		data->AddVariable(ped);
	}

	return data;
}

FEMathElemData::FEMathElemData(FEState* state, FEMathElemDataField* pdf) : FEElemData_T<float, DATA_ITEM>(state, pdf)
{
	m_pdf = pdf;
}

void FEMathElemData::eval(int n, float* pv)
{
	if ((m_pdf == nullptr) || (pv == nullptr)) return;

	double time = m_state->m_time;
	FEPostModel& fem = *GetFSModel();
	vec3f r[FSElement::MAX_NODES];
	int ne = fem.GetElementCoords(n, m_state->m_id, r);
	vec3f c(0, 0, 0);
	for (int i = 0; i < ne; ++i) c += r[i];
	c /= ne;

	std::vector<double> vars = { c.x, c.y, c.z, time };

	for (int i = 0; i < m_vars.size(); ++i)
	{
		Post::FEElemData_T<float, DATA_ITEM>* vari = m_vars[i];
		float vi = 0.f;
		if (vari && vari->active(n)) vari->eval(n, &vi);
		vars.push_back(vi);
	}

	double v = m_pdf->value(vars);
	*pv = (float)v;
}

FEMathVec3DataField::FEMathVec3DataField(Post::FEPostModel* fem, unsigned int flag) : ModelDataField(fem, DATA_VEC3, DATA_NODE, NODE_DATA, flag)
{
	for (int i = 0; i < 3; ++i)
	{
		m_eq[i] = "";
		m_math[i].AddVariable("x");
		m_math[i].AddVariable("y");
		m_math[i].AddVariable("z");
		m_math[i].AddVariable("t");
	}
	BuildMath();
}

void FEMathVec3DataField::BuildMath()
{
	for (int i = 0; i < 3; ++i)
	{
		m_math[i].Create(m_eq[i]);
	}
}

vec3d FEMathVec3DataField::value(double x, double y, double z, double t)
{
	vec3d v;
	std::vector<double> vars = { x, y, z, t };
	if (m_math[0].IsValid()) v.x = m_math[0].value_s(vars);
	if (m_math[1].IsValid()) v.y = m_math[1].value_s(vars);
	if (m_math[2].IsValid()) v.z = m_math[2].value_s(vars);
	return v;
}

FEMathVec3Data::FEMathVec3Data(FEState* state, FEMathVec3DataField* pdf) : FENodeData_T<vec3f>(state, pdf)
{
	m_pdf = pdf;
}

// evaluate all the nodal data for this state
void FEMathVec3Data::eval(int n, vec3f* pv)
{
	if (pv == nullptr) return;

	FEState& state = *m_state;
	int ntime = state.GetID();
	double time = (double)state.m_time;

	FEPostModel& fem = *GetFSModel();
	vec3f r = fem.NodePosition(n, ntime);
	vec3d v = m_pdf->value(r.x, r.y, r.z, time);

	vec3f vf;
	vf.x = (float)(v.x);
	vf.y = (float)(v.y);
	vf.z = (float)(v.z);
	*pv = vf;
}

FEMathMat3DataField::FEMathMat3DataField(Post::FEPostModel* fem, unsigned int flag) : ModelDataField(fem, DATA_MAT3, DATA_NODE, NODE_DATA, flag)
{
	for (int i = 0; i < 9; ++i)
	{
		m_eq[i] = "";
		m_math[i].AddVariable("x");
		m_math[i].AddVariable("y");
		m_math[i].AddVariable("z");
		m_math[i].AddVariable("t");
	}
	BuildMath();
}

void FEMathMat3DataField::BuildMath()
{
	for (int i = 0; i < 9; ++i)
	{
		m_math[i].Create(m_eq[i]);
	}
}

mat3d FEMathMat3DataField::value(double x, double y, double z, double t)
{
	mat3d m;
	std::vector<double> vars = { x, y, z, t };
	if (m_math[0].IsValid()) m[0][0] = m_math[0].value_s(vars);
	if (m_math[1].IsValid()) m[0][1] = m_math[1].value_s(vars);
	if (m_math[2].IsValid()) m[0][2] = m_math[2].value_s(vars);
	if (m_math[3].IsValid()) m[1][0] = m_math[3].value_s(vars);
	if (m_math[4].IsValid()) m[1][1] = m_math[4].value_s(vars);
	if (m_math[5].IsValid()) m[1][2] = m_math[5].value_s(vars);
	if (m_math[6].IsValid()) m[2][0] = m_math[6].value_s(vars);
	if (m_math[7].IsValid()) m[2][1] = m_math[7].value_s(vars);
	if (m_math[8].IsValid()) m[2][2] = m_math[8].value_s(vars);
	return m;
}

FEMathMat3Data::FEMathMat3Data(FEState* state, FEMathMat3DataField* pdf) : FENodeData_T<mat3f>(state, pdf)
{
	m_pdf = pdf;
}

// evaluate the nodal data for this state
void FEMathMat3Data::eval(int n, mat3f* pv)
{
	if ((m_pdf==nullptr)||(pv == nullptr)) return;

	FEState& state = *m_state;
	int ntime = state.GetID();
	double time = (double)state.m_time;

	FEPostModel& fem = *GetFSModel();
	vec3f r = fem.NodePosition(n, ntime);

	mat3d m = m_pdf->value(r.x, r.y, r.z, time);
	*pv = mat3f(m[0][0], m[0][1], m[0][2], m[1][3], m[1][4], m[1][5], m[2][6], m[2][7], m[2][8]);
}
