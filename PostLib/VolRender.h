/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#pragma once
#include <ImageLib/3DImage.h>
#include <ImageLib/RGBAImage.h>
#include <FSCore/box.h>
#include "GLImageRenderer.h"
#include "ColorMap.h"

namespace Post {

class CImageModel;

class CVolRender : public CGLImageRenderer
{
	enum { ALPHA_SCALE, MIN_INTENSITY, MAX_INTENSITY, MIN_ALPHA, MAX_ALPHA, AMIN, AMAX, COLOR_MAP, LIGHTING, LIGHTING_STRENGTH, AMBIENT, SPECULAR, LIGHT_POS };

public:
	CVolRender(CImageModel* img);
	virtual ~CVolRender();

	void Clear();

	void Create();

	void Update() override;

	void Render(CGLContext& rc) override;

	void Reset();

	vec3d GetLightPosition();
	void SetLightPosition(const vec3d& r);

	int GetColorMap() const { return m_Col.GetColorMap(); }
	void SetColorMap(int n) { m_Col.SetColorMap(n); }

	bool UpdateData(bool bsave = true) override;

protected:
	void RenderX(int inc);
	void RenderY(int inc);
	void RenderZ(int inc);

	void Colorize(CRGBAImage& imd, CImage& ims);

	void CalcAttenuation();
	void DepthCueX(CRGBAImage& im, int n);
	void DepthCueY(CRGBAImage& im, int n);
	void DepthCueZ(CRGBAImage& im, int n);

	void UpdateVolRender();
	void UpdateRGBImages();

public:
	Post::CColorTexture	m_Col;		//!< color texture
	GLColor	m_amb;				//!< ambient color
	GLColor m_spc;				//!< specular color
	int		m_I0, m_I1;			// intensity range
	int		m_A0, m_A1;			// transparency range
	int		m_Amin, m_Amax;		// clamp transparency levels
	double	m_alpha;			// alpha scale factor
	double	m_shadeStrength;
	bool	m_blight;			// use lighting

protected:
	C3DImage		m_im3d;	// resampled 3D image data
	C3DImage		m_att;	// attenuation map (for lighting)

	CImage*	m_sliceX;
	CImage*	m_sliceY;
	CImage*	m_sliceZ;

	CRGBAImage*	m_pImx;	// Image array in x-direction
	CRGBAImage*	m_pImy;	// Image array in y-direction
	CRGBAImage*	m_pImz;	// Image array in x-direction
	unsigned int m_texID;

	int m_nx;	// nr of images in x-direction
	int m_ny;	// nr of images in y-direction
	int	m_nz;	// nr of images in z-direction

	double m_ax, m_ay, m_az;	//!< alpha scale factors

	bool	m_bcalc_lighting;	//!< calculate shading?
	vec3d	m_light;	// light direction

	int	m_LUT[256], m_LUTC[4][256];
};
}
