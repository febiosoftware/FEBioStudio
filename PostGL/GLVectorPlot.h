#pragma once
#include "GLPlot.h"

class GLUquadric;

namespace Post {

class CGLVectorPlot : public CGLPlot 
{
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
	CGLVectorPlot(CGLModel* po);
	virtual ~CGLVectorPlot();

	void Render(CGLContext& rc);

	void SetScaleFactor(float g) { m_scale = g; }
	double GetScaleFactor() { return m_scale; }

	void SetDensity(float d) { m_dens = d; }
	double GetDensity() { return m_dens; }

	int GetVectorType() { return m_nvec; }
	void SetVectorType(int ntype);

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

	void Update(int ntime, float dt, bool breset);

	CPropertyList* propertyList();

	void UpdateTexture() { m_Col.UpdateTexture(); }

private:
	// render a vector n at position r
	void RenderVector(const vec3f& r, vec3f v, GLUquadric* pglyph);

protected:
	float	m_scale;
	float	m_dens;
	int		m_seed;

	int		m_nvec;		// vector field
	int		m_nglyph;	// glyph type
	int		m_ncol;		// color type

	bool	m_bnorm;		// normalize vectors or not
	bool	m_bautoscale;	// auto scale the vectors
	bool	m_bshowHidden;	// show vectors on hidden materials

	GLColor			m_gcl;	// glyph color (for GLYPH_COL_SOLID)
	CColorTexture	m_Col;	// glyph color (for not GLYPH_COL_SOLID)

	vector<vec2f>	m_rng;	// nodal ranges
	DataMap<vec3f>	m_map;	// nodal values map
	
	int				m_ntime;	// current time at which this plot is evaluated
	vector<vec3f>	m_val;	// current values
	vec2f			m_crng;	// current range

	float			m_fscale;	// total scale factor for rendering
};
}
