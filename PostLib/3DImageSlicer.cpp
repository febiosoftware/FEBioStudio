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
#include "3DImageSlicer.h"

using namespace Post;

C3DImageSlicer::C3DImageSlicer(CImageModel* img) : 
	m_xSlicer(nullptr),
	m_ySlicer(nullptr),
	m_zSlicer(nullptr)
{
	m_xSlicer.SetOrientation(0);
	m_ySlicer.SetOrientation(1);
	m_zSlicer.SetOrientation(2);
}

void C3DImageSlicer::SetImageModel(CImageModel* img)
{
	CGLImageRenderer::SetImageModel(img);
	m_xSlicer.SetImageModel(img);
	m_ySlicer.SetImageModel(img);
	m_zSlicer.SetImageModel(img);
}

void C3DImageSlicer::SetSliceImage(int slice, float offset, CImage* img)
{
	CImageSlicer* p = nullptr;
	switch (slice)
	{
	case 0: p = &m_xSlicer; break;
	case 1: p = &m_ySlicer; break;
	case 2: p = &m_zSlicer; break;
	default:
		return;
	}

	p->SetOffset(offset);
	p->SetImageSlice(img);
}

void C3DImageSlicer::Render(GLRenderEngine& re, CGLContext& rc)
{
	if (GetImageModel() == nullptr) return;

	m_xSlicer.Render(re, rc);
	m_ySlicer.Render(re, rc);
	m_zSlicer.Render(re, rc);
}
