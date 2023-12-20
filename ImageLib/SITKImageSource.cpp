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

#include "SITKImageSource.h"
#include "ImageModel.h"
#include <ImageLib/ImageSITK.h>
#include <FSCore/FSDir.h>
#include <unordered_set>

using namespace Post;

#ifdef HAS_ITK

namespace sitk = itk::simple;

static const std::unordered_set<sitk::PixelIDValueEnum> supportedTypes = {sitk::sitkInt8, sitk::sitkUInt8, sitk::sitkInt16, 
    sitk::sitkUInt16, sitk::sitkInt32, sitk::sitkUInt32, sitk::sitkVectorInt8, sitk::sitkVectorUInt8, sitk::sitkVectorInt16, 
    sitk::sitkVectorUInt16, sitk::sitkFloat32, sitk::sitkFloat64};

//========================================================================

CITKImageSource::CITKImageSource(CImageModel* imgModel, const std::string& filename, ImageFileType type) 
    : CImageSource(CImageSource::ITK, imgModel), m_filename(filename), m_fileType(type)
{
    SetName(FSDir::fileName(filename));
}

CITKImageSource::CITKImageSource(CImageModel* imgModel)
    : CImageSource(CImageSource::ITK, imgModel)
{

}

bool CITKImageSource::Load()
{
    CImageSITK* im = new CImageSITK;

    sitk::Image sitkImage;

    if(m_fileType == ImageFileType::DICOM)
    {
        sitk::ImageSeriesReader reader;

        string absolutePath = FSDir::fileDir(m_filename);
        const std::vector<std::string> dicom_names = sitk::ImageSeriesReader::GetGDCMSeriesFileNames(absolutePath);
        reader.SetFileNames( dicom_names );
        
        
        try {
			// this can throw exceptions. 
			// If this is called while loading the fs2 file, this could cause problems.
			// Therefore, we catch the exception and just retrn false.
			sitkImage = reader.Execute();

            // Sometimes the origin and spacing info is stored in the DICOM headers
            sitk::ImageFileReader headerReader;
            headerReader.SetFileName(dicom_names[0]);
            headerReader.LoadPrivateTagsOn();
            headerReader.ReadImageInformation();

            ////// origin
            // "Image Position (Patient)" tag
            string imgPosTag = "0020|0032";
            if(headerReader.HasMetaDataKey(imgPosTag))
            {
                string val = headerReader.GetMetaData(imgPosTag);

                std::vector<double> origin = sitkImage.GetOrigin();
                
                int pos = 0;
                for(int i = 0; i < origin.size(); i++)
                {
                    int oldPos = pos;

                    pos = val.find('\\', pos);

                    if(pos >= 0) pos++;

                    origin[i] = atof(val.substr(oldPos, pos - oldPos).c_str());
                    
                    if(pos == -1)
                    {
                        break;
                    }
                }

                sitkImage.SetOrigin(origin);
            }

            ////// Direction
            // "Image Orientation (Patient)" tag
            string imgOrientTag = "0020|0037";
            if(headerReader.HasMetaDataKey(imgOrientTag))
            {
                string val = headerReader.GetMetaData(imgOrientTag);

                std::vector<double> dicomOr(6,0);
                
                int pos = 0;
                for(int i = 0; i < 6; i++)
                {
                    int oldPos = pos;

                    pos = val.find('\\', pos);

                    if(pos >= 0) pos++;

                    dicomOr[i] = atof(val.substr(oldPos, pos - oldPos).c_str());
                    
                    if(pos == -1)
                    {
                        break;
                    }
                }

                vec3d a(dicomOr[0], dicomOr[1], dicomOr[2]);
                vec3d b(dicomOr[3], dicomOr[4], dicomOr[5]);

                vec3d c = a^b;

                std::vector<double> orientation {a.x, b.x, c.x, a.y, b.y, c.y, a.z, b.z, c.z};

                sitkImage.SetDirection(orientation);
            }

            ////// spacing
            // "Pixel Spacing" tag
            string spacingTag = "0028|0030";
            if(headerReader.HasMetaDataKey(spacingTag))
            {
                string val = headerReader.GetMetaData(spacingTag);

                std::vector<double> spacing = sitkImage.GetSpacing();
                
                int pos = 0;
                for(int i = 0; i < spacing.size(); i++)
                {
                    int oldPos = pos;

                    pos = val.find('\\', pos);

                    if(pos >= 0) pos++;

                    spacing[i] = atof(val.substr(oldPos, pos - oldPos).c_str());
                    
                    if(pos == -1)
                    {
                        break;
                    }
                }

                sitkImage.SetSpacing(spacing);
            }
		}
		catch (...)
		{
			return false;
		}
    }
    else
    {
        sitk::ImageFileReader reader;
        reader.SetFileName(m_filename);

		try {
			// this can throw exceptions. 
			// If this is called while loading the fs2 file, this could cause problems.
			// Therefore, we catch the exception and just return false.
            sitkImage = reader.Execute();
		}
		catch (...)
		{
			return false;
		}
    }

    if(supportedTypes.count(sitkImage.GetPixelID()) == 0)
    {
        delete im;

        throw std::runtime_error("FEBio Studio does not yet support " + sitkImage.GetPixelIDTypeAsString() + " images.");
    }

    im->SetItkImage(sitkImage);

	AssignImage(im);

	return true;
}

void CITKImageSource::Save(OArchive& ar)
{
    ar.WriteChunk(0, m_filename);
    ar.WriteChunk(1, (int)m_type);
    ar.WriteChunk(2, (int)m_fileType);

	if (m_originalImage)
	{
		BOX box = m_originalImage->GetBoundingBox();
		ar.WriteChunk(100, box.x0);
		ar.WriteChunk(101, box.y0);
		ar.WriteChunk(102, box.z0);
		ar.WriteChunk(103, box.x1);
		ar.WriteChunk(104, box.y1);
		ar.WriteChunk(105, box.z1);
	}
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
            m_type = type;
            break;
        }
        case 2:
        {
            int fileType;
            ar.read(fileType);
            m_fileType = (ImageFileType)fileType;
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
    if(m_img && foundBox)
    {
        m_img->SetBoundingBox(tempBox);
    }

}

//========================================================================

CITKSeriesImageSource::CITKSeriesImageSource(CImageModel* imgModel, const std::vector<std::string>& filenames)
    : CImageSource(CImageSource::SERIES, imgModel), m_filenames(filenames)
{
    SetName(FSDir::fileName(filenames[0]));
}

CITKSeriesImageSource::CITKSeriesImageSource(CImageModel* imgModel)
    : CImageSource(CImageSource::SERIES, imgModel)
{

}

template<class pType> void CITKSeriesImageSource::CopySliceData(pType* buffer, int pixelType, int nx, int ny)
{
    sitk::ImageFileReader reader;

    for(auto filename : m_filenames)
    {
        reader.SetFileName(filename);
        sitk::Image slice = reader.Execute();

        if(slice.GetDimension() != 2)
        {
            throw std::runtime_error("All images in the stack must have be 2 dimensional.");
        }

        if(slice.GetPixelID() != pixelType)
        {
            throw std::runtime_error("All images in the stack must have the same pixel type.");
        }

        if(slice.GetWidth() != nx || slice.GetHeight() != ny)
        {
            throw std::runtime_error("All images in the stack must have the same pixel dimensions");
        }
        
        pType* sliceBytes = (pType*)slice.GetBufferAsVoid();
        for(int index = 0; index < nx*ny; index++)
        {
            *buffer = sliceBytes[index];
            buffer++;
        }
    }
}

bool CITKSeriesImageSource::Load()
{
    CImageSITK* im = new CImageSITK;

    sitk::ImageFileReader reader;
    reader.SetFileName(m_filenames[0]);
    sitk::Image slice = reader.Execute();

    unsigned int nx = slice.GetWidth();
    unsigned int ny = slice.GetHeight();
    unsigned int nz = m_filenames.size();

    if(supportedTypes.count(slice.GetPixelID()) == 0)
    {
        delete im;

        throw std::runtime_error("FEBio Studio does not yet support " + slice.GetPixelIDTypeAsString() + " images.");
    }

    sitk::Image sitkImage(nx, ny, nz, slice.GetPixelID());

    try
    {
        switch (slice.GetPixelID())
        {
        case sitk::sitkUInt8:
            CopySliceData<uint8_t>(sitkImage.GetBufferAsUInt8(), slice.GetPixelID(), nx, ny);
            break;
        case sitk::sitkInt8:
            CopySliceData<int8_t>(sitkImage.GetBufferAsInt8(), slice.GetPixelID(), nx, ny);
            break;
        case sitk::sitkUInt16:
            CopySliceData<uint16_t>(sitkImage.GetBufferAsUInt16(), slice.GetPixelID(), nx, ny);
            break;
        case sitk::sitkInt16:
            CopySliceData<int16_t>(sitkImage.GetBufferAsInt16(), slice.GetPixelID(), nx, ny);
            break;
        case sitk::sitkVectorUInt8:
            CopySliceData<uint8_t>(sitkImage.GetBufferAsUInt8(), slice.GetPixelID(), nx, ny);
            break;
        case sitk::sitkVectorInt8:
            CopySliceData<int8_t>(sitkImage.GetBufferAsInt8(), slice.GetPixelID(), nx, ny);
            break;
        case sitk::sitkVectorUInt16:
            CopySliceData<uint16_t>(sitkImage.GetBufferAsUInt16(), slice.GetPixelID(), nx, ny);
            break;
        case sitk::sitkVectorInt16:
            CopySliceData<int16_t>(sitkImage.GetBufferAsInt16(), slice.GetPixelID(), nx, ny);
            break;
        case sitk::sitkFloat32:
            CopySliceData<float>(sitkImage.GetBufferAsFloat(), slice.GetPixelID(), nx, ny);
            break;
        case sitk::sitkFloat64:
            CopySliceData<double>(sitkImage.GetBufferAsDouble(), slice.GetPixelID(), nx, ny);
            break;
        }
    }
    catch(const std::exception& e)
    {
        delete im;
        throw e;
    }
    
    im->SetItkImage(sitkImage);

	AssignImage(im);

	return true;
}

void CITKSeriesImageSource::Save(OArchive& ar)
{
    for(auto filename : m_filenames)
    {
        ar.WriteChunk(0, filename);
    }

	if (m_originalImage)
	{
		BOX box = m_originalImage->GetBoundingBox();
		ar.WriteChunk(100, box.x0);
		ar.WriteChunk(101, box.y0);
		ar.WriteChunk(102, box.z0);
		ar.WriteChunk(103, box.x1);
		ar.WriteChunk(104, box.y1);
		ar.WriteChunk(105, box.z1);
	}
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
    : CImageSource(0, imgModel) {}
CITKImageSource::CITKImageSource(CImageModel* imgModel) : CImageSource(0, imgModel) {}
bool CITKImageSource::Load() { return false; }
void CITKImageSource::Save(OArchive& ar) {}
void CITKImageSource::Load(IArchive& ar) {}
//========================================================================
CITKSeriesImageSource::CITKSeriesImageSource(CImageModel* imgModel, const std::vector<std::string>& filenames)
    : CImageSource(0, imgModel) {}
CITKSeriesImageSource::CITKSeriesImageSource(CImageModel* imgModel) : CImageSource(0, imgModel) {}
bool CITKSeriesImageSource::Load() { return false; }
void CITKSeriesImageSource::Save(OArchive& ar) {}
void CITKSeriesImageSource::Load(IArchive& ar) {}
#endif

//========================================================================
