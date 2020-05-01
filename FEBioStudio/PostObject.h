#pragma once
#include <GeomLib/GMeshObject.h>

namespace Post
{
	class CGLModel;
}

class CPostObject : public GMeshObject
{
public:
	CPostObject(Post::CGLModel* glm);
	~CPostObject();

	// is called whenever the selection has changed
	void UpdateSelection() override;

	void UpdateMesh() override;

	BOX GetBoundingBox();

private:
	Post::CGLModel* m_glm;
};

