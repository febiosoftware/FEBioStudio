#include "stdafx.h"
#include "FEMathData.h"
#include "FEPostModel.h"

using namespace Post;

FEMathData::FEMathData(FEState* state, FEMathDataField* pdf) : FENodeData_T<float>(state, pdf)
{
	m_pdf = pdf;
}

// evaluate all the nodal data for this state
void FEMathData::eval(int n, float* pv)
{
	FEPostModel& fem = *GetFEModel();


	double time = m_state->m_time;

	FEPostMesh& mesh = *m_state->GetFEMesh();

	CMathParser math;
	math.set_variable("t", time);

	int ierr;
	FENode& node = mesh.Node(n);

	vec3f r = fem.NodePosition(n, m_state->GetID());
	math.set_variable("x", (double)r.x);
	math.set_variable("y", (double)r.y);
	math.set_variable("z", (double)r.z);

	const std::string& eq = m_pdf->EquationString();

	double v = math.eval(eq.c_str(), ierr);

	if (pv) *pv = (float) v;
}


FEMathVec3Data::FEMathVec3Data(FEState* state, FEMathVec3DataField* pdf) : FENodeData_T<vec3f>(state, pdf)
{
	m_pdf = pdf;
}

// evaluate all the nodal data for this state
void FEMathVec3Data::eval(int n, vec3f* pv)
{
	FEPostModel& fem = *GetFEModel();

	FEState& state = *m_state;
	int ntime = state.GetID();
	double time = (double)state.m_time;

	FEPostMesh& mesh = *state.GetFEMesh();

	CMathParser math;
	math.set_variable("t", time);

	int ierr;
	FENode& node = mesh.Node(n);

	vec3f r = fem.NodePosition(n, ntime);
	math.set_variable("x", (double)r.x);
	math.set_variable("y", (double)r.y);
	math.set_variable("z", (double)r.z);

	const std::string& x = m_pdf->EquationString(0);
	const std::string& y = m_pdf->EquationString(1);
	const std::string& z = m_pdf->EquationString(2);

	vec3f v;
	v.x = (float)math.eval(x.c_str(), ierr);
	v.y = (float)math.eval(y.c_str(), ierr);
	v.z = (float)math.eval(z.c_str(), ierr);

	if (pv) *pv = v;
}

FEMathMat3Data::FEMathMat3Data(FEState* state, FEMathMat3DataField* pdf) : FENodeData_T<mat3f>(state, pdf)
{
	m_pdf = pdf;
}

// evaluate the nodal data for this state
void FEMathMat3Data::eval(int n, mat3f* pv)
{
	if (pv == nullptr) return;

	FEPostModel& fem = *GetFEModel();

	FEState& state = *m_state;
	int ntime = state.GetID();
	double time = (double)state.m_time;

	FEPostMesh& mesh = *state.GetFEMesh();

	CMathParser math;
	math.set_variable("t", time);

	int ierr;
	FENode& node = mesh.Node(n);

	vec3f r = fem.NodePosition(n, ntime);
	math.set_variable("x", (double)r.x);
	math.set_variable("y", (double)r.y);
	math.set_variable("z", (double)r.z);

	float m[9] = { 0.f };
	for (int i = 0; i < 9; ++i)
	{
		const std::string& eq = m_pdf->EquationString(i);
		m[i] = (float) math.eval(eq.c_str(), ierr);
	}

	*pv = mat3f(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
}
