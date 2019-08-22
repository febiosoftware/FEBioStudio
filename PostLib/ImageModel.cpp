#include "stdafx.h"
#include "ImageModel.h"
#include "3DImage.h"
#include "GLImageRenderer.h"
#include <assert.h>
using namespace Post;

CImageModel::CImageModel(CGLModel* mdl) : CGLObject(mdl)
{
	m_pImg = 0;
	m_box = BOX(0., 0., 0., 1., 1., 1.);
	m_showBox = true;
}

CImageModel::~CImageModel()
{
	if (m_pImg) { delete m_pImg; m_pImg = 0; }
	for (int i = 0; i < (int)m_render.size(); ++i) delete m_render[i];
	m_render.clear();
}

bool CImageModel::LoadImageData(const std::string& fileName, int nx, int ny, int nz, const BOX& box)
{
	C3DImage* im = new C3DImage;
	if (im->Create(nx, ny, nz) == false)
	{
		delete im;
		return false;
	}

	if (im->LoadFromFile(fileName.c_str(), 8) == false)
	{
		delete im;
		return false;
	}

	delete m_pImg;
	m_pImg = im;

	m_file = fileName;
	return true;
}

bool CImageModel::ShowBox() const
{
	return m_showBox;
}

void CImageModel::ShowBox(bool b)
{
	m_showBox = b;
}

void CImageModel::Render(Post::CGLContext& rc)
{
	// render the volume image data if present
	if (IsActive())
	{
		for (int j = 0; j < ImageRenderers(); ++j)
		{
			Post::CGLImageRenderer* pir = GetImageRenderer(j);
			if (pir && pir->IsActive())
			{
//				if (pir->AllowClipping()) CGLPlaneCutPlot::EnableClipPlanes();
//				else CGLPlaneCutPlot::DisableClipPlanes();
				pir->Render(rc);
			}
		}
	}
}


void CImageModel::SetFileName(const std::string& fileName)
{
	m_file = fileName;
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
