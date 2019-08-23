#include "stdafx.h"
#include "ImageModel.h"
#include "3DImage.h"
#include "GLImageRenderer.h"
#include <FSCore/FSDir.h>
#include <assert.h>
using namespace Post;

CImageModel::CImageModel(CGLModel* mdl) : CGLObject(mdl)
{
	AddBoolParam(true, "show box");
	AddDoubleParam(0, "x0");
	AddDoubleParam(0, "y0");
	AddDoubleParam(0, "z0");
	AddDoubleParam(0, "x1");
	AddDoubleParam(0, "y1");
	AddDoubleParam(0, "z1");

	m_pImg = 0;
	m_box = BOX(0., 0., 0., 1., 1., 1.);
	m_showBox = true;

	m_imageSize[0] = 0;
	m_imageSize[1] = 0;
	m_imageSize[2] = 0;

	UpdateData(false);
}

CImageModel::~CImageModel()
{
}

void CImageModel::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_showBox = GetBoolValue(0);
		m_box.x0 = GetFloatValue(1);
		m_box.y0 = GetFloatValue(2);
		m_box.z0 = GetFloatValue(3);
		m_box.x1 = GetFloatValue(4);
		m_box.y1 = GetFloatValue(5);
		m_box.z1 = GetFloatValue(6);
		for (int i = 0; i < (int)m_render.Size(); ++i) m_render[i]->Update();
	}
	else
	{
		SetBoolValue(0, m_showBox);
		SetFloatValue(1, m_box.x0);
		SetFloatValue(2, m_box.y0);
		SetFloatValue(3, m_box.z0);
		SetFloatValue(4, m_box.x1);
		SetFloatValue(5, m_box.y1);
		SetFloatValue(6, m_box.z1);
	}
}

bool CImageModel::LoadImageData(const std::string& fileName, int nx, int ny, int nz, const BOX& box)
{
	C3DImage* im = new C3DImage;
	if (im->Create(nx, ny, nz) == false)
	{
		delete im;
		return false;
	}

	// do string-substitution
	string abspath = FSDir::toAbsolutePath(fileName);

	if (im->LoadFromFile(abspath.c_str(), 8) == false)
	{
		delete im;
		return false;
	}

	delete m_pImg;
	m_pImg = im;

	m_imageSize[0] = nx;
	m_imageSize[1] = ny;
	m_imageSize[2] = nz;

	m_box = box;
	UpdateData(false);

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

size_t CImageModel::RemoveRenderer(CGLImageRenderer* render)
{
	return m_render.Remove(render);
}

void CImageModel::AddImageRenderer(CGLImageRenderer* render)
{
	assert(render);
	m_render.Add(render);
}

void CImageModel::Save(OArchive& ar)
{
	ar.BeginChunk(0);
	{
		FSObject::Save(ar);
	}
	ar.EndChunk();
	ar.WriteChunk(1, m_file);
	ar.WriteChunk(2, m_imageSize, 3);
}

void CImageModel::Load(IArchive& ar)
{
	while (ar.OpenChunk() == IO_OK)
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case 0:
			FSObject::Load(ar);
			break;
		case 1:
			ar.read(m_file);
			break;
		case 2: 
			ar.read(m_imageSize, 3);
			break;
		}
		ar.CloseChunk();
	}

	// let's try to load the file
	UpdateData();
	LoadImageData(m_file, m_imageSize[0], m_imageSize[1], m_imageSize[2], GetBoundingBox());
}
