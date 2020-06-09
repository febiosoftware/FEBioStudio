#pragma once
#include <MathLib/math3d.h>
#include <GLLib/GLContext.h>
class CGLView;

class GGrid
{
public:
	GGrid();

	void SetView(CGLView* pv) { m_pview = pv; }

	vec3d Intersect(vec3d r, vec3d t, bool bsnap);

	double GetScale() { return m_scale; }

	void Render(CGLContext& rc);

protected:
	vec3d Snap(vec3d r);

public:
	vec3d	m_o;	// plane origin
	quatd	m_q;	// plane orientation

	double	m_scale;	// scale of grid (ie. distance between lines)

protected:
	CGLView*	m_pview;
};
