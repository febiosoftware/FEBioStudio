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
#include <MeshLib/GMesh.h>

namespace Post {

class CGLSlicePlot : public CGLLegendPlot  
{
	enum { 
		DATA_FIELD, 
		COLOR_MAP, 
		RANGE_DIVS,
		GRAD_SMOOTH,
		TRANSPARENCY,
		CLIP, 
		SHOW_LEGEND, 
		SLICES, 
		SLICE_OFFSET, 
		SLICE_RANGE,
		USER_SLICES,
		RANGE, 
		RANGE_MAX, 
		RANGE_MIN, 
		NORMAL_X, 
		NORMAL_Y, 
		NORMAL_Z, 
		SHOW_BOX 
	};

public:
	CGLSlicePlot();

	void SetBoundingBox(BOX box) { m_box = box; }

	int GetSlices();
	void SetSlices(int nslices);

	vec3f GetPlaneNormal() { return m_norm; }
	void SetPlaneNormal(vec3f& n) { m_norm = n; }

	void Render(GLRenderEngine& re, GLContext& rc) override;

	int GetEvalField() { return m_nfield; }
	void SetEvalField(int n);

	CColorTexture* GetColorMap() { return &m_Col; }

	void Update(int ntime, float dt, bool breset) override;

	void SetRangeType(int ntype) { m_nrange = ntype; }
	int GetRangeType() const { return m_nrange; }

	float GetUserRangeMax() const { return m_fmax; }
	void SetUserRangeMax(float f) { m_fmax = f; }
	float GetUserRangeMin() const { return m_fmin; }
	void SetUserRangeMin(float f) { m_fmin = f; }

	float GetSliceOffset() const { return m_offset; }
	void SetSliceOffset(float f);

	void UpdateTexture() override { m_Col.UpdateTexture(); }

	bool UpdateData(bool bsave = true) override;

	void Update() override;

protected:
	int UpdateSlice(float ref, std::vector<std::pair<int, float> >& activeElements);

	void UpdateBoundingBox();

	void UpdateMesh();
	int CountFaces(std::vector<std::pair<int, float> >& activeElements);

protected:
	int			m_nslices;	// nr. of iso surface slices
	BOX			m_box;		// box to use for slices	
	vec3f		m_norm;

	int			m_nrange;		//!< range option (0=dynamic, 1=user)
	float		m_fmin, m_fmax;	//!< user-defined range 
	float		m_offset;
	vec2d		m_slice_range;
	std::vector<double> m_user_slices;

	int				m_nfield;	// data field
	CColorTexture	m_Col;		// colormap

	vector<vec2f>	m_rng;	// value range
	DataMap<float>	m_map;	// nodal values map
	vector<float>	m_val;	// current nodal values
	vec2f			m_crng;	// current range

	int m_lastTime;
	float	m_lastDt;

	GMesh	m_mesh;
};
}
