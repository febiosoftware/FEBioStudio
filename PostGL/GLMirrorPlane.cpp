#include "stdafx.h"
#include "GLMirrorPlane.h"
#include "PostLib/PropertyList.h"
#include "GLModel.h"
using namespace Post;

class CMirrorPlaneProps : public CPropertyList
{
public:
	CMirrorPlaneProps(CGLMirrorPlane* mp) : m_mirrorPlane(mp)
	{
		addProperty("Mirror Plane", CProperty::Enum)->setEnumValues(QStringList() << "X" << "Y" << "Z");
		addProperty("Show plane", CProperty::Bool);
		addProperty("Transparency", CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Offset", CProperty::Float);
	}

	QVariant GetPropertyValue(int i)
	{
		switch (i)
		{
		case 0: return m_mirrorPlane->m_plane; break;
		case 1: return m_mirrorPlane->m_showPlane; break;
		case 2: return m_mirrorPlane->m_transparency; break;
		case 3: return m_mirrorPlane->m_offset; break;
		}
		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		switch (i)
		{
		case 0: m_mirrorPlane->m_plane = v.toInt(); break;
		case 1: m_mirrorPlane->m_showPlane = v.toBool(); break;
		case 2: m_mirrorPlane->m_transparency = v.toFloat(); break;
		case 3: m_mirrorPlane->m_offset = v.toFloat(); break;
		}
	}

private:
	CGLMirrorPlane* m_mirrorPlane;
};

CGLMirrorPlane::CGLMirrorPlane(CGLModel* fem) : CGLPlot(fem)
{
	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "MirrorPlane.%02d", n++);
	SetName(szname);

	m_plane = 0;
	m_showPlane = true;
	m_transparency = 0.25f;
	m_offset = 0.f;
}

CPropertyList* CGLMirrorPlane::propertyList()
{
	return new CMirrorPlaneProps(this);
}

void CGLMirrorPlane::Render(CGLContext& rc)
{
	// plane normal
	vec3f scl;
	switch (m_plane)
	{
	case 0: m_norm = vec3f(1.f, 0.f, 0.f); scl = vec3f(-1.f, 1.f, 1.f); break;
	case 1: m_norm = vec3f(0.f, 1.f, 0.f); scl = vec3f(1.f, -1.f, 1.f); break;
	case 2: m_norm = vec3f(0.f, 0.f, 1.f); scl = vec3f(1.f, 1.f, -1.f); break;
	}

	// render the flipped model
	CGLModel* m = GetModel();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(-m_offset*m_norm.x, -m_offset*m_norm.y, -m_offset*m_norm.z);
	glScalef(scl.x, scl.y, scl.z);

	glFrontFace(GL_CW);
	m->Render(rc);
	glFrontFace(GL_CCW);

	glPopMatrix();

	// render the plane
	if (m_showPlane) RenderPlane();
}

//-----------------------------------------------------------------------------
void CGLMirrorPlane::RenderPlane()
{
	CGLModel* mdl = GetModel();

	BOUNDINGBOX box = mdl->GetFEModel()->GetBoundingBox();

	// plane center
	vec3f rc = box.Center();
	switch (m_plane)
	{
	case 0: rc.x = 0.f; break;
	case 1: rc.y = 0.f; break;
	case 2: rc.z = 0.f; break;
	}

	glPushMatrix();

	glTranslatef(rc.x, rc.y, rc.z);
	glTranslatef(-0.5f*m_offset*m_norm.x, -0.5f*m_offset*m_norm.y, -0.5f*m_offset*m_norm.z);

	quatd q = quatd(vec3d(0, 0, 1), m_norm);
	float w = q.GetAngle();
	if (w != 0)
	{
		vec3d v = q.GetVector();
		glRotated(w * 180 / PI, v.x, v.y, v.z);
	}

	float R = box.Radius();

	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	GLdouble r = fabs(m_norm.x);
	GLdouble g = fabs(m_norm.y);
	GLdouble b = fabs(m_norm.z);

	glColor4d(r, g, b, m_transparency);
	glDepthMask(false);
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);
	{
		glVertex3f(-R, -R, 0);
		glVertex3f(R, -R, 0);
		glVertex3f(R, R, 0);
		glVertex3f(-R, R, 0);
	}
	glEnd();
	glDepthMask(true);

	glColor3ub(255, 255, 0);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINE_LOOP);
	{
		glVertex3f(-R, -R, 0);
		glVertex3f(R, -R, 0);
		glVertex3f(R, R, 0);
		glVertex3f(-R, R, 0);
	}
	glEnd();

	glPopMatrix();

	// restore attributes
	glPopAttrib();
}
