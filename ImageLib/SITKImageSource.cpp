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
#include "3DImage.h"
#include "SITKTools.h"
#include <FSCore/FSDir.h>
#include <cstring>
#include <filesystem>

using namespace Post;
namespace fs = std::filesystem;

#ifdef HAS_ITK

//========================================================================

CITKImageSource::CITKImageSource(CImageModel* imgModel, const std::string& filename, int type) 
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
    C3DImage* im = new C3DImage;

    sitk::Image sitkImage;

    if(m_fileType == CITKImageSource::DICOM)
    {
        sitk::ImageSeriesReader reader;

        string absolutePath = FSDir::fileDir(m_filename);
        const std::vector<std::string> dicom_names = sitk::ImageSeriesReader::GetGDCMSeriesFileNames(absolutePath);
        reader.SetFileNames( dicom_names );
        
        
        try {
			// this can throw exceptions. 
			// If this is called while loading the fsm file, this could cause problems.
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
			// If this is called while loading the fsm file, this could cause problems.
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

    // Copy to 3DImage
    CopyTo3DImage(im, sitkImage);

	AssignImage(im);

	return true;
}

void CITKImageSource::Save(OArchive& ar)
{
    // save image path relative to model file
    fs::path image = m_filename;
    fs::path mdl = ar.GetFilename();

    string relFilename = fs::relative(image, mdl.parent_path()).string();

    ar.WriteChunk(0, relFilename);
    ar.WriteChunk(1, m_type);
    ar.WriteChunk(2, m_fileType);

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
            m_fileType = fileType;
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

    // Convert relative file path back to absolute file path
    fs::path image = m_filename;
    fs::path mdl = ar.GetFilename();

    // Old files may have saved absolute paths
    if(image.is_relative())
    {
        m_filename = fs::absolute(mdl.parent_path() / image).string();
    }

    // Read in image data
    Load();

    // Set location of image if it was saved
    if(m_img && foundBox)
    {
		tempBox.m_valid = true;
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

bool CITKSeriesImageSource::Load()
{
    C3DImage* im = new C3DImage;

    sitk::ImageFileReader reader;
    reader.SetFileName(m_filenames[0]);

    sitk::Image slice;

    try {
        // this can throw exceptions. 
        // If this is called while loading the fsm file, this could cause problems.
        // Therefore, we catch the exception and just return false.
        slice = reader.Execute();
    }
    catch (...)
    {
        return false;
    }

    unsigned int nx = slice.GetWidth();
    unsigned int ny = slice.GetHeight();
    unsigned int nz = m_filenames.size();

    if(supportedTypes.count(slice.GetPixelID()) == 0)
    {
        delete im;

        throw std::runtime_error("FEBio Studio does not yet support " + slice.GetPixelIDTypeAsString() + " images.");
    }

    im->Create(nx, ny, nz, nullptr, typeMap.at(slice.GetPixelID()));
    uint8_t* imgBuff = im->GetBytes();
    uint64_t sliceSize = (uint64_t)nx*(uint64_t)ny*(uint64_t)im->BPS();

    try
    {
        sitk::ImageFileReader reader;

        for(int index = 0; index < m_filenames.size(); index++)
        {
            reader.SetFileName(m_filenames[index]);
            sitk::Image slice;

            try {
                // this can throw exceptions. 
                // If this is called while loading the fsm file, this could cause problems.
                // Therefore, we catch the exception and just return false.
                slice = reader.Execute();
            }
            catch (...)
            {
                return false;
            }

            if(slice.GetDimension() != 2)
            {
                throw std::runtime_error("All images in the stack must have be 2 dimensional.");
            }

            if(slice.GetPixelID() != slice.GetPixelID())
            {
                throw std::runtime_error("All images in the stack must have the same pixel type.");
            }

            if(slice.GetWidth() != nx || slice.GetHeight() != ny)
            {
                throw std::runtime_error("All images in the stack must have the same pixel dimensions");
            }
            
            uint8_t* sliceBuff = (uint8_t*)slice.GetBufferAsVoid();

            std::memcpy(imgBuff + sliceSize*index, sliceBuff, sliceSize);
        }

    }
    catch(const std::exception& e)
    {
        delete im;
        throw e;
    }

	AssignImage(im);

	return true;
}

void CITKSeriesImageSource::Save(OArchive& ar)
{
    for(auto filename : m_filenames)
    {
        // save image path relative to model file
        fs::path image = filename;
        fs::path mdl = ar.GetFilename();

        string relFilename = fs::relative(image, mdl.parent_path()).string();

        ar.WriteChunk(0, relFilename);
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

            // Convert relative file path back to absolute file path
            fs::path image = temp;
            fs::path mdl = ar.GetFilename();

            // Old files may have saved absolute paths
            if(image.is_relative())
            {
                temp = fs::absolute(mdl.parent_path() / image).string();
            }

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
    if(!Load()) return;

    // Set location of image if it was saved
    if(foundBox)
    {
        m_img->SetBoundingBox(tempBox);
    }
}

#else
//========================================================================
CITKImageSource::CITKImageSource(CImageModel* imgModel, const std::string& filename, int type)
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
