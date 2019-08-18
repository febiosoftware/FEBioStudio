#pragma once
#include <MathLib/math3d.h>

//-----------------------------------------------------------------------------
class GM_LINE;
class GM_CIRCLE_ARC;

//-----------------------------------------------------------------------------
class GM_LINE
{
public:
	GM_LINE(){}
	GM_LINE(vec2d a, vec2d b) : m_r0(a), m_r1(b) {}

	double Length() { return (m_r1 - m_r0).norm(); }
	double Project(vec2d r, double& D);
	vec2d Point(double l);
	vec2d Tangent(double l);

	bool Intersect(GM_LINE& e, double& w);
	bool Intersect(GM_CIRCLE_ARC& e, double& w);

public:
	vec2d	m_r0, m_r1;
};

//-----------------------------------------------------------------------------
class GM_CIRCLE_ARC
{
public:
	GM_CIRCLE_ARC() { m_R = 0; m_w0 = m_w1 = 0; }
	GM_CIRCLE_ARC(vec2d c, double R, double w0, double w1);
	GM_CIRCLE_ARC(vec2d c, vec2d a, vec2d b, int nw = 1);

	double Coord(vec2d& q);
	double Length();
	double Project(vec2d r, double& D);
	vec2d Point(double l);
	vec2d Tangent(double l);

	bool Intersect(GM_LINE& e, double& w);
	bool Intersect(GM_CIRCLE_ARC& e, double& w);

public:
	vec2d	m_c;
	double	m_R;
	double	m_w0, m_w1;
};

//-----------------------------------------------------------------------------
// Ellipsoidal arc (aligned with x,y coord system)
class GM_ARC
{
public:
	GM_ARC() { m_R0 = m_R1 = 0; m_w0 = m_w1 = 0; }
	GM_ARC(vec2d c, double R0, double R1, double w0, double w1);
	GM_ARC(vec2d c, vec2d a, vec2d b, int nw = 1);

	double Coord(vec2d& q);
	double Length();
	double Project(vec2d r, double& D);
	vec2d Point(double l);
	vec2d Tangent(double l);

	bool Intersect(GM_LINE& e, double& w);
	bool Intersect(GM_CIRCLE_ARC& e, double& w);

public:
	vec2d	m_c;			// center point
	double	m_R0, m_R1;		// radii of ellipsoid
	double	m_w0, m_w1;		// start/end parameter
};
