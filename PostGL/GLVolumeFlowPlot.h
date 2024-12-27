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
#include <GLLib/GLMesh.h>
#include <vector>

namespace Post {

class GLVolumeFlowPlot : public CGLLegendPlot
{
	enum {DATA_FIELD, COLOR_MAP, SMOOTH_COLOR_MAP, RANGE_DIVISIONS, OPACITY_SCALE, OPACITY_STRENGTH, MESH_DIVISIONS, SHOW_LEGEND, MAX_RANGE_TYPE, USER_MAX, MIN_RANGE_TYPE, USER_MIN};

	enum { MAX_MESH_DIVS = 5};

public:
	class Slice
	{
	public:
		struct Face {
			float	v[3];
			vec3d	r[3];
		};

		void clear() { m_Face.clear(); }

		void reserve(size_t n) { m_Face.reserve(n); }

		void add(const Face& f) { m_Face.push_back(f); }

	public:
		std::vector<Face>	m_Face;
	};

public:
	GLVolumeFlowPlot();

	void Render(GLRenderEngine& re, CGLContext& rc) override;

	void Update(int ntime, float dt, bool breset) override;

	bool UpdateData(bool bsave = true) override;

private:
	void CreateSlices(std::vector<Slice>& slice, const vec3d& normal);
	void CreateSlice(Slice& slice, const vec3d& normal, float w);
	void UpdateNodalData(int ntime, bool breset);
	void UpdateBoundingBox();
	void UpdateMesh(std::vector<Slice>& slice, GLTriMesh& mesh);

private:
	int			m_nfield;
	float		m_offset;
	float		m_alpha;
	float		m_gain;
	bool		m_bsmooth;
	int			m_nDivs;
	int			m_meshDivs;
	DATA_RANGE	m_range;	// range for legend
	CColorTexture	m_Col;		// colormap

private:
	vector<vec2f>	m_rng;	// value range
	DataMap<float>	m_map;	// nodal values map
	vector<float>	m_val;	// current nodal values
	BOX				m_box;

	GLTriMesh	m_mesh;
};
} // namespace Post
