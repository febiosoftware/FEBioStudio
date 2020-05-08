#include "stdafx.h"
#include "ImageModel.h"
#include <ImageLib/3DImage.h>
#include "GLImageRenderer.h"
#include <FSCore/FSDir.h>
#include <assert.h>
using namespace Post;

CImageSource::CImageSource(CImageModel* imgModel)
{
	AddStringParam("", "file name")->SetState(Param_VISIBLE);
	AddIntParam(0, "NX")->SetState(Param_VISIBLE);
	AddIntParam(1, "NY")->SetState(Param_VISIBLE);
	AddIntParam(2, "NZ")->SetState(Param_VISIBLE);

	m_img = nullptr;
	m_imgModel = imgModel;
}

CImageModel* CImageSource::GetImageModel()
{
	return m_imgModel;
}

void CImageSource::SetImageModel(CImageModel* imgModel) 
{
	m_imgModel = imgModel;
}

CImageSource::~CImageSource()
{
	delete m_img;
}

void CImageSource::SetFileName(const std::string& file)
{
	SetStringValue(0, file);
}

std::string CImageSource::GetFileName() const
{
	return GetStringValue(0);
}

bool CImageSource::LoadImageData(const std::string& fileName, int nx, int ny, int nz)
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

	SetStringValue(0, fileName);
	SetIntValue(1, nx);
	SetIntValue(2, ny);
	SetIntValue(3, nz);

	delete m_img;
	m_img = im;

	return true;
}

void CImageSource::Save(OArchive& ar)
{
	FSObject::Save(ar);
}

int CImageSource::Width() const { return GetIntValue(1);  }
int CImageSource::Height() const { return GetIntValue(2); }
int CImageSource::Depth() const { return GetIntValue(3); }

void CImageSource::Load(IArchive& ar)
{
	FSObject::Load(ar);
	string file = GetFileName();
	LoadImageData(file, Width(), Height(), Depth());
}

//========================================================================

CImageModel::CImageModel(CGLModel* mdl) : CGLObject(mdl)
{
	AddBoolParam(true, "show box");
	AddDoubleParam(0, "x0");
	AddDoubleParam(0, "y0");
	AddDoubleParam(0, "z0");
	AddDoubleParam(1, "x1");
	AddDoubleParam(1, "y1");
	AddDoubleParam(1, "z1");

	m_box = BOX(0., 0., 0., 1., 1., 1.);
	m_showBox = true;
	m_img = nullptr;

	UpdateData(false);
}

CImageModel::~CImageModel()
{
	delete m_img;
}

bool CImageModel::UpdateData(bool bsave)
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

	return false;
}

bool CImageModel::LoadImageData(const std::string& fileName, int nx, int ny, int nz, const BOX& box)
{
	if (m_img == nullptr) m_img = new CImageSource(this);

	if (m_img->LoadImageData(fileName, nx, ny, nz) == false)
	{
		delete m_img;
		m_img = nullptr;
		return false;
	}

	// set the default name by extracting the base of the file name
	string fileBase = FSDir::fileBase(fileName);
	m_img->SetName(fileBase);

	m_box = box;
	UpdateData(false);

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

void CImageModel::Render(CGLContext& rc)
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

	if (m_img)
	{
		ar.BeginChunk(1);
		{
			m_img->Save(ar);
		}
		ar.EndChunk();
	}
}

CImageSource* CImageModel::GetImageSource()
{ 
	return m_img; 
}

void CImageModel::Load(IArchive& ar)
{
	delete m_img; m_img = nullptr;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case 0:
			FSObject::Load(ar);
			break;
		case 1:
			{
				m_img = new CImageSource(this);
				m_img->Load(ar);
			}
			break;
		}
		ar.CloseChunk();
	}

	// let's try to load the file
	UpdateData();
}
