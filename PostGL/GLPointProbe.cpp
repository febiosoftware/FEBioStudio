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
#include "GLPointProbe.h"
#include "GLModel.h"
#include <MeshLib/MeshTools.h>
#include <PostLib/constants.h>
#include <MeshLib/FENodeNodeList.h>
#include <MeshTools/FESelection.h>
#include <GLLib/glx.h>
#include <sstream>
#include <FSCore/ClassDescriptor.h>
using namespace Post;

REGISTER_CLASS(GLPointProbe, CLASS_PLOT, "probe", 0);

GLPointProbe::GLPointProbe()
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
	m_bshowPath = true;

	AddBoolParam(true, "track_data", "Track Model Data");
	AddVecParam(m_initPos, "position", "Initial position");
	AddDoubleParam(m_size, "size", "Size scale factor");
	AddColorParam(m_col, "color", "Color");
	AddBoolParam(m_bfollow, "follow", "Follow");
	AddBoolParam(m_bshowPath, "show_path", "Show Path");
	AddColorParam(GLColor(255, 0, 0), "path_color", "Path Color");

	m_lastTime = 0;
	m_lastdt = 1.0;
	m_R = 1.0;
	m_elem = -1;
}

bool GLPointProbe::UpdateData(bool bsave)
{
	if (bsave)
	{
		vec3d r = m_initPos;

		m_initPos = GetVecValue(INIT_POS);
		m_size = GetFloatValue(SIZE);
		m_col = GetColorValue(COLOR);
		m_bfollow = GetBoolValue(FOLLOW);
		m_bshowPath = GetBoolValue(SHOW_PATH);

		if (!(r == m_initPos)) Update();
	}
	else
	{
		SetVecValue(INIT_POS, m_initPos);
		SetFloatValue(SIZE, m_size);
		SetColorValue(COLOR, m_col);
		SetBoolValue(FOLLOW, m_bfollow);
		SetBoolValue(SHOW_PATH, m_bshowPath);
	}

	return false;
}

void GLPointProbe::Render(GLRenderEngine& re, CGLContext& rc)
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

	int ntime = GetModel()->CurrentTimeIndex();
	if (m_bshowPath && (m_path.size() > ntime) && (ntime >= 1))
	{
		GLColor c = GetColorValue(PATH_COLOR);
		glColor3ub(c.r, c.g, c.b);
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i <= ntime; ++i)
		{
			vec3d& r = m_path[i];
			glx::vertex3d(r);
		}
		glEnd();
		glPopAttrib();
	}
}

void GLPointProbe::Update()
{
	Update(m_lastTime, m_lastdt, true);
}

void GLPointProbe::Update(int ntime, float dt, bool breset)
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

	FEPostModel* fem = mdl->GetFSModel();

	if (breset)
	{
		m_path.clear();
		int nstates = fem->GetStates();
		m_path.resize(nstates);
	}

	// update the size of the probe
	BOX box = mdl->GetFSModel()->GetBoundingBox();
	m_R = 0.05*box.GetMaxExtent();

	// see if we need to revaluate the FEFindElement object
	// We evaluate it when the plot needs to be reset, or when the model has a displacement map
	bool bdisp = mdl->HasDisplacementMap();
	if (bdisp == false) return;

	vec3f p0 = to_vec3f(m_initPos);
	m_elem = fem->ProjectToMesh(ntime, p0, m_pos, m_bfollow);

	m_path[ntime] = m_pos;
}

void GLPointProbe::SetInitialPosition(const vec3d& r)
{
	m_initPos = r;
	SetVecValue(INIT_POS, r);
}

bool GLPointProbe::TrackModelData() const
{
	return GetBoolValue(TRACK_DATA);
}

GLColor GLPointProbe::GetColor() const
{
	return m_col;
}

void GLPointProbe::SetColor(const GLColor& c)
{
	m_col = c;
}

vec3d GLPointProbe::GetInitialPosition() const
{
	return m_initPos;
}

void GLPointProbe::GetInitialPosition(const vec3d& p)
{
	m_initPos = p;
	SetVecValue(INIT_POS, p);
}

double GLPointProbe::DataValue(int nfield, int nstep)
{
	if (TrackModelData())
	{
		FEPostModel& fem = *GetModel()->GetFSModel();
		float val = 0.f;
		vec3f p0 = to_vec3f(m_initPos);
		int nelem = fem.ProjectToMesh(nstep, p0, m_pos, m_bfollow);
		if (nelem >= 0)
		{
			float data[FSElement::MAX_NODES];
			fem.EvaluateElement(nelem, nstep, nfield, data, val);
		}
		return val;
	}
	else
	{
		double val = 0.0;
		if ((nstep >= 0) && (nstep < m_path.size()))
		{
			vec3d r = m_path[nstep];
			switch (nfield)
			{
			case 1: val = r.x; break;
			case 2: val = r.y; break;
			case 3: val = r.z; break;
			}
		}
		return val;
	}
}

vec3d GLPointProbe::Position(int nstep)
{
	if ((nstep >= 0) && (nstep < m_path.size())) return m_path[nstep];
	return vec3d(0, 0, 0);
}

class GLPointProbeSelection : public FESelection
{
public:
	GLPointProbeSelection(GLPointProbe* pc) : FESelection(SELECT_OBJECTS), m_pc(pc) { Update(); }

public:
	void Invert() {}
	void Translate(vec3d dr)
	{
		vec3d r = m_pc->GetInitialPosition() + dr;
		m_pc->SetInitialPosition(r);
		Update();
	}

	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}

	quatd GetOrientation() { return quatd(); }

	FEItemListBuilder* CreateItemList() { return nullptr; }

	void Update()
	{
		vec3d r = m_pc->GetInitialPosition();
		m_box = BOX(r, r);
		m_pc->Update();
	}

	int Count() const { return 1; }

private:
	GLPointProbe* m_pc;
};

bool GLPointProbe::Intersects(Ray& ray, Intersection& q)
{
	int ntime = GetModel()->CurrentTimeIndex();
	if (ntime != 0) return false;

	double R = GetFloatValue(SIZE);

	vec3d p = GetInitialPosition();
	double l = ((p - ray.origin) * ray.direction) / ray.direction.norm2();
	if (l > 0)
	{
		vec3d r = ray.origin + ray.direction * l;
		double R2 = (p - r).norm2();
		if (R2 < R * R)
		{
			q.point = r;
			q.m_index = 0;
			return true;
		}
	}

	return false;
}

FESelection* GLPointProbe::SelectComponent(int index)
{
	return new GLPointProbeSelection(this);
}

void GLPointProbe::ClearSelection()
{

}
