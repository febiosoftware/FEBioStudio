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

#pragma once
#include <ImageLib/3DImage.h>
#include <FSCore/box.h>
#include "GLImageRenderer.h"
#include <ImageLib/RGBAImage.h>
#include <PostGL/ColorTexture.h>
#include "ColorMap.h"

class CImageModel;
class CImage;

namespace Post {

class CImageSlicer : public CGLImageRenderer
{
	enum { ORIENTATION, OFFSET, COLOR_MAP, TRANSPARENCY };

public:
	CImageSlicer(CImageModel* img);
	~CImageSlicer();

	void Create();

	void Update() override;

	void Render(GLRenderEngine& re, GLContext& rc) override;

	int GetOrientation() const;
	void SetOrientation(int n);

	double GetOffset() const;
	void SetOffset(double f);

	int GetColorMap() const { return m_Col.GetColorMap(); }
	void SetColorMap(int n) { m_Col.SetColorMap(n); }

	bool UpdateData(bool bsave = true) override;

    void SetImageSlice(CImage* img);

private:
	void BuildLUT();

	void UpdateSlice();

    template<class pType>
    void CreateCRGBAImage(CImage& slice);

private:
	CRGBAImage		m_im;	// 2D image that will be displayed
	int				m_LUTC[4][256];	// color lookup table
	bool			m_reloadTexture;

    CImage* m_imageSlice; // optional slice of image to be rendered instead of the calculated slice

	Post::CColorTexture	m_Col;

	unsigned int m_texID;
};
}
