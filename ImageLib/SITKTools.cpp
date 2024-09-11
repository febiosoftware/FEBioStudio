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

#include "SITKTools.h"
#include <cstring>

#ifdef HAS_ITK

void CopyTo3DImage(C3DImage* img, sitk::Image sitkImg)
{
    uint64_t nx = sitkImg.GetWidth();
    uint64_t ny = sitkImg.GetHeight();
    uint64_t nz = sitkImg.GetDepth();
    if(nz == 0) nz = 1;

    int pixelType = typeMap.at(sitkImg.GetPixelID());

    img->Create(nx, ny, nz, nullptr, pixelType);

    uint8_t* sitkBuff = (uint8_t*)sitkImg.GetBufferAsVoid();
    uint8_t* imgBuff = img->GetBytes();
    uint64_t size = nx * ny * nz * (uint64_t)img->BPS();

    std::memcpy(imgBuff, sitkBuff, size);

    // set physical dimensions and orientation
    std::vector<double> origin = sitkImg.GetOrigin();
    std::vector<double> spacing = sitkImg.GetSpacing();
    BOX box(origin[0],origin[1],origin[2],spacing[0]*nx+origin[0],spacing[1]*ny+origin[1],spacing[2]*nz+origin[2]);
    img->SetBoundingBox(box);

    std::vector<double> sitkOrientation = sitkImg.GetDirection();
    mat3d orientation(sitkOrientation[0], sitkOrientation[1], sitkOrientation[2], sitkOrientation[3], sitkOrientation[4], 
        sitkOrientation[5], sitkOrientation[6], sitkOrientation[7], sitkOrientation[8]);
    img->SetOrientation(orientation);
}

sitk::Image SITKImageFrom3DImage(C3DImage* img)
{
    BOX box = img->GetBoundingBox();
    unsigned int nx = img->Width();
    unsigned int ny = img->Height();
    unsigned int nz = img->Depth();

    sitk::Image sitkImg = SITKImageFromBuffer(nx, ny, nz, img->GetBytes(), img->PixelType());

    sitkImg.SetOrigin({box.x0, box.y0, box.z0});
    sitkImg.SetSpacing({(box.x1 - box.x0)/nx, (box.y1 - box.y0)/ny, (box.z1 - box.z0)/nz});

    mat3d orientation = img->GetOrientation();
    std::vector<double> orient {orientation[0][0], orientation[0][1], orientation[0][2], orientation[1][0], 
        orientation[1][1], orientation[1][2], orientation[2][0], orientation[2][1], orientation[2][2]};
    sitkImg.SetDirection(orient);

    return sitkImg;
}

sitk::Image SITKImageFromBuffer(unsigned int nx, unsigned int ny, unsigned int nz, uint8_t* data, int pixelType)
{
    sitk::ImportImageFilter filter;    
    filter.SetSize({nx, ny, nz});

    switch (pixelType)
    {
    case CImage::UINT_8:
        filter.SetBufferAsUInt8((uint8_t*)data);
        break;
    case CImage::INT_8:
        filter.SetBufferAsInt8((int8_t*)data);
        break;
    case CImage::UINT_16:
        filter.SetBufferAsUInt16((uint16_t*)data);
        break;
    case CImage::INT_16:
        filter.SetBufferAsInt16((int16_t*)data);
        break;
    case CImage::UINT_32:
        filter.SetBufferAsUInt32((uint32_t*)data);
        break;
    case CImage::INT_32:
        filter.SetBufferAsInt32((int32_t*)data);
        break;
    case CImage::UINT_RGB8:
        filter.SetBufferAsUInt8((uint8_t*)data, 3);
        break;
    case CImage::INT_RGB8:
        filter.SetBufferAsInt8((int8_t*)data, 3);
        break;
    case CImage::UINT_RGB16:
        filter.SetBufferAsUInt16((uint16_t*)data, 3);
        break;
    case CImage::INT_RGB16:
        filter.SetBufferAsInt16((int16_t*)data, 3);
        break;
    case CImage::REAL_32:
        filter.SetBufferAsFloat((float*)data);
        break;
    case CImage::REAL_64:
        filter.SetBufferAsDouble((double*)data);
        break;
    default:
        assert(false);
    }

    return filter.Execute();
}    

bool WriteSITKImage(C3DImage* img, const std::string& filename)
{
    itk::simple::Image itkImage = SITKImageFrom3DImage(img);

    sitk::ImageFileWriter writer;
    writer.SetFileName(filename);

    try
    {
        writer.Execute(itkImage);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';

        return false;
    }
    
    return true;
}

#endif