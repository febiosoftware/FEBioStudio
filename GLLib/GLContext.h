#pragma once
#include <MathLib/math3d.h>

class CGLCamera;

class CGLContext
{
public:
	CGLContext();
	~CGLContext();

public:
	CGLCamera*	m_cam;
	int			m_x, m_y;
	quatd		m_q;

	bool		m_showMesh;
	bool		m_showOutline;
	bool		m_bext;
	float		m_springThick;
};
