#pragma once
#include <vector>
#include <string>
#include <FSCore/box.h>
#include "GLObject.h"

namespace Post {

class C3DImage;
class CGLImageRenderer;

class CImageModel : public CGLObject
{
public:
	CImageModel(CGLModel* mdl);
	~CImageModel();

	void SetFileName(const std::string& fileName);
	std::string GetFileName() { return m_file; }

	bool LoadImageData(const std::string& fileName, int nx, int ny, int nz, const BOX& box);

	C3DImage* Get3DImage() { return m_pImg; }

	int ImageRenderers() const { return (int)m_render.size(); }
	CGLImageRenderer* GetImageRenderer(int i) { return m_render[i]; }
	bool RemoveRenderer(CGLImageRenderer* render);

	void AddImageRenderer(CGLImageRenderer* render);

	const BOX& GetBoundingBox() const { return m_box; }

	void SetBoundingBox(BOX b) { m_box = b; }

	bool ShowBox() const;

	void ShowBox(bool b);

	void Render(Post::CGLContext& rc);

private:
	std::string		m_file;						//!< file name of image data
	BOX				m_box;						//!< physical dimensions of image
	C3DImage*		m_pImg;						//!< 3D image
	bool			m_showBox;					//!< show box in Graphics View
	std::vector<CGLImageRenderer*>	m_render;	//!< image renderers
};
}
