#pragma once
#include <MathLib/math3d.h>
#include <MeshLib/Intersect.h>

class CGLView;

// class that can be used to map screen to world and vice versa
// NOTE: make sure to call makeCurrent before using this class!
class GLViewTransform
{
public:
	GLViewTransform(CGLView* view);

	// convert a point in world coordinates to screen coordinates
	// the return value is a vec3d where x, y are screen coordinates
	// and z is the normalized distance to screen
	vec3d WorldToScreen(const vec3d& r);

	// calculate a ray that starts at the screen position and points forward
	Ray PointToRay(int x, int y);

	// Is the point inside the viewing frustrum (p is in device coordinates)
	bool IsVisible(const vec3d& p);

private:
	CGLView*	m_view;	
	matrix		m_PM, m_PMi;
	int			m_vp[4];
	vector<double>	c, q;
	int			m_dpr;
};
