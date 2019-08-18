#pragma once
#include "GLObject.h"

namespace Post {

class CImageModel;

class CGLImageRenderer : public CGLVisual
{
public:
	CGLImageRenderer(CImageModel* img = nullptr) : m_img(img) {}

	CImageModel* GetImageModel() { return m_img; }

private:
	CImageModel*	m_img;
};
}
