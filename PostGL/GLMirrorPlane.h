#pragma once
#include "GLPlot.h"

namespace Post {

class CGLMirrorPlane : public CGLPlot
{
	enum { PLANE, SHOW_PLANE, TRANSPARENCY, OFFSET };

public:
	CGLMirrorPlane(CGLModel* fem);

	// render the object to the 3D view
	void Render(CGLContext& rc) override;

	void UpdateData(bool bsave = true) override;

private:
	void RenderPlane();

public:
	int		m_plane;
	float	m_transparency;
	bool	m_showPlane;
	float	m_offset;

private:
	vec3f	m_norm;

	bool	m_is_rendering;
};
}
