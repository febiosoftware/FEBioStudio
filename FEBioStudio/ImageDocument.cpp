/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include "ImageDocument.h"
#include <PostLib/ImageSlicer.h>
#include "MainWindow.h"

CImageDocument::CImageDocument(CMainWindow* wnd) : CGLDocument(wnd),
    activeModel(nullptr), m_xSlice(nullptr), m_ySlice(nullptr), m_zSlice(nullptr)
{
    
}

CImageDocument::~CImageDocument()
{
    CleanSlices();
}

void CImageDocument::AddImageModel(Post::CImageModel* img)
{
    CGLDocument::AddImageModel(img);
}

void CImageDocument::SetActiveModel(Post::CImageModel* imgModel)
{
    activeModel = imgModel;

    CleanSlices();

    m_xSlice = new Post::CImageSlicer(activeModel);
    m_ySlice = new Post::CImageSlicer(activeModel);
    m_zSlice = new Post::CImageSlicer(activeModel);
    
    m_xSlice->SetOrientation(0);
    m_ySlice->SetOrientation(1);
    m_zSlice->SetOrientation(2);

    m_xSlice->Create();
    m_ySlice->Create();
    m_zSlice->Create();

    GetMainWindow()->UpdateImageView();
}

void CImageDocument::SetActiveModel(int index)
{
    if(index >= 0 && index < ImageModels())
    {
        SetActiveModel(GetImageModel(index));
    }
}

void CImageDocument::CleanSlices()
{
    if(m_xSlice) delete m_xSlice;
    if(m_ySlice) delete m_ySlice;
    if(m_zSlice) delete m_zSlice;
}