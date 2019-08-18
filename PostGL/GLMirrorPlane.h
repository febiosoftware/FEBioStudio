#pragma once
#include "GLPlot.h"

namespace Post {

class CGLMirrorPlane : public CGLPlot
{
public:
	CGLMirrorPlane(CGLModel* fem);

	CPropertyList* propertyList() override;

	// render the object to the 3D view
	void Render(CGLContext& rc) override;

private:
	void RenderPlane();

public:
	int		m_plane;
	float	m_transparency;
	bool	m_showPlane;
	float	m_offset;

private:
	vec3f	m_norm;
};
}
