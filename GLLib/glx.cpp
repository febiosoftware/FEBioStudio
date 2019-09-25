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

void GLX::drawQuad4(vec3d r[4], vec3d n[4], GLColor c)
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

void GLX::drawQuad4(vec3d r[4], vec3d& n, GLColor& c)
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

void GLX::drawQuad4(vec3d r[4], vec3f n[4])
{
	glBegin(GL_QUADS);
	{
		glNormal3f(n[0].x, n[0].y, n[0].z); glVertex3d(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[1].x, n[1].y, n[1].z); glVertex3d(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[2].x, n[2].y, n[2].z); glVertex3d(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glVertex3d(r[3].x, r[3].y, r[3].z);
	}
	glEnd();
}

void GLX::drawQuad4(vec3d r[4], vec3f n[4], float t[4])
{
	glBegin(GL_QUADS);
	{
		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);
	}
	glEnd();
}

void GLX::drawQuad8(vec3d r[8], vec3f n[8], float t[8])
{
	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[7].x, n[7].y, n[7].z); glTexCoord1f(t[7]); glVertex3f(r[7].x, r[7].y, r[7].z);
		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);

		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);
		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);

		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);

		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);
		glNormal3f(n[7].x, n[7].y, n[7].z); glTexCoord1f(t[7]); glVertex3f(r[7].x, r[7].y, r[7].z);

		glNormal3f(n[7].x, n[7].y, n[7].z); glTexCoord1f(t[7]); glVertex3f(r[7].x, r[7].y, r[7].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);

		glNormal3f(n[7].x, n[7].y, n[7].z); glTexCoord1f(t[7]); glVertex3f(r[7].x, r[7].y, r[7].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);
	}
	glEnd();
}

void GLX::drawQuad9(vec3d r[9], vec3f n[9], float t[9])
{
	const int T[8][3] = {
		{ 0,4,8 },{ 8,7,0 },{ 4,1,5 },{ 5,8,4 },
		{ 7,8,6 },{ 6,3,7 },{ 8,5,2 },{ 2,6,8 } };

	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[T[0][0]].x, n[T[0][0]].y, n[T[0][0]].z); glTexCoord1f(t[T[0][0]]); glVertex3f(r[T[0][0]].x, r[T[0][0]].y, r[T[0][0]].z);
		glNormal3f(n[T[0][1]].x, n[T[0][1]].y, n[T[0][1]].z); glTexCoord1f(t[T[0][1]]); glVertex3f(r[T[0][1]].x, r[T[0][1]].y, r[T[0][1]].z);
		glNormal3f(n[T[0][2]].x, n[T[0][2]].y, n[T[0][2]].z); glTexCoord1f(t[T[0][2]]); glVertex3f(r[T[0][2]].x, r[T[0][2]].y, r[T[0][2]].z);

		glNormal3f(n[T[1][0]].x, n[T[1][0]].y, n[T[1][0]].z); glTexCoord1f(t[T[1][0]]); glVertex3f(r[T[1][0]].x, r[T[1][0]].y, r[T[1][0]].z);
		glNormal3f(n[T[1][1]].x, n[T[1][1]].y, n[T[1][1]].z); glTexCoord1f(t[T[1][1]]); glVertex3f(r[T[1][1]].x, r[T[1][1]].y, r[T[1][1]].z);
		glNormal3f(n[T[1][2]].x, n[T[1][2]].y, n[T[1][2]].z); glTexCoord1f(t[T[1][2]]); glVertex3f(r[T[1][2]].x, r[T[1][2]].y, r[T[1][2]].z);

		glNormal3f(n[T[2][0]].x, n[T[2][0]].y, n[T[2][0]].z); glTexCoord1f(t[T[2][0]]); glVertex3f(r[T[2][0]].x, r[T[2][0]].y, r[T[2][0]].z);
		glNormal3f(n[T[2][1]].x, n[T[2][1]].y, n[T[2][1]].z); glTexCoord1f(t[T[2][1]]); glVertex3f(r[T[2][1]].x, r[T[2][1]].y, r[T[2][1]].z);
		glNormal3f(n[T[2][2]].x, n[T[2][2]].y, n[T[2][2]].z); glTexCoord1f(t[T[2][2]]); glVertex3f(r[T[2][2]].x, r[T[2][2]].y, r[T[2][2]].z);

		glNormal3f(n[T[3][0]].x, n[T[3][0]].y, n[T[3][0]].z); glTexCoord1f(t[T[3][0]]); glVertex3f(r[T[3][0]].x, r[T[3][0]].y, r[T[3][0]].z);
		glNormal3f(n[T[3][1]].x, n[T[3][1]].y, n[T[3][1]].z); glTexCoord1f(t[T[3][1]]); glVertex3f(r[T[3][1]].x, r[T[3][1]].y, r[T[3][1]].z);
		glNormal3f(n[T[3][2]].x, n[T[3][2]].y, n[T[3][2]].z); glTexCoord1f(t[T[3][2]]); glVertex3f(r[T[3][2]].x, r[T[3][2]].y, r[T[3][2]].z);

		glNormal3f(n[T[4][0]].x, n[T[4][0]].y, n[T[4][0]].z); glTexCoord1f(t[T[4][0]]); glVertex3f(r[T[4][0]].x, r[T[4][0]].y, r[T[4][0]].z);
		glNormal3f(n[T[4][1]].x, n[T[4][1]].y, n[T[4][1]].z); glTexCoord1f(t[T[4][1]]); glVertex3f(r[T[4][1]].x, r[T[4][1]].y, r[T[4][1]].z);
		glNormal3f(n[T[4][2]].x, n[T[4][2]].y, n[T[4][2]].z); glTexCoord1f(t[T[4][2]]); glVertex3f(r[T[4][2]].x, r[T[4][2]].y, r[T[4][2]].z);

		glNormal3f(n[T[5][0]].x, n[T[5][0]].y, n[T[5][0]].z); glTexCoord1f(t[T[5][0]]); glVertex3f(r[T[5][0]].x, r[T[5][0]].y, r[T[5][0]].z);
		glNormal3f(n[T[5][1]].x, n[T[5][1]].y, n[T[5][1]].z); glTexCoord1f(t[T[5][1]]); glVertex3f(r[T[5][1]].x, r[T[5][1]].y, r[T[5][1]].z);
		glNormal3f(n[T[5][2]].x, n[T[5][2]].y, n[T[5][2]].z); glTexCoord1f(t[T[5][2]]); glVertex3f(r[T[5][2]].x, r[T[5][2]].y, r[T[5][2]].z);

		glNormal3f(n[T[6][0]].x, n[T[6][0]].y, n[T[6][0]].z); glTexCoord1f(t[T[6][0]]); glVertex3f(r[T[6][0]].x, r[T[6][0]].y, r[T[6][0]].z);
		glNormal3f(n[T[6][1]].x, n[T[6][1]].y, n[T[6][1]].z); glTexCoord1f(t[T[6][1]]); glVertex3f(r[T[6][1]].x, r[T[6][1]].y, r[T[6][1]].z);
		glNormal3f(n[T[6][2]].x, n[T[6][2]].y, n[T[6][2]].z); glTexCoord1f(t[T[6][2]]); glVertex3f(r[T[6][2]].x, r[T[6][2]].y, r[T[6][2]].z);

		glNormal3f(n[T[7][0]].x, n[T[7][0]].y, n[T[7][0]].z); glTexCoord1f(t[T[7][0]]); glVertex3f(r[T[7][0]].x, r[T[7][0]].y, r[T[7][0]].z);
		glNormal3f(n[T[7][1]].x, n[T[7][1]].y, n[T[7][1]].z); glTexCoord1f(t[T[7][1]]); glVertex3f(r[T[7][1]].x, r[T[7][1]].y, r[T[7][1]].z);
		glNormal3f(n[T[7][2]].x, n[T[7][2]].y, n[T[7][2]].z); glTexCoord1f(t[T[7][2]]); glVertex3f(r[T[7][2]].x, r[T[7][2]].y, r[T[7][2]].z);
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

void GLX::drawTri3(vec3d r[3], vec3f n[3])
{
	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[0].x, n[0].y, n[0].z); glVertex3d(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[1].x, n[1].y, n[1].z); glVertex3d(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[2].x, n[2].y, n[2].z); glVertex3d(r[2].x, r[2].y, r[2].z);
	}
	glEnd();
}

void GLX::drawTri3(vec3d r[3], vec3f n[3], float t[3])
{
	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3d(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3d(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3d(r[2].x, r[2].y, r[2].z);
	}
	glEnd();
}

void GLX::drawTri6(vec3d r[6], vec3f n[6], float t[6])
{
	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);

		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);

		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);

		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
	}
	glEnd();
}

void GLX::drawTri7(vec3d r[7], vec3f n[7], float t[7])
{
	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);

		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);

		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);

		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);

		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);

		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
	}
	glEnd();
}

void GLX::drawTri10(vec3d r[10], vec3f n[10], float t[10])
{
	glBegin(GL_TRIANGLES);
	{
		vertex(n[0], r[0], t[0]); vertex(n[3], r[3], t[3]); vertex(n[7], r[7], t[7]);
		vertex(n[1], r[1], t[1]); vertex(n[5], r[5], t[5]); vertex(n[4], r[4], t[4]);
		vertex(n[2], r[2], t[2]); vertex(n[8], r[8], t[8]); vertex(n[6], r[6], t[6]);
		vertex(n[9], r[9], t[9]); vertex(n[7], r[7], t[7]); vertex(n[3], r[3], t[3]);
		vertex(n[9], r[9], t[9]); vertex(n[3], r[3], t[3]); vertex(n[4], r[4], t[4]);
		vertex(n[9], r[9], t[9]); vertex(n[4], r[4], t[4]); vertex(n[5], r[5], t[5]);
		vertex(n[9], r[9], t[9]); vertex(n[5], r[5], t[5]); vertex(n[6], r[6], t[6]);
		vertex(n[9], r[9], t[9]); vertex(n[6], r[6], t[6]); vertex(n[8], r[8], t[8]);
		vertex(n[9], r[9], t[9]); vertex(n[8], r[8], t[8]); vertex(n[7], r[7], t[7]);
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
