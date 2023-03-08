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
#include "ImageModel.h"
#include "ImageSource.h"
#include <GLLib/GLContext.h>
#include <GLLib/GLCamera.h>
#include <ImageLib/3DImage.h>
#include <FEBioStudio/ImageViewSettings.h>
#include <sstream>
#include <limits>
using namespace Post;

static int n = 1;

CVolumeRenderer::CVolumeRenderer(CImageModel* img) : CGLImageRenderer(img)
{
	// AddDoubleParam(0.1, "alpha scale")->SetFloatRange(0.0, 1.0);
	// AddDoubleParam(0.5, "min intensity")->SetFloatRange(0.0, 1.0);
	// AddDoubleParam(1.0, "max intensity")->SetFloatRange(0.0, 1.0);
	AddChoiceParam(0, "Color map")->SetEnumNames("Grayscale\0Red\0Green\0Blue\0Fire\0");

	m_texID = 0;
	m_prgID = 0;

	std::stringstream ss;
	ss << "VolumeRender" << n++;
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

	C3DImage& im3d = *src->Get3DImage();

	// get the original image dimensions
	int nx = im3d.Width();
	int ny = im3d.Height();
	int nz = im3d.Depth();

    int pType = im3d.PixelType();

	// find the min and max values
    size_t N  = nx * ny * nz;
    if(pType == C3DImage::INT_RGB8 || pType == C3DImage::UINT_RGB8 || pType == C3DImage::INT_RGB16 || pType == C3DImage::UINT_RGB16)
    {
        N *= 3;
    }

    int max8 = 255;
    int max16 = 65535;

    float max;
    float min;
	if (pType == C3DImage::INT_8 || pType == C3DImage::INT_RGB8)
	{
        int8_t* data = (int8_t*)im3d.GetBytes();

		max = (float)*std::max_element(data, data+N)/(max8/2);
        min = (float)*std::min_element(data, data+N)/(max8/2);
    }
    else if(pType == C3DImage::UINT_8 || pType == C3DImage::UINT_RGB8)
    {
        uint8_t* data = (uint8_t*)im3d.GetBytes();

        max = (float)*std::max_element(data, data+N)/max8;
        min = (float)*std::min_element(data, data+N)/max8;
    }
    else if( pType == C3DImage::INT_16 || pType == C3DImage::INT_RGB16)
    {
		int16_t* data = (int16_t*)im3d.GetBytes();

		max = (float)*std::max_element(data, data+N)/(max16/2);
        min = (float)*std::min_element(data, data+N)/(max16/2);
	}
    else if(pType == C3DImage::UINT_16 || pType == C3DImage::UINT_RGB16)
    {
        uint16_t* data = (uint16_t*)im3d.GetBytes();

		max = (float)*std::max_element(data, data+N)/max16;
        min = (float)*std::min_element(data, data+N)/max16;
    }

    m_Iscale = 1/(max - min);
    m_IscaleMin = min;


	glBindTexture(GL_TEXTURE_3D, m_texID);

	//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// set texture parameter for 2D textures
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	int n = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &n);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	switch (im3d.PixelType())
	{
	case C3DImage::INT_8  : glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_BYTE , im3d.GetBytes()); break;
	case C3DImage::INT_16 : glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_SHORT, im3d.GetBytes()); break;
	case C3DImage::INT_RGB8: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_BYTE , im3d.GetBytes()); break;
	case C3DImage::INT_RGB16: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_SHORT, im3d.GetBytes()); break;
    case C3DImage::UINT_8  : glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE , im3d.GetBytes()); break;
	case C3DImage::UINT_16 : glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_SHORT, im3d.GetBytes()); break;
	case C3DImage::UINT_RGB8: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_UNSIGNED_BYTE , im3d.GetBytes()); break;
	case C3DImage::UINT_RGB16: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_UNSIGNED_SHORT, im3d.GetBytes()); break;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, n);

	// allocate vertex arrays
	double wx = (double)nx;
	double wy = (double)ny;
	double wz = (double)nz;
	double wt = 2 * sqrt(wx * wx + wy * wy + wz * wz);
	m_nslices = (int)wt;

	// max 5 triangles per slice, 3 vertex per triangle, (m_nslices+1) slices
	m_mesh.AllocVertexBuffers(5 * 3 * (m_nslices + 1), GLMesh::FLAG_VERTEX | GLMesh::FLAG_TEXTURE);
}

const char* shadertxt_8bit = \
"#version 120\n"\
"uniform sampler3D sampler;                               "\
"uniform float Imin;                                      "\
"uniform float Imax;                                      "\
"uniform float Iscl;                                      "\
"uniform float IsclMin;                                   "\
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
"	vec4 t = texture3D(sampler, gl_TexCoord[0].xyz);      "\
"   float f = (t.x-IsclMin)*Iscl;                         "\
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


const char* shadertxt_rgb = \
"#version 120\n"\
"uniform sampler3D sampler;                               "\
"uniform float Imin;                                      "\
"uniform float Iscl;                                      "\
"uniform float IsclMin;                                   "\
"uniform float Imax;                                      "\
"uniform float Amin;                                      "\
"uniform float Amax;                                      "\
"uniform float gamma;                                     "\
"uniform vec3 col1;                                       "\
"uniform vec3 col2;                                       "\
"uniform vec3 col3;                                       "\
"uniform int cmap;                                        "\
"void main(void)                                          "\
"{                                                        "\
"   vec4 t = (texture(sampler, gl_TexCoord[0].xyz)-IsclMin)*Iscl;"\
"   t.x = (t.x - Imin) / (Imax - Imin);                   "\
"   t.x = clamp(t.x, 0.0, 1.0);                           "\
"   t.y = (t.y - Imin) / (Imax - Imin);                   "\
"   t.y = clamp(t.y, 0.0, 1.0);                           "\
"   t.z = (t.z - Imin) / (Imax - Imin);                   "\
"   t.z = clamp(t.z, 0.0, 1.0);                           "\
"   float f = t.x;                                        "\
"   if (t.y > f) f = t.y;                                 "\
"   if (t.z > f) f = t.z;                                 "\
"   if (f <= 0.0) discard;                                "\
"   if (gamma != 1.0) f = pow(f, gamma);                  "\
"   float a = Amin + f*(Amax - Amin);                     "\
"   vec3 c1 = vec3(t.x*col1.x, t.x*col1.y, t.x*col1.z);   "\
"   vec3 c2 = vec3(t.y*col2.x, t.y*col2.y, t.y*col2.z);   "\
"   vec3 c3 = vec3(t.z*col3.x, t.z*col3.y, t.z*col3.z);   "\
"   vec3 c4 = c1 + c2 + c3;                               "\
"   gl_FragColor = gl_Color*vec4(c4.x, c4.y, c4.z, a);    "\
"}                                                        "\
"";

const char* vertex_shader = \
"#version 130\n"\
"void main(void)                                          "\
"{                                                        "\
"   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;   "\
"}                                                        "\
"";

void CVolumeRenderer::InitShaders()
{
	// load texture data
	CImageModel& img = *GetImageModel();
	CImageSource* src = img.GetImageSource();
	if (src == nullptr) return;

	C3DImage& im3d = *src->Get3DImage();

	const char* shadertxt = nullptr;
	switch (im3d.PixelType())
	{
    case C3DImage::UINT_8  : shadertxt = shadertxt_8bit; break;
	case C3DImage::INT_8  : shadertxt = shadertxt_8bit; break;
	case C3DImage::UINT_16  : shadertxt = shadertxt_8bit; break;
    case C3DImage::INT_16 : shadertxt = shadertxt_8bit; break;
	case C3DImage::UINT_RGB8  : shadertxt = shadertxt_rgb; break;
    case C3DImage::INT_RGB8: shadertxt = shadertxt_rgb; break;
	case C3DImage::UINT_RGB16  : shadertxt = shadertxt_rgb; break;
    case C3DImage::INT_RGB16: shadertxt = shadertxt_rgb; break;
	default:
		return;
	}

	// create the fragment shader
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
//	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);

	// set the shader text
	glShaderSource(fragShader, 1, &shadertxt, NULL);
//	glShaderSource(vertShader, 1, &vertex_shader, NULL);

	// compile the shader
	int success;
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		const int MAX_INFO_LOG_SIZE = 1024;
		GLchar infoLog[MAX_INFO_LOG_SIZE];
		glGetShaderInfoLog(fragShader, MAX_INFO_LOG_SIZE, NULL, infoLog);
		fprintf(stderr, infoLog);
	}

//	glCompileShader(vertShader);
//	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
//	if (success == 0)
//	{
//		const int MAX_INFO_LOG_SIZE = 1024;
//		GLchar infoLog[MAX_INFO_LOG_SIZE];
//		glGetShaderInfoLog(vertShader, MAX_INFO_LOG_SIZE, NULL, infoLog);
//		fprintf(stderr, infoLog);
//	}

	// create the program
	m_prgID = glCreateProgram();
	glAttachShader(m_prgID, fragShader);
//	glAttachShader(m_prgID, vertShader);

	glLinkProgram(m_prgID);
	glGetProgramiv(m_prgID, GL_LINK_STATUS, &success); 

	glDeleteShader(fragShader);
//	glDeleteShader(vertShader);
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

void CVolumeRenderer::Render(CGLContext& rc)
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
	glUseProgram(m_prgID);

	GLint IminID = glGetUniformLocation(m_prgID, "Imin");
	GLint ImaxID = glGetUniformLocation(m_prgID, "Imax");
	GLint AminID = glGetUniformLocation(m_prgID, "Amin");
	GLint AmaxID = glGetUniformLocation(m_prgID, "Amax");
	GLint cmapID = glGetUniformLocation(m_prgID, "cmap");
	GLint gammaID = glGetUniformLocation(m_prgID, "gamma");
	GLint IsclID   = glGetUniformLocation(m_prgID, "Iscl");
    GLint IsclMinID   = glGetUniformLocation(m_prgID, "IsclMin");

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
	glUniform1f(IsclID, m_Iscale);
    glUniform1f(IsclMinID, m_IscaleMin);

	if ((im3d.PixelType() == C3DImage::INT_RGB8) || (im3d.PixelType() == C3DImage::UINT_RGB8) 
        || (im3d.PixelType() == C3DImage::INT_RGB16) || (im3d.PixelType() == C3DImage::UINT_RGB16))
	{
		GLint col1ID = glGetUniformLocation(m_prgID, "col1");
		GLint col2ID = glGetUniformLocation(m_prgID, "col2");
		GLint col3ID = glGetUniformLocation(m_prgID, "col3");

		float hue1 = vs->GetFloatValue(CImageViewSettings::CHANNEL1_HUE);
		float hue2 = vs->GetFloatValue(CImageViewSettings::CHANNEL2_HUE);
		float hue3 = vs->GetFloatValue(CImageViewSettings::CHANNEL3_HUE);

		GLColor col1 = HSV2RGB(360.0 * hue1, 1.0, 1.0);
		GLColor col2 = HSV2RGB(360.0 * hue2, 1.0, 1.0);
		GLColor col3 = HSV2RGB(360.0 * hue3, 1.0, 1.0);

		float c1[3] = { col1.r / 255.f, col1.g / 255.f, col1.b / 255.f };
		float c2[3] = { col2.r / 255.f, col2.g / 255.f, col2.b / 255.f };
		float c3[3] = { col3.r / 255.f, col3.g / 255.f, col3.b / 255.f };

		glUniform3f(col1ID, c1[0], c1[1], c1[2]);
		glUniform3f(col2ID, c2[0], c2[1], c2[2]);
		glUniform3f(col3ID, c3[0], c3[1], c3[2]);
	}

	double g[3][2] = {
		{vs->GetFloatValue(CImageViewSettings::CLIPX_MIN), vs->GetFloatValue(CImageViewSettings::CLIPX_MAX)},
		{vs->GetFloatValue(CImageViewSettings::CLIPY_MIN), vs->GetFloatValue(CImageViewSettings::CLIPY_MAX)},
		{vs->GetFloatValue(CImageViewSettings::CLIPZ_MIN), vs->GetFloatValue(CImageViewSettings::CLIPZ_MAX)}
	};

	// get the bounding box
	BOX box = img.GetBoundingBox();
	vec3d r0 = box.r0();
	vec3d r1 = box.r1();

	// activate clipping planes
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

	// get the view direction
	vec3d view(0, 0, 1);
	quatd q = rc.m_cam->GetOrientation();
	q.Inverse().RotateVector(view);

	// the normal will be view direction
	glNormal3d(view.x, view.y, view.z);

	// update geometry and render
	UpdateGeometry(view);
	m_mesh.Render();

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
	vec3d r0 = box.r0();
	vec3d r1 = box.r1();

	// coordinates of box
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

				vr[0] = c[n1].x * (1 - w) + c[n2].x * w;
				vr[1] = c[n1].y * (1 - w) + c[n2].y * w;
				vr[2] = c[n1].z * (1 - w) + c[n2].z * w;

				vt[0] = (vr[0] - r0.x) / W;
				vt[1] = (vr[1] - r0.y) / H;
				vt[2] = (vr[2] - r0.z) / D;

				m_mesh.AddVertex(vr, nullptr, vt);
			}

			// on to the next face
			pf += 3;
		}
	}
	m_mesh.EndMesh();
}
