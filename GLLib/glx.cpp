#include "stdafx.h"
#include <qopengl.h>
#include "glx.h"

//-----------------------------------------------------------------------------
void GLX::translate(const vec3d& r)
{
	glTranslated(r.x, r.y, r.z);
}

//-----------------------------------------------------------------------------
void GLX::rotate(const quatd& q)
{
	double w = q.GetAngle();
	if (w != 0)
	{
		vec3d r = q.GetVector();
		glRotated(180*w/PI, r.x, r.y, r.z);
	}
}

//-----------------------------------------------------------------------------
void GLX::drawLine(double x0, double y0, double x1, double y1, double a0, double a1, GLColor c, int n)
{
	double x, y;
	double f, g;
	double h1, h2, h3, a;
	glBegin(GL_LINE_STRIP);
	for (int i=0; i<=n; ++i)
	{
		f = ((double) i / (double) n);
		g = (1.0 - f);
		h1 = (g*(1.0 - 2.0*f));
		h2 = (f*(1.0 - 2.0*g));
		h3 = (4.0*f*g);

		a = (255.0*(h1*a0 + h2*a0 + h3*a1));
		if (a>255) a = 255;
		if (a < 0) a = 0;
		glColor4ub(c.r, c.g, c.b, (uchar) a);

		x = x0 + i*(x1 - x0)/n;
		y = y0 + i*(y1 - y0)/n;

		glVertex2d(x, y);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawCircle(double R, int N)
{
	double x, y;
	glBegin(GL_LINE_LOOP);
	{
		for (int i=0; i<N; ++i)
		{
			x = R*cos(i*2*PI/N);
			y = R*sin(i*2*PI/N);
			glVertex3d(x,y,0);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawCircle(const vec3d& c, double R, int N)
{
	double x, y;
	glBegin(GL_LINE_LOOP);
	{
		for (int i=0; i<N; ++i)
		{
			x = c.x + R*cos(i*2*PI/N);
			y = c.y + R*sin(i*2*PI/N);
			glVertex3d(x,y,c.z);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawPoint(const vec3d& r)
{
	glBegin(GL_POINTS);
	{
		glVertex3d(r.x, r.y, r.z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawLine(const vec3d& a, const vec3d& b)
{
	glBegin(GL_LINES);
	{
		glVertex3d(a.x, a.y, a.z);
		glVertex3d(b.x, b.y, b.z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawLine(const vec3d& a, const vec3d& b, const GLColor& colA, const GLColor& colB)
{
	glBegin(GL_LINES);
	{
		glColor3ub(colA.r, colA.g, colA.b); glVertex3d(a.x, a.y, a.z);
		glColor3ub(colB.r, colB.g, colB.b); glVertex3d(b.x, b.y, b.z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawLine_(const vec3d& a, const vec3d& b, const GLColor& colA, const GLColor& colB)
{
	glColor3ub(colA.r, colA.g, colA.b); glVertex3d(a.x, a.y, a.z);
	glColor3ub(colB.r, colB.g, colB.b); glVertex3d(b.x, b.y, b.z);
}

//-----------------------------------------------------------------------------
void GLX::drawArc(const vec3d& c, double R, double w0, double w1, int N)
{
	glBegin(GL_LINE_STRIP);
	{
		for (int i=0; i<=N; ++i)
		{
			double w = w0 + i*(w1 - w0)/N;
			double x = c.x + R*cos(w);
			double y = c.y + R*sin(w);
			glVertex3d(x, y, c.z);
		}		
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawHelix(const vec3d& a, const vec3d& b, double R, double p, int N)
{
    vec3d c = b - a;
    double L = c.Length();
    
    if (L > 2*R) {
        c.Normalize();
        vec3d e = c ^ vec3d(1,0,0);
        if (e.Length() == 0) e = c ^ vec3d(0,1,0);
        e.Normalize();
        vec3d d = e ^ c; d.Normalize();
        double fp = (L - 2*R)/p;
        int n = fp*N;
        double dq = 2*PI/N;
        
        vec3d va = a + c*R;
        vec3d vb = b - c*R;
        vec3d x;
        glBegin(GL_LINE_STRIP);
        {
            glVertex3d(a.x,a.y,a.z);
            glVertex3d(va.x,va.y,va.z);
            for (int i=0; i<n; ++i)
            {
                x = va + (d*cos(i*dq) + e*sin(i*dq))*R + c*(((L - 2*R)*i)/n);
                glVertex3d(x.x,x.y,x.z);
            }
            glVertex3d(vb.x,vb.y,vb.z);
            glVertex3d(b.x,b.y,b.z);
        }
        glEnd();
    }
    else
        GLX::drawLine(a, b);
}

void GLX::drawQuad(vec3d r[4], vec3d n[4], GLColor c)
{
	glColor4ub(c.r, c.g, c.b, c.a);

	glBegin(GL_QUADS);
	{
		glNormal3d(n[0].x, n[0].y, n[0].z);
		glVertex3d(r[0].x, r[0].y, r[0].z);

		glNormal3d(n[1].x, n[1].y, n[1].z);
		glVertex3d(r[1].x, r[1].y, r[1].z);

		glNormal3d(n[2].x, n[2].y, n[2].z);
		glVertex3d(r[2].x, r[2].y, r[2].z);

		glNormal3d(n[3].x, n[3].y, n[3].z);
		glVertex3d(r[3].x, r[3].y, r[3].z);
	}
	glEnd();
}

void GLX::drawQuad(vec3d r[4], vec3d& n, GLColor& c)
{
	glColor4ub(c.r, c.g, c.b, c.a);
	glNormal3d(n.x, n.y, n.z);

	glBegin(GL_QUADS);
	{
		glVertex3d(r[0].x, r[0].y, r[0].z);
		glVertex3d(r[1].x, r[1].y, r[1].z);
		glVertex3d(r[2].x, r[2].y, r[2].z);
		glVertex3d(r[3].x, r[3].y, r[3].z);
	}
	glEnd();
}

void GLX::drawTriangle(vec3d r[3], vec3d n[3], GLColor c)
{
	glColor4ub(c.r, c.g, c.b, c.a);

	glBegin(GL_TRIANGLES);
	{
		glNormal3d(n[0].x, n[0].y, n[0].z);
		glVertex3d(r[0].x, r[0].y, r[0].z);

		glNormal3d(n[1].x, n[1].y, n[1].z);
		glVertex3d(r[1].x, r[1].y, r[1].z);

		glNormal3d(n[2].x, n[2].y, n[2].z);
		glVertex3d(r[2].x, r[2].y, r[2].z);
	}
	glEnd();
}

void GLX::drawTriangle(vec3d r[3], vec3d& n, GLColor& c)
{
	glColor4ub(c.r, c.g, c.b, c.a);
	glNormal3d(n.x, n.y, n.z);

	glBegin(GL_TRIANGLES);
	{
		glVertex3d(r[0].x, r[0].y, r[0].z);
		glVertex3d(r[1].x, r[1].y, r[1].z);
		glVertex3d(r[2].x, r[2].y, r[2].z);
	}
	glEnd();
}

void GLX::drawLine(double x0, double y0, double x1, double y1)
{
	glBegin(GL_LINES);
	{
		glVertex2d(x0, y0);
		glVertex2d(x1, y1);
	}
	glEnd();
}

void GLX::drawLine(double x0, double y0, double z0, double x1, double y1, double z1)
{
	glBegin(GL_LINES);
	{
		glVertex3d(x0, y0, z0);
		glVertex3d(x1, y1, z1);
	}
	glEnd();
}

void GLX::drawLine(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2)
{
	glBegin(GL_LINE_STRIP);
	{
		glVertex3d(x0, y0, z0);
		glVertex3d(x1, y1, z1);
		glVertex3d(x2, y2, z2);
	}
	glEnd();
}
