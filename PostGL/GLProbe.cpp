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
#include "GLProbe.h"
#include "GLModel.h"
#include <MeshLib/MeshTools.h>
#include <PostLib/constants.h>
#include <MeshTools/FETetGenMesher.h>
#include <MeshLib/FENodeNodeList.h>
#include <GLLib/glx.h>
#include <sstream>
using namespace Post;

REGISTER_CLASS(GLProbe, CLASS_PLOT, "probe", 0);

GLProbe::GLProbe()
{
	SetTypeString("probe");

	static int n = 1;
	char sz[256] = { 0 };
	sprintf(sz, "Probe%d", n++);
	SetName(sz);

	m_initPos = vec3d(0, 0, 0);
	m_size = 1.0;
	m_col = GLColor::White();
	m_bfollow = true;

	AddVecParam(m_initPos, "Initial position");
	AddDoubleParam(m_size, "Size scale factor");
	AddColorParam(m_col, "Color");
	AddBoolParam(m_bfollow, "Follow");

	m_lastTime = 0;
	m_lastdt = 1.0;
	m_R = 1.0;
	m_elem = -1;
}

bool GLProbe::UpdateData(bool bsave)
{
	if (bsave)
	{
		vec3d r = m_initPos;

		m_initPos = GetVecValue(INIT_POS);
		m_size = GetFloatValue(SIZE);
		m_col = GetColorValue(COLOR);
		m_bfollow = GetBoolValue(FOLLOW);

		if (!(r == m_initPos)) Update();
	}
	else
	{
		SetVecValue(INIT_POS, m_initPos);
		SetFloatValue(SIZE, m_size);
		SetColorValue(COLOR, m_col);
		SetBoolValue(FOLLOW, m_bfollow);
	}

	return false;
}

void GLProbe::Render(CGLContext& rc)
{
	double R = m_R * m_size;
	GLUquadricObj* pobj = gluNewQuadric();
	glColor3ub(m_col.r, m_col.g, m_col.b);
	glPushMatrix();
	{
		glTranslated(m_pos.x, m_pos.y, m_pos.z);
		gluSphere(pobj, R, 32, 32);
	}
	glPopMatrix();

	gluDeleteQuadric(pobj);
}

void GLProbe::Update()
{
	Update(m_lastTime, m_lastdt, true);
}

bool ProjectToElement(FEElement& el, const vec3f& p, vec3f* x0, vec3f* xt, vec3f& q)
{
	int ne = el.Nodes();
	BOX box;
	for (int i = 0; i < ne; ++i) box += x0[i];
	if (box.IsInside(p) == false) return false;

	double r[3] = { 0,0,0 };
	project_inside_element(el, p, r, x0);
	if (IsInsideElement(el, r, 0.001))
	{
		q = el.eval(xt, r[0], r[1], r[2]);
		return true;
	}
	return false;
}

void GLProbe::Update(int ntime, float dt, bool breset)
{
	if ((breset == false) && (ntime == m_lastTime) && (dt == m_lastdt)) return;

	m_lastTime = ntime;
	m_lastdt = dt;

	m_pos = m_initPos;
	m_elem = -1;

	CGLModel* mdl = GetModel();
	if (mdl == nullptr) return;

	FEPostMesh* mesh = mdl->GetActiveMesh();
	if (mesh == nullptr) return;

	// update the size of the probe
	BOX box = mdl->GetFEModel()->GetBoundingBox();
	m_R = 0.05*box.GetMaxExtent();

	// see if we need to revaluate the FEFindElement object
	// We evaluate it when the plot needs to be reset, or when the model has a displacement map
	bool bdisp = mdl->HasDisplacementMap();
	if (bdisp == false) return;

	vec3f p0 = to_vec3f(m_initPos);
	m_elem = ProjectToMesh(ntime, p0, m_pos);
}

int GLProbe::ProjectToMesh(int nstate, const vec3f& r0, vec3d& rt)
{
	CGLModel* mdl = GetModel();
	if (mdl == nullptr) return -1;

	Post::FEState* state = mdl->GetFEModel()->GetState(nstate);
	Post::FERefState* ps = state->m_ref;
	Post::FEPostMesh& mesh = *state->GetFEMesh();

	int nelem = -1;
	vec3f x0[FEElement::MAX_NODES];
	vec3f xt[FEElement::MAX_NODES];
	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FEElement& el = mesh.Element(i);
		int ne = el.Nodes();
		for (int j = 0; j < el.Nodes(); ++j)
		{
			x0[j] = ps->m_Node[el.m_node[j]].m_rt;
			xt[j] = to_vec3f(mesh.Node(el.m_node[j]).r);
		}

		if (m_bfollow)
		{
			vec3f q;
			if (ProjectToElement(el, r0, x0, xt, q))
			{
				rt = q;
				nelem = i;
				break;
			}
		}
		else
		{
			vec3f q;
			if (ProjectToElement(el, r0, x0, x0, q))
			{
				rt = q;
				nelem = i;
				break;
			}
		}
	}

	return nelem;
}

GLColor GLProbe::GetColor() const
{
	return m_col;
}

void GLProbe::SetColor(const GLColor& c)
{
	m_col = c;
}

double GLProbe::DataValue(int nfield, int nstep)
{
	FEPostModel& fem = *GetModel()->GetFEModel();
	float val = 0.f;
	vec3f p0 = to_vec3f(m_initPos);
	int nelem = ProjectToMesh(nstep, p0, m_pos);
	if (nelem >= 0)
	{
		float data[FEElement::MAX_NODES];
		fem.EvaluateElement(nelem, nstep, nfield, data, val);
	}
	return val;
}
