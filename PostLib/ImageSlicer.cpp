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

#include "stdafx.h"
#include "ImageSlicer.h"
#include <ImageLib/ImageModel.h>
#include <FSCore/ColorMapManager.h>
#include <assert.h>
#include <sstream>

using std::stringstream;
using namespace Post;


CImageSlicer::CImageSlicer(CImageModel* img) : m_imageSlice(nullptr), CGLImageRenderer(img)
{
	static int n = 1;
	stringstream ss;
	ss << "ImageSlicer" << n++;
	SetName(ss.str());

	AddIntParam(2, "Image orientation")->SetEnumNames("X\0Y\0Z\0");
	AddDoubleParam(0.5, "Image offset")->SetFloatRange(0.0, 1.0);
	AddIntParam(0, "Color map")->SetEnumNames("@color_map");
	AddDoubleParam(1, "Transparency")->SetFloatRange(0.0, 1.0);

	m_Col.SetColorMap(ColorMapManager::GRAY);

	UpdateData(false);
}

CImageSlicer::~CImageSlicer()
{
}

int CImageSlicer::GetOrientation() const { return GetIntValue(ORIENTATION); }
void CImageSlicer::SetOrientation(int n) { SetIntValue(ORIENTATION, n); };

double CImageSlicer::GetOffset() const { return GetFloatValue(OFFSET); }
void CImageSlicer::SetOffset(double f) { SetFloatValue(OFFSET, f); }

bool CImageSlicer::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		UpdateSlice();
	}
	else
	{
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
	}

	return false;
}

void CImageSlicer::SetImageSlice(CImage* img)
{
    // This slice needs to be converted to 8 bit before assigned
    assert(img->PixelType() == CImage::UINT_8 || img->PixelType() == CImage::UINT_RGB8);

    m_imageSlice = img; 
    UpdateSlice();
}

void CImageSlicer::Create()
{
	C3DImage* img = GetImageModel()->Get3DImage();
	if (img == nullptr) return;

	// call update to initialize all other data
	Update();
}

void CImageSlicer::Update()
{
	UpdateData();
	UpdateSlice();
}

template<class pType> void CImageSlicer::CreateCRGBAImage(CImage& slice)
{
    auto imgModel = GetImageModel();

    size_t N  = slice.Width() * slice.Height();
    if(slice.IsRGB())
    {
        N *= 3;
    }

    pType* imgData = (pType*)imgModel->Get3DImage()->GetBytes();

    double min, max;
    imgModel->Get3DImage()->GetMinMax(min, max);

    // The Image settings panel isn't available in the post view
    // double minThresh = m_imgModel->GetViewSettings()->GetFloatValue(CImageViewSettings::MIN_INTENSITY);
    // double maxThresh = m_imgModel->GetViewSettings()->GetFloatValue(CImageViewSettings::MAX_INTENSITY);

	// create the 2D image
	m_im.Create(slice.Width(), slice.Height());

	// colorize the image
	pType* ps = (pType*)slice.GetBytes();
	uint8_t* pd = m_im.GetBytes();

    if(slice.IsRGB())
    {
        for (int i = 0; i<N; i++, ps+=3, pd += 4)
        {
            int val1 = 255*(((double)ps[0])-min)/(max-min);
            int val2 = 255*(((double)ps[1])-min)/(max-min);
            int val3 = 255*(((double)ps[2])-min)/(max-min);

            pd[0] = m_LUTC[0][val1];
            pd[1] = m_LUTC[1][val2];
            pd[2] = m_LUTC[2][val3];
            pd[3] = m_LUTC[3][255];
        }
    }
    else
    {
        for (int i = 0; i<N; i++, ps++, pd += 4)
        {
            int val = 255*(((double)*ps)-min)/(max-min);
            pd[0] = m_LUTC[0][val];
            pd[1] = m_LUTC[1][val];
            pd[2] = m_LUTC[2][val];
            pd[3] = m_LUTC[3][val];
        }
    }
}

void CImageSlicer::UpdateSlice()
{
	C3DImage& im3d = *GetImageModel()->Get3DImage();

    // build the looktp table
	BuildLUT();

	int nop = GetOrientation();
	double off = GetOffset();

    // If a manual slice has been passed in, use it. Otherwise create one from the 3DImage
    // It is assumed that this manual slice has already been converted to 8 bit
    if(m_imageSlice)
    {
        // get the image dimensions
        int W = m_imageSlice->Width();
        int H = m_imageSlice->Height();

        // create the 2D image
        m_im.Create(W, H);

        // colorize the image
        int nn = W*H;
        uint8_t* ps = m_imageSlice->GetBytes();
        uint8_t* pd = m_im.GetBytes();

        if(m_imageSlice->PixelType() == CImage::UINT_8)
        {
            for (int i = 0; i<nn; i++, ps++, pd += 4)
            {
                int val = *ps;
                pd[0] = m_LUTC[0][val];
                pd[1] = m_LUTC[1][val];
                pd[2] = m_LUTC[2][val];
                pd[3] = m_LUTC[3][val];
            }
        }
        else if(m_imageSlice->PixelType() == CImage::UINT_RGB8)
        {
            for (int i = 0; i<nn; i++, ps+=3, pd += 4)
            {
                pd[0] = ps[0];
                pd[1] = ps[1];
                pd[2] = ps[2];
                pd[3] = 255;
            }
        }
        else
        {
            assert(false);
        }

        
    }
    else
    {
        // For 2D images, the X, Y options shouldn't do anything
        if (im3d.Depth() == 1)
        {
            nop = 2;
        }

        CImage slice;
        switch (nop)
        {
        case 0: // X
            im3d.GetSampledSliceX(slice, off);
            break;
        case 1: // Y
            im3d.GetSampledSliceY(slice, off);
            break;
        case 2: // Z
            im3d.GetSampledSliceZ(slice, off);
            break;
        default:
            assert(false);
        }

        switch (im3d.PixelType())
        {
        case CImage::UINT_8:
            CreateCRGBAImage<uint8_t>(slice);
            break;
        case CImage::INT_8:
            CreateCRGBAImage<int8_t>(slice);
            break;
        case CImage::UINT_16:
            CreateCRGBAImage<uint16_t>(slice);
            break;
        case CImage::INT_16:
            CreateCRGBAImage<int16_t>(slice);
            break;
        case CImage::UINT_32:
            CreateCRGBAImage<uint32_t>(slice);
            break;
        case CImage::INT_32:
            CreateCRGBAImage<int32_t>(slice);
            break;
        case CImage::UINT_RGB8:
            CreateCRGBAImage<uint8_t>(slice);
            break;
        case CImage::INT_RGB8:
            CreateCRGBAImage<int8_t>(slice);
            break;
        case CImage::UINT_RGB16:
            CreateCRGBAImage<uint16_t>(slice);
            break;
        case CImage::INT_RGB16:
            CreateCRGBAImage<int16_t>(slice);
            break;
        case CImage::REAL_32:
            CreateCRGBAImage<float>(slice);
            break;
        case CImage::REAL_64:
            CreateCRGBAImage<double>(slice);
            break;
        default:
            assert(false);
        }
    }

	m_tex.SetImage(&m_im);
}

void CImageSlicer::BuildLUT()
{
	CColorMap& map = m_Col.ColorMap();

	float f = GetFloatValue(TRANSPARENCY);
	uint8_t a = uint8_t(255.f * f);

	// build the LUT
	for (int i = 0; i<256; ++i)
	{
		float w = (float)i / 255.f;
		GLColor c = map.map(w);
		m_LUTC[0][i] = c.r;
		m_LUTC[1][i] = c.g;
		m_LUTC[2][i] = c.b;
		m_LUTC[3][i] = a;
	}
}

//! Render textures
void CImageSlicer::Render(GLRenderEngine& re, GLContext& rc)
{
	int nx = m_im.Width();
	int ny = m_im.Height();

	BOX box = GetImageModel()->GetBoundingBox();

	re.setMaterial(GLMaterial::CONSTANT, GLColor::White(), GLMaterial::TEXTURE_2D, false);
	re.setTexture(m_tex);

	int nop = GetOrientation();
	double off = GetOffset();

	vec3d r0(0,0,0);
	vec3d r1 = box.r1() - box.r0();

	double x[4] = { 0 }, y[4] = { 0 }, z[4] = { 0 };
	switch (nop)
	{
	case 0:
		x[0] = x[1] = x[2] = x[3] = r0.x + off*(r1.x - r0.x);
		y[0] = y[3] = r1.y;  y[1] = y[2] = r0.y;
		z[0] = z[1] = r0.z;  z[2] = z[3] = r1.z;
		break;
	case 1:
		y[0] = y[1] = y[2] = y[3] = r0.y + off*(r1.y - r0.y);
		x[0] = x[3] = r1.x;  x[1] = x[2] = r0.x;
		z[0] = z[1] = r0.z;  z[2] = z[3] = r1.z;
		break;
	case 2:
		z[0] = z[1] = z[2] = z[3] = r0.z + off*(r1.z - r0.z);
		x[0] = x[3] = r1.x;  x[1] = x[2] = r0.x;
		y[0] = y[1] = r0.y;  y[2] = y[3] = r1.y;
		break;
	}

	re.begin(GLRenderEngine::QUADS);
	{
		re.texCoord2d(1, 0); re.vertex(vec3d(x[0], y[0], z[0]));
		re.texCoord2d(0, 0); re.vertex(vec3d(x[1], y[1], z[1]));
		re.texCoord2d(0, 1); re.vertex(vec3d(x[2], y[2], z[2]));
		re.texCoord2d(1, 1); re.vertex(vec3d(x[3], y[3], z[3]));
	}
	re.end();
}
