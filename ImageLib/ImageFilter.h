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
#include <FSCore/math3d.h>
#include <FSCore/FSObject.h>

namespace Post{
class CGLModel;
};

class CImageModel;

class CImageFilter : public FSObject
{
public:
    enum TYPES
    {
        THRESHOLD = 0, MEAN, GAUSSBLUR, WARP, ADAPTHISTEQ
    };

public:
    CImageFilter(int type, CImageModel* model);

    virtual void ApplyFilter() = 0;

    int Type() { return m_type; }

    void SetImageModel(CImageModel* model);

	CImageModel* GetImageModel();

protected:
    CImageModel* m_model;

private:
    int m_type;
};

class ThresholdImageFilter : public CImageFilter
{
public:
    ThresholdImageFilter(CImageModel* model = nullptr);

    void ApplyFilter() override;
};

class MeanImageFilter : public CImageFilter
{
public:
    MeanImageFilter(CImageModel* model = nullptr);

    void ApplyFilter() override;
};

class GaussianImageFilter : public CImageFilter
{
public:
    GaussianImageFilter(CImageModel* model = nullptr);

    void ApplyFilter() override;
};

class AdaptiveHistogramEqualizationFilter : public CImageFilter
{
public:
    AdaptiveHistogramEqualizationFilter(CImageModel* model = nullptr);

    void ApplyFilter() override;
};

class WarpImageFilter : public CImageFilter
{
	enum { SCALE_DIM };

public:
	WarpImageFilter(Post::CGLModel* glm);
	void ApplyFilter() override;

private:
	Post::CGLModel* m_glm;
};
