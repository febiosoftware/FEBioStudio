#pragma once
#include "PostLib/GLObject.h"
#include "PostLib/FEModel.h"

namespace Post {
//-----------------------------------------------------------------------------
class CGLPlane : public CGLVisual
{
public:
	CGLPlane(Post::FEModel* pfem);
	~CGLPlane(void);

	void Render(CGLContext& rc);

	void Create(int n[3]);

	vec3d Normal() { return m_e[2]; }

protected:
	Post::FEModel*	m_pfem;	// pointer to mesh

	vec3d		m_rc;
	vec3d		m_e[3];
};
}
