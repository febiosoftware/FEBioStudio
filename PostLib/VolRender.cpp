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
#include "VolRender.h"
#include <GLLib/GLContext.h>
#include "ImageModel.h"
#include "ColorMap.h"
#include <ImageLib/3DGradientMap.h>
#include <sstream>

using std::stringstream;
using namespace Post;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVolRender::CVolRender(CImageModel* img) : CGLImageRenderer(img)
{
	static int n = 1;
	stringstream ss;
	ss << "VolumeRender" << n++;
	SetName(ss.str());

	AddDoubleParam(0, "alpha scale")->SetFloatRange(0.0, 1.0);
	AddIntParam(0, "min intensity")->SetIntRange(0, 255);
	AddIntParam(0, "max intensity")->SetIntRange(0, 255);
	AddIntParam(0, "min alpha")->SetIntRange(0, 255);
	AddIntParam(0, "max alpha")->SetIntRange(0, 255);
	AddIntParam(0, "Amin")->SetIntRange(0, 255);
	AddIntParam(0, "Amax")->SetIntRange(0, 255);
	AddIntParam(0, "Color map")->SetEnumNames("@color_map");
	AddBoolParam(true, "Lighting effect");
	AddDoubleParam(0, "Lighting strength");
	AddColorParam(GLColor::White(), "Ambient color");
	AddColorParam(GLColor::White(), "Specular color");
	AddVecParam(vec3d(1, 1, 1), "Light direction");

	m_pImx = m_pImy = m_pImz = 0;
	m_sliceX = m_sliceY = m_sliceZ = 0;
	m_nx = m_ny = m_nz = 0;

	m_blight = false;
	m_bcalc_lighting = true;
	m_light = vec3d(-1., -1., -1.);

	m_spc = GLColor(255, 255, 255);

	m_texID = 0;

	m_alpha = 0.2;
	m_I0 = 0;
	m_I1 = 255;
	m_A0 = 0;
	m_A1 = 255;
	m_Amin = 0;
	m_Amax = 255;
	m_shadeStrength = 0.1;

	// use default grayscale color map
	m_Col.SetColorMap(ColorMapManager::GRAY);

	UpdateData(false);

	Reset();
}

CVolRender::~CVolRender()
{
	Clear();
}

bool CVolRender::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_alpha = GetFloatValue(ALPHA_SCALE);
		m_I0 = GetIntValue(MIN_INTENSITY);
		m_I1 = GetIntValue(MAX_INTENSITY);
		m_A0 = GetIntValue(MIN_ALPHA);
		m_A1 = GetIntValue(MAX_ALPHA);
		m_Amin = GetIntValue(AMIN);
		m_Amax = GetIntValue(AMAX);
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		m_blight = GetBoolValue(LIGHTING);
		m_shadeStrength = GetFloatValue(LIGHTING_STRENGTH);
		m_amb = GetColorValue(AMBIENT);
		m_spc = GetColorValue(SPECULAR);
		m_light = GetVecValue(LIGHT_POS);

		UpdateVolRender();
	}
	else
	{
		SetFloatValue(ALPHA_SCALE, m_alpha);
		SetIntValue(MIN_INTENSITY, m_I0);
		SetIntValue(MAX_INTENSITY, m_I1);
		SetIntValue(MIN_ALPHA, m_A0);
		SetIntValue(MAX_ALPHA, m_A1);
		SetIntValue(AMIN, m_Amin);
		SetIntValue(AMAX, m_Amax);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetBoolValue(LIGHTING, m_blight);
		SetFloatValue(LIGHTING_STRENGTH, m_shadeStrength);
		SetColorValue(AMBIENT, m_amb);
		SetColorValue(SPECULAR, m_spc);
		SetVecValue(LIGHT_POS, m_light);
	}

	return false;
}

vec3d CVolRender::GetLightPosition()
{ 
	return m_light; 
}

void CVolRender::SetLightPosition(const vec3d& r) 
{ 
	m_light = r; 
	m_bcalc_lighting = true; 
}

void CVolRender::Reset()
{
	m_Col.SetColorMap(ColorMapManager::GRAY);
	m_amb = GLColor(0,0,0);

	m_alpha = 0.1f;

	m_blight = false;
	m_bcalc_lighting = true;

	m_shadeStrength = 0.2f;

	m_I0 = 0;
	m_I1 = 255;
	m_A0 = 0;
	m_A1 = 255;
	m_Amin = 0;
	m_Amax = 255;
}

void CVolRender::Clear()
{
	delete [] m_sliceX; m_sliceX = 0;
	delete [] m_sliceY; m_sliceY = 0;
	delete [] m_sliceZ; m_sliceZ = 0;

	delete [] m_pImx; m_pImx = 0; m_nx = 0;
	delete [] m_pImy; m_pImy = 0; m_ny = 0;
	delete [] m_pImz; m_pImz = 0; m_nz = 0;
}

//-----------------------------------------------------------------------------
// Create new data for volume renderer
void CVolRender::Create()
{
	CImageModel& img = *GetImageModel();
	CImageSource* src = img.GetImageSource();
	if (src == nullptr) return;

	C3DImage& im3d = *src->Get3DImage();

	// get the original image dimensions
	int w = im3d.Width();
	int h = im3d.Height();
	int d = im3d.Depth();

	// clear the old data
	Clear();

	// find new image dimenions
	m_nx = closest_pow2(w);
	m_ny = closest_pow2(h);
	m_nz = closest_pow2(d);
	m_im3d.Create(m_nx, m_ny, m_nz);

	// resample image
	im3d.StretchBlt(m_im3d);

	// allocate slices
	m_sliceX = new CImage[m_nx];
	m_pImx = new CRGBAImage[m_nx];
	for (int i = 0; i < m_nx; ++i)
	{
		m_sliceX[i].Create(m_ny, m_nz);
		m_pImx[i].Create(m_ny, m_nz);
	}

	m_sliceY = new CImage[m_ny]; 
	m_pImy = new CRGBAImage[m_ny];
	for (int i = 0; i < m_ny; ++i)
	{
		m_sliceY[i].Create(m_nx, m_nz);
		m_pImy[i].Create(m_nx, m_nz);
	}
	
	m_sliceZ = new CImage[m_nz]; 
	m_pImz = new CRGBAImage[m_nz];
	for (int i = 0; i < m_nz; ++i)
	{
		m_sliceZ[i].Create(m_nx, m_ny);
		m_pImz[i].Create(m_nx, m_ny);
	}

#pragma omp parallel default(shared)
	{
#pragma omp for
		for (int i = 0; i < m_nx; ++i) m_im3d.GetSliceX(m_sliceX[i], i);

#pragma omp for
		for (int i = 0; i < m_ny; ++i) m_im3d.GetSliceY(m_sliceY[i], i);

#pragma omp for
		for (int i = 0; i < m_nz; ++i) m_im3d.GetSliceZ(m_sliceZ[i], i);
	}

	// calculate alpha scale factors
/*	BOX b = img.GetBoundingBox();
	float fx = b.Width() / m_nx;
	float fy = b.Height() / m_ny;
	float fz = b.Depth() / m_nz;
	float f = fx;
	if (fy > f) f = fy;
	if (fz > f) f = fz;
*/
	m_ax = 1.0;// f / fx;// (double)nd / (double)m_nx;
	m_ay = 1.0;// f / fy;// (double)nd / (double)m_ny;
	m_az = 1.0;// f / fz;// (double)nd / (double)m_nz;

	// update image data
	Update();
}

//-----------------------------------------------------------------------------
//! Calculate the attenuation factors for volume shading
void CVolRender::CalcAttenuation()
{
	m_att.Create(m_nx, m_ny, m_nz);

	vec3d l = m_light; l.Normalize();

	const BOX& b = GetImageModel()->GetBoundingBox();

	C3DGradientMap map(m_im3d, b);

#pragma omp parallel for default(shared)
	for (int k = 0; k < m_nz; ++k)
	{
		for (int j = 0; j < m_ny; ++j)
			for (int i = 0; i < m_nx; ++i)
			{
				vec3d f = to_vec3d(map.Value(i, j, k)); f.Normalize();
				double a = f*l;
				if (a < 0.0) a = 0.0;
				m_att.value(i, j, k) = (Byte)(255.0*a);
			}
	}
}

//-----------------------------------------------------------------------------
//! Update texture images for volume rendering
void CVolRender::Update()
{
	UpdateData();
	UpdateVolRender();
}

void CVolRender::UpdateVolRender()
{
	// calculate attenuation factors
	if (m_bcalc_lighting)
	{
		CalcAttenuation();
		m_bcalc_lighting = false;
	}

	CColorMap& map = m_Col.ColorMap();

	// build the LUT
	int DI = m_I1 - m_I0;
	if (DI == 0) DI = 1;
	for (int i = 0; i < 256; ++i)
	{
		m_LUT[i] = (i < m_I0 ? 0 : (i > m_I1 ? 255 : 1 + 253 * (i - m_I0) / DI));

		float w = (float)i / 255.f;
		GLColor c = map.map(w);

		m_LUTC[0][i] = c.r;
		m_LUTC[1][i] = c.g;
		m_LUTC[2][i] = c.b;
		m_LUTC[3][i] = (i == 0 ? m_Amin : (i == 255 ? m_Amax : (m_A0 + i*(m_A1 - m_A0) / 255)));
	}

	UpdateRGBImages();
}

void CVolRender::UpdateRGBImages()
{
	#pragma omp parallel default(shared)
	{
		#pragma omp for 
		for (int i = 0; i < m_nx; i++)
		{
			CImage& im = m_sliceX[i];
			Colorize(m_pImx[i], im);
			if (m_blight) DepthCueX(m_pImx[i], i);
		}

		// create the y-images
		#pragma omp for 
		for (int i = 0; i < m_ny; i++)
		{
			CImage& im = m_sliceY[i];
			Colorize(m_pImy[i], im);
			if (m_blight) DepthCueY(m_pImy[i], i);
		}

		// create the z-images
		#pragma omp for 
		for (int i = 0; i < m_nz; i++)
		{
			CImage& im = m_sliceZ[i];
			Colorize(m_pImz[i], im);
			if (m_blight) DepthCueZ(m_pImz[i], i);
		}
	}
}

//-----------------------------------------------------------------------------
//! Colorize the texture images
void CVolRender::Colorize(CRGBAImage& imd, CImage& ims)
{
	int nx = imd.Width();
	int ny = imd.Height();
	int nn = nx*ny;
	Byte* ps = ims.GetBytes();
	Byte* pd = imd.GetBytes();
	for (int i=0; i<nn; i++, ps++, pd+=4)
	{
		int val = m_LUT[*ps];
		pd[0] = m_LUTC[0][val];
		pd[1] = m_LUTC[1][val];
		pd[2] = m_LUTC[2][val];
		pd[3] = m_LUTC[3][val];
	}
}

//-----------------------------------------------------------------------------
//! Attenuate X-textures
void CVolRender::DepthCueX(CRGBAImage& im, int n)
{
	int nx = im.Width();
	int ny = im.Height();
	int nz = m_att.Width();

	Byte* p = im.GetBytes();
	for (int j=0; j<ny; ++j)
		for (int i=0; i<nx; ++i, p += 4)
		{
			double a = m_att.value(n, i, j) / 255.0;
			double w = m_shadeStrength*a + (1.0 - m_shadeStrength);
			double s = m_shadeStrength*a*a;
			p[0] = (Byte) (((p[0]*(1.0 - s) + s*m_spc.r)*w + m_amb.r*(1.0 - w)));
			p[1] = (Byte) (((p[1]*(1.0 - s) + s*m_spc.g)*w + m_amb.g*(1.0 - w)));
			p[2] = (Byte) (((p[2]*(1.0 - s) + s*m_spc.r)*w + m_amb.b*(1.0 - w)));
		}
}

//-----------------------------------------------------------------------------
//! Attenuate Y-textures
void CVolRender::DepthCueY(CRGBAImage& im, int n)
{
	int nx = im.Width();
	int ny = im.Height();
	int nz = m_att.Height();

	Byte* p = im.GetBytes();
	for (int j=0; j<ny; ++j)
		for (int i=0; i<nx; ++i, p += 4)
		{
			double a = m_att.value(i, n, j) / 255.0;
			double w = m_shadeStrength*a + (1.0 - m_shadeStrength);
			double s = m_shadeStrength*a*a;
			p[0] = (Byte) (((p[0]*(1.0 - s) + s*m_spc.r)*w + m_amb.r*(1.0 - w)));
			p[1] = (Byte) (((p[1]*(1.0 - s) + s*m_spc.g)*w + m_amb.g*(1.0 - w)));
			p[2] = (Byte) (((p[2]*(1.0 - s) + s*m_spc.r)*w + m_amb.b*(1.0 - w)));
		}
}

//-----------------------------------------------------------------------------
//! Attenuate Z-textures
void CVolRender::DepthCueZ(CRGBAImage& im, int n)
{
	int nx = im.Width();
	int ny = im.Height();
	int nz = m_att.Depth();

	Byte* p = im.GetBytes();
	for (int j=0; j<ny; ++j)
		for (int i=0; i<nx; ++i, p += 4)
		{
			double a = m_att.value(i, j, n) / 255.0;
			double w = m_shadeStrength*a + (1.0 - m_shadeStrength);
			double s = m_shadeStrength*a*a;
			p[0] = (Byte) (((p[0]*(1.0 - s) + s*m_spc.r)*w + m_amb.r*(1.0 - w)));
			p[1] = (Byte) (((p[1]*(1.0 - s) + s*m_spc.g)*w + m_amb.g*(1.0 - w)));
			p[2] = (Byte) (((p[2]*(1.0 - s) + s*m_spc.r)*w + m_amb.b*(1.0 - w)));
		}
}

//-----------------------------------------------------------------------------
//! Render textures
void CVolRender::Render(CGLContext& rc)
{
	if (m_texID == 0)
	{
		glGenTextures(1, &m_texID);

		glBindTexture(GL_TEXTURE_2D, m_texID);

		// set texture parameter for 2D textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	}
	else glBindTexture(GL_TEXTURE_2D, m_texID);

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	
	vec3d r(0,0,1);
	quatd q = rc.m_q;
	q.Inverse().RotateVector(r);

	double x = fabs(r.x);
	double y = fabs(r.y);
	double z = fabs(r.z);

	if ((x > y) && (x > z)) { RenderX(r.x > 0 ? 1 : -1); }
	if ((y > x) && (y > z)) { RenderY(r.y > 0 ? 1 : -1); }
	if ((z > y) && (z > x)) { RenderZ(r.z > 0 ? 1 : -1); }

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CVolRender::RenderX(int inc)
{
	double a = m_alpha * m_ax;
	glColor4d(1,1,1,a);

	double x;
	double fx;

	if (m_nx == 1) fx = 1; else fx = 1.0 / (m_nx - 1.0);

	int n0, n1;
	if (inc == 1) { n0 = 0; n1 = m_nx; }
	else { n0 = m_nx-1; n1 = -1; }

	double e0 = 0.0;
	double e1 = 1.0 - e0;

	const BOX& box = GetImageModel()->GetBoundingBox();

	for (int i=n0; i != n1; i += inc)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, m_ny, m_nz, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pImx[i].GetBytes());

		x = box.x0 + i*(box.x1 - box.x0)*fx;

		glBegin(GL_QUADS);
		{
			glTexCoord2d(e0,e0); glVertex3d(x, box.y0, box.z0);
			glTexCoord2d(e1,e0); glVertex3d(x, box.y1, box.z0);
			glTexCoord2d(e1,e1); glVertex3d(x, box.y1, box.z1);
			glTexCoord2d(e0,e1); glVertex3d(x, box.y0, box.z1);
		}
		glEnd();
	}
}

//-----------------------------------------------------------------------------
void CVolRender::RenderY(int inc)
{
	double a = m_alpha * m_ay;
	glColor4d(1,1,1,a);

	double y;
	double fy;

	if (m_ny == 1) fy = 1; else fy = 1.0 / (m_ny - 1.0);

	int n0, n1;
	if (inc == 1) { n0 = 0; n1 = m_ny; }
	else { n0 = m_ny-1; n1 = -1; }

	double e0 = 0.0;
	double e1 = 1.0 - e0;

	const BOX& box = GetImageModel()->GetBoundingBox();

	for (int i=n0; i != n1; i += inc)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, m_nx, m_nz, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pImy[i].GetBytes());

		y = box.y0 + i*(box.y1 - box.y0)*fy;

		glBegin(GL_QUADS);
		{
			glTexCoord2d(e0,e0); glVertex3d(box.x0, y, box.z0);
			glTexCoord2d(e1,e0); glVertex3d(box.x1, y, box.z0);
			glTexCoord2d(e1,e1); glVertex3d(box.x1, y, box.z1);
			glTexCoord2d(e0,e1); glVertex3d(box.x0, y, box.z1);
		}
		glEnd();
	}
}

//-----------------------------------------------------------------------------
void CVolRender::RenderZ(int inc)
{
	double a = m_alpha * m_az;
	glColor4d(1,1,1,a);

	double z;
	double fz;

	if (m_nz == 1) fz = 1; else fz = 1.0 / (m_nz - 1.0);

	int n0, n1;
	if (inc == 1) { n0 = 0; n1 = m_nz; }
	else { n0 = m_nz-1; n1 = -1; }

	double e0 = 0.0;
	double e1 = 1.0 - e0;

	const BOX& box = GetImageModel()->GetBoundingBox();

	for (int i=n0; i != n1; i += inc)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, m_nx, m_ny, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pImz[i].GetBytes());

		z = box.z0 + i*(box.z1 - box.z0)*fz;

		glBegin(GL_QUADS);
		{
			glTexCoord2d(e0,e0); glVertex3d(box.x0, box.y0, z);
			glTexCoord2d(e1,e0); glVertex3d(box.x1, box.y0, z);
			glTexCoord2d(e1,e1); glVertex3d(box.x1, box.y1, z);
			glTexCoord2d(e0,e1); glVertex3d(box.x0, box.y1, z);
		}
		glEnd();
	}
}
