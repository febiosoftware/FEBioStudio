#include "stdafx.h"
#include "GImageObject.h"
#include <qopengl.h>

//-----------------------------------------------------------------------------
// find the power of 2 that is closest to n
int closest_pow2(int n)
{
	const int PMIN = 16;
	const int PMAX = 512;
	int p = (int)pow(2.0, (int)(0.5 + log((double)n) / log(2.0)));
	if (p < PMIN) p = PMIN;
	if (p > PMAX) p = PMAX;
	return p;
}

GImageObject::GImageObject() : GObject(GIMAGEOBJECT)
{
	m_im = nullptr;

	AddIntParam(0, "X-slice", "X-slice");
	AddIntParam(1, "X-slice", "X-slice");
	AddIntParam(2, "X-slice", "X-slice");

	m_tex[0] = 0;
	m_tex[1] = 0;
	m_tex[2] = 0;

	m_reload[0] = true;
	m_reload[1] = true;
	m_reload[2] = true;
	m_off[0] = 0;
	m_off[1] = 0;
	m_off[2] = 0;
}

GImageObject::~GImageObject()
{
	delete m_im;
}

void GImageObject::SetImageStack(ImageStack* im)
{
	if (m_im) delete m_im;
	m_im = im;

	// original image dimensions
	int nx = m_im->Width();
	int ny = m_im->Height();
	int nz = m_im->Depth();

	// find new image dimenions
	m_nx = closest_pow2(nx);
	m_ny = closest_pow2(ny);
	m_nz = closest_pow2(nz);

	// allocate imaged data
	m_slice[0].Create(m_ny, m_nz);
	m_slice[1].Create(m_nx, m_nz);
	m_slice[2].Create(m_nx, m_ny);

	Update();
}

void GImageObject::SetBox(BOX box)
{
	m_box = box;
}

void GImageObject::Update()
{
	if (m_im == nullptr) return;

	m_off[0] = GetIntValue(0);
	m_off[1] = GetIntValue(1);
	m_off[2] = GetIntValue(2);

	int nx = m_im->Width();
	int ny = m_im->Height();
	int nz = m_im->Depth();

	Image im;
	m_im->GetSampledSliceX(im, (double) m_off[0] / (double) nx); m_slice[0].StretchBlt(im);
	m_im->GetSampledSliceX(im, (double) m_off[1] / (double) ny); m_slice[1].StretchBlt(im);
	m_im->GetSampledSliceX(im, (double) m_off[2] / (double) nz); m_slice[2].StretchBlt(im);

	m_reload[0] = true;
	m_reload[1] = true;
	m_reload[2] = true;
}

void GImageObject::Render()
{
	RenderSlice(0);
	RenderSlice(1);
	RenderSlice(2);
}

//-----------------------------------------------------------------------------
//! Render textures
void GImageObject::RenderSlice(int i)
{
	if (m_tex[i] == 0)
	{
		glDisable(GL_TEXTURE_2D);

		glGenTextures(1, &m_tex[i]);
		glBindTexture(GL_TEXTURE_2D, m_tex[i]);

		// set texture parameter for 2D textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else glBindTexture(GL_TEXTURE_2D, m_tex[i]);

	RGBAImage& im = m_slice[i];

	int nx = im.Width();
	int ny = im.Height();
	if (m_reload[i] && (nx*ny > 0))
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, nx, ny, 0, GL_RGBA, GL_UNSIGNED_BYTE, im.GetBytes());
		m_reload[i] = false;
	}

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4d(1, 1, 1, 1);

	double x[4], y[4], z[4], w;
	switch (i)
	{
	case 0:
		w = (double)m_off[0] / (double) m_im->Width();
		x[0] = x[1] = x[2] = x[3] = m_box.x0 + w*(m_box.x1 - m_box.x0);
		y[0] = y[3] = m_box.y1;  y[1] = y[2] = m_box.y0;
		z[0] = z[1] = m_box.z0;  z[2] = z[3] = m_box.z1;
		break;
	case 1:
		w = (double)m_off[1] / (double)m_im->Height();
		y[0] = y[1] = y[2] = y[3] = m_box.y0 + w*(m_box.y1 - m_box.y0);
		x[0] = x[3] = m_box.x1;  x[1] = x[2] = m_box.x0;
		z[0] = z[1] = m_box.z0;  z[2] = z[3] = m_box.z1;
		break;
	case 2:
		w = (double)m_off[2] / (double)m_im->Depth();
		z[0] = z[1] = z[2] = z[3] = m_box.z0 + w*(m_box.z1 - m_box.z0);
		x[0] = x[3] = m_box.x1;  x[1] = x[2] = m_box.x0;
		y[0] = y[1] = m_box.y0;  y[2] = y[3] = m_box.y1;
		break;
	}

	glBegin(GL_QUADS);
	{
		glTexCoord2d(1, 0); glVertex3d(x[0], y[0], z[0]);
		glTexCoord2d(0, 0); glVertex3d(x[1], y[1], z[1]);
		glTexCoord2d(0, 1); glVertex3d(x[2], y[2], z[2]);
		glTexCoord2d(1, 1); glVertex3d(x[3], y[3], z[3]);
	}
	glEnd();

	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
}
