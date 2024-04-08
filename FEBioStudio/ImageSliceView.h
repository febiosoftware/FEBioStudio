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

#include <QWidget>
#include <PostLib/3DImageSlicer.h>
#include "PixelInfoSource.h"

class QGridLayout;
class QResizeEvent;

class CGLView;
class CMainWindow;
class CGLContext;
class FSObject;
class CImageModel;
class CImageSlice;

class CImageSliceView : public QWidget, public CSliceInfoSource
{
    Q_OBJECT

public:
    CImageSliceView(CMainWindow* wnd, QWidget* parent = 0);

    void Update();

    CImageModel* GetImageModel() { return m_imgModel; }

    void RenderSlicers(CGLContext& rc);

    void SetInspector(CDlgPixelInspector* inspector) override;

public slots:
    void ModelTreeSelectionChanged(FSObject* obj);
    void SliceUpdated(int direction, float offset);
    void SliceClicked(int direction, QPoint point);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    CMainWindow* m_wnd;

public:
    QGridLayout* m_layout;

    CImageModel* m_imgModel;

    CImageSlice* m_xSlice;
    CImageSlice* m_ySlice;
    CImageSlice* m_zSlice;

    Post::C3DImageSlicer m_slicer;

    CGLView* m_glView;
};

class CImageSliceViewRender : public Post::CGLImageRenderer
{
public:
	CImageSliceViewRender(CImageSliceView* sliceView);

	void Render(CGLContext& rc) override;

private:
	CImageSliceView* m_sliceView;
};
