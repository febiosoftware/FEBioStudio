#include "stdafx.h"
#include "ImageModel.h"
#include "3DImage.h"
#include "GLImageRenderer.h"
#include <assert.h>
using namespace Post;

CImageModel::CImageModel(CGLModel* mdl) : CGLObject(mdl)
{
	m_pImg = 0;
	m_box = BOUNDINGBOX(0.f, 0.f, 0.f, 1.f, 1.f, 1.f);
	m_showBox = true;
}

CImageModel::~CImageModel()
{
	if (m_pImg) { delete m_pImg; m_pImg = 0; }
	for (int i = 0; i < (int)m_render.size(); ++i) delete m_render[i];
	m_render.clear();
}

bool CImageModel::ShowBox() const
{
	return m_showBox;
}

void CImageModel::ShowBox(bool b)
{
	m_showBox = b;
}

void CImageModel::SetFileName(const std::string& fileName)
{
	m_file = fileName;
}

void CImageModel::Set3DImage(C3DImage* img, BOUNDINGBOX b)
{
	assert(m_pImg == nullptr);
	m_pImg = img;
	m_box = b;
}

bool CImageModel::RemoveRenderer(CGLImageRenderer* render)
{
	for (int i = 0; i < (int)m_render.size(); ++i)
	{
		if (m_render[i] == render)
		{
			delete render;
			m_render.erase(m_render.begin() + i);
			return true;
		}
	}
	return false;
}

void CImageModel::AddImageRenderer(CGLImageRenderer* render)
{
	assert(render);
	m_render.push_back(render);
}
