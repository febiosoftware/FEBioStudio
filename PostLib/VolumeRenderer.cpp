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
#include <GL/glew.h>
#include <ImageLib/ImageModel.h>
#include <ImageLib/ImageSource.h>
#include <GLLib/GLContext.h>
#include <GLLib/GLProgram.h>
#include <GLLib/GLCamera.h>
#include <ImageLib/3DImage.h>
#include <FEBioStudio/ImageViewSettings.h>
#include <sstream>
#include <limits>
using namespace Post;

static int ncount = 1;

GLProgram VRprg;

CVolumeRenderer::CVolumeRenderer(CImageModel* img) : CGLImageRenderer(img)
{
	// AddDoubleParam(0.1, "alpha scale")->SetFloatRange(0.0, 1.0);
	// AddDoubleParam(0.5, "min intensity")->SetFloatRange(0.0, 1.0);
	// AddDoubleParam(1.0, "max intensity")->SetFloatRange(0.0, 1.0);
	AddChoiceParam(0, "Color map")->SetEnumNames("Grayscale\0Red\0Green\0Blue\0Fire\0");

	m_texID = 0;

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

	// generate a texture ID
	if (m_texID == 0) glGenTextures(1, &m_texID);

	ReloadTexture();

	m_vrInit = true;
	m_vrReset = false;
	InitShaders();
}

void CVolumeRenderer::ReloadTexture()
{
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

    int pType = im3d.PixelType();

	// find the min and max values
    size_t N  = nx * ny * nz;
    if(pType == CImage::INT_RGB8 || pType == CImage::UINT_RGB8 || pType == CImage::INT_RGB16 || pType == CImage::UINT_RGB16)
    {
        N *= 3;
    }

    constexpr int max8 = std::numeric_limits<unsigned char>::max();
    constexpr int max16 = std::numeric_limits<unsigned short>::max();
    constexpr uint32_t max32 = std::numeric_limits<unsigned int>::max();

    double min, max;
    im3d.GetMinMax(min, max);
	switch (pType)
	{
	case CImage::INT_8:
	case CImage::INT_RGB8:
	{
		max /= max8 / 2;
		min /= max8 / 2;
	}
	break;
	case CImage::UINT_8:
	case CImage::UINT_RGB8:
	{
		max /= max8;
		min /= max8;
	}
	break;
	case CImage::INT_RGB16:
	{
		max /= max16 / 2;
		min /= max16 / 2;
	}
	break;
	case CImage::UINT_16:
	case CImage::UINT_RGB16:
	{
		max /= max16;
		min /= max16;
	}
	break;
	case CImage::INT_16:
	case CImage::INT_32:
	case CImage::UINT_32:
	{
		// Don't do anything for these cases since we scale the data
		// before we send it to OpenGL
	}
	break;
	case CImage::REAL_32:
	case CImage::REAL_64:
	{
		// floating point images are copied and scaled when calling glTexImage3D, so we can just 
		// set the range to [0,1]
		max = 1.f;
		min = 0.f;
	}
	break;
	default:
		assert(false);
	}

	if (max == min) max++;
	m_Iscale = 1.f /(max - min);
	m_IscaleMin = min;

	glBindTexture(GL_TEXTURE_3D, m_texID);

	//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// set texture parameter for 2D textures
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	int oldUnpackAlignment = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	switch (im3d.PixelType())
	{
	case CImage::INT_8     : glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_BYTE , im3d.GetBytes()); break;
	case CImage::INT_16:
	{
		GLbyte* d = new GLbyte[N];
		short* s = (short*)im3d.GetBytes();
		for (size_t i = 0; i < N; ++i) d[i] = (GLbyte)(255 * (s[i] - m_IscaleMin) * m_Iscale);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE, d);
		delete[] d;

		m_Iscale = 1.f;
		m_IscaleMin = 0.f;
	}
	break;
	case CImage::INT_32    :
    case CImage::UINT_32   :
	{
		// We're doing the scaling here, because it appears
		// that OpenGL only maps the upper 16 bits to the range [0,1].
		// If the image only fills the lower 16 bits, we won't see anything 
		GLbyte* d = new GLbyte[N];
		int* s = (int*)im3d.GetBytes();
		for (size_t i = 0; i < N; ++i) d[i] = (GLbyte)(255*(s[i] - m_IscaleMin) * m_Iscale);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE, d);
		delete[] d;

		m_Iscale = 1.f;
		m_IscaleMin = 0.f;
	}
	break;
	case CImage::INT_RGB8  : glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_BYTE , im3d.GetBytes()); break;
	case CImage::INT_RGB16 : glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_SHORT, im3d.GetBytes()); break;
	case CImage::UINT_8    : glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE , im3d.GetBytes()); break;
	case CImage::UINT_16   : glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_SHORT, im3d.GetBytes()); break;
	case CImage::UINT_RGB8 : glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_UNSIGNED_BYTE , im3d.GetBytes()); break;
	case CImage::UINT_RGB16: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_UNSIGNED_SHORT, im3d.GetBytes()); break;
	case CImage::REAL_32: 
	{
		// Opengl expects that values between 0 and 1, so we need to scale the buffer
		float* d = new float[N];
		float* s = (float*)im3d.GetBytes();
		float fmax = *std::max_element(s, s + N); if (fmax == 0.f) fmax = 1.f;
		for (size_t i = 0; i < N; ++i) d[i] = (s[i] / fmax);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_FLOAT, d);
		delete[] d;
	}
	break;
	case CImage::REAL_64: 
	{
		// OpenGL doesn't have a 64-bit real (i.e. double precision), so we need to make a float copy.
		float* d = new float[N];
		double* s = (double*)im3d.GetBytes();
		double dmax = *std::max_element(s, s + N); if (dmax == 0.0) dmax = 1.0;
		for (size_t i = 0; i < N; ++i) d[i] = (float)(s[i] / dmax);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_FLOAT, d);
		delete[] d;
	}
	break;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);

	// allocate vertex arrays
	double wx = (double)nx;
	double wy = (double)ny;
	double wz = (double)nz;
	double wt = 2 * sqrt(wx * wx + wy * wy + wz * wz);
	m_nslices = (int)wt;

	// max 5 triangles per slice, (m_nslices+1) slices
	m_mesh.Create(5 * (m_nslices + 1), GLMesh::FLAG_TEXTURE);
}

const char* shadertxt_8bit = \
"#version 120\n"\
"uniform sampler3D sampler;                               \n"\
"uniform float Imin;                                      \n"\
"uniform float Imax;                                      \n"\
"uniform float Iscl;                                      \n"\
"uniform float IsclMin;                                   \n"\
"uniform float Amin;                                      \n"\
"uniform float Amax;                                      \n"\
"uniform float gamma;                                     \n"\
"uniform int cmap;                                        \n"\
"vec3 grayScale(const float f);                           \n"\
"vec3 red(const float f);                                 \n"\
"vec3 green(const float f);                               \n"\
"vec3 blue(const float f);                                \n"\
"vec3 fire(const float f);                                \n"\
"void main(void)                                          \n"\
"{                                                        \n"\
"	vec4 t = texture3D(sampler, gl_TexCoord[0].xyz);      \n"\
"   float f = (t.x-IsclMin)*Iscl;                         \n"\
"   f = (f - Imin) / (Imax - Imin);                       \n"\
"   f = clamp(f, 0.0, 1.0);                               \n"\
"   if (f <= 0.0) discard;                                \n"\
"   float a = Amin + f*(Amax - Amin);                     \n"\
"   if (gamma != 1.0) f = pow(f, gamma);                  \n"\
"   vec3 c3;                                              \n"\
"   if      (cmap == 0) { c3 = grayScale(f); }            \n"\
"   else if (cmap == 1) { c3 = red(f); }                  \n"\
"   else if (cmap == 2) { c3 = green(f); }                \n"\
"   else if (cmap == 3) { c3 = blue(f); }                 \n"\
"   else if (cmap == 4) { c3 = fire(f); }                 \n"\
"   else c3 = vec3(0,0,0);                                \n"\
"   vec4 c4 = vec4(c3.x, c3.y, c3.z, a);                  \n"\
"   gl_FragColor = gl_Color*c4;                           \n "\
"}                                                        \n"\
"                                                         \n"\
"vec3 grayScale(const float f) { return vec3(f, f, f);}   \n"\
"vec3 red(const float f)   { return vec3(f, 0, 0);}       \n"\
"vec3 green(const float f) { return vec3(0, f, 0);}       \n"\
"vec3 blue(const float f)  { return vec3(0, 0, f);}       \n"\
"vec3 fire(const float f)  {                              \n"\
"   vec3 c1 = vec3(0.0, 0., 0.);                          \n"\
"   vec3 c2 = vec3(0.5, 0., 1.);                          \n"\
"   vec3 c3 = vec3(1.0, 0., 0.);                          \n"\
"   vec3 c4 = vec3(1.0, 1., 0.);                          \n"\
"   vec3 c5 = vec3(1.0, 1., 1.);                          \n"\
"   vec3 c = vec3(0.0,0.,0.);                             \n"\
"   float wa, wb;                                         \n"\
"   if      (f >= 0.75) { wb = 2.0*(f - 0.75); wa = 1.0 - wb; c = c4*vec3(wa,wa,wa) + c5*vec3(wb,wb,wb); }\n"\
"   else if (f >= 0.50) { wb = 2.0*(f - 0.50); wa = 1.0 - wb; c = c3*vec3(wa,wa,wa) + c4*vec3(wb,wb,wb); }\n"\
"   else if (f >= 0.25) { wb = 2.0*(f - 0.25); wa = 1.0 - wb; c = c2*vec3(wa,wa,wa) + c3*vec3(wb,wb,wb); }\n"\
"   else if (f >= 0.00) { wb = 2.0*(f - 0.00); wa = 1.0 - wb; c = c1*vec3(wa,wa,wa) + c2*vec3(wb,wb,wb); }\n"\
"  return vec3(c.x, c.y, c.z);                            \n"\
"}                                                        \n"\
"";


const char* shadertxt_rgb = \
"#version 120\n"\
"uniform sampler3D sampler;                               \n"\
"uniform float Iscl;                                      \n"\
"uniform float IsclMin;                                   \n"\
"uniform float Imin;                                      \n"\
"uniform float Imax;                                      \n"\
"uniform float Amin;                                      \n"\
"uniform float Amax;                                      \n"\
"uniform float gamma;                                     \n"\
"uniform vec3 col1;                                       \n"\
"uniform vec3 col2;                                       \n"\
"uniform vec3 col3;                                       \n"\
"uniform int cmap;                                        \n"\
"void main(void)                                          \n"\
"{                                                        \n"\
"   vec4 t = (texture3D(sampler, gl_TexCoord[0].xyz)-IsclMin)*Iscl;\n"\
"   t.x = (t.x - Imin) / (Imax - Imin);                   \n"\
"   t.x = clamp(t.x, 0.0, 1.0);                           \n"\
"   t.y = (t.y - Imin) / (Imax - Imin);                   \n"\
"   t.y = clamp(t.y, 0.0, 1.0);                           \n"\
"   t.z = (t.z - Imin) / (Imax - Imin);                   \n"\
"   t.z = clamp(t.z, 0.0, 1.0);                           \n"\
"   float f = t.x;                                        \n"\
"   if (t.y > f) f = t.y;                                 \n"\
"   if (t.z > f) f = t.z;                                 \n"\
"   if (f <= 0.0) discard;                                \n"\
"   if (gamma != 1.0) f = pow(f, gamma);                  \n"\
"   float a = Amin + f*(Amax - Amin);                     \n"\
"   vec3 c1 = vec3(t.x*col1.x, t.x*col1.y, t.x*col1.z);   \n"\
"   vec3 c2 = vec3(t.y*col2.x, t.y*col2.y, t.y*col2.z);   \n"\
"   vec3 c3 = vec3(t.z*col3.x, t.z*col3.y, t.z*col3.z);   \n"\
"   vec3 c4 = c1 + c2 + c3;                               \n"\
"   gl_FragColor = gl_Color*vec4(c4.x, c4.y, c4.z, a);    \n"\
"}                                                        \n"\
"";

const char* vertex_shader = \
"#version 130\n"\
"void main(void)                                          \n"\
"{                                                        \n"\
"   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex; \n "\
"}                                                        \n"\
"";

void CVolumeRenderer::InitShaders()
{
	// load texture data
	CImageModel& img = *GetImageModel();
	CImageSource* src = img.GetImageSource();
	if (src == nullptr) return;

	if (src->Get3DImage() == nullptr) return;
	C3DImage& im3d = *src->Get3DImage();

	const char* shadertxt = nullptr;
	switch (im3d.PixelType())
	{
    case CImage::UINT_8  : shadertxt = shadertxt_8bit; break;
	case CImage::INT_8  : shadertxt = shadertxt_8bit; break;
	case CImage::UINT_16  : shadertxt = shadertxt_8bit; break;
    case CImage::INT_16 : shadertxt = shadertxt_8bit; break;
    case CImage::UINT_32  : shadertxt = shadertxt_8bit; break;
    case CImage::INT_32 : shadertxt = shadertxt_8bit; break;
	case CImage::UINT_RGB8  : shadertxt = shadertxt_rgb; break;
    case CImage::INT_RGB8: shadertxt = shadertxt_rgb; break;
	case CImage::UINT_RGB16  : shadertxt = shadertxt_rgb; break;
    case CImage::INT_RGB16: shadertxt = shadertxt_rgb; break;
	case CImage::REAL_32: shadertxt = shadertxt_8bit; break;
	case CImage::REAL_64: shadertxt = shadertxt_8bit; break;
	default:
		return;
	}

	// create the program
	VRprg.Create(nullptr, shadertxt);
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

void CVolumeRenderer::Render(GLRenderEngine& re, CGLContext& rc)
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

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_3D);
	glDisable(GL_LIGHTING);

	// bind texture
	glBindTexture(GL_TEXTURE_3D, m_texID);

	// set program
	VRprg.Use();

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

	VRprg.SetFloat("Imin"   , Imin);
	VRprg.SetFloat("Imax"   , Imax);
	VRprg.SetFloat("Amin"   , Amin);
	VRprg.SetFloat("Amax"   , Amax);
	VRprg.SetFloat("gamma"  , gamma);
	VRprg.SetInt  ("cmap"   , cmap);
	VRprg.SetFloat("Iscl"   , m_Iscale);
	VRprg.SetFloat("IsclMin", m_IscaleMin);

	if ((im3d.PixelType() == CImage::INT_RGB8) || (im3d.PixelType() == CImage::UINT_RGB8) 
        || (im3d.PixelType() == CImage::INT_RGB16) || (im3d.PixelType() == CImage::UINT_RGB16))
	{
		float hue1 = vs->GetFloatValue(CImageViewSettings::CHANNEL1_HUE);
		float hue2 = vs->GetFloatValue(CImageViewSettings::CHANNEL2_HUE);
		float hue3 = vs->GetFloatValue(CImageViewSettings::CHANNEL3_HUE);

		GLColor col1 = HSV2RGB(360.0 * hue1, 1.0, 1.0);
		GLColor col2 = HSV2RGB(360.0 * hue2, 1.0, 1.0);
		GLColor col3 = HSV2RGB(360.0 * hue3, 1.0, 1.0);

		float c1[3] = { col1.r / 255.f, col1.g / 255.f, col1.b / 255.f };
		float c2[3] = { col2.r / 255.f, col2.g / 255.f, col2.b / 255.f };
		float c3[3] = { col3.r / 255.f, col3.g / 255.f, col3.b / 255.f };

		VRprg.SetFloat3("col1", c1);
		VRprg.SetFloat3("col2", c2);
		VRprg.SetFloat3("col3", c3);
	}

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
	GLdouble clip[6][4] = {
		{ 1, 0, 0, -(g[0][0]*(dr.x))},
		{-1, 0, 0,  (g[0][1]*(dr.x))},
		{ 0, 1, 0, -(g[1][0]*(dr.y))},
		{ 0,-1, 0,  (g[1][1]*(dr.y))},
		{ 0, 0, 1, -(g[2][0]*(dr.z))},
		{ 0, 0,-1,  (g[2][1]*(dr.z))},
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

	// get the view direction
	vec3d view(0, 0, 1);
	quatd q = rc.m_cam->GetOrientation();
	q.Inverse().RotateVector(view);

	mat3d Q = GetImageModel()->GetOrientation();
	mat3d Qt = Q.transpose();
	view = Qt * view;

	// the normal will be view direction
	glNormal3d(view.x, view.y, view.z);

	// update geometry and render
	UpdateGeometry(view);
	m_mesh.Render(GLMesh::FLAG_TEXTURE);

	// clean up
	glUseProgram(0);

	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_CLIP_PLANE1);
	glDisable(GL_CLIP_PLANE2);
	glDisable(GL_CLIP_PLANE3);
	glDisable(GL_CLIP_PLANE4);
	glDisable(GL_CLIP_PLANE5);

	glPopAttrib();
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
	GLdouble vr[3];
	GLdouble vt[3];
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
