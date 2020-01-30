#pragma once
#include "GLPlot.h"
#include <vector>

namespace Post {

class GLVolumeFlowPlot : public CGLPlot
{
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
	GLVolumeFlowPlot(CGLModel* mdl);

	void Render(CGLContext& rc) override;

	void Update(int ntime, float dt, bool breset) override;

	void UpdateData(bool bsave = true) override;

private:
	void CreateSlice(Slice& slice, const vec3d& normal, float w);
	void UpdateNodalData(int ntime, bool breset);
	void RenderSlices(std::vector<Slice>& slice, int step);

private:
	int			m_nfield;
	int			m_nrange;		//!< range option (0=dynamic, 1=user)
	float		m_fmin, m_fmax;	//!< user-defined range 
	float		m_offset;
	float		m_alpha;
	CColorTexture	m_Col;		// colormap

private:
	std::vector<Slice>	m_slice_X;
	std::vector<Slice>	m_slice_Y;
	std::vector<Slice>	m_slice_Z;

private:
	vector<vec2f>	m_rng;	// value range
	DataMap<float>	m_map;	// nodal values map
	vector<float>	m_val;	// current nodal values
	vec2f			m_crng;	// current range
	BOX				m_box;
};
} // namespace Post
