#include "stdafx.h"
#include "GLDisplacementMap.h"
#include "GLModel.h"
using namespace Post;

//-----------------------------------------------------------------------------
CGLDisplacementMap::CGLDisplacementMap(CGLModel* po) : CGLDataMap(po)
{
	AddIntParam(0, "Data field")->SetEnumNames("@data_vec3");
	AddDoubleParam(1.0, "Scale factor");

	char szname[128] = { 0 };
	sprintf(szname, "Displacement Map");
	SetName(szname);

	m_scl = 1.f;
	UpdateData(false);
}

//-----------------------------------------------------------------------------
void CGLDisplacementMap::UpdateData(bool bsave)
{
	if (bsave)
	{
		FEPostModel* pfem = GetModel()->GetFEModel();

		bool bupdate = false;
		int dispField = GetIntValue(DATA_FIELD);
		if (pfem && (pfem->GetDisplacementField() != dispField))
		{
			pfem->SetDisplacementField(dispField);
			bupdate = true;
		}

		float scl = GetFloatValue(SCALE);
		if (scl != m_scl)
		{
			m_scl = scl;
			bupdate = true;
		}

		if (bupdate) UpdateNodes();
	}
	else
	{
		FEPostModel* pfem = GetModel()->GetFEModel();
		if (pfem) SetIntValue(DATA_FIELD, pfem->GetDisplacementField());
		SetFloatValue(SCALE, m_scl);
	}
}

//-----------------------------------------------------------------------------
void CGLDisplacementMap::Activate(bool b)
{
	CGLObject::Activate(b);

	if (b == false)
	{
		CGLModel* po = GetModel();
		FEState* state = po->GetActiveState();
		Post::FERefState& ref = *state->m_ref;
		FEMeshBase* pm = state->GetFEMesh();
		for (int i = 0; i<pm->Nodes(); ++i) pm->Node(i).r = ref.m_Node[i].m_rt;
		pm->UpdateNormals();
	}
}

//-----------------------------------------------------------------------------

void CGLDisplacementMap::Update(int ntime, float dt, bool breset)
{
	UpdateData();

	CGLModel* po = GetModel();
	FEMeshBase* pm = po->GetActiveMesh();
	FEPostModel* pfem = po->GetFEModel();

	// get the number of states and make sure we have something
	int N = pfem->GetStates();
	if (N == 0) return;

	// get the two bracketing time states
	int n0 = ntime;
	int n1 = (ntime + 1 >= N ? ntime : ntime + 1);
	if (dt == 0.f) n1 = n0;

	m_du.resize(pm->Nodes());

	if (n0 == n1)
	{
		// update the states
		UpdateState(n0, breset);
		FEState& s1 = *pfem->GetState(n0);

		// get the reference state
		Post::FERefState& ref = *s1.m_ref;

		// set the current nodal positions
		for (int i = 0; i<pm->Nodes(); ++i)
		{
			vec3f du = s1.m_NODE[i].m_rt - ref.m_Node[i].m_rt;
			m_du[i] = du;
		}
	}
	else
	{
		FEState& s1 = *pfem->GetState(n0);
		FEState& s2 = *pfem->GetState(n1);

		// get the reference state
		Post::FERefState& ref = *s2.m_ref;
		assert(s1.m_ref == s2.m_ref);

		// update the states
		UpdateState(n0, breset);
		UpdateState(n1, breset);

		// calculate weight factor
		float df = s2.m_time - s1.m_time;
		if (df == 0) df = 1.f;
		float w = dt / df;

		// set the current nodal positions
		for (int i = 0; i<pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);

			// get nodal displacements
			vec3f r0 = ref.m_Node[i].m_rt;
			vec3f d1 = s1.m_NODE[i].m_rt - r0;
			vec3f d2 = s2.m_NODE[i].m_rt - r0;

			// evaluate current displacement
			vec3f du = d2*w + d1*(1.f - w);
			m_du[i] = du;
		}
	}

	UpdateNodes();
}

//-----------------------------------------------------------------------------
void CGLDisplacementMap::UpdateState(int ntime, bool breset)
{
	CGLModel* po = GetModel();
	FEMeshBase* pm = po->GetActiveMesh();
	FEPostModel* pfem = po->GetFEModel();

	int N = pfem->GetStates();

	// TODO: This does not look right the correct place for this
	if (breset || (N != m_ntag.size())) m_ntag.assign(N, -1);

	int nfield = pfem->GetDisplacementField();

	if ((nfield >= 0) && (m_ntag[ntime] != nfield))
	{
		m_ntag[ntime] = nfield;

		FEState& s = *pfem->GetState(ntime);

		// get the reference state
		Post::FERefState& ref = *s.m_ref;

		// set the current nodal positions
		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);
			vec3f dr = pfem->EvaluateNodeVector(i, ntime, nfield);

			// the actual nodal position is stored in the state
			// this is the field that will be used for strain calculations
			s.m_NODE[i].m_rt = ref.m_Node[i].m_rt + dr;
		}
	}
}

//-----------------------------------------------------------------------------
void CGLDisplacementMap::UpdateNodes()
{
	CGLModel* po = GetModel();
	FEMeshBase* pm = po->GetActiveMesh();

	if (m_du.empty()) return;
	assert(m_du.size() == pm->Nodes());

	// get the reference state
	Post::FERefState& ref = *po->GetFEModel()->GetState(0)->m_ref;

	for (int i = 0; i < pm->Nodes(); ++i)
	{
		FENode& node = pm->Node(i);
		vec3d r0 = ref.m_Node[i].m_rt;
		node.r = r0 + m_du[i] * m_scl;
	}

	// update the normals
	pm->UpdateNormals();
}
