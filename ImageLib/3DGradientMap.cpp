#include "stdafx.h"
#include "3DGradientMap.h"

//=============================================================================
// C3DGradientMap
//=============================================================================

C3DGradientMap::C3DGradientMap(C3DImage& im, BOX box) : m_im(im), m_box(box)
{

}

C3DGradientMap::~C3DGradientMap()
{
}

vec3f C3DGradientMap::Value(int i, int j, int k)
{
	// get the image dimensions
	int nx = m_im.Width();
	int ny = m_im.Height();
	int nz = m_im.Depth();

	float dxi = (nx - 1.f) / (float)m_box.Width();
	float dyi = (ny - 1.f) / (float)m_box.Height();
	float dzi = (nz - 1.f) / (float)m_box.Depth();

	// calculate the gradient
	vec3f r;

	// x-component
	if (i == 0) r.x = ((float)m_im.value(i + 1, j, k) - (float)m_im.value(i, j, k)) * dxi;
	else if (i == nx - 1) r.x = ((float)m_im.value(i, j, k) - (float)m_im.value(i - 1, j, k)) * dxi;
	else r.x = ((float)m_im.value(i + 1, j, k) - (float)m_im.value(i - 1, j, k)) * (0.5f*dxi);

	// y-component
	if (j == 0) r.y = ((float)m_im.value(i, j + 1, k) - (float)m_im.value(i, j, k)) * dyi;
	else if (j == ny - 1) r.y = ((float)m_im.value(i, j, k) - (float)m_im.value(i, j - 1, k)) * dyi;
	else r.y = ((float)m_im.value(i, j + 1, k) - (float)m_im.value(i, j - 1, k)) * (0.5f*dyi);

	// z-component
	if (k == 0) r.z = ((float)m_im.value(i, j, k + 1) - (float)m_im.value(i, j, k)) * dzi;
	else if (k == nz - 1) r.z = ((float)m_im.value(i, j, k) - (float)m_im.value(i, j, k - 1)) * dzi;
	else r.z = ((float)m_im.value(i, j, k + 1) - (float)m_im.value(i, j, k - 1)) * (0.5f*dzi);

	return r;
}
