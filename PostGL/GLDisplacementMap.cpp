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
#include "GLDisplacementMap.h"
#include "GLModel.h"
using namespace Post;

//-----------------------------------------------------------------------------
CGLDisplacementMap::CGLDisplacementMap(CGLModel* po) : CGLDataMap(po)
{
	AddIntParam(0, "data_field")->SetEnumNames("@data_vec3");
	AddVecParam(vec3d(1,1,1), "scale_factor");

	char szname[128] = { 0 };
	sprintf(szname, "Displacement Map");
	SetName(szname);

	m_scl = vec3d(1,1,1);
	UpdateData(false);
}

//-----------------------------------------------------------------------------
bool CGLDisplacementMap::UpdateData(bool bsave)
{
	if (bsave)
	{
		Post::CGLModel& glm = *GetModel();
		FEPostModel* pfem = glm.GetFSModel();

		bool bupdate = false;
		int dispField = GetIntValue(DATA_FIELD);
		if (pfem && (pfem->GetDisplacementField() != dispField))
		{
			pfem->SetDisplacementField(dispField);
			glm.UpdateDisplacements(pfem->CurrentTimeIndex(), true);
			bupdate = true;
		}

		vec3d scl = GetVecValue(SCALE);
		if (!(scl == m_scl))
		{
			m_scl = scl;
			bupdate = true;
		}

		if (bupdate) UpdateNodes();
	}
	else
	{
		FEPostModel* pfem = GetModel()->GetFSModel();
		if (pfem) SetIntValue(DATA_FIELD, pfem->GetDisplacementField());
		SetVecValue(SCALE, m_scl);
	}

	return false;
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
		FSMeshBase* pm = state->GetFEMesh();
		for (int i = 0; i<pm->Nodes(); ++i) pm->Node(i).r = to_vec3d(ref.m_Node[i].m_rt);
		pm->UpdateNormals();
	}
}

//-----------------------------------------------------------------------------

void CGLDisplacementMap::Update(int ntime, float dt, bool breset)
{
	UpdateData();

	CGLModel* po = GetModel();
	FSMeshBase* pm = po->GetActiveMesh();
	FEPostModel* pfem = po->GetFSModel();

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
			FSNode& node = pm->Node(i);

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
	FEPostModel* pfem = po->GetFSModel();
	if (pfem == nullptr)
	{
		m_ntag.clear();
		return;
	}

	FSMeshBase* pm = pfem->GetState(ntime)->GetFEMesh();
	if (pfem == nullptr) {
		m_ntag.clear(); return;
	}

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
			FSNode& node = pm->Node(i);
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
	FEState* state = po->GetActiveState();
	FSMeshBase* pm = po->GetActiveMesh();

	if (m_du.empty()) return;
	assert(m_du.size() == pm->Nodes());

	// get the reference state
	Post::FERefState& ref = *state->m_ref;

	vec3d s = m_scl;

	for (int i = 0; i < pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		vec3d r0 = to_vec3d(ref.m_Node[i].m_rt);
		node.r.x = r0.x + m_du[i].x * m_scl.x;
		node.r.y = r0.y + m_du[i].y * m_scl.y;
		node.r.z = r0.z + m_du[i].z * m_scl.z;
	}

	// update the normals
	pm->UpdateNormals();
}
