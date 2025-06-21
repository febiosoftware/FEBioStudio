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
#include "GLPlotHelicalAxis.h"
#include "GLModel.h"
#include <FSCore/ClassDescriptor.h>
#include <GLLib/glx.h>
#include <sstream>
using namespace Post;
using namespace std;

REGISTER_CLASS(GLPlotHelicalAxis, CLASS_PLOT, "helical_axis", 0);

GLPlotHelicalAxis::GLPlotHelicalAxis()
{
	SetTypeString("helical_axis");

	static int n = 1;
	char sz[256] = { 0 };
	sprintf(sz, "HelicalAxis%d", n++);
	SetName(sz);

	AddIntParam(0, "reference");
	AddIntParam(0, "object");
	AddDoubleParam(1, "scale")->SetFloatRange(0, 10, 0.1);
	AddColorParam(GLColor(255, 255, 0), "color");

	// set the render-order to 1, so this gets drawn after the model is drawn
	SetRenderOrder(1);
}

void string_to_enums(const list<string>& enumString, char* buf)
{
	char* c = buf;
	for (auto& s : enumString)
	{
		strcpy(c, s.data());
		c += s.length() + 1;
	}
	*c = 0;
}


void GLPlotHelicalAxis::SetGLModel(CGLModel* mdl)
{
	m_mdl = mdl;
	if (mdl == nullptr) return;

	m_rb.clear();
	std::list<string> nameList;
	FEPostModel* fem = m_mdl->GetFSModel();
	for (int i = 0; i < fem->PointObjects(); ++i)
	{
		Post::FEPostModel::PointObject* po = fem->GetPointObject(i);
		if (po->m_tag == 1)
		{
			string name = po->GetName();
			m_rb.push_back(i);
			nameList.push_back(name);
		}
	}

	char* src = new char[1024];
	char* trg = new char[1024];

	string_to_enums(nameList, trg);
	nameList.push_front("(world)");
	string_to_enums(nameList, src);

	Param& p0 = GetParam(0); p0.CopyEnumNames(src);
	Param& p1 = GetParam(1); p1.CopyEnumNames(trg);

	delete[] trg;
	delete[] src;
}

bool GLPlotHelicalAxis::UpdateData(bool bsave)
{
	if (m_mdl == nullptr) return false;

	if (bsave)
	{
		bool doUpdate = false;

		if (m_refIndex != GetIntValue(REF)) { m_refIndex = GetIntValue(REF); doUpdate = true; }
		if (m_objIndex != GetIntValue(OBJ)) { m_objIndex = GetIntValue(REF); doUpdate = true; }

		if (doUpdate) Update();
	}
	else
	{
	}

	return false;
}

void GLPlotHelicalAxis::Render(CGLContext& rc)
{
	if (m_mdl == nullptr) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	BOX box = m_mdl->GetFSModel()->GetBoundingBox();
	glPushMatrix();
	glx::translate(pos);
	glx::rotate(rot);

	GLColor col = GetColorValue(COLOR);
	glx::glcolor(col);

	double R = 0.05 * box.Radius();
	double s = GetFloatValue(SCALE);

	glx::renderHelicalAxis(R*s);
	glPopMatrix();

	glPopAttrib();
}

void GLPlotHelicalAxis::Update()
{
	Update(m_lastTime, m_lastDt, true);
}

void GLPlotHelicalAxis::Update(int ntime, float dtFractional, bool breset)
{
	if ((breset == false) && (ntime == m_lastTime) && (dtFractional == m_lastDt)) return;

	m_lastTime = ntime;
	m_lastDt = dtFractional;
	if (m_mdl == nullptr) return;

	FEPostModel* fem = m_mdl->GetFSModel();

	int n0 = GetIntValue(REF) - 1;
	int n1 = GetIntValue(OBJ);

	Post::FEPostModel::PointObject* ref = fem->GetPointObject(n0);
	Post::FEPostModel::PointObject* obj = fem->GetPointObject(n1);
	if (obj == nullptr) return;

	FEState* state = fem->GetState(ntime);

	// we need to figure out time step increment
	double dt = 0;
	if (fem->GetStates() <= 1) dt = 1; // better than 0
	else if (ntime < fem->GetStates() - 1)
	{
		FEState* nextState = fem->GetState(ntime + 1);
		dt = (nextState->m_time - state->m_time);
	}
	else
	{
		FEState* prevState = fem->GetState(ntime - 1);
		dt = (state->m_time - prevState->m_time);
	}
	// It's possible for states to have the same time stamp (e.g. debug runs)
	// so we need to check again for zero
	if (dt == 0) dt = 1;

	int r_index = obj->FindObjectDataIndex("Position");
	int v_index = obj->FindObjectDataIndex("Velocity");
	int w_index = obj->FindObjectDataIndex("Angular velocity");

	if (ref == nullptr)
	{
		OBJ_POINT_DATA& objData = state->m_objPt[n1];

		vec3d x = to_vec3d(objData.data->get<vec3f>(r_index));
		vec3d v = to_vec3d(objData.data->get<vec3f>(v_index));
		vec3d w = to_vec3d(objData.data->get<vec3f>(w_index));

		// instanteneous helical axis
		double rdm = v.norm();
		vec3d n(w); n.unit();
		double tdot = v * n;
		vec3d r = obj->m_pos;
		double w2 = w.norm2();
		vec3d s = (w2 > 0) ? (w ^ (v - n * tdot + (r ^ w))) / (w * w) : vec3d(0, 0, 0);
		if ((w2 == 0) && (rdm > 0)) {
			tdot = rdm / dt;
			n = v.Normalize();
			w = n;
		}

		pos = s;
		w.unit();
		rot = quatd(vec3d(0, 0, 1), w);
	}
	else
	{
		OBJ_POINT_DATA& refData = state->m_objPt[n0];
		OBJ_POINT_DATA& objData = state->m_objPt[n1];

		// incremental rotation in spatial frame
		vec3d oma = to_vec3d(refData.data->get<vec3f>(w_index));
		vec3d omb = to_vec3d(objData.data->get<vec3f>(w_index));
		vec3d va = to_vec3d(refData.data->get<vec3f>(v_index));
		vec3d vb = to_vec3d(objData.data->get<vec3f>(v_index));
		
		// get the relative motion
		vec3d omega = (omb - oma);
		vec3d rdot = vb - va;
		double rdm = rdot.norm();
		vec3d n(omega); n.unit();
		vec3d w(omega);
		double tdot = rdot * n;
		vec3d ra = ref->m_pos;
		vec3d rb = obj->m_pos;
		vec3d r = rb - ra;
		double w2 = w * w;
		vec3d s = (w2 > 0) ? (w ^ (rdot - n * tdot + (r ^ w))) / (w * w) : vec3d(0, 0, 0);
		if ((w2 == 0) && (rdm > 0)) {
			tdot = rdm;
			n = rdot.Normalize();
			omega = n;
		}
		// now transform the helical axis to be represented relative to the ground
		s -= obj->m_pos;
		omega = obj->m_rot.Inverse() * omega;

		pos = s;
		omega.unit();
		rot = quatd(vec3d(0, 0, 1), omega);
	}
}
