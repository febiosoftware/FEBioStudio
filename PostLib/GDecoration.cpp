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
	glBegin(GL_POINTS);
	{
		glVertex3f(pos.x, pos.y, pos.z);
	}
	glEnd();
}

void GLineDecoration::render()
{
	if (p1 && p2)
	{
		vec3f& r1 = p1->position();
		vec3f& r2 = p2->position();
		glBegin(GL_LINES);
		{
			glVertex3f(r1.x, r1.y, r1.z);
			glVertex3f(r2.x, r2.y, r2.z);
		}
		glEnd();
	}
}
