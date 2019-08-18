// ColorMap.h: interface for the CColorMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLORMAP_H__5CE5C222_17D8_4BD3_8CDB_D4FF17C64525__INCLUDED_)
#define AFX_COLORMAP_H__5CE5C222_17D8_4BD3_8CDB_D4FF17C64525__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MathLib/math3d.h>

typedef unsigned char byte;

#define MAX_MAP_COLORS	5

#define MAX_PAL_COLORS 32

#define COLOR_MAP_JET		0
#define COLOR_MAP_GRAY		1
#define COLOR_MAP_AUTUMN	2
#define COLOR_MAP_WINTER	3
#define COLOR_MAP_SPRING	4
#define COLOR_MAP_SUMMER	5
#define COLOR_MAP_RED 		6
#define COLOR_MAP_GREEN		7
#define COLOR_MAP_BLUE		8
#define COLOR_MAP_RBB		9
#define COLOR_MAP_FIRE		10

class CGLTexture1D;

class CColorMap  
{
public:
	CColorMap();
	virtual ~CColorMap();

	void jet   ();
	void gray  ();
	void autumn();
	void winter();
	void spring();
	void summer();
	void red   ();
	void green ();
	void blue  ();
	void rbb   ();
	void fire  ();

	int Type() { return m_ntype; }
	void SetType(int ntype);

	GLCOLOR map(float fval);

	void SetRange(float min, float max) { m_min = min; m_max = max; }
	void GetRange(float& min, float& max) { min = m_min; max = m_max; }

//	void Create1DTexture(CGLTexture1D& tex, int ndivs);

protected:
	GLCOLOR	m_col[MAX_MAP_COLORS];
	int		m_pos[MAX_MAP_COLORS];

	int		m_ntype;

	float	m_min;
	float	m_max;
};

#endif // !defined(AFX_COLORMAP_H__5CE5C222_17D8_4BD3_8CDB_D4FF17C64525__INCLUDED_)
