#pragma once
#include "GLObject.h"

namespace Post {

class CImageModel;

class CGLImageRenderer : public CGLVisual
{
public:
	CGLImageRenderer(CImageModel* img = nullptr);

	CImageModel* GetImageModel();

private:
	CImageModel*	m_img;
};
}
