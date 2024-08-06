/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "3DImage.h"

#ifdef HAS_ITK

#include <SimpleITK.h>

class CImageSITK : public C3DImage
{
    class Imp;

public:
    static itk::simple::Image SITKImageFrom3DImage(C3DImage* img);
    static bool WriteSITKImage(C3DImage* img, const std::string& filename);

private:
    static itk::simple::Image SITKImageFromBuffer(unsigned int nx, unsigned int ny, unsigned int nz, uint8_t* data, int pixelType);

public:
    CImageSITK();
    ~CImageSITK();

    bool Create(int nx, int ny, int nz, uint8_t* data = nullptr, int pixelType = CImage::UINT_8) override;

    BOX GetBoundingBox() override;
    void SetBoundingBox(BOX& box) override;

    mat3d GetOrientation() override;
    void SetOrientation(mat3d& orientation) override;

    itk::simple::Image GetSItkImage();
    void SetItkImage(itk::simple::Image image, bool setBuffer = true);

private:
    itk::simple::Image m_sitkImage;
    bool m_itkOwnsBuffer;
};

#endif