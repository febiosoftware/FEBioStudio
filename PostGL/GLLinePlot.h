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
#include <PostLib/FEState.h>
namespace Post {

//-----------------------------------------------------------------------------
struct LINESEGMENT
{
	// nodal coordinates
	vec3f	m_r0;
	vec3f	m_r1;

	// tangents
	vec3d	m_t0;
	vec3d	m_t1;

	// values
	float	m_val[2];
	float	m_user_data[2];
	int		m_elem[2];

	// segment ID
	int	m_segId;

	// flags to identify ends
	int m_end[2];
};

class LineData
{
public:
	LineData() {}

	int Lines() const { return (int)m_Line.size(); }
	LINESEGMENT& Line(int n) { return m_Line[n]; }
	const LINESEGMENT& Line(int n) const { return m_Line[n]; }

	void Add(LINESEGMENT& line) { m_Line.push_back(line); }

	void processLines();

	void AddLine(vec3f a, vec3f b, float data_a = 0.f, float data_b = 0.f, int el0 = -1, int el1 = -1)
	{
		vec3f t = b - a; t.Normalize();

		LINESEGMENT L;
		L.m_segId = 0;
		L.m_r0 = a;
		L.m_r1 = b;
		L.m_t0 = L.m_t1 = t;
		L.m_user_data[0] = data_a;
		L.m_user_data[1] = data_b;
		L.m_elem[0] = el0;
		L.m_elem[1] = el1;
		L.m_end[0] = L.m_end[1] = 0;
		m_Line.push_back(L);
	}

private:
	vector<LINESEGMENT>	m_Line;
};

class LineDataModel;

class LineDataSource
{
public:
	LineDataSource(LineDataModel* mdl);
	virtual ~LineDataSource() {}

	virtual bool Load(const char* szfile) = 0;
	virtual bool Reload() = 0;

	LineDataModel* GetLineDataModel() { return m_mdl; }

private:
	LineDataModel* m_mdl;
};

class LineDataModel 
{
public:
	LineDataModel(FEPostModel* fem);
	~LineDataModel();

	void Clear();

	LineData& GetLineData(size_t n) { return m_line[n]; }

	int States() const { return (int)m_line.size(); }

	FEPostModel* GetFEModel() { return m_fem; }

	void SetLineDataSource(LineDataSource* src) { m_src = src; }
	LineDataSource* GetLineDataSource() { return m_src; }

	bool Reload() 
	{
		if (m_src) return m_src->Reload();
		else return false;
	}

private:
	LineDataSource* m_src;
	FEPostModel* m_fem;
	std::vector<LineData>	m_line;
};

//-----------------------------------------------------------------------------
// Line rendering of imported line data
class CGLLinePlot : public CGLLegendPlot
{
	enum { DATA_FIELD, COLOR_MODE, SOLID_COLOR, COLOR_MAP, RENDER_MODE, LINE_WIDTH, MAX_RANGE_TYPE, USER_MAX, MIN_RANGE_TYPE, USER_MIN, SHOW_ALWAYS, SHOW_LEGEND };

	enum COLOR_MODE {
		COLOR_SOLID,
		COLOR_SEGMENT,
		COLOR_LINE_DATA,
		COLOR_MODEL_DATA
	};

public:
	CGLLinePlot();
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

	void Reload() override;

public:
	void SetLineDataModel(LineDataModel* lineData);

protected:
	void RenderLines(FEState& s, int ntime);
	void Render3DLines(FEState& s, int ntime);
	void Render3DSmoothLines(FEState& s, int ntime);
	bool ShowLine(LINESEGMENT& l, FEState& s);

private:
	float		m_line;		//!< line thickness
	GLColor		m_col;		//!< rendering color
	int			m_nmode;	//!< rendering mode
	int			m_ncolor;	//!< color option
	int			m_nfield;
	bool		m_show;		//!< hide when containing elements are hidden
	DATA_RANGE	m_range;	// range for legend
	bool		m_showLegend;
	CColorTexture	m_Col;	//!< line color (when m_ncolor is not solid)

	LineDataModel* m_lineData;
};

//-----------------------------------------------------------------------------
#define MAX_POINT_DATA_FIELDS	32

class PointData
{
public:
	struct POINT
	{
		int		nlabel;
		vec3f	m_r;
		float	val[MAX_POINT_DATA_FIELDS];
	};

public:

	void AddPoint(vec3f a, int nlabel);

	void AddPoint(vec3f a, const std::vector<float>& data, int nlabel);

	int Points() const { return (int)m_pt.size(); }

	POINT& Point(int n) { return m_pt[n]; }

public:
	vector<POINT>	m_pt;
};

//-----------------------------------------------------------------------------
class PointDataSource;

class PointDataModel
{
public:
	PointDataModel(FEPostModel* fem);
	~PointDataModel();
	void Clear();

	FEPostModel* GetFEModel() { return m_fem; }

	void SetPointDataSource(PointDataSource* src) { m_src = src; }
	PointDataSource* GetPointDataSource() { return m_src; }

	void AddDataField(const std::string& dataName);
	std::vector<std::string> GetDataNames() { return m_dataNames; }

	int GetDataFields() const { return (int)m_dataNames.size(); }

	PointData& GetPointData(size_t state) { return m_point[state]; }

	bool Reload();

private:
	FEPostModel* m_fem;
	PointDataSource* m_src;
	std::vector<PointData>	m_point;
	std::vector<std::string>	m_dataNames;
};

class PointDataSource
{
public:
	PointDataSource(PointDataModel* mdl) : m_mdl(mdl) { mdl->SetPointDataSource(this); }
	virtual ~PointDataSource() {}

	virtual bool Load(const char* szfile) = 0;
	virtual bool Reload() = 0;

	PointDataModel* GetPointDataModel() { return m_mdl; }

private:
	PointDataModel* m_mdl;
};

//-----------------------------------------------------------------------------
// point cloud rendering of imported point data
class CGLPointPlot : public CGLLegendPlot
{
	enum { POINT_SIZE, RENDER_MODE, COLOR_MODE, SOLID_COLOR, COLOR_MAP, DATA_FIELD, SHOW_LEGEND, MAX_RANGE_TYPE, USER_MAX, MIN_RANGE_TYPE, USER_MIN};

public:
	CGLPointPlot();
	virtual ~CGLPointPlot();

	void Render(CGLContext& rc) override;

	float GetPointSize() { return m_pointSize; }
	void SetPointSize(float f) { m_pointSize = f; }

	GLColor GetPointColor() { return m_solidColor; }
	void SetPointColor(GLColor c) { m_solidColor = c; }

	bool UpdateData(bool bsave = true) override;

public:
	void SetPointDataModel(PointDataModel* pointData);

	void Reload() override;

private:
	void RenderPoints();
	void RenderSpheres();

private:
	float		m_pointSize;	//!< point size
	int			m_renderMode;	//!< render mode
	int			m_colorMode;	//!< color mode
	GLColor		m_solidColor;	//!< rendering color
	CColorTexture	m_Col;		//!< line color (when m_ncolor is not solid)
	bool		m_showLegend;	//!< show legend bar or not
	DATA_RANGE	m_range;	// range for legend

	PointDataModel* m_pointData;
};
}
