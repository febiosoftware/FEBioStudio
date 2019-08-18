#pragma once
#include "math3d.h"

namespace Post {

class CGLCamera;

class CGLContext
{
public:
	CGLContext();
	~CGLContext();

public:
	CGLCamera*	m_cam;
	int			m_x, m_y;
	quat4f		m_q;

	bool		m_showMesh;
	bool		m_showOutline;
	bool		m_bext;
	float		m_springThick;
};
}
