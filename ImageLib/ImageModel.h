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
#include <PostLib/GLImageRenderer.h>
#include <ImageLib/ImageFilter.h>
#include <ImageLib/ImageAnalysis.h>
#include <PostLib/GLObject.h>
#include <FEBioStudio/ImageViewSettings.h>

namespace Post
{
    class CGLImageRenderer;
}

class CImageSource;
class C3DImage;

class CImageModel : public Post::CGLObject
{
public:
	CImageModel(Post::CGLModel* mdl);
	~CImageModel();

    void SetImageSource(CImageSource* imgSource);
    bool Load();

	void Reload();

	int ImageRenderers() const { return (int)m_render.Size(); }
	Post::CGLImageRenderer* GetImageRenderer(int i) { return m_render[i]; }
	size_t RemoveRenderer(Post::CGLImageRenderer* render);
    void UpdateRenderers();

	void AddImageRenderer(Post::CGLImageRenderer* render);

    // Applies filters using GUI thread!
    void ApplyFilters();
    void ClearFilters();
    int ImageFilters() const { return (int)m_filters.Size(); }
	CImageFilter* GetImageFilter(int i) { return m_filters[i]; }
	void RemoveFilter(CImageFilter* filter);
    void MoveFilter(int fromIndex, int toIndex);
	void AddImageFilter(CImageFilter* imageFilter);
    void FilterPropsChanged() { m_unappliedFilters = true; }
    bool AreFiltersUnapplied() { return m_unappliedFilters; }

    int ImageAnalyses() const { return (int)m_analyses.Size(); }
	CImageAnalysis* GetImageAnalysis(int i) { return m_analyses[i]; }
    void AddImageAnalysis(CImageAnalysis* analysis) { m_analyses.Add(analysis); }
	void RemoveAnalysis(CImageAnalysis* analysis);

	BOX GetBoundingBox();

	void SetBoundingBox(BOX b);

	mat3d GetOrientation();

	bool ShowBox() const;

	void ShowBox(bool b);

	void Render(CGLContext& rc);

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	CImageSource* GetImageSource();

    CImageViewSettings* GetViewSettings() { return &viewSettings; }

	C3DImage* Get3DImage();

public:
	bool ExportRAWImage(const std::string& filename);
    bool ExportSITKImage(const std::string& filename);

private:
	bool			m_showBox;					//!< show box in Graphics View
    bool m_unappliedFilters;	//!< filters applied
	FSObjectList<Post::CGLImageRenderer>	m_render;	//!< image renderers
	FSObjectList<CImageFilter> m_filters;
    FSObjectList<CImageAnalysis> m_analyses;

	CImageSource*	m_img;

    CImageViewSettings viewSettings;
};
