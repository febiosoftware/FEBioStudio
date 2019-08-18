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
#include "GLContext.h"
#include "ImageModel.h"
#include "PropertyList.h"
#include <sstream>
using namespace Post;

//-----------------------------------------------------------------------------
class CGLVolRenderProps : public CPropertyList
{
public:
	CGLVolRenderProps(CVolRender* vr) : m_vr(vr)
	{
		QStringList cols;

		for (int i = 0; i<ColorMapManager::ColorMaps(); ++i)
		{
			string name = ColorMapManager::GetColorMapName(i);
			cols << name.c_str();
		}

		addProperty("alpha scale", CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("min intensity", CProperty::Int)->setIntRange(0, 255);
		addProperty("max intensity", CProperty::Int)->setIntRange(0, 255);
		addProperty("min alpha", CProperty::Int)->setIntRange(0, 255);
		addProperty("max alpha", CProperty::Int)->setIntRange(0, 255);
		addProperty("Amin", CProperty::Int)->setIntRange(0, 255);
		addProperty("Amax", CProperty::Int)->setIntRange(0, 255);
		addProperty("Color map", CProperty::Enum)->setEnumValues(cols);
		addProperty("Lighting effect", CProperty::Bool);
		addProperty("Lighting strength", CProperty::Float);
		addProperty("Ambient color", CProperty::Color);
		addProperty("Specular color", CProperty::Color);
		//		addProperty("Light direction", CProperty::DataVec3);
	}

	QVariant GetPropertyValue(int i)
	{
		switch (i)
		{
		case  0: return m_vr->m_alpha; break;
		case  1: return m_vr->m_I0; break;
		case  2: return m_vr->m_I1; break;
		case  3: return m_vr->m_A0; break;
		case  4: return m_vr->m_A1; break;
		case  5: return m_vr->m_Amin; break;
		case  6: return m_vr->m_Amax; break;
		case  7: return m_vr->GetColorMap(); break;
		case  8: return m_vr->m_blight; break;
		case  9: return m_vr->m_shadeStrength; break;
		case 10: return toQColor(m_vr->m_amb); break;
		case 11: return toQColor(m_vr->m_spc); break;
			//		case 10: return m_vr->GetLightPosition(); break;
		}
		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& val)
	{
		switch (i)
		{
		case  0: m_vr->m_alpha = val.toFloat(); break;
		case  1: m_vr->m_I0 = val.toInt(); break;
		case  2: m_vr->m_I1 = val.toInt(); break;
		case  3: m_vr->m_A0 = val.toInt(); break;
		case  4: m_vr->m_A1 = val.toInt(); break;
		case  5: m_vr->m_Amin = val.toInt(); break;
		case  6: m_vr->m_Amax = val.toInt(); break;
		case  7: m_vr->SetColorMap(val.value<int>()); break;
		case  8: m_vr->m_blight = val.toBool(); break;
		case  9: m_vr->m_shadeStrength = val.toFloat(); break;
		case 10: m_vr->m_amb = toGLColor(val.value<QColor>()); break;
		case 11: m_vr->m_spc = toGLColor(val.value<QColor>()); break;
			//		case 10: break;
		}

		m_vr->Update();
	}

private:
	CVolRender*	m_vr;
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVolRender::CVolRender(CImageModel* img) : CGLImageRenderer(img)
{
	static int n = 1;
	stringstream ss;
	ss << "VolumeRender" << n++;
	SetName(ss.str());

	m_pImx = m_pImy = m_pImz = 0;
	m_sliceX = m_sliceY = m_sliceZ = 0;
	m_nx = m_ny = m_nz = 0;

	m_blight = false;
	m_bcalc_lighting = true;
	m_light = vec3f(-1.f, -1.f, -1.f);

	m_spc = GLColor(255, 255, 255);

	m_texID = 0;

	Reset();
}

CVolRender::~CVolRender()
{
	Clear();
}

CPropertyList* CVolRender::propertyList()
{
	return new CGLVolRenderProps(this);
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
	C3DImage& im3d = *img.Get3DImage();

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
/*	BOUNDINGBOX b = img.GetBoundingBox();
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

	vec3f l = m_light; l.Normalize();

	BOUNDINGBOX& b = GetImageModel()->GetBoundingBox();

	C3DGradientMap map(m_im3d, b);

#pragma omp parallel for default(shared)
	for (int k = 0; k < m_nz; ++k)
	{
		for (int j = 0; j < m_ny; ++j)
			for (int i = 0; i < m_nx; ++i)
			{
				vec3f f = map.Value(i, j, k); f.Normalize();
				float a = f*l;
				if (a < 0.f) a = 0.f;
				m_att.value(i, j, k) = (byte)(255.f*a);
			}
	}
}

//-----------------------------------------------------------------------------
//! Update texture images for volume rendering
void CVolRender::Update()
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
	byte* ps = ims.GetBytes();
	byte* pd = imd.GetBytes();
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

	byte* p = im.GetBytes();
	for (int j=0; j<ny; ++j)
		for (int i=0; i<nx; ++i, p += 4)
		{
			float a = m_att.value(n, i, j) / 255.f;
			float w = m_shadeStrength*a + (1.f - m_shadeStrength);
			float s = m_shadeStrength*a*a;
			p[0] = (byte) (((p[0]*(1.f - s) + s*m_spc.r)*w + m_amb.r*(1.f - w)));
			p[1] = (byte) (((p[1]*(1.f - s) + s*m_spc.g)*w + m_amb.g*(1.f - w)));
			p[2] = (byte) (((p[2]*(1.f - s) + s*m_spc.r)*w + m_amb.b*(1.f - w)));
		}
}

//-----------------------------------------------------------------------------
//! Attenuate Y-textures
void CVolRender::DepthCueY(CRGBAImage& im, int n)
{
	int nx = im.Width();
	int ny = im.Height();
	int nz = m_att.Height();

	byte* p = im.GetBytes();
	for (int j=0; j<ny; ++j)
		for (int i=0; i<nx; ++i, p += 4)
		{
			float a = m_att.value(i, n, j) / 255.f;
			float w = m_shadeStrength*a + (1.f - m_shadeStrength);
			float s = m_shadeStrength*a*a;
			p[0] = (byte) (((p[0]*(1.f - s) + s*m_spc.r)*w + m_amb.r*(1.f - w)));
			p[1] = (byte) (((p[1]*(1.f - s) + s*m_spc.g)*w + m_amb.g*(1.f - w)));
			p[2] = (byte) (((p[2]*(1.f - s) + s*m_spc.r)*w + m_amb.b*(1.f - w)));
		}
}

//-----------------------------------------------------------------------------
//! Attenuate Z-textures
void CVolRender::DepthCueZ(CRGBAImage& im, int n)
{
	int nx = im.Width();
	int ny = im.Height();
	int nz = m_att.Depth();

	byte* p = im.GetBytes();
	for (int j=0; j<ny; ++j)
		for (int i=0; i<nx; ++i, p += 4)
		{
			float a = m_att.value(i, j, n) / 255.f;
			float w = m_shadeStrength*a + (1.f - m_shadeStrength);
			float s = m_shadeStrength*a*a;
			p[0] = (byte) (((p[0]*(1.f - s) + s*m_spc.r)*w + m_amb.r*(1.f - w)));
			p[1] = (byte) (((p[1]*(1.f - s) + s*m_spc.g)*w + m_amb.g*(1.f - w)));
			p[2] = (byte) (((p[2]*(1.f - s) + s*m_spc.r)*w + m_amb.b*(1.f - w)));
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

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	
	vec3f r(0,0,1);
	quat4f q = rc.m_q;
	q.Inverse().RotateVector(r);

	double x = fabs(r.x);
	double y = fabs(r.y);
	double z = fabs(r.z);

	if ((x > y) && (x > z)) { RenderX(r.x > 0 ? 1 : -1); }
	if ((y > x) && (y > z)) { RenderY(r.y > 0 ? 1 : -1); }
	if ((z > y) && (z > x)) { RenderZ(r.z > 0 ? 1 : -1); }

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
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

	BOUNDINGBOX& box = GetImageModel()->GetBoundingBox();

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

	BOUNDINGBOX& box = GetImageModel()->GetBoundingBox();

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

	BOUNDINGBOX& box = GetImageModel()->GetBoundingBox();

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
