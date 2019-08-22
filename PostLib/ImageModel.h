#pragma once
#include <vector>
#include <string>
#include <FSCore/box.h>
#include <FSCore/FSObjectList.h>
#include <PostLib/GLImageRenderer.h>
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

	int ImageRenderers() const { return (int)m_render.Size(); }
	CGLImageRenderer* GetImageRenderer(int i) { return m_render[i]; }
	size_t RemoveRenderer(CGLImageRenderer* render);

	void AddImageRenderer(CGLImageRenderer* render);

	const BOX& GetBoundingBox() const { return m_box; }

	void SetBoundingBox(BOX b) { m_box = b; }

	bool ShowBox() const;

	void ShowBox(bool b);

	void Render(Post::CGLContext& rc);

	void UpdateData(bool bsave = true) override;

private:
	std::string		m_file;						//!< file name of image data
	BOX				m_box;						//!< physical dimensions of image
	C3DImage*		m_pImg;						//!< 3D image
	bool			m_showBox;					//!< show box in Graphics View
	FSObjectList<CGLImageRenderer>	m_render;	//!< image renderers
};
}
