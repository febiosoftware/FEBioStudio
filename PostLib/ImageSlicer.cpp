/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#ifdef WIN32
#include <Windows.h>
#include <gl/GL.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif
#ifdef LINUX
#include <GL/gl.h>
#endif
#include "ImageSlicer.h"
#include "ImageModel.h"
#include <assert.h>
#include <sstream>

using std::stringstream;
using namespace Post;


CImageSlicer::CImageSlicer(CImageModel* img) : CGLImageRenderer(img)
{
	static int n = 1;
	stringstream ss;
	ss << "ImageSlicer" << n++;
	SetName(ss.str());

	AddIntParam(0, "Image orientation")->SetEnumNames("X\0Y\0Z\0");
	AddDoubleParam(0.5, "Image offset")->SetFloatRange(0.0, 1.0);
	AddIntParam(0, "Color map")->SetEnumNames("@color_map");
	AddDoubleParam(1, "Transparency")->SetFloatRange(0.0, 1.0);

	m_Col.SetColorMap(ColorMapManager::GRAY);

	m_texID = 0;
	m_reloadTexture = true;

	UpdateData(false);
}

CImageSlicer::~CImageSlicer()
{
}

int CImageSlicer::GetOrientation() const { return GetIntValue(ORIENTATION); }
void CImageSlicer::SetOrientation(int n) { SetIntValue(ORIENTATION, n); };

double CImageSlicer::GetOffset() const { return GetFloatValue(OFFSET); }
void CImageSlicer::SetOffset(double f) { SetFloatValue(OFFSET, f); }

bool CImageSlicer::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		UpdateSlice();
	}
	else
	{
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
	}

	return false;
}

void CImageSlicer::Create()
{
	CImageSource* src = GetImageModel()->GetImageSource();
	if (src == nullptr) return;

	// call update to initialize all other data
	Update();
}

void CImageSlicer::Update()
{
	UpdateData();
	UpdateSlice();
}

void CImageSlicer::UpdateSlice()
{
	CImageSource* src = GetImageModel()->GetImageSource();
	C3DImage& im3d = *src->Get3DImage();

	int nop = GetOrientation();
	double off = GetOffset();

	// get the 2D image
	CImage im2d;
	switch (nop)
	{
	case 0: // X
		im3d.GetSampledSliceX(im2d, off);
		break;
	case 1: // Y
		im3d.GetSampledSliceY(im2d, off);
		break;
	case 2: // Z
		im3d.GetSampledSliceZ(im2d, off);
		break;
	default:
		assert(false);
	}

	// get the image dimensions
	int W = im2d.Width();
	int H = im2d.Height();

	// build the looktp table
	BuildLUT();

	// create the 2D image
	m_im.Create(W, H);

	// colorize the image
	int nn = W*H;
	Byte* ps = im2d.GetBytes();
	Byte* pd = m_im.GetBytes();
	for (int i = 0; i<nn; i++, ps++, pd += 4)
	{
		int val = *ps;
		pd[0] = m_LUTC[0][val];
		pd[1] = m_LUTC[1][val];
		pd[2] = m_LUTC[2][val];
		pd[3] = m_LUTC[3][val];
	}

	m_reloadTexture = true;
}

void CImageSlicer::BuildLUT()
{
	CColorMap& map = m_Col.ColorMap();

	float f = GetFloatValue(TRANSPARENCY);
	Byte a = Byte(255.f * f);

	// build the LUT
	for (int i = 0; i<256; ++i)
	{
		float w = (float)i / 255.f;
		GLColor c = map.map(w);
		m_LUTC[0][i] = c.r;
		m_LUTC[1][i] = c.g;
		m_LUTC[2][i] = c.b;
		m_LUTC[3][i] = a;
	}
}

//-----------------------------------------------------------------------------
//! Render textures
void CImageSlicer::Render(CGLContext& rc)
{
	if (m_texID == 0)
	{
		glDisable(GL_TEXTURE_2D);

		glGenTextures(1, &m_texID);
		glBindTexture(GL_TEXTURE_2D, m_texID);

		// set texture parameter for 2D textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else glBindTexture(GL_TEXTURE_2D, m_texID);

	int nx = m_im.Width();
	int ny = m_im.Height();
	if (m_reloadTexture && (nx*ny > 0))
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, nx, ny, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_im.GetBytes());
		m_reloadTexture = false;
	}

	BOX box = GetImageModel()->GetBoundingBox();

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	int nop = GetOrientation();
	double off = GetOffset();

	//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4d(1, 1, 1, 1);

	double x[4], y[4], z[4];
	switch (nop)
	{
	case 0:
		x[0] = x[1] = x[2] = x[3] = box.x0 + off*(box.x1 - box.x0);
		y[0] = y[3] = box.y1;  y[1] = y[2] = box.y0;
		z[0] = z[1] = box.z0;  z[2] = z[3] = box.z1;
		break;
	case 1:
		y[0] = y[1] = y[2] = y[3] = box.y0 + off*(box.y1 - box.y0);
		x[0] = x[3] = box.x1;  x[1] = x[2] = box.x0;
		z[0] = z[1] = box.z0;  z[2] = z[3] = box.z1;
		break;
	case 2:
		z[0] = z[1] = z[2] = z[3] = box.z0 + off*(box.z1 - box.z0);
		x[0] = x[3] = box.x1;  x[1] = x[2] = box.x0;
		y[0] = y[1] = box.y0;  y[2] = y[3] = box.y1;
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

	glPopAttrib();
}
