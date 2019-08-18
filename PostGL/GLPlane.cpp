#include "stdafx.h"

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "GLPlane.h"
using namespace Post;

CGLPlane::CGLPlane(FEModel* pm)
{
	m_pfem = pm;
	m_e[2] = vec3f(0,0,1);
}

CGLPlane::~CGLPlane(void)
{
}

void CGLPlane::Create(int n[3])
{
	FEMeshBase* pm = m_pfem->GetFEMesh(0);
	if (pm && (n[0] > 0) && (n[1] > 0) && (n[2] > 0))
	{
		FENode& n1 = pm->Node(n[0]-1);
		FENode& n2 = pm->Node(n[1]-1);
		FENode& n3 = pm->Node(n[2]-1);

		vec3f r1 = n1.m_rt;
		vec3f r2 = n2.m_rt;
		vec3f r3 = n3.m_rt;

		m_rc = (r1 + r2 + r3)/3.f;

		BOUNDINGBOX box = m_pfem->GetBoundingBox();
		double R = box.Radius();

		m_e[0] = r1 - r2;
		m_e[1] = r3 - r2;
		m_e[2] = m_e[0] ^ m_e[1]; 
		m_e[1] = m_e[2] ^ m_e[0];

		m_e[0].Normalize();
		m_e[1].Normalize();
		m_e[2].Normalize();
	}
}

void CGLPlane::Render(CGLContext& rc)
{
	FEMeshBase* pm = m_pfem->GetFEMesh(0);

	glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_CULL_FACE);
	GLfloat zero[4] = {0.f};
	GLfloat one[4] = {1.f, 1.f, 1.f, 1.f};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	glLightfv(GL_LIGHT0, GL_SPECULAR, zero);

	double R = m_pfem->GetBoundingBox().Radius();

	glPushMatrix();
	glTranslatef(m_rc.x, m_rc.y, m_rc.z);

	quat4f q(vec3f(0,0,1), m_e[2]);
	double w = q.GetAngle();
	if (w != 0)
	{
		vec3f r = q.GetVector();
		glRotatef((float)(w*RAD2DEG), r.x, r.y, r.z);
	}

	glColor4ub(255, 0, 0, 128);
	glNormal3d(0,0,1);
	glBegin(GL_QUADS);
	{
		glVertex2d(-R, -R); 
		glVertex2d( R, -R); 
		glVertex2d( R,  R); 
		glVertex2d(-R,  R); 
	}
	glEnd();

	glPopMatrix();

	glPopAttrib();
}
