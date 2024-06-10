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

enum class ImageFileType;

#include <vector>
#include <string>
#include <FSCore/FSObjectList.h>
#include <FSCore/box.h>
#include <FSCore/FSThreadedTask.h>

class C3DImage;

namespace Post 
{
    class CGLImageRenderer;
}

class CImageModel;

class CImageSource : public FSThreadedTask
{
public:
    enum Types 
    { 
        RAW = 0, ITK, SERIES, TIFF, DICOM
    };

public:
	CImageSource(int type, CImageModel* imgModel = nullptr);
	~CImageSource();

    virtual bool Load() = 0;

    int Type() { return m_type; }

    virtual void Save(OArchive& ar) = 0;
	virtual void Load(IArchive& ar) = 0;

	C3DImage* Get3DImage() { return m_img; }

    void ClearFilters();
    C3DImage* GetImageToFilter(bool allocate = false, int pixelType = -1);

public:
	CImageModel* GetImageModel();
	void SetImageModel(CImageModel* imgModel);

protected:
    void AssignImage(C3DImage* im);

protected:
    int m_type;

	C3DImage*	m_img;
    C3DImage*   m_originalImage;
	CImageModel*	m_imgModel;
    unsigned char* data = nullptr;
};

class CRawImageSource : public CImageSource
{
public:
    CRawImageSource(CImageModel* imgModel, const std::string& filename, int imgType, int nx, int ny, int nz, BOX box, bool byteSwap);
    CRawImageSource(CImageModel* imgModel);

	void SetFileName(const std::string& filename);

    bool Load() override;

    void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	bool LoadFromFile(const char* szfile, C3DImage* im);

private:
    std::string m_filename;
	int m_type;
    int m_nx, m_ny, m_nz;
    BOX m_box;
    bool m_byteSwap;
};