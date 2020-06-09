#include "stdafx.h"
#include "GLViewTransform.h"
#include "GLView.h"
#include <GLLib/GView.h>

GLViewTransform::GLViewTransform(CGLView* view) : m_view(view), m_PM(4, 4), m_PMi(4, 4), q(4, 0.0), c(4, 0.0)
{
	view->SetupProjection();
	view->PositionCamera();

	double p[16], m[16];
	glGetDoublev(GL_PROJECTION_MATRIX, p);
	glGetDoublev(GL_MODELVIEW_MATRIX, m);

	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// calculate projection matrix
	matrix P(4, 4);
	for (int i = 0; i<4; ++i)
		for (int j = 0; j<4; ++j) P(i, j) = p[j * 4 + i];

	// calculate modelview matrix
	matrix M(4, 4);
	for (int i = 0; i<4; ++i)
		for (int j = 0; j<4; ++j) M(i, j) = m[j * 4 + i];

	// multiply them together
	m_PM = P*M;

	// calculate inverse
	m_PMi = m_PM.inverse();

	// store the viewport
	view->GetViewport(m_vp);

	// store device pixel ration
	m_dpr = view->GetDevicePixelRatio();
}

vec3d GLViewTransform::WorldToScreen(const vec3d& r)
{
	// get the homogeneous coordinates
	q[0] = r.x; q[1] = r.y; q[2] = r.z; q[3] = 1.0;

	// calculcate clip coordinates
	m_PM.mult(q, c);

	// calculate device coordinates
	vec3d d;
	d.x = c[0] / c[3];
	d.y = c[1] / c[3];
	d.z = c[2] / c[3];

	int W = m_vp[2];
	int H = m_vp[3];
	float xd = W*((d.x + 1.f)*0.5f);
	float yd = H - H*((d.y + 1.f)*0.5f);

	return vec3d(xd / m_dpr, yd / m_dpr, d.z);
}

Ray GLViewTransform::PointToRay(int x, int y)
{
	// adjust for high resolution displays
	x *= m_dpr;
	y *= m_dpr;

	// flip the y-axis
	y = m_vp[3] - y;

	// convert to devices coordinates
	double W = m_vp[2];
	double H = m_vp[3];
	double xd = 2.0* x / W - 1.0;
	double yd = 2.0* y / H - 1.0;

	CGView* view = m_view->GetView();
	if (view == nullptr) return Ray();

	// get the projection mode
	bool ortho = view->OrhographicProjection();

	double fnear = view->GetNearPlane();
	double ffar = view->GetFarPlane();

	// convert to clip coordinates
	vector<double> c(4);
	c[3] = (ortho ? 1.0 : fnear);
	c[0] = xd*c[3];
	c[1] = yd*c[3];
	c[2] = -c[3];

	// convert to world coordinates
	vector<double> r_near(4), r_far(4);
	m_PMi.mult(c, r_near);

	// do back clip point
	c[3] = (ortho ? 1.0 : ffar);
	c[0] = xd*c[3];
	c[1] = yd*c[3];
	c[2] = c[3];
	m_PMi.mult(c, r_far);

	vec3d r0 = vec3d(r_near[0], r_near[1], r_near[2]);
	vec3d r1 = vec3d(r_far[0], r_far[1], r_far[2]);
	vec3d n = r1 - r0; n.Normalize();

	Ray ray = { r0, n };
	return ray;
}

bool GLViewTransform::IsVisible(const vec3d& p)
{
	double W = m_vp[2];
	double H = m_vp[3];
	return ((p.x > 0) && (p.x < W) && (p.y > 0) && (p.y < H) && (p.z > -1) && (p.z < 1));
}
