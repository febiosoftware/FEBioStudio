#include "stdafx.h"
#include "GLImageRenderer.h"
using namespace Post;

CGLImageRenderer::CGLImageRenderer(CImageModel* img) : m_img(img) 
{
}

CImageModel* CGLImageRenderer::GetImageModel()
{ 
	return m_img; 
}
