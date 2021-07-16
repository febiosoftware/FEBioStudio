/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "ImageModel.h"
#include <ImageLib/3DImage.h>
#include <ImageLib/ITKImage.h>
#include "GLImageRenderer.h"
#include <FSCore/FSDir.h>
#include <assert.h>

#ifdef HAS_TEEM
#include <ImageLib/compatibility.h>
#endif

#ifdef HAS_DICOM
#include <dcmtk/dcmimgle/dcmimage.h>
#include <bitset>
#endif

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


#ifdef HAS_TEEM

//TODO: Maybe see if we can break this function up a bit? 
//      See much how much of Yong's code we can break off.
bool CImageSource::LoadTiffData(std::wstring &fileName)
{
  C3DImage* im = new C3DImage;
  std::unique_ptr<TIFReader> reader = std::make_unique<TIFReader>();

  // Returns a nrrd based on templated function
  Nrrd* nrrdStruct = GetNrrd<TIFReader>(reader,fileName);

  auto [nx,ny,npages,bits] = reader->GetTiffInfo();

  BOX box(nx, ny, npages, nx + reader->GetXSpc(), ny+reader->GetYSpc(), npages+reader->GetZSpc());
  m_imgModel->SetBoundingBox(box);
 
  if(im->Create(nx,ny,npages,reader->GetRawImage()) == false)
  {
	delete im;
	return false;
  }

  SetValues(ws2s(fileName),nx,ny,npages);
  AssignImage(im);

  return true;
}

bool CImageSource::LoadNrrdData(std::wstring& filename)
{
  C3DImage* im = new C3DImage();
  std::unique_ptr<NRRDReader> reader = std::make_unique<NRRDReader>();

  Nrrd* nrrdStruct = GetNrrd<NRRDReader>(reader,filename);
  
  int nx = reader->GetXSize();
  int ny = reader->GetYSize();
  int nz = reader->GetSliceNum();
  int dataSize = nx * ny * nz;

  Byte* data = static_cast<Byte*>(nrrdStruct->data);

  Byte* dataBuf;

  if (nrrdStruct->type == nrrdTypeUShort || nrrdStruct->type == nrrdTypeShort)
  {
    dataBuf = new Byte[dataSize];
    for (int i = 0; i < dataSize; ++i)
    {
      dataBuf[i] = data[2*i];
    }
  }
  else
    dataBuf = data;


  BOX box(nx, ny, nz, nx+reader->GetXSpc(), ny+reader->GetYSpc(), nz+reader->GetZSpc());
  m_imgModel->SetBoundingBox(box);

  if (im->Create(nx, ny, nz, dataBuf) == false)
  {
    delete im;
    return false;
  }

  SetValues(ws2s(filename),nx,ny,nz);
  AssignImage(im);

  return true;
}
#endif

#ifdef HAS_ITK
bool CImageSource::LoadITKData(const std::string& filename)
{
  CITKImage* im = new CITKImage();  

  if(im->LoadFromFile(filename.c_str()))
  {
    delete im;
    return false;
  }

  BOX box(0,0,0,1,1,1);
  m_imgModel->SetBoundingBox(box);

  SetValues(filename,im->Width(),im->Height(),im->Depth());
  AssignImage(im);

  return true;
}
#endif

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

  SetValues(fileName,nx,ny,nz);
  AssignImage(im);

  return true;
}

void CImageSource::SetValues(const std::string& fileName, int x, int y, int z)
{
	SetStringValue(0, fileName);
	SetIntValue(1, x);
	SetIntValue(2, y);
	SetIntValue(3, z);
}

void CImageSource::AssignImage(C3DImage* im)
{
  delete m_img;
  m_img = im;
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

#ifdef HAS_TEEM
bool CImageModel::LoadTiffData(std::wstring &fileName)
{
	if (m_img == nullptr) m_img = new CImageSource(this);

	if (m_img->LoadTiffData(fileName) == false)
	{
		delete m_img;
		m_img = nullptr;
		return false;
	}

	// set the default name by extracting the base of the file name
	string fileBase = FSDir::fileBase(ws2s(fileName));
	m_img->SetName(fileBase);

	UpdateData(false);

	return true;
}

bool CImageModel::LoadNrrdData(std::wstring& filename)
{
  if(m_img == nullptr) m_img = new CImageSource(this);

  if (m_img->LoadNrrdData(filename) == false)
  {
    delete m_img;
    m_img = nullptr;
    return false;
  }
  std::string fileBase = FSDir::fileBase(ws2s(filename));
  m_img->SetName(fileBase);

  UpdateData(false);

  return true;
}
#endif

#ifdef HAS_ITK
bool CImageModel::LoadITKData(const std::string& filename)
{
	if (m_img == nullptr) m_img = new CImageSource(this);

	if (m_img->LoadITKData(filename) == false)
	{
		delete m_img;
		m_img = nullptr;
		return false;
	}

	// set the default name by extracting the base of the file name
	string fileBase = FSDir::fileBase(filename);
	m_img->SetName(fileBase);

	UpdateData(false);

	return true;
}
#endif

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
