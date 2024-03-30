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

#include "DICOMImageSource.h"
#include <ImageLib/ImageModel.h>
#include <ImageLib/3DImage.h>

#ifdef HAS_DCMTK
#include <dcmtk/dcmimgle/dcmimage.h>

CDICOMImageSource::CDICOMImageSource(CImageModel* imgModel, const std::vector<std::string>& filenames)
    : CImageSource(DICOM, imgModel), m_filenames(filenames)
{

}

CDICOMImageSource::CDICOMImageSource(CImageModel* imgModel)
    : CImageSource(DICOM, imgModel)
{
    
}

bool CDICOMImageSource::Load()
{
    auto tempImage = std::make_unique<DicomImage>(m_filenames[0].c_str());

    const size_t nx = tempImage->getWidth();
    const size_t ny = tempImage->getHeight();
    size_t nz;

    if (m_filenames.size() == 1)
    {
        nz = tempImage->getNumberOfFrames();
    }
    else
    { 
        nz = m_filenames.size();
    }

    const DiPixel* rawData = tempImage->getInterData();
    EP_Representation type = rawData->getRepresentation();  // An Enum that gets the type 

    int pixelType;
    int bps;

    if(type == EPR_Uint8)
    {
        pixelType = CImage::UINT_8;
        bps = 1;
    }
    else if(type == EPR_Sint8)
    {
        pixelType = CImage::INT_8;
        bps = 1;
    }
    else if(type == EPR_Uint16 )
    {
        pixelType = CImage::UINT_16;
        bps = 2;
    }
    else if(type == EPR_Sint16)
    {
        pixelType = CImage::INT_16;
        bps = 2;
    }
    else
    {
        throw std::runtime_error("Unsupported pixel type.");
    }

    BOX box(nx, ny, nz, nx+tempImage->getWidthHeightRatio(), ny+tempImage->getHeightWidthRatio(), nz+1.0);
    GetImageModel()->SetBoundingBox(box);

    size_t size = nx*ny*nz*bps;
    size_t sliceSize = nx*ny*bps;

    uint8_t* data = new uint8_t[size];

    const uint8_t* temp;

    for(int file = 0; file < m_filenames.size(); file++)
    {
        auto dicomImage = std::make_unique<DicomImage>(m_filenames[file].c_str());
        rawData = dicomImage->getInterData();

        temp = static_cast<const uint8_t*>(rawData->getData());

        for(size_t index = 0; index < sliceSize; index++)
        {
            data[file*sliceSize + index] = temp[index];
        }

    }
    // memcpy(data, static_cast<const uint8_t*>(rawData->getData()), size);

    C3DImage* im = new C3DImage();
    if (im->Create(nx, ny, nz, data, 0, pixelType) == false)
    {
        delete im;
        return false;
    }

    AssignImage(im);

    return true;
}

void CDICOMImageSource::Save(OArchive& ar)
{

}

void CDICOMImageSource::Load(IArchive& ar)
{

}
#else
CDICOMImageSource::CDICOMImageSource(CImageModel* imgModel, const std::vector<std::string>& filenames)
    : CImageSource(DICOM, imgModel) {}
CDICOMImageSource::CDICOMImageSource(CImageModel* imgModel)
    : CImageSource(DICOM, imgModel) {}

bool CDICOMImageSource::Load() { return false; }
void CDICOMImageSource::Save(OArchive& ar) {}
void CDICOMImageSource::Load(IArchive& ar) {}
#endif