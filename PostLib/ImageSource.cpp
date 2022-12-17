/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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

#include "ImageSource.h"
#include "ImageModel.h"
#include <ImageLib/3DImage.h>
#include <ImageLib/ImageSITK.h>

using namespace Post;

CImageSource::CImageSource(CImageModel* imgModel)
{
	AddStringParam("", "file name")->SetState(Param_VISIBLE);
	AddIntParam(0, "NX")->SetState(Param_VISIBLE);
	AddIntParam(1, "NY")->SetState(Param_VISIBLE);
	AddIntParam(2, "NZ")->SetState(Param_VISIBLE);

	m_img = nullptr;
    m_originalImage = nullptr;
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
    if(m_img != m_originalImage)
    {
        delete m_img;
    }
	
    delete m_originalImage;
}

void CImageSource::ClearFilters()
{
    if( m_img != m_originalImage)
    {
        delete m_img;
        m_img = m_originalImage;
    }
}

C3DImage* CImageSource::GetImageToFilter(bool allocate)
{

#ifdef HAS_ITK

    if(m_img == m_originalImage)
    {
        if(allocate)
        {
            int nx = m_originalImage->Width();
            int ny = m_originalImage->Height();
            int nz = m_originalImage->Depth();

            m_img = new CImageSITK(nx, ny, nz);
        }
        else
        {
            m_img = new CImageSITK();
        }
    }

#else

    if(m_img == m_originalImage)
    {
        int nx = m_originalImage->Width();
        int ny = m_originalImage->Height();
        int nz = m_originalImage->Depth();

        m_img = new C3DImage();
        m_img->Create(nx, ny, nz);
    }

#endif

    return m_img;
}

// void CImageSource::SetValues(const std::string& fileName, int x, int y, int z)
// {
// 	SetStringValue(0, fileName);
// 	SetIntValue(1, x);
// 	SetIntValue(2, y);
// 	SetIntValue(3, z);
// }

void CImageSource::AssignImage(C3DImage* im)
{
//   delete m_img;
//   m_img = im;

    delete m_originalImage;
    m_originalImage = im;
    m_img = im;
}

// void CImageSource::Save(OArchive& ar)
// {
// 	FSObject::Save(ar);
// }

// int CImageSource::Width() const { return GetIntValue(1);  }
// int CImageSource::Height() const { return GetIntValue(2); }
// int CImageSource::Depth() const { return GetIntValue(3); }

// void CImageSource::Load(IArchive& ar)
// {
// 	FSObject::Load(ar);
// 	string file = GetFileName();
// 	LoadImageData(file, Width(), Height(), Depth());
// }

//========================================================================

CRawImageSource::CRawImageSource(CImageModel* imgModel, const std::string& filename, int nx, int ny, int nz)
    : CImageSource(imgModel), m_filename(filename), m_nx(nx), m_ny(ny), m_nz(nz)
{

}

bool CRawImageSource::Load()
{
    C3DImage* im = new C3DImage;
    if (im->Create(m_nx, m_ny, m_nz) == false)
    {
        delete im;
        return false;
    }

    if (im->LoadFromFile(m_filename.c_str(), 8) == false)
    {
        delete im;
        return false;
    }

    AssignImage(im);

    return true;
}


#ifdef HAS_ITK

//========================================================================

CITKImageSource::CITKImageSource(CImageModel* imgModel, const std::string& filename, ImageFileType type) 
    : CImageSource(imgModel), m_filename(filename), m_type(type)
{
    
}

bool CITKImageSource::Load()
{
    CImageSITK* im = new CImageSITK();  

	if(!im->LoadFromFile(m_filename.c_str(), m_type == ImageFileType::DICOM))
	{
		delete im;
		return false;
	}

	std::vector<unsigned int> size = im->GetSize();
	std::vector<double> origin = im->GetOrigin();
	std::vector<double> spacing = im->GetSpacing();

	BOX box(origin[0],origin[1],origin[2],spacing[0]*size[0],spacing[1]*size[1],spacing[2]*size[2]);
	m_imgModel->SetBoundingBox(box);

	AssignImage(im);

	return true;
}

//========================================================================

CITKSeriesImageSource::CITKSeriesImageSource(CImageModel* imgModel, const std::vector<std::string>& filenames)
    : CImageSource(imgModel), m_filenames(filenames)
{

}

bool CITKSeriesImageSource::Load()
{
    CImageSITK* im = new CImageSITK();  

	if(!im->LoadFromStack(m_filenames))
	{
		delete im;
		return false;
	}

	std::vector<unsigned int> size = im->GetSize();
	std::vector<double> origin = im->GetOrigin();
	std::vector<double> spacing = im->GetSpacing();

	BOX box(origin[0],origin[1],origin[2],spacing[0]*size[0],spacing[1]*size[1],spacing[2]*size[2]);
	m_imgModel->SetBoundingBox(box);

	AssignImage(im);

	return true;
}

#else
//========================================================================
CITKImageSource::CITKImageSource(CImageModel* imgModel, const std::string& filename, ImageFileType type)
    : CImageSource(imgModel) {}
bool CITKImageSource::Load() { return false; }
//========================================================================
CITKSeriesImageSource::CITKSeriesImageSource(CImageModel* imgModel, const std::vector<std::string>& filenames)
    : CImageSource(imgModel) {}
bool CITKSeriesImageSource::Load() { return false; }
#endif

//========================================================================
