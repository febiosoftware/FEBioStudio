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
#include "PostLib/DataMap.h"
#include <GLLib/GLMesh.h>

namespace Post {

class CGLIsoSurfacePlot : public CGLPlot
{
	enum { DATA_FIELD, COLOR_MAP, TRANSPARENCY, CLIP, HIDDEN, SLICES, LEGEND, SMOOTH, RANGE_TYPE, USER_MAX, USER_MIN };

public:
	enum RANGE_TYPE {
		RNG_DYNAMIC,
		RNG_STATIC,
		RNG_USER
	};

public:
	CGLIsoSurfacePlot();

	int GetSlices();
	void SetSlices(int nslices);

	void Render(GLRenderEngine& re, GLContext& rc) override;

	void Update(int ntime, float dt, bool breset);

	bool RenderSmooth() { return m_bsmooth; }
	void RenderSmooth(bool b) { m_bsmooth = b; }

	int GetEvalField() { return m_nfield; }
	void SetEvalField(int n);

	CColorTexture* GetColorMap() { return &m_Col; }

	bool CutHidden() { return m_bcut_hidden; }
	void CutHidden(bool b) { m_bcut_hidden = b; }

	void UpdateTexture() { m_Col.UpdateTexture(); }

	void SetRangeType(int n) { m_rangeType = n; }
	int GetRangeType() const { return m_rangeType; }

	void SetUserRangeMin(double rangeMin) { m_userMin = rangeMin; }
	double GetUserRangeMin() const { return m_userMin; }

	void SetUserRangeMax(double rangeMax) { m_userMax = rangeMax; }
	double GetUserRangeMax() const { return m_userMax; }

	void Update();

	bool UpdateData(bool bsave = true);

protected:
	void UpdateMesh();
	void UpdateSlice(GLMesh& mesh, float ref, GLColor col);

protected:
	int		m_nslices;		// nr. of iso surface slices
	bool	m_bsmooth;		// render smooth or not
	bool	m_bcut_hidden;	//!< cut hidden materials or not
	double	m_transparency;

	int		m_rangeType;				//!< dynamic, static, or user-defined
	double	m_userMin, m_userMax;		//!< range for user-defined range

	double	m_rngMin, m_rngMax;

	int				m_nfield;	// data field
	CColorTexture	m_Col;		// colormap

	vector<vec2f>	m_rng;	// value range
	DataMap<float>	m_map;	// nodal values map
	VectorMap		m_GMap;	// nodal gradient values map

	vec2f			m_crng;
	vector<float>	m_val;	// current nodal values
	vector<vec3f>	m_grd;	// current gradient values

	GLMesh	m_renderMesh; // the mesh to render

	int		m_lastTime;
	float	m_lastdt;
};
}
