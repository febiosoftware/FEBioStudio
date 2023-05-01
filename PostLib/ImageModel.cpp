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
#include "ImageModel.h"
#include "ImageSource.h"
#include <ImageLib/3DImage.h>
#include <ImageLib/ImageSITK.h>
#include <ImageLib/ImageFilter.h>
#include <ImageLib/FiberODFAnalysis.h>
#include "GLImageRenderer.h"
#include <PostLib/VolumeRenderer.h>
#include <PostLib/TiffReader.h>
#include <FSCore/FSDir.h>
#include <assert.h>

using namespace Post;

enum SaveIDs { BASE = 0, IMAGESOURCE, FILTERS, ANALYSES, VIEWSETTINGS};

CImageModel::CImageModel(CGLModel* mdl) : CGLObject(mdl)
{
	m_showBox = true;
	m_img = nullptr;
}

CImageModel::~CImageModel()
{
	delete m_img;
}

void CImageModel::SetImageSource(CImageSource* imgSource)
{
    if(m_img)
    {
        delete m_img;
        m_img = nullptr;
    }

    m_img = imgSource;
}

bool CImageModel::Load()
{
    if(!m_img) return false;

    if (!m_img->Load())
	{
		delete m_img;
		m_img = nullptr;
		return false;
	}

	return true;
}

bool CImageModel::ShowBox() const
{
	return m_showBox;
}

void CImageModel::ShowBox(bool b)
{
	m_showBox = b;
}

void CImageModel::Render(CGLContext& rc)
{
	// render the volume image data if present
	if (IsActive())
	{
		for (int j = 0; j < ImageRenderers(); ++j)
		{
			Post::CGLImageRenderer* pir = GetImageRenderer(j);
			if (pir && pir->IsActive())
			{
//				if (pir->AllowClipping()) CGLPlaneCutPlot::EnableClipPlanes();
//				else CGLPlaneCutPlot::DisableClipPlanes();
				pir->Render(rc);
			}
		}
	}
}

void CImageModel::ApplyFilters()
{
    m_img->ClearFilters();

	for(int index = 0; index < m_filters.Size(); index++)
	{
		m_filters[index]->ApplyFilter();
	}

	for (int i = 0; i < (int)m_render.Size(); ++i)
	{
		m_render[i]->Update();
	} 
}

void CImageModel::ClearFilters()
{
    m_img->ClearFilters();
}

size_t CImageModel::RemoveRenderer(CGLImageRenderer* render)
{
	return m_render.Remove(render);
}

void CImageModel::UpdateRenderers()
{
    for (int i = 0; i < (int)m_render.Size(); ++i)
	{
		m_render[i]->Update();
	} 
}

void CImageModel::AddImageRenderer(CGLImageRenderer* render)
{
	assert(render);
	m_render.Add(render);
}

size_t CImageModel::RemoveFilter(CImageFilter* filter)
{
    return m_filters.Remove(filter);
}

void CImageModel::AddImageFilter(CImageFilter* imageFilter)
{
    imageFilter->SetImageModel(this);
	m_filters.Add(imageFilter);
}

size_t CImageModel::RemoveAnalysis(CImageAnalysis* analysis)
{
    return m_analyses.Remove(analysis);
}

BOX CImageModel::GetBoundingBox()
{
    if(m_img && m_img->Get3DImage())
    {
        return m_img->Get3DImage()->GetBoundingBox();
    }

    return BOX(0,0,0,1,1,1);
}

void CImageModel::SetBoundingBox(BOX b)
{
    if(m_img && m_img->Get3DImage())
    {
        m_img->Get3DImage()->SetBoundingBox(b);
    }
}

void CImageModel::Save(OArchive& ar)
{
	ar.BeginChunk(BASE);
	{
		FSObject::Save(ar);
	}
	ar.EndChunk();

	if (m_img)
	{
		ar.BeginChunk(IMAGESOURCE);
		{
            ar.BeginChunk((int)m_img->Type());
            {
                m_img->Save(ar);
            }
            ar.EndChunk();
		}
		ar.EndChunk();
	}

	if (m_filters.IsEmpty() == false)
	{
		ar.BeginChunk(FILTERS);
		{
			for (int index = 0; index < m_filters.Size(); index++)
			{
				ar.BeginChunk(m_filters[index]->Type());
				{
					m_filters[index]->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

    if (m_analyses.IsEmpty() == false)
	{
		ar.BeginChunk(ANALYSES);
		{
			for (int index = 0; index < m_analyses.Size(); index++)
			{
				ar.BeginChunk(m_analyses[index]->Type());
				{
					m_analyses[index]->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

    ar.BeginChunk(VIEWSETTINGS);
    {
        viewSettings.Save(ar);
    }
    ar.EndChunk();
}

CImageSource* CImageModel::GetImageSource()
{ 
	return m_img; 
}

Byte CImageModel::ValueAtGlobalPos(vec3d pos)
{
    BOX box = m_img->Get3DImage()->GetBoundingBox();

    if(pos.x < box.x0 || pos.x > box.x1 ||
        pos.y < box.y0 || pos.y > box.y1 ||
        pos.z < box.z0 || pos.z > box.z1)
    {
        return 0;
    }

	if (Get3DImage()->Depth() <= 1)
	{
		double x = (pos.x - box.x0) / (box.x1 - box.x0);
		double y = (pos.y - box.y0) / (box.y1 - box.y0);
		return m_img->Get3DImage()->Value(x, y, 0);
	}
	else
	{
		double x = (pos.x - box.x0) / (box.x1 - box.x0);
		double y = (pos.y - box.y0) / (box.y1 - box.y0);
		double z = (pos.z - box.z0) / (box.z1 - box.z0);

		return m_img->Get3DImage()->Peek(x, y, z);
	}
}

C3DImage* CImageModel::Get3DImage()
{ 
    return (m_img ? m_img->Get3DImage() : nullptr); 
}

void CImageModel::Load(IArchive& ar)
{
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case BASE:
			FSObject::Load(ar);
			break;
		case IMAGESOURCE:
			{
				while (ar.OpenChunk() == IArchive::IO_OK)
                {
                    int nid2 = ar.GetChunkID();

                    switch (nid2)
                    {
                    case CImageSource::RAW:
                        m_img = new CRawImageSource(this);
                        m_img->Load(ar);
                        break;
                    case CImageSource::ITK:
                        m_img = new CITKImageSource(this);
                        m_img->Load(ar);
                        break;
                    case CImageSource::SERIES:
                        m_img = new CITKSeriesImageSource(this);
                        m_img->Load(ar);
                        break;
                    case CImageSource::TIFF:
                        m_img = new CTiffImageSource(this);
                        m_img->Load(ar);
                        break;
                    default:
                        break;
                    }
					ar.CloseChunk();
				}
			}
			break;
        case FILTERS:
			{
				while (ar.OpenChunk() == IArchive::IO_OK)
                {
                    int nid2 = ar.GetChunkID();

                    switch (nid2)
                    {
                    case CImageFilter::THRESHOLD:
                    {
                        auto temp = new ThresholdImageFilter(this);
                        temp->Load(ar);
                        m_filters.Add(temp);
                        break;
                    }
                    case CImageFilter::MEAN:
                    {
                        auto temp = new MeanImageFilter(this);
                        temp->Load(ar);
                        m_filters.Add(temp);
                    }
                    case CImageFilter::GAUSSBLUR:
                    {
                        auto temp = new GaussianImageFilter(this);
                        temp->Load(ar);
                        m_filters.Add(temp);
                    }
                    case CImageFilter::ADAPTHISTEQ:
                    {
                        auto temp = new AdaptiveHistogramEqualizationFilter(this);
                        temp->Load(ar);
                        m_filters.Add(temp);
                    }
                    default:
                        break;
                    }
					ar.CloseChunk();
				}
			}
			break;
        case ANALYSES:
			{
				while (ar.OpenChunk() == IArchive::IO_OK)
                {
                    int nid2 = ar.GetChunkID();

                    switch (nid2)
                    {
                    case CImageAnalysis::FIBERODF:
                    {
                        auto temp = new CFiberODFAnalysis(this);
                        temp->Load(ar);
                        m_analyses.Add(temp);
                        break;
                    }
                    default:
                        break;
                    }
					ar.CloseChunk();
				}
			}
			break;
        case VIEWSETTINGS:
            viewSettings.Load(ar);
            break;
        default:
            break;
		}
		ar.CloseChunk();
	}

    ApplyFilters();

    // Create Volume Renderer
    auto vr = new Post::CVolumeRenderer(this);
    vr->Create();
    m_render.Add(vr);
}

bool CImageModel::ExportRAWImage(const std::string& filename)
{
	C3DImage* im = Get3DImage();
	if (im == nullptr) return false;

	Byte* pb = im->GetBytes();
	if (pb == nullptr) return false;

	int nx = im->Width();
	int ny = im->Height();
	int nz = im->Depth();

	int nsize = nx * ny * nz;
	if (nsize <= 0) return false;

	FILE* fp = fopen(filename.c_str(), "wb");
	if (fp == nullptr) return false;

	int nread = fwrite(pb, nsize, 1, fp);

	fclose(fp);

	return (nread == 1);
}
