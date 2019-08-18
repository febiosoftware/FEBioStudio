#pragma once
#include "GLPlot.h"
#ifdef __APPLE__
    #include <OpenGL/GLU.h>
#else
    #include <GL/glu.h>
#endif

namespace Post {

class GLTensorPlot : public CGLPlot
{
public:
	enum Glyph_Type {
		Glyph_Arrow,
		Glyph_Line,
		Glyph_Sphere,
		Glyph_Box
	};

	enum Glyph_Color_Type {
		Glyph_Col_Solid,
		Glyph_Col_Norm
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
	GLTensorPlot(CGLModel* po);
	~GLTensorPlot();

	void Render(CGLContext& rc);

	CColorTexture* GetColorMap();

	CPropertyList* propertyList();

	void Update(int ntime, float dt, bool breset);

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
	void RenderGlyphs(TENSOR& t, float scale, GLUquadricObj* glyph);
	void RenderArrows(TENSOR& t, float scale, GLUquadricObj* glyph);
	void RenderLines(TENSOR& t, float scale, GLUquadricObj* glyph);
	void RenderSphere(TENSOR& t, float scale, GLUquadricObj* glyph);
	void RenderBox(TENSOR& t, float scale, GLUquadricObj* glyph);

protected:
	int		m_ntensor;	// tensor field
	int		m_nglyph;	// glyph type
	int		m_ncol;		// color type

	float	m_scale;
	float	m_dens;
	int		m_seed;

	bool	m_bshowHidden;	// show tensors on hidden materials
	bool	m_bautoscale;	// auto scale the vectors
	bool	m_bnormalize;

	int				m_ntime;
	DataMap<TENSOR>	m_map;	// nodal values map
	vector<TENSOR>	m_val;	// current nodal values

	GLColor			m_gcl;	// glyph color (for GLYPH_COL_SOLID)
	CColorTexture	m_Col;	// glyph color (for not GLYPH_COL_SOLID)

	int		m_nmethod;
};
}
