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

#include "3DImage.h"
#include <string>
#include <unordered_set>
#include <unordered_map>

#ifdef HAS_ITK

#include <SimpleITK.h>

namespace sitk = itk::simple;

static const std::unordered_set<sitk::PixelIDValueEnum> supportedTypes = {sitk::sitkInt8, sitk::sitkUInt8, sitk::sitkInt16, 
    sitk::sitkUInt16, sitk::sitkInt32, sitk::sitkUInt32, sitk::sitkVectorInt8, sitk::sitkVectorUInt8, sitk::sitkVectorInt16, 
    sitk::sitkVectorUInt16, sitk::sitkFloat32, sitk::sitkFloat64};

static const std::unordered_map<sitk::PixelIDValueEnum, int> typeMap = {{sitk::sitkInt8, CImage::INT_8}, 
    {sitk::sitkUInt8, CImage::UINT_8}, {sitk::sitkInt16, CImage::INT_16}, {sitk::sitkUInt16, CImage::UINT_16}, 
    {sitk::sitkInt32, CImage::INT_32}, {sitk::sitkUInt32, CImage::UINT_32}, {sitk::sitkVectorInt8, CImage::INT_RGB8}, 
    {sitk::sitkVectorUInt8, CImage::UINT_RGB8}, {sitk::sitkVectorInt16, CImage::INT_RGB16}, 
    {sitk::sitkVectorUInt16, CImage::UINT_RGB16}, {sitk::sitkFloat32, CImage::REAL_32}, {sitk::sitkFloat64, CImage::REAL_64}};

void CopyTo3DImage(C3DImage* img, sitk::Image sitkImg);

sitk::Image SITKImageFrom3DImage(C3DImage* img);
sitk::Image SITKImageFromBuffer(unsigned int nx, unsigned int ny, unsigned int nz, uint8_t* data, int pixelType);

bool WriteSITKImage(C3DImage* img, const std::string& filename);

#endif