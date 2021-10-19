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
#include <vector>
#include <string>
#include <memory>
#include <FSCore/box.h>
#include <FSCore/FSObjectList.h>
#include "GLImageRenderer.h"
#include <ImageLib/ImageFilter.h>
#include "GLObject.h"

enum class ImageFileType {RAW, DICOM, TIFF, OMETIFF, SEQUENCE};

class C3DImage;
class CITKImage;

namespace Post {

class CImageModel;
class CGLImageRenderer;

class CImageSource : public FSObject
{
public:
	CImageSource(CImageModel* imgModel = nullptr);
	~CImageSource();

	void SetFileName(const std::string& fileName);
	std::string GetFileName() const;

	bool LoadImageData(const std::string& fileName, int nx, int ny, int nz);
	bool LoadITKData(const std::string &filename, ImageFileType type);

	C3DImage* Get3DImage() { return m_img; }

    void ClearFilters();
    CITKImage* GetImageToFilter(bool allocate = false);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	int Width() const;
	int Height() const;
	int Depth() const;

public:
	CImageModel* GetImageModel();
	void SetImageModel(CImageModel* imgModel);

private:
    void SetValues(const std::string &fileName, int x, int y, int z);
    void AssignImage(C3DImage* im);

	C3DImage*	m_img;
    C3DImage*   m_originalImage;
	CImageModel*	m_imgModel;
    unsigned char* data = nullptr;
};

class CImageModel : public CGLObject
{
public:
	CImageModel(CGLModel* mdl);
	~CImageModel();

	bool LoadImageData(const std::string& fileName, int nx, int ny, int nz, const BOX& box);
	bool LoadITKData(const std::string &filename, ImageFileType type);

	int ImageRenderers() const { return (int)m_render.Size(); }
	CGLImageRenderer* GetImageRenderer(int i) { return m_render[i]; }
	size_t RemoveRenderer(CGLImageRenderer* render);

	void AddImageRenderer(CGLImageRenderer* render);

    int ImageFilters() const { return (int)m_filters.Size(); }
	CImageFilter* GetImageFilter(int i) { return m_filters[i]; }
	size_t RemoveFilter(CImageFilter* filter);

	void AddImageFilter(CImageFilter* imageFilter);

	const BOX& GetBoundingBox() const { return m_box; }

	void SetBoundingBox(BOX b) { m_box = b; }

	bool ShowBox() const;

	void ShowBox(bool b);

	void Render(CGLContext& rc);

	void ApplyFilters();

	bool UpdateData(bool bsave = true) override;

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	CImageSource* GetImageSource();

private:
	BOX				m_box;						//!< physical dimensions of image
	bool			m_showBox;					//!< show box in Graphics View
	FSObjectList<CGLImageRenderer>	m_render;	//!< image renderers
	FSObjectList<CImageFilter> m_filters;

	CImageSource*	m_img;
};
}
