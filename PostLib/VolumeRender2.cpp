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
	int n = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &n);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE, im3d.GetBytes());
	glPixelStorei(GL_UNPACK_ALIGNMENT, n);

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
	int n = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &n);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE, im3d.GetBytes());
	glPixelStorei(GL_UNPACK_ALIGNMENT, n);
}

const char* shadertxt = \
"uniform sampler3D sampler;                               "\
"uniform float Imin;                                      "\
"uniform float Imax;                                      "\
"uniform float Amin;                                      "\
"uniform float Amax;                                      "\
"uniform float gamma;                                     "\
"uniform int cmap;                                        "\
"vec3 grayScale(const float f);                           "\
"vec3 red(const float f);                                 "\
"vec3 green(const float f);                               "\
"vec3 blue(const float f);                                "\
"vec3 fire(const float f);                                "\
"void main(void)                                          "\
"{                                                        "\
"	vec4 t = texture(sampler, gl_TexCoord[0]);            "\
"   float f = t.x;                                        "\
"   f = (f - Imin) / (Imax - Imin);                       "\
"   f = clamp(f, 0.0, 1.0);                               "\
"   if (f <= 0.0) discard;                                "\
"   float a = Amin + f*(Amax - Amin);                     "\
"   if (gamma != 1.0) f = pow(f, gamma);                  "\
"   vec3 c3;                                              "\
"   if      (cmap == 0) { c3 = grayScale(f); }            "\
"   else if (cmap == 1) { c3 = red(f); }                  "\
"   else if (cmap == 2) { c3 = green(f); }                "\
"   else if (cmap == 3) { c3 = blue(f); }                 "\
"   else if (cmap == 4) { c3 = fire(f); }                 "\
"   else c3 = vec3(0,0,0);                                "\
"   vec4 c4 = vec4(c3.x, c3.y, c3.z, a);                  "\
"   gl_FragColor = gl_Color*c4;                            "\
"}                                                        "\
"                                                         "\
"vec3 grayScale(const float f) { return vec3(f, f, f);}   "\
"vec3 red(const float f)   { return vec3(f, 0, 0);}       "\
"vec3 green(const float f) { return vec3(0, f, 0);}       "\
"vec3 blue(const float f)  { return vec3(0, 0, f);}       "\
"vec3 fire(const float f)  {                              "\
"   vec3 c1 = vec3(0.0, 0., 0.);                          "\
"   vec3 c2 = vec3(0.5, 0., 1.);                          "\
"   vec3 c3 = vec3(1.0, 0., 0.);                          "\
"   vec3 c4 = vec3(1.0, 1., 0.);                          "\
"   vec3 c5 = vec3(1.0, 1., 1.);                          "\
"   vec3 c = vec3(0.0,0.,0.);                             "\
"   float wa, wb;                                         "\
"   if      (f >= 0.75) { wb = 2.0*(f - 0.75); wa = 1.0 - wb; c = c4*vec3(wa,wa,wa) + c5*vec3(wb,wb,wb); }"\
"   else if (f >= 0.50) { wb = 2.0*(f - 0.50); wa = 1.0 - wb; c = c3*vec3(wa,wa,wa) + c4*vec3(wb,wb,wb); }"\
"   else if (f >= 0.25) { wb = 2.0*(f - 0.25); wa = 1.0 - wb; c = c2*vec3(wa,wa,wa) + c3*vec3(wb,wb,wb); }"\
"   else if (f >= 0.00) { wb = 2.0*(f - 0.00); wa = 1.0 - wb; c = c1*vec3(wa,wa,wa) + c2*vec3(wb,wb,wb); }"\
"  return vec3(c.x, c.y, c.z);                            "\
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
	GLColor c((GLubyte)(255.0 * r), (GLubyte)(255.0 * g), (GLubyte)(255.0 * b));
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
	GLColor c((GLubyte)(255.0 * r), (GLubyte)(255.0 * g), (GLubyte)(255.0 * b));
	return c;
}

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
	GLint AminID = glGetUniformLocation(m_prgID, "Amin");
	GLint AmaxID = glGetUniformLocation(m_prgID, "Amax");
	GLint cmapID = glGetUniformLocation(m_prgID, "cmap");
	GLint gammaID = glGetUniformLocation(m_prgID, "gamma");

	// float Imin = (float) GetFloatValue(MIN_INTENSITY);
	// float Imax = (float) GetFloatValue(MAX_INTENSITY);
	// int cmap = (int)GetIntValue(COLOR_MAP);

	CImageViewSettings* vs = GetImageModel()->GetViewSettings();

	float Imin = (float) vs->GetFloatValue(CImageViewSettings::MIN_INTENSITY);
	float Imax = (float) vs->GetFloatValue(CImageViewSettings::MAX_INTENSITY);
	float Amin = (float) vs->GetFloatValue(CImageViewSettings::MIN_ALPHA);
	float Amax = (float) vs->GetFloatValue(CImageViewSettings::MAX_ALPHA);
	float gamma = (float) vs->GetFloatValue(CImageViewSettings::GAMMA);
	float hue = vs->GetFloatValue(CImageViewSettings::HUE);
	float sat = vs->GetFloatValue(CImageViewSettings::SAT);
	float lum = vs->GetFloatValue(CImageViewSettings::LUM);
	int cmap = (int) GetIntValue(COLOR_MAP);

	GLColor col = HSV2RGB(360.0*hue, sat, lum);

	glUniform1f(IminID, Imin);
	glUniform1f(ImaxID, Imax);
	glUniform1f(AminID, Amin);
	glUniform1f(AmaxID, Amax);
	glUniform1f(gammaID, gamma);
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

	double g[3][2] = {
		{vs->GetFloatValue(CImageViewSettings::CLIPX_MIN), vs->GetFloatValue(CImageViewSettings::CLIPX_MAX)},
		{vs->GetFloatValue(CImageViewSettings::CLIPY_MIN), vs->GetFloatValue(CImageViewSettings::CLIPY_MAX)},
		{vs->GetFloatValue(CImageViewSettings::CLIPZ_MIN), vs->GetFloatValue(CImageViewSettings::CLIPZ_MAX)}
	};

	GLdouble clip[6][4] = {
		{ 1, 0, 0, -(r0.x + g[0][0]*(r1.x - r0.x))},
		{-1, 0, 0,  (r0.x + g[0][1]*(r1.x - r0.x))},
		{ 0, 1, 0, -(r0.y + g[1][0]*(r1.y - r0.y))},
		{ 0,-1, 0,  (r0.y + g[1][1]*(r1.y - r0.y))},
		{ 0, 0, 1, -(r0.z + g[2][0]*(r1.z - r0.z))},
		{ 0, 0,-1,  (r0.z + g[2][1]*(r1.z - r0.z))},
	};

	glClipPlane(GL_CLIP_PLANE0, clip[0]); glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE1, clip[1]); glEnable(GL_CLIP_PLANE1);
	glClipPlane(GL_CLIP_PLANE2, clip[2]); glEnable(GL_CLIP_PLANE2);
	glClipPlane(GL_CLIP_PLANE3, clip[3]); glEnable(GL_CLIP_PLANE3);
	glClipPlane(GL_CLIP_PLANE4, clip[4]); glEnable(GL_CLIP_PLANE4);
	glClipPlane(GL_CLIP_PLANE5, clip[5]); glEnable(GL_CLIP_PLANE5);

	// Prepare for rendering of the scene
	double alphaScale = GetImageModel()->GetViewSettings()->GetFloatValue(CImageViewSettings::ALPHA_SCALE);
	glColor4ub(col.r, col.g, col.b, (GLubyte) (255.0*alphaScale));
//	glColor4ub(255, 0, 0, (GLubyte) (255.0*alphaScale));
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

	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_CLIP_PLANE1);
	glDisable(GL_CLIP_PLANE2);
	glDisable(GL_CLIP_PLANE3);
	glDisable(GL_CLIP_PLANE4);
	glDisable(GL_CLIP_PLANE5);

	glPopAttrib();
}
