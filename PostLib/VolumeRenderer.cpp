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
#include "VolumeRenderer.h"
#include <ImageLib/ImageModel.h>
#include <ImageLib/ImageSource.h>
#include <GLLib/GLContext.h>
#include <GLLib/GLCamera.h>
#include <ImageLib/3DImage.h>
#include <FEBioStudio/ImageViewSettings.h>
#include <OGLLib/OGLMesh.h>
#include <sstream>
#include <limits>
using namespace Post;

static int ncount = 1;

CVolumeRenderer::CVolumeRenderer(CImageModel* img) : CGLImageRenderer(img)
{
	// AddDoubleParam(0.1, "alpha scale")->SetFloatRange(0.0, 1.0);
	// AddDoubleParam(0.5, "min intensity")->SetFloatRange(0.0, 1.0);
	// AddDoubleParam(1.0, "max intensity")->SetFloatRange(0.0, 1.0);
	AddChoiceParam(0, "Color map")->SetEnumNames("Grayscale\0Red\0Green\0Blue\0Fire\0");

	std::stringstream ss;
	ss << "VolumeRender" << ncount++;
	SetName(ss.str());

	m_vrInit = false;
	m_vrReset = false;
}

CVolumeRenderer::~CVolumeRenderer()
{
}

void CVolumeRenderer::Create()
{
	m_vrReset = true;
}

void CVolumeRenderer::Reset()
{
	m_vrReset = true;
}

void CVolumeRenderer::Update()
{
	Create();
}

void CVolumeRenderer::Init()
{
	assert(m_vrInit == false);
	if (m_vrInit) return;

	if (GetImageModel() == nullptr) return;

	ReloadTexture();

	m_vrInit = true;
	m_vrReset = false;
}

void CVolumeRenderer::ReloadTexture()
{
	// load texture data
	CImageModel& img = *GetImageModel();
	CImageSource* src = img.GetImageSource();
	if (src == nullptr) return;

	if (src->Get3DImage() == nullptr) return;
	C3DImage* im3d = src->Get3DImage();

	m_tex.Set3DImage(im3d);

	// allocate vertex arrays
	int nx = im3d->Width();
	int ny = im3d->Height();
	int nz = im3d->Depth();

	double wx = (double)nx;
	double wy = (double)ny;
	double wz = (double)nz;
	double wt = 2 * sqrt(wx * wx + wy * wy + wz * wz);
	m_nslices = (int)wt;

	// max 5 triangles per slice, (m_nslices+1) slices
	m_mesh.Create(5 * (m_nslices + 1), OGLMesh::FLAG_TEXTURE);
}

extern int LUT[256][15];
extern int ET_HEX[12][2];

GLColor HSL2RGB(double H, double S, double L)
{
	double C = (1.0 - fabs(2.0 * L - 1.0)) * S;
	double Hp = H / 60.0;
	double X = C * (1.0 - fabs(fmod(Hp, 2.0) - 1.0));
	double R1 = 0, G1 = 0, B1 = 0;
	if      ((Hp >= 0) && (Hp < 1)) { R1 = C; G1 = X; B1 = 0; }
	else if ((Hp >= 1) && (Hp < 2)) { R1 = X; G1 = C; B1 = 0; }
	else if ((Hp >= 2) && (Hp < 3)) { R1 = 0; G1 = C; B1 = X; }
	else if ((Hp >= 3) && (Hp < 4)) { R1 = 0; G1 = X; B1 = C; }
	else if ((Hp >= 4) && (Hp < 5)) { R1 = X; G1 = 0; B1 = C; }
	else if ((Hp >= 5) && (Hp < 6)) { R1 = C; G1 = 0; B1 = X; }
	double m = L - C / 2.0;
	double r = R1 + m, g = G1 + m, b = B1 + m;
	GLColor c((uint8_t)(255.0 * r), (uint8_t)(255.0 * g), (uint8_t)(255.0 * b));
	return c;
}

GLColor HSV2RGB(double H, double S, double V)
{
	double C = V*S;
	double Hp = H / 60.0;
	double X = C * (1.0 - fabs(fmod(Hp, 2.0) - 1.0));
	double R1 = 0, G1 = 0, B1 = 0;
	if      ((Hp >= 0) && (Hp < 1)) { R1 = C; G1 = X; B1 = 0; }
	else if ((Hp >= 1) && (Hp < 2)) { R1 = X; G1 = C; B1 = 0; }
	else if ((Hp >= 2) && (Hp < 3)) { R1 = 0; G1 = C; B1 = X; }
	else if ((Hp >= 3) && (Hp < 4)) { R1 = 0; G1 = X; B1 = C; }
	else if ((Hp >= 4) && (Hp < 5)) { R1 = X; G1 = 0; B1 = C; }
	else if ((Hp >= 5) && (Hp < 6)) { R1 = C; G1 = 0; B1 = X; }
	double m = V - C;
	double r = R1 + m, g = G1 + m, b = B1 + m;
	GLColor c((uint8_t)(255.0 * r), (uint8_t)(255.0 * g), (uint8_t)(255.0 * b));
	return c;
}

void CVolumeRenderer::Render(GLRenderEngine& re, GLContext& rc)
{
	// make sure volume renderer is initialized
	if (m_vrInit == false) Init();
	else if (m_vrReset) 
	{
		ReloadTexture();
		m_vrReset = false;
	}

	// If we failed to initialize, we're done
	if (m_vrInit == false) return;

	// load texture data
	CImageModel& img = *GetImageModel();
	CImageSource* src = img.GetImageSource();
	if (src == nullptr) return;

	if (src->Get3DImage() == nullptr) return;
	C3DImage& im3d = *src->Get3DImage();

	// get the original image dimensions
	int nx = im3d.Width();
	int ny = im3d.Height();
	int nz = im3d.Depth();

	CImageViewSettings* vs = GetImageModel()->GetViewSettings();

	m_tex.Imin = (float) vs->GetFloatValue(CImageViewSettings::MIN_INTENSITY);
	m_tex.Imax = (float) vs->GetFloatValue(CImageViewSettings::MAX_INTENSITY);
	m_tex.Amin = (float) vs->GetFloatValue(CImageViewSettings::MIN_ALPHA);
	m_tex.Amax = (float) vs->GetFloatValue(CImageViewSettings::MAX_ALPHA);
	m_tex.gamma = (float) vs->GetFloatValue(CImageViewSettings::GAMMA);
	m_tex.hue = vs->GetFloatValue(CImageViewSettings::HUE);
	m_tex.sat = vs->GetFloatValue(CImageViewSettings::SAT);
	m_tex.lum = vs->GetFloatValue(CImageViewSettings::LUM);
	m_tex.cmap = (int) GetIntValue(COLOR_MAP);

	GLColor col = HSV2RGB(360.0*m_tex.hue, m_tex.sat, m_tex.lum);

	if ((im3d.PixelType() == CImage::INT_RGB8) || (im3d.PixelType() == CImage::UINT_RGB8) 
        || (im3d.PixelType() == CImage::INT_RGB16) || (im3d.PixelType() == CImage::UINT_RGB16))
	{
		float hue1 = vs->GetFloatValue(CImageViewSettings::CHANNEL1_HUE);
		float hue2 = vs->GetFloatValue(CImageViewSettings::CHANNEL2_HUE);
		float hue3 = vs->GetFloatValue(CImageViewSettings::CHANNEL3_HUE);

		m_tex.col1 = HSV2RGB(360.0 * hue1, 1.0, 1.0);
		m_tex.col2 = HSV2RGB(360.0 * hue2, 1.0, 1.0);
		m_tex.col3 = HSV2RGB(360.0 * hue3, 1.0, 1.0);
	}

	re.pushState();

	re.setMaterial(GLMaterial::CONSTANT, GLColor::White(), GLMaterial::TEXTURE_3D);
	re.setTexture(m_tex);

	double g[3][2] = {
		{vs->GetFloatValue(CImageViewSettings::CLIPX_MIN), vs->GetFloatValue(CImageViewSettings::CLIPX_MAX)},
		{vs->GetFloatValue(CImageViewSettings::CLIPY_MIN), vs->GetFloatValue(CImageViewSettings::CLIPY_MAX)},
		{vs->GetFloatValue(CImageViewSettings::CLIPZ_MIN), vs->GetFloatValue(CImageViewSettings::CLIPZ_MAX)}
	};

	// get the bounding box
	BOX box = img.GetBoundingBox();
	vec3d r0 = box.r0();
	vec3d dr = box.r1() - box.r0();

	// activate clipping planes
	double clip[6][4] = {
		{ 1, 0, 0, -(g[0][0]*(dr.x))},
		{-1, 0, 0,  (g[0][1]*(dr.x))},
		{ 0, 1, 0, -(g[1][0]*(dr.y))},
		{ 0,-1, 0,  (g[1][1]*(dr.y))},
		{ 0, 0, 1, -(g[2][0]*(dr.z))},
		{ 0, 0,-1,  (g[2][1]*(dr.z))},
	};

	re.setClipPlane(0, clip[0]); re.enable(GLRenderEngine::CLIPPLANE0);
	re.setClipPlane(1, clip[1]); re.enable(GLRenderEngine::CLIPPLANE1);
	re.setClipPlane(2, clip[2]); re.enable(GLRenderEngine::CLIPPLANE2);
	re.setClipPlane(3, clip[3]); re.enable(GLRenderEngine::CLIPPLANE3);
	re.setClipPlane(4, clip[4]); re.enable(GLRenderEngine::CLIPPLANE4);
	re.setClipPlane(5, clip[5]); re.enable(GLRenderEngine::CLIPPLANE5);

	// Prepare for rendering of the scene
	double alphaScale = GetImageModel()->GetViewSettings()->GetFloatValue(CImageViewSettings::ALPHA_SCALE);
	re.setColor(GLColor(col.r, col.g, col.b, (unsigned char) (255.0*alphaScale)));

	// get the view direction
	vec3d view(0, 0, 1);
	quatd q = rc.m_cam->GetOrientation();
	q.Inverse().RotateVector(view);

	mat3d Q = GetImageModel()->GetOrientation();
	mat3d Qt = Q.transpose();
	view = Qt * view;

	// the normal will be view direction
	re.normal(view);

	// update geometry and render
	UpdateGeometry(view);
	m_mesh.Render(OGLMesh::FLAG_TEXTURE);

	// clean up
	re.setMaterial(GLMaterial::INVALID, GLColor::White());

	re.disable(GLRenderEngine::CLIPPLANE0);
	re.disable(GLRenderEngine::CLIPPLANE1);
	re.disable(GLRenderEngine::CLIPPLANE2);
	re.disable(GLRenderEngine::CLIPPLANE3);
	re.disable(GLRenderEngine::CLIPPLANE4);
	re.disable(GLRenderEngine::CLIPPLANE5);

	re.popState();
}

void CVolumeRenderer::UpdateGeometry(const vec3d& view)
{
	CImageModel& img = *GetImageModel();

	// get the bounding box corners
	BOX box = img.GetBoundingBox();
	double W = box.Width();
	double H = box.Height();
	double D = box.Depth();
	vec3d dr = box.r1() - box.r0();

	// coordinates of box
	vec3d c[8];
	c[0] = vec3d(   0,    0, 0);
	c[1] = vec3d(dr.x,    0, 0);
	c[2] = vec3d(dr.x, dr.y, 0);
	c[3] = vec3d(   0, dr.y, 0);
	c[4] = vec3d(   0,    0, dr.z);
	c[5] = vec3d(dr.x,    0, dr.z);
	c[6] = vec3d(dr.x, dr.y, dr.z);
	c[7] = vec3d(   0, dr.y, dr.z);

	// find the min, max projection distance
	double tmin = 1e99, tmax = -1e99;
	for (int i = 0; i < 8; ++i)
	{
		double ti = view * c[i];
		if (ti < tmin) tmin = ti;
		if (ti > tmax) tmax = ti;
	}

	m_mesh.BeginMesh();
	double vr[3];
	double vt[3];
	for (int i = 0; i <= m_nslices; ++i)
	{
		double t = tmin + (double)i * (tmax - tmin) / m_nslices;

		// figure out the case
		int ncase = 0;
		double nv[8];
		for (int k = 0; k < 8; ++k)
		{
			nv[k] = c[k] * view;
			if (nv[k] <= t) ncase |= (1 << k);
		}

		// loop over faces
		int* pf = LUT[ncase];
		for (int l = 0; l < 5; l++)
		{
			if (*pf == -1) break;

			// calculate nodal positions
			for (int k = 0; k < 3; k++)
			{
				int n1 = ET_HEX[pf[k]][0];
				int n2 = ET_HEX[pf[k]][1];

				double w = (t - nv[n1]) / (nv[n2] - nv[n1]);

				// position coordinates
				vr[0] = c[n1].x * (1 - w) + c[n2].x * w;
				vr[1] = c[n1].y * (1 - w) + c[n2].y * w;
				vr[2] = c[n1].z * (1 - w) + c[n2].z * w;

				// texture coordinates
				vt[0] = (vr[0]) / W;
				vt[1] = (vr[1]) / H;
				vt[2] = (vr[2]) / D;

				m_mesh.AddVertex(vr, nullptr, vt);
			}

			// on to the next face
			pf += 3;
		}
	}
	m_mesh.EndMesh();
}
