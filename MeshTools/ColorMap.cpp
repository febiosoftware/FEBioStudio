// ColorMap.cpp: implementation of the CColorMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ColorMap.h"

GLColor pal[MAX_PAL_COLORS] = {
	GLColor(255,  0,  0),
	GLColor(  0,255,  0),
	GLColor(  0,  0,255),
	GLColor(255,255,  0),
	GLColor(255,  0,255),
	GLColor(  0,255,255),
	GLColor(255,128,  0),
	GLColor(255,  0,128),
	GLColor(128,255,  0),
	GLColor(  0,255,128),
	GLColor(128,  0,255),
	GLColor(  0,128,255),
	GLColor(128,  0,  0),
	GLColor(  0,128,  0),
	GLColor(  0,  0,128),
	GLColor(128,128,  0),
	GLColor(128,  0,128),
	GLColor(  0,128,128),
	GLColor(255,255,255),
	GLColor(192,192,192),
	GLColor(164,164,164),
	GLColor(128,128,128),
	GLColor( 92, 92, 92),
	GLColor( 64, 64, 64),
	GLColor( 32, 32, 32)};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CColorMap::CColorMap()
{
	m_min = 0.f;
	m_max = 255.f;
	jet(); // default color map
}

CColorMap::~CColorMap()
{

}

GLColor CColorMap::map(float fval)
{
	int val = (int)(255*(fval - m_min)/(m_max - m_min));

	int n = MAX_MAP_COLORS - 1;

	if (val <= m_pos[0]) return m_col[0];
	if (val >= m_pos[n]) return m_col[n];

	int l = 0;
	while (m_pos[l] < val) ++l;

	int dp = m_pos[l] - m_pos[l-1];

	GLColor c1 = m_col[l];
	GLColor c2 = m_col[l-1];

	GLColor col;
	col.r = byte(((val - m_pos[l-1])*c1.r + (m_pos[l]-val)*c2.r) / dp);
	col.g = byte(((val - m_pos[l-1])*c1.g + (m_pos[l]-val)*c2.g) / dp);
	col.b = byte(((val - m_pos[l-1])*c1.b + (m_pos[l]-val)*c2.b) / dp);
	col.a = byte(((val - m_pos[l-1])*c1.a + (m_pos[l]-val)*c2.a) / dp);

	return col;
}

void CColorMap::SetType(int ntype)
{
	switch (ntype)
	{
	case COLOR_MAP_JET    : jet    (); break;
	case COLOR_MAP_GRAY   : gray   (); break;
	case COLOR_MAP_AUTUMN : autumn (); break;
	case COLOR_MAP_WINTER : winter (); break;
	case COLOR_MAP_SPRING : spring (); break;
	case COLOR_MAP_SUMMER : summer (); break;
	case COLOR_MAP_RED    : red    (); break;
	case COLOR_MAP_GREEN  : green  (); break;
	case COLOR_MAP_BLUE   : blue   (); break;
	case COLOR_MAP_RBB    : rbb    (); break;
	case COLOR_MAP_FIRE   : fire   (); break;
	}
}

void CColorMap::gray()
{
	m_ntype = COLOR_MAP_GRAY;

	m_pos[0] =   0; m_col[0] = GLColor(  0,  0,  0);
	m_pos[1] =  64; m_col[1] = GLColor( 64, 64, 64);
	m_pos[2] = 128; m_col[2] = GLColor(128,128,128);
	m_pos[3] = 192; m_col[3] = GLColor(192,192,192);
	m_pos[4] = 255; m_col[4] = GLColor(255,255,255);
}

void CColorMap::jet()
{
	m_ntype = COLOR_MAP_JET;

	m_pos[0] =   0; m_col[0] = GLColor(  0,  0,255);
	m_pos[1] =  80; m_col[1] = GLColor(  0,255,255);
	m_pos[2] = 128; m_col[2] = GLColor(  0,255,  0);
	m_pos[3] = 176; m_col[3] = GLColor(255,255,  0);
	m_pos[4] = 255; m_col[4] = GLColor(255,  0,  0);
}

void CColorMap::red()
{
	m_ntype = COLOR_MAP_RED;

	m_pos[0] =   0; m_col[0] = GLColor(  0,  0,  0);
	m_pos[1] =  64; m_col[1] = GLColor( 64,  0,  0);
	m_pos[2] = 128; m_col[2] = GLColor(128,  0,  0);
	m_pos[3] = 192; m_col[3] = GLColor(192,  0,  0);
	m_pos[4] = 255; m_col[4] = GLColor(255,  0,  0);
}

void CColorMap::green()
{
	m_ntype = COLOR_MAP_GREEN;

	m_pos[0] =   0; m_col[0] = GLColor(  0,  0,  0);
	m_pos[1] =  64; m_col[1] = GLColor(  0, 64,  0);
	m_pos[2] = 128; m_col[2] = GLColor(  0,128,  0);
	m_pos[3] = 192; m_col[3] = GLColor(  0,192,  0);
	m_pos[4] = 255; m_col[4] = GLColor(  0,255,  0);
}

void CColorMap::blue()
{
	m_ntype = COLOR_MAP_BLUE;

	m_pos[0] =   0; m_col[0] = GLColor(  0,  0,  0);
	m_pos[1] =  64; m_col[1] = GLColor(  0,  0, 64);
	m_pos[2] = 128; m_col[2] = GLColor(  0,  0,128);
	m_pos[3] = 192; m_col[3] = GLColor(  0,  0,192);
	m_pos[4] = 255; m_col[4] = GLColor(  0,  0,255);
}

void CColorMap::autumn()
{
	m_ntype = COLOR_MAP_AUTUMN;

	m_pos[0] =   0; m_col[0] = GLColor( 50, 50,  0);
	m_pos[1] =  64; m_col[1] = GLColor(100, 50,  0);
	m_pos[2] = 128; m_col[2] = GLColor(150, 75,  0);
	m_pos[3] = 192; m_col[3] = GLColor(200,150,  0);
	m_pos[4] = 255; m_col[4] = GLColor(255,200,  0);
}

void CColorMap::winter()
{
	m_ntype = COLOR_MAP_WINTER;

	m_pos[0] =   0; m_col[0] = GLColor(255,  0,255);
	m_pos[1] =  64; m_col[1] = GLColor(128,  0,255);
	m_pos[2] = 128; m_col[2] = GLColor(  0,  0,255);
	m_pos[3] = 192; m_col[3] = GLColor(  0,255,255);
	m_pos[4] = 255; m_col[4] = GLColor(255,255,255);
}

void CColorMap::spring()
{
	m_ntype = COLOR_MAP_SPRING;

	m_pos[0] =   0; m_col[0] = GLColor(  0,255,  0);
	m_pos[1] =  64; m_col[1] = GLColor(128,255,  0);
	m_pos[2] = 128; m_col[2] = GLColor(255,255,  0);
	m_pos[3] = 192; m_col[3] = GLColor(255,255,128);
	m_pos[4] = 255; m_col[4] = GLColor(255,255,255);
}

void CColorMap::summer()
{
	m_ntype = COLOR_MAP_SUMMER;

	m_pos[0] =   0; m_col[0] = GLColor(255,128,  0);
	m_pos[1] =  64; m_col[1] = GLColor(255,128,  0);
	m_pos[2] = 128; m_col[2] = GLColor(255,255,  0);
	m_pos[3] = 192; m_col[3] = GLColor(255,255,128);
	m_pos[4] = 255; m_col[4] = GLColor(255,255,255);
}

void CColorMap::rbb()
{
	m_ntype = COLOR_MAP_RBB;

	m_pos[0] =   0; m_col[0] = GLColor(128,128,255);
	m_pos[1] =  64; m_col[1] = GLColor(  0,  0,255);
	m_pos[2] = 128; m_col[2] = GLColor(  0,  0,  0);
	m_pos[3] = 192; m_col[3] = GLColor(255,  0,  0);
	m_pos[4] = 255; m_col[4] = GLColor(255,128,128); // salmon
}

void CColorMap::fire()
{
	m_ntype = COLOR_MAP_FIRE;

	m_pos[0] =   0; m_col[0] = GLColor(  0,  0,  0); // black
	m_pos[1] =  64; m_col[1] = GLColor(128,  0,255); // purple
	m_pos[2] = 128; m_col[2] = GLColor(255,  0,  0); // red
	m_pos[3] = 192; m_col[3] = GLColor(255,255,  0); // yellow
	m_pos[4] = 255; m_col[4] = GLColor(255,255,255); // white
}
/*
void CColorMap::Create1DTexture(CGLTexture1D& tex, int ndivs)
{
	int n = tex.Size();
	GLubyte* pb = tex.GetBytes();

	int l;
	GLColor c;
	for (int i=0; i<n; i++, pb+=3)
	{
		l = i*ndivs/n;

		c = map(m_min + l*(m_max-m_min)/(ndivs-1));

		pb[0] = c.r;
		pb[1] = c.g;
		pb[2] = c.b;
	}
}
*/
