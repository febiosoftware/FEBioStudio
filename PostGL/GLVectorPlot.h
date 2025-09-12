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

#pragma once
#include "GLPlot.h"

class GLRenderEngine;

namespace Post {

class CGLVectorPlot : public CGLPlot 
{
	enum { 
		DATA_FIELD, 
		COLOR_MAP, 
		CLIP, 
		SHOW_HIDDEN, 
		DENSITY, 
		GLYPH, 
		GLYPH_COLOR, 
		SOLID_COLOR, 
		NORMALIZE, 
		AUTO_SCALE, 
		SCALE,
		ASPECT_RATIO,
		RANGE_TYPE,
		USER_MAX,
		USER_MIN
	};

public:
	// glyph types	
	enum Glyph_Type {
		GLYPH_ARROW,
		GLYPH_CONE,
		GLYPH_CYLINDER,
		GLYPH_SPHERE,
		GLYPH_BOX,
		GLYPH_LINE
	};

	// glyph color types
	enum Glyph_Color_Type {
		GLYPH_COL_SOLID,
		GLYPH_COL_LENGTH,
		GLYPH_COL_ORIENT
	};

public:
	CGLVectorPlot();

	void Render(GLRenderEngine& re, GLContext& rc) override;

	void SetScaleFactor(float g) { m_scale = g; }
	double GetScaleFactor() { return m_scale; }

	void SetDensity(float d) { m_dens = d; }
	double GetDensity() { return m_dens; }

	int GetVectorField() { return m_nvec; }
	void SetVectorField(int ntype);

	int GetGlyphType() { return m_nglyph; }
	void SetGlyphType(int ntype) { m_nglyph = ntype; }

	int GetColorType() { return m_ncol; }
	void SetColorType(int ntype) { m_ncol = ntype; }

	GLColor GetGlyphColor() { return m_gcl; }
	void SetGlyphColor(GLColor c) { m_gcl = c; }

	bool NormalizeVectors() { return m_bnorm; }
	void NormalizeVectors(bool b) { m_bnorm = b; }

	bool GetAutoScale() { return m_bautoscale; }
	void SetAutoScale(bool b) { m_bautoscale = b; }

	bool ShowHidden() const { return m_bshowHidden; }
	void ShowHidden(bool b) { m_bshowHidden = b; }

	CColorTexture* GetColorMap() { return &m_Col; }

	void Update(int ntime, float dt, bool breset) override;

	void UpdateTexture() override { m_Col.UpdateTexture(); }

	bool UpdateData(bool bsave = true) override;

	void Update() override;

private:
	// render a vector n at position r
	void RenderVector(GLRenderEngine& re, const vec3f& r, vec3f v);

	void UpdateState(int nstate);

protected:
	float	m_scale;
	float	m_dens;
	int		m_seed;

	int		m_nvec;		// vector field
	int		m_nglyph;	// glyph type
	int		m_ncol;		// color type

	int		m_lastTime;
	float	m_lastDt;

	float	m_ar;

	bool	m_bnorm;		// normalize vectors or not
	bool	m_bautoscale;	// auto scale the vectors
	bool	m_bshowHidden;	// show vectors on hidden materials

	GLColor			m_gcl;	// glyph color (for GLYPH_COL_SOLID)
	CColorTexture	m_Col;	// glyph color (for not GLYPH_COL_SOLID)

	std::vector<vec2f>	m_rng;	// nodal ranges
	DataMap<vec3f>	m_map;	// nodal values map
	
	int				m_ntime;	// current time at which this plot is evaluated
	std::vector<vec3f>	m_val;	// current values
	vec2f			m_crng;	// current range

	int				m_rngType;
	double			m_usr[2];	// user range
	vec2f			m_staticRange;

	float			m_fscale;	// total scale factor for rendering
};
}
