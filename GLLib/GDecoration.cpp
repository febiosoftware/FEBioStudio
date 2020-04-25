#include "GDecoration.h"
#ifdef WIN32
#include <Windows.h>
#include <gl/gl.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif
#ifdef LINUX
#include <GL/gl.h>
#endif

void GPointDecoration::render()
{
	glColor3ub(m_col.r, m_col.g, m_col.b);
	glBegin(GL_POINTS);
	{
		glVertex3f(pos.x, pos.y, pos.z);
	}
	glEnd();
}

GLineDecoration::GLineDecoration(const vec3f& a, const vec3f& b)
{
	m_del = true;
	p1 = new GPointDecoration(a);
	p2 = new GPointDecoration(b);
}

GLineDecoration::~GLineDecoration()
{
	if (m_del)
	{
		delete p1;
		delete p2;
	}
}

void GLineDecoration::render()
{
	if (p1 && p2)
	{
		vec3f& r1 = p1->position();
		vec3f& r2 = p2->position();
		glColor3ub(m_col.r, m_col.g, m_col.b);
		glBegin(GL_LINES);
		{
			glVertex3f(r1.x, r1.y, r1.z);
			glVertex3f(r2.x, r2.y, r2.z);
		}
		glEnd();
	}
}

GArcDecoration::GArcDecoration(const vec3f& c, const vec3f& p0, const vec3f& p1, int ndivs, double scale)
{
	m_c = c;
	m_e0 = p0 - c; double l0 = m_e0.Length(); m_e0.Normalize();
	m_e1 = p1 - c; double l1 = m_e1.Length(); m_e1.Normalize();

	double lmin = (l0 <= l1 ? l0 : l1);
	lmin *= scale;

	m_scale = lmin;

	if (lmin <= 0.0)
	{
		m_divs = 0;
	}
	else m_divs = ndivs;
}

void GArcDecoration::render()
{
	if (m_divs == 0) return;

	quatd Q0(0.0, vec3d(0, 0, 1));
	quatd Q1(m_e0, m_e1);

	vec3d c(m_c);
	vec3d p0 = c + m_e0*m_scale;

	glColor3ub(m_col.r, m_col.g, m_col.b);
	glBegin(GL_LINES);
	for (int i = 0; i <= m_divs; ++i)
	{
		double t = i / (double)m_divs;
		quatd Q = quatd::lerp(Q0, Q1, t);

		vec3d rt = m_e0;
		Q.RotateVector(rt);

		vec3d p1 = c + rt*m_scale;

		glVertex3d(p0.x, p0.y, p0.z);
		glVertex3d(p1.x, p1.y, p1.z);

		p0 = p1;
	}
	glEnd();
}

GCompositeDecoration::GCompositeDecoration()
{

}

GCompositeDecoration::~GCompositeDecoration()
{
	for (int i = 0; i < m_deco.size(); ++i) delete m_deco[i];
}

void GCompositeDecoration::AddDecoration(GDecoration* deco)
{
	m_deco.push_back(deco);
}

void GCompositeDecoration::render()
{
	for (int i = 0; i < m_deco.size(); ++i) m_deco[i]->render();
}
