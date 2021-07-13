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
#include "GLPlot.h"
#include <PostLib/FEState.h>
namespace Post {

//-----------------------------------------------------------------------------
// Line rendering of imported line data
class CGLLinePlot : public CGLPlot
{
	enum { DATA_FIELD, COLOR_MODE, SOLID_COLOR, COLOR_MAP, RENDER_MODE, LINE_WIDTH, SHOW_ALWAYS };

	enum COLOR_MODE {
		COLOR_SOLID,
		COLOR_SEGMENT,
		COLOR_LINE_DATA,
		COLOR_MODEL_DATA
	};

public:
	CGLLinePlot(CGLModel* po);
	virtual ~CGLLinePlot();

	void Render(CGLContext& rc) override;

	float GetLineWidth() { return m_line; }
	void SetLineWidth(float f) { m_line = f; }

	GLColor GetSolidColor() { return m_col; }
	void SetSolidColor(GLColor c) { m_col = c; }

	int GetRenderMode() { return m_nmode; }
	void SetRenderMode(int m) { m_nmode = m; }

	int GetColorMode() { return m_ncolor; }
	void SetColorMode(int m);

	int GetDataField() { return m_nfield; }
	void SetDataField(int n);

	CColorTexture* GetColorMap() { return &m_Col; }
	void UpdateTexture() override { m_Col.UpdateTexture(); }

	void Update(int ntime, float dt, bool breset) override;

	bool UpdateData(bool bsave = true) override;

protected:
	void RenderLines(FEState& s);
	void Render3DLines(FEState& s);
	void Render3DSmoothLines(FEState& s);
	bool ShowLine(LINEDATA& l, FEState& s);

private:
	float		m_line;		//!< line thickness
	GLColor		m_col;		//!< rendering color
	int			m_nmode;	//!< rendering mode
	int			m_ncolor;	//!< color option
	int			m_nfield;
	bool		m_show;		//!< hide when containing elements are hidden
	CColorTexture	m_Col;	//!< line color (when m_ncolor is not solid)

private:
	vec2f	m_rng;
};

//-----------------------------------------------------------------------------
// point cloud rendering of imported point data
class CGLPointPlot : public CGLPlot
{
	enum {MAX_SETTINGS = 4};

	enum { POINT_SIZE, COLOR, RENDER_MODE };

	struct SETTINGS
	{
		float		size;		//!< point size
		GLColor		col;		//!< rendering color
		int			nmode;		//!< rendering mode
		int			nvisible;	//!< visible flag
	};

public:
	CGLPointPlot(CGLModel* po);
	virtual ~CGLPointPlot();

	void Render(CGLContext& rc) override;

	float GetPointSize(int n = 0) { return m_set[n].size; }
	void SetPointSize(float f, int n = 0) { m_set[n].size = f; }

	GLColor GetPointColor(int n = 0) { return m_set[n].col; }
	void SetPointColor(GLColor c, int n = 0) { m_set[n].col = c; }

	int GetRenderMode(int n = 0) { return m_set[n].nmode; }
	void SetRenderMode(int m, int n = 0) { m_set[n].nmode = m; }

	int GetVisible(int n = 0) { return m_set[n].nvisible; }
	void SetVisible(int nvis, int n = 0) { m_set[n].nvisible = nvis; }

	bool UpdateData(bool bsave = true) override;

private:
	SETTINGS	m_set[MAX_SETTINGS];
};
}
