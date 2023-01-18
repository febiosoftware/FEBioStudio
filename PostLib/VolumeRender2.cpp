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
#include "VolumeRender2.h"
#include <GL/glew.h>
#include "ImageModel.h"
#include "ImageSource.h"
#include <GLLib/GLContext.h>
#include <ImageLib/3DImage.h>
#include <FEBioStudio/ImageViewSettings.h>
#include <sstream>
using namespace Post;

static int n = 1;

CVolumeRender2::CVolumeRender2(CImageModel* img) : CGLImageRenderer(img)
{
	// AddDoubleParam(0.1, "alpha scale")->SetFloatRange(0.0, 1.0);
	// AddDoubleParam(0.5, "min intensity")->SetFloatRange(0.0, 1.0);
	// AddDoubleParam(1.0, "max intensity")->SetFloatRange(0.0, 1.0);
	AddChoiceParam(0, "Color map")->SetEnumNames("Grayscale\0Red\0Green\0Blue\0Fire\0");

	m_texID = 0;

	std::stringstream ss;
	ss << "VolumeRender" << n++;
	SetName(ss.str());

	m_vrInit = false;
	m_vrReset = false;
}

CVolumeRender2::~CVolumeRender2()
{

}

void CVolumeRender2::Create()
{
	m_vrReset = true;
}

void CVolumeRender2::Update()
{
	Create();
}

void CVolumeRender2::Init()
{
	assert(m_vrInit == false);
	if (m_vrInit) return;

	if (InitTexture())
	{
		m_vrInit = true;
		m_vrReset = false;
		InitShaders();
	}
}

bool CVolumeRender2::InitTexture()
{
	// load texture data
	CImageModel& img = *GetImageModel();
	CImageSource* src = img.GetImageSource();
	if (src == nullptr) return false;

	C3DImage& im3d = *src->Get3DImage();

	// get the original image dimensions
	int nx = im3d.Width();
	int ny = im3d.Height();
	int nz = im3d.Depth();

	if (m_texID == 0) glGenTextures(1, &m_texID);

	glBindTexture(GL_TEXTURE_3D, m_texID);

//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// set texture parameter for 2D textures
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	// This will copy the image data to texture memory, so in principle we won't need im3d anymore
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE, im3d.GetBytes());

	return true;
}

void CVolumeRender2::ReloadTexture()
{
	// load texture data
	CImageModel& img = *GetImageModel();
	CImageSource* src = img.GetImageSource();
	if (src == nullptr) return;

	C3DImage& im3d = *src->Get3DImage();

	// get the original image dimensions
	int nx = im3d.Width();
	int ny = im3d.Height();
	int nz = im3d.Depth();

	glBindTexture(GL_TEXTURE_3D, m_texID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE, im3d.GetBytes());
}

const char* shadertxt = \
"uniform sampler3D sampler;                               "\
"uniform float Imin;                                      "\
"uniform float Imax;                                      "\
"uniform int cmap;                                        "\
"vec4 grayScale(const float f);                           "\
"vec4 red(const float f);                                 "\
"vec4 green(const float f);                               "\
"vec4 blue(const float f);                                "\
"vec4 fire(const float f);                                "\
"void main(void)                                          "\
"{                                                        "\
"	vec4 c = texture(sampler, gl_TexCoord[0]);            "\
"   float f = c.x;                                        "\
"   f = (f - Imin) / (Imax - Imin);                       "\
"   f = clamp(f, 0.0, 1.0);                               "\
"   if (f <= 0.0) discard;                                "\
"   if      (cmap == 0) { c = grayScale(f); }             "\
"   else if (cmap == 1) { c = red(f); }                   "\
"   else if (cmap == 2) { c = green(f); }                 "\
"   else if (cmap == 3) { c = blue(f); }                  "\
"   else if (cmap == 4) { c = fire(f); }                  "\
"   else c = vec4(0,0,0,f);                               "\
"   gl_FragColor = gl_Color*c;                            "\
"}                                                        "\
"                                                         "\
"vec4 grayScale(const float f) { return vec4(f, f, f, f);}"\
"vec4 red(const float f)   { return vec4(f, 0, 0, f);}    "\
"vec4 green(const float f) { return vec4(0, f, 0, f);}    "\
"vec4 blue(const float f)  { return vec4(0, 0, f, f);}    "\
"vec4 fire(const float f)  {                              "\
"   vec3 c1 = vec3(0.0, 0., 0.);                                  "\
"   vec3 c2 = vec3(0.5, 0., 1.);                                  "\
"   vec3 c3 = vec3(1.0, 0., 0.);                                   "\
"   vec3 c4 = vec3(1.0, 1., 0.);                                   "\
"   vec3 c5 = vec3(1.0, 1., 1.);                                   "\
"   vec3 c = vec3(0.0,0.,0.);                                            "\
"   float wa, wb;                                     "\
"   if      (f >= 0.75) { wb = 2.0*(f - 0.75); wa = 1.0 - wb; c = c4*vec3(wa,wa,wa) + c5*vec3(wb,wb,wb); }"\
"   else if (f >= 0.50) { wb = 2.0*(f - 0.50); wa = 1.0 - wb; c = c3*vec3(wa,wa,wa) + c4*vec3(wb,wb,wb); }"\
"   else if (f >= 0.25) { wb = 2.0*(f - 0.25); wa = 1.0 - wb; c = c2*vec3(wa,wa,wa) + c3*vec3(wb,wb,wb); }"\
"   else if (f >= 0.00) { wb = 2.0*(f - 0.00); wa = 1.0 - wb; c = c1*vec3(wa,wa,wa) + c2*vec3(wb,wb,wb); }"\
"  return vec4(c.x, c.y, c.z, f);                               "\
"}                                                        "\
"";

void CVolumeRender2::InitShaders()
{
	// create the fragment shader
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// set the shader text
	glShaderSource(fragShader, 1, &shadertxt, NULL);

	// compile the shader
	glCompileShader(fragShader);
	int success;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (success == false)
	{
		const int MAX_INFO_LOG_SIZE = 1024;
		GLchar infoLog[MAX_INFO_LOG_SIZE];
		glGetShaderInfoLog(fragShader, MAX_INFO_LOG_SIZE, NULL, infoLog);
		fprintf(stderr, infoLog);
	}
	assert(success);

	// create the program
	m_prgID = glCreateProgram();
	glAttachShader(m_prgID, fragShader);

	glLinkProgram(m_prgID);
	glGetProgramiv(m_prgID, GL_LINK_STATUS, &success); 
	assert(success);
}

extern int LUT[256][15];
extern int ET_HEX[12][2];

void CVolumeRender2::Render(CGLContext& rc)
{
	// load texture data
	CImageModel& img = *GetImageModel();
	CImageSource* src = img.GetImageSource();
	if (src == nullptr) return;

	C3DImage& im3d = *src->Get3DImage();

	// get the original image dimensions
	int nx = im3d.Width();
	int ny = im3d.Height();
	int nz = im3d.Depth();

	// make sure volume renderer is initialized
	if (m_vrInit == false) Init();
	else if (m_vrReset) 
	{
		ReloadTexture();
		m_vrReset = false;
	}

	// If we failed to initialize, we're done
	if (m_vrInit == false) return;

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_3D);
	glDisable(GL_LIGHTING);
	
	// bind texture
	glBindTexture(GL_TEXTURE_3D, m_texID);

	// set program
	glUseProgram(m_prgID);

	GLint IminID = glGetUniformLocation(m_prgID, "Imin");
	GLint ImaxID = glGetUniformLocation(m_prgID, "Imax");
	GLint cmapID = glGetUniformLocation(m_prgID, "cmap");

	// float Imin = (float) GetFloatValue(MIN_INTENSITY);
	// float Imax = (float) GetFloatValue(MAX_INTENSITY);
	// int cmap = (int)GetIntValue(COLOR_MAP);

    float Imin = (float) GetImageModel()->GetViewSettings()->GetFloatValue(CImageViewSettings::MIN_INTENSITY);
	float Imax = (float) GetImageModel()->GetViewSettings()->GetFloatValue(CImageViewSettings::MAX_INTENSITY);
	int cmap = (int)GetIntValue(COLOR_MAP);

	glUniform1f(IminID, Imin);
	glUniform1f(ImaxID, Imax);
	glUniform1i(cmapID, cmap);

	// get the view direction
	vec3d view(0, 0, 1);
	quatd q = rc.m_q;
	q.Inverse().RotateVector(view);

	// get the bounding box corners
	BOX box = img.GetBoundingBox();
	double W = box.Width();
	double H = box.Height();
	double D = box.Depth();
	vec3d r0 = box.r0();
	vec3d r1 = box.r1();
	vec3d c[8];
	c[0] = vec3d(r0.x, r0.y, r0.z);
	c[1] = vec3d(r1.x, r0.y, r0.z);
	c[2] = vec3d(r1.x, r1.y, r0.z);
	c[3] = vec3d(r0.x, r1.y, r0.z);
	c[4] = vec3d(r0.x, r0.y, r1.z);
	c[5] = vec3d(r1.x, r0.y, r1.z);
	c[6] = vec3d(r1.x, r1.y, r1.z);
	c[7] = vec3d(r0.x, r1.y, r1.z);

	// find the min, max projection distance
	double tmin = 1e99, tmax = -1e99;
	for (int i = 0; i < 8; ++i)
	{
		double ti = view * c[i];
		if (ti < tmin) tmin = ti;
		if (ti > tmax) tmax = ti;
	}

	// Prepare for rendering of the scene
	double alphaScale = GetImageModel()->GetViewSettings()->GetFloatValue(CImageViewSettings::ALPHA_SCALE);
	glColor4f(1.f, 1.f, 1.f, alphaScale);
	glBegin(GL_TRIANGLES);

	// the normal will be view direction
	glNormal3d(view.x, view.y, view.z);

	// set a sampling density
	double wx = (double)nx;
	double wy = (double)ny;
	double wz = (double)nz;
	double wt = 2*sqrt(wx * wx + wy * wy + wz * wz);
	int nt = (int)wt;
	for (int i = 0; i <= nt; ++i)
	{
		double t = tmin + (double)i * (tmax - tmin) / nt;

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
			vec3d vr[3], vt[3];
			for (int k = 0; k < 3; k++)
			{
				int n1 = ET_HEX[pf[k]][0];
				int n2 = ET_HEX[pf[k]][1];

				double w = (t - nv[n1]) / (nv[n2] - nv[n1]);

				vr[k] = c[n1] * (1 - w) + c[n2] * w;
				vt[k].x = (vr[k].x - r0.x) / W;
				vt[k].y = (vr[k].y - r0.y) / H;
				vt[k].z = (vr[k].z - r0.z) / D;
			}

			// render the face
			glTexCoord3d(vt[0].x, vt[0].y, vt[0].z); glVertex3d(vr[0].x, vr[0].y, vr[0].z);
			glTexCoord3d(vt[1].x, vt[1].y, vt[1].z); glVertex3d(vr[1].x, vr[1].y, vr[1].z);
			glTexCoord3d(vt[2].x, vt[2].y, vt[2].z); glVertex3d(vr[2].x, vr[2].y, vr[2].z);

			// on to the next face
			pf += 3;
		}
	}
	glEnd();

	glUseProgram(0);

	glPopAttrib();
}
