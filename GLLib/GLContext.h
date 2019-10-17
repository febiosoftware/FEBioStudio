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

	// the following is needed by planecuts that track the selection
	bool	m_btrack;		// tracking is on
	vec3d	m_track_pos;	// tracked position
	quatd	m_track_rot;	// tracked orientation

	bool		m_showMesh;
	bool		m_showOutline;
	bool		m_bext;
	float		m_springThick;
};
