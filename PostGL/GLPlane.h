#pragma once
#include <PostLib/GLObject.h>

namespace Post {

	class FEPostModel;

//-----------------------------------------------------------------------------
class CGLPlane : public CGLVisual
{
public:
	CGLPlane(Post::FEPostModel* pfem);
	~CGLPlane(void);

	void Render(CGLContext& rc);

	void Create(int n[3]);

	vec3d Normal() { return m_e[2]; }

protected:
	Post::FEPostModel*	m_pfem;	// pointer to mesh

	vec3d		m_rc;
	vec3d		m_e[3];
};
}
