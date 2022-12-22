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

CImageSource::CImageSource(int type, CImageModel* imgModel)
    : m_type(type), m_imgModel(imgModel), m_img(nullptr), m_originalImage(nullptr)
{
	// AddStringParam("", "file name")->SetState(Param_VISIBLE);
	// AddIntParam(0, "NX")->SetState(Param_VISIBLE);
	// AddIntParam(1, "NY")->SetState(Param_VISIBLE);
	// AddIntParam(2, "NZ")->SetState(Param_VISIBLE);
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

//========================================================================

CRawImageSource::CRawImageSource(CImageModel* imgModel, const std::string& filename, int nx, int ny, int nz, BOX box)
    : CImageSource(CImageSource::RAW, imgModel), m_filename(filename), m_nx(nx), m_ny(ny), m_nz(nz), m_box(box)
{

}

CRawImageSource::CRawImageSource(CImageModel* imgModel)
    : CImageSource(CImageSource::RAW, imgModel)
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

    im->SetBoundingBox(m_box);

    AssignImage(im);

    return true;
}

void CRawImageSource::Save(OArchive& ar)
{
    ar.WriteChunk(0, m_filename);
    
    ar.WriteChunk(1, m_nx);
    ar.WriteChunk(2, m_ny);
    ar.WriteChunk(3, m_nx);

    ar.WriteChunk(4, m_box.x0);
    ar.WriteChunk(5, m_box.y0);
    ar.WriteChunk(6, m_box.z0);
    ar.WriteChunk(7, m_box.x1);
    ar.WriteChunk(8, m_box.y1);
    ar.WriteChunk(9, m_box.z1);
}

void CRawImageSource::Load(IArchive& ar)
{
    BOX tempBox;
    bool foundBox = false;

    while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case 0:
			ar.read(m_filename);
			break;

		case 1:
			ar.read(m_nx);
            break;
        case 2:
			ar.read(m_ny);
            break;
        case 3:
			ar.read(m_nz);
            break;

        case 4:
			ar.read(m_box.x0);
            break;
        case 5:
			ar.read(m_box.y0);
            break;
        case 6:
			ar.read(m_box.z0);
            break;
        case 7:
			ar.read(m_box.x1);
            break;
        case 8:
			ar.read(m_box.y1);
            break;
        case 9:
			ar.read(m_box.z1);
            break;

        case 100:
			ar.read(tempBox.x0);
            foundBox = true;
            break;
        case 101:
			ar.read(tempBox.y0);
            break;
        case 102:
			ar.read(tempBox.z0);
            break;
        case 103:
			ar.read(tempBox.x1);
            break;
        case 104:
			ar.read(tempBox.y1);
            break;
        case 105:
			ar.read(tempBox.z1);
            break;
		}
		ar.CloseChunk();
	}

    // Read in image data
    Load();

    // Set location of image if it was saved
    if(foundBox)
    {
        m_img->SetBoundingBox(tempBox);
    }

}


#ifdef HAS_ITK

//========================================================================

CITKImageSource::CITKImageSource(CImageModel* imgModel, const std::string& filename, ImageFileType type) 
    : CImageSource(CImageSource::ITK, imgModel), m_filename(filename), m_type(type)
{
    
}

CITKImageSource::CITKImageSource(CImageModel* imgModel)
    : CImageSource(CImageSource::ITK, imgModel)
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

	AssignImage(im);

	return true;
}

void CITKImageSource::Save(OArchive& ar)
{
    ar.WriteChunk(0, m_filename);
    ar.WriteChunk(1, (int)m_type);

    BOX box = m_originalImage->GetBoundingBox();
    ar.WriteChunk(100, box.x0);
    ar.WriteChunk(101, box.y0);
    ar.WriteChunk(102, box.z0);
    ar.WriteChunk(103, box.x1);
    ar.WriteChunk(104, box.y1);
    ar.WriteChunk(105, box.z1);
}

void CITKImageSource::Load(IArchive& ar)
{
    BOX tempBox;
    bool foundBox = false;

    while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case 0:
			ar.read(m_filename);
			break;
		case 1:
        {
            int type;
            ar.read(type);
            m_type = (ImageFileType)type;
            break;
        }
			

        case 100:
			ar.read(tempBox.x0);
            foundBox = true;
            break;
        case 101:
			ar.read(tempBox.y0);
            break;
        case 102:
			ar.read(tempBox.z0);
            break;
        case 103:
			ar.read(tempBox.x1);
            break;
        case 104:
			ar.read(tempBox.y1);
            break;
        case 105:
			ar.read(tempBox.z1);
            break;
		}
		ar.CloseChunk();
	}

    // Read in image data
    Load();

    // Set location of image if it was saved
    if(foundBox)
    {
        m_img->SetBoundingBox(tempBox);
    }

}

//========================================================================

CITKSeriesImageSource::CITKSeriesImageSource(CImageModel* imgModel, const std::vector<std::string>& filenames)
    : CImageSource(CImageSource::SERIES, imgModel), m_filenames(filenames)
{

}

CITKSeriesImageSource::CITKSeriesImageSource(CImageModel* imgModel)
    : CImageSource(CImageSource::SERIES, imgModel)
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

	AssignImage(im);

	return true;
}

void CITKSeriesImageSource::Save(OArchive& ar)
{
    for(auto filename : m_filenames)
    {
        ar.WriteChunk(0, filename);
    }

    BOX box = m_originalImage->GetBoundingBox();
    ar.WriteChunk(100, box.x0);
    ar.WriteChunk(101, box.y0);
    ar.WriteChunk(102, box.z0);
    ar.WriteChunk(103, box.x1);
    ar.WriteChunk(104, box.y1);
    ar.WriteChunk(105, box.z1);
}

void CITKSeriesImageSource::Load(IArchive& ar)
{
    BOX tempBox;
    bool foundBox = false;

    while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case 0:
        {
            std::string temp;
            ar.read(temp);
            m_filenames.push_back(temp);
            break;
        }

        case 100:
			ar.read(tempBox.x0);
            foundBox = true;
            break;
        case 101:
			ar.read(tempBox.y0);
            break;
        case 102:
			ar.read(tempBox.z0);
            break;
        case 103:
			ar.read(tempBox.x1);
            break;
        case 104:
			ar.read(tempBox.y1);
            break;
        case 105:
			ar.read(tempBox.z1);
            break;
		}
		ar.CloseChunk();
	}

    // Read in image data
    Load();

    // Set location of image if it was saved
    if(foundBox)
    {
        m_img->SetBoundingBox(tempBox);
    }
}

#else
//========================================================================
CITKImageSource::CITKImageSource(CImageModel* imgModel, const std::string& filename, ImageFileType type)
    : CImageSource(ImageSourceType::ITK, imgModel) {}
bool CITKImageSource::Load() { return false; }
void CITKImageSource::Save(OArchive& ar) {}
void CITKImageSource::Load(OArchive& ar) {}
//========================================================================
CITKSeriesImageSource::CITKSeriesImageSource(CImageModel* imgModel, const std::vector<std::string>& filenames)
    : CImageSource(ImageSourceType::SERIES, imgModel) {}
bool CITKSeriesImageSource::Load() { return false; }
void CITKSeriesImageSource::Save(OArchive& ar) {}
void CITKSeriesImageSource::Load(OArchive& ar) {}
#endif

//========================================================================
