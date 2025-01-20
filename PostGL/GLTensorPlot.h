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
#include "stdafx.h"
#include "GLPlot.h"

namespace Post {

class GLTensorPlot : public CGLLegendPlot
{
	enum { DATA_FIELD, METHOD, COLOR_MAP, RANGE_DIVS, CLIP, SHOW_HIDDEN, SCALE, DENSITY, GLYPH, GLYPH_COLOR, SOLID_COLOR, AUTO_SCALE, NORMALIZE, MAX_RANGE_TYPE, USER_MAX, MIN_RANGE_TYPE, USER_MIN };

public:
	enum Glyph_Type {
		Glyph_Arrow,
		Glyph_Line,
		Glyph_Sphere,
		Glyph_Box
	};

	enum Glyph_Color_Type {
		Glyph_Col_Solid,
		Glyph_Col_Norm,
		Glyph_Col_Anisotropy
	};

	enum Vector_Method {
		VEC_EIGEN,
		VEC_COLUMN,
		VEC_ROW
	};

private:
	struct TENSOR {
		vec3f	r[3];	// principal directions
		float	l[3];	// eigen values
		float	f;		// "norm"
	};

public:
	GLTensorPlot();

	void Render(GLRenderEngine& re, GLContext& rc) override;

	CColorTexture* GetColorMap();

	void Update(int ntime, float dt, bool breset) override;

	bool UpdateData(bool bsave = true) override;

public:
	int GetTensorField() { return m_ntensor; }
	void SetTensorField(int nfield);

	int GetVectorMethod() const { return m_nmethod; }
	void SetVectorMethod(int m);

	void SetScaleFactor(float g) { m_scale = g; }
	double GetScaleFactor() { return m_scale; }

	void SetDensity(float d) { m_dens = d; }
	double GetDensity() { return m_dens; }

	bool ShowHidden() const { return m_bshowHidden; }
	void ShowHidden(bool b) { m_bshowHidden = b; }

	int GetGlyphType() { return m_nglyph; }
	void SetGlyphType(int ntype) { m_nglyph = ntype; }

	int GetColorType() { return m_ncol; }
	void SetColorType(int ntype) { m_ncol = ntype; }

	GLColor GetGlyphColor() { return m_gcl; }
	void SetGlyphColor(GLColor c) { m_gcl = c; }

	bool GetAutoScale() { return m_bautoscale; }
	void SetAutoScale(bool b) { m_bautoscale = b; }

	bool GetNormalize() { return m_bnormalize; }
	void SetNormalize(bool b) { m_bnormalize = b; }

protected:
	void RenderGlyphs(GLRenderEngine& re, TENSOR& t, float scale);
	void RenderArrows(GLRenderEngine& re, TENSOR& t, float scale);
	void RenderLines(GLRenderEngine& re, TENSOR& t, float scale);
	void RenderSphere(GLRenderEngine& re, TENSOR& t, float scale);
	void RenderBox(GLRenderEngine& re, TENSOR& t, float scale);

	void Update() override;

protected:
	int		m_ntensor;	// tensor field
	int		m_nglyph;	// glyph type
	int		m_ncol;		// color type
	int		m_ndivs;	// range divisions
	DATA_RANGE	m_range;

	float	m_scale;
	float	m_dens;
	int		m_seed;

	bool	m_bshowHidden;	// show tensors on hidden materials
	bool	m_bautoscale;	// auto scale the vectors
	bool	m_bnormalize;

	int				m_ntime;
	DataMap<TENSOR>	m_map;	// nodal values map
	std::vector<TENSOR>	m_val;	// current nodal values

	GLColor			m_gcl;	// glyph color (for GLYPH_COL_SOLID)
	CColorTexture	m_Col;	// glyph color (for not GLYPH_COL_SOLID)

	int		m_nmethod;

	int		m_lastTime;
	float	m_lastDt;
	int		m_lastCol;
};
}
