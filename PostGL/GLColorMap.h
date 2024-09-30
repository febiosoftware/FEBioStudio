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
#include "GLDataMap.h"
#include <GLWLib/GLLegendBar.h>
#include <GLLib/GLTexture1D.h>
#include <PostLib/ColorMap.h>

namespace Post {

class CGLModel;

//-----------------------------------------------------------------------------

class CGLColorMap : public CGLDataMap
{
	enum { DATA_FIELD, DATA_SMOOTH, COLOR_MAP, NODAL_VALS, RANGE_DIVS, SHOW_LEGEND, MAX_RANGE_TYPE, USER_MAX, MIN_RANGE_TYPE, USER_MIN, SHOW_MINMAX_MARKERS, INACTIVE_COLOR };

public:
	CGLColorMap(CGLModel* po);
	~CGLColorMap();

	Post::CColorTexture* GetColorMap() { return &m_Col; }

	void Update(int ntime, float dt, bool breset) override;

	int GetEvalField() const { return m_nfield; }
	void SetEvalField(int n);

	void GetRange(float* pd) { pd[0] = m_range.min; pd[1] = m_range.max; }
	int GetMaxRangeType() { return m_range.maxtype; }
	int GetMinRangeType() { return m_range.mintype; }
	vec3d GetMinPosition() const { return m_rmin; }
	vec3d GetMaxPosition() const { return m_rmax; }
	bool ShowMinMaxMarkers() const;

	void SetRange(float* pd) { m_range.min = pd[0]; m_range.max = pd[1]; }
	void SetRangeMax(float f) { m_range.max = f; }
	void SetRangeMin(float f) { m_range.min = f; }
//	void SetRangeType(int n) { m_range.maxtype = m_range.mintype = n; m_breset = true; }
	void SetMaxRangeType(int n) { m_range.maxtype = n; m_breset = true; }
	void SetMinRangeType(int n) { m_range.mintype = n; m_breset = true; }

	void DisplayNodalValues(bool b) { m_bDispNodeVals = b; }
	bool DisplayNodalValues() { return m_bDispNodeVals; }

	bool ShowLegend() { return m_pbar->visible(); }
	void ShowLegend(bool b) { if (b) m_pbar->show(); else m_pbar->hide(); }

	bool GetColorSmooth();
	void SetColorSmooth(bool b);

	GLColor GetInactiveColor();

	void Activate(bool b) override { CGLObject::Activate(b); ShowLegend(b); }

private:
	void UpdateState(int ntime, bool breset);

	bool UpdateData(bool bsave = true) override;

	void Update() override;

protected:
	int		m_nfield;
	bool	m_breset;	// reset the range when the field has changed
	DATA_RANGE	m_range;	// range for legend
	vec3d	m_rmin, m_rmax;	// global indicators of min, max

public:
	bool	m_bDispNodeVals;	// render nodal values

	Post::CColorTexture	m_Col;	// colormap used for rendering

	GLLegendBar*	m_pbar;	// the legend bar

	static int	m_defaultRngType;
};
}
