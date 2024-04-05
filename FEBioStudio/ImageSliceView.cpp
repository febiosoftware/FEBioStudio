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

#include <QGridLayout>
#include <QBoxLayout>
#include <QLabel>
#include "MainWindow.h"
#include "Document.h"
#include "GLView.h"
#include <ImageLib/3DImage.h>
#include <PostLib/ImageSlicer.h>
#include <ImageLib/ImageModel.h>
#include "DlgPixelInspector.h"

#include "ImageSliceView.h"
#include "ImageSlice.h"

CImageSliceView::CImageSliceView(CMainWindow* wnd, QWidget* parent)
    : QWidget(parent), m_wnd(wnd), m_imgModel(nullptr), m_slicer(nullptr)
{
    m_layout = new QGridLayout;

    m_xSlice = new CImageSlice(CImageSlice::X);
    m_ySlice = new CImageSlice(CImageSlice::Y);
    m_zSlice = new CImageSlice(CImageSlice::Z);
    m_infoSlice = m_xSlice;

    m_layout->addWidget(m_xSlice, 0, 0);
    m_layout->addWidget(m_ySlice, 0, 1);
    m_layout->addWidget(m_zSlice, 1, 0);

    m_glView = new CGLView(m_wnd);
	m_glView->GetViewSettings().m_bgrid = false;
	m_glView->AllocateDefaultWidgets(false);
	m_glView->ShowContextMenu(false);
    m_layout->addWidget(m_glView, 1, 1);

    setLayout(m_layout);

    QObject::connect(m_xSlice, &CImageSlice::updated, this, &CImageSliceView::SliceUpdated);
    QObject::connect(m_ySlice, &CImageSlice::updated, this, &CImageSliceView::SliceUpdated);
    QObject::connect(m_zSlice, &CImageSlice::updated, this, &CImageSliceView::SliceUpdated);

    QObject::connect(m_xSlice, &CImageSlice::focusChanged, this, &CImageSliceView::SliceClicked);
    QObject::connect(m_ySlice, &CImageSlice::focusChanged, this, &CImageSliceView::SliceClicked);
    QObject::connect(m_zSlice, &CImageSlice::focusChanged, this, &CImageSliceView::SliceClicked);
}

void CImageSliceView::Update()
{
    CGLDocument* doc = m_wnd->GetGLDocument();

    if(doc)
    {   
        if(doc->GetUIViewMode() == CGLDocument::SLICE_VIEW)
        {
            m_xSlice->Update();
            m_ySlice->Update();
            m_zSlice->Update();
        }
    }
}

void CImageSliceView::RenderSlicers(CGLContext& rc)
{
	m_slicer.Render(rc);
}

void CImageSliceView::SetInspector(CDlgPixelInspector* inspector)
{
    if(!inspector)
    {
        m_xSlice->DrawRect(false);
        m_ySlice->DrawRect(false);
        m_zSlice->DrawRect(false);
    }

    CPixelInfoSource::SetInspector(inspector);
}

void CImageSliceView::ModelTreeSelectionChanged(FSObject* obj)
{
    m_imgModel = dynamic_cast<CImageModel*>(obj);

    // Forces recalc of min and max values on the image
    if(m_imgModel && m_imgModel->Get3DImage())
    {
        double min, max;
        m_imgModel->Get3DImage()->GetMinMax(min, max);
    }

    m_slicer.SetImageModel(m_imgModel);

    m_xSlice->SetImage(m_imgModel);
    m_ySlice->SetImage(m_imgModel);
    m_zSlice->SetImage(m_imgModel);

    Update();

    if(m_inspector) 
    {
        UpdatePixelInfo();
        m_inspector->UpdateData();
    }
}

void CImageSliceView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    Update();
}

void CImageSliceView::SliceUpdated(int direction, float offset)
{
    CGLDocument* doc = m_wnd->GetGLDocument();

    if(!doc) return;
    
	CImage* img = nullptr;
	switch (direction)
	{
	case CImageSlice::X: img = m_xSlice->GetDisplaySlice(); break;
	case CImageSlice::Y: img = m_ySlice->GetDisplaySlice(); break;
	case CImageSlice::Z: img = m_zSlice->GetDisplaySlice(); break;
	}
	if (img) m_slicer.SetSliceImage(direction, offset, img);

    m_glView->repaint();

    if(m_inspector) 
    {
        UpdatePixelInfo();
        m_inspector->UpdateData();
    }
}

void CImageSliceView::SliceClicked(int direction, QPoint point)
{
    m_infoPoint = point;

    switch (direction)
    {
    case CImageSlice::X:
        m_infoSlice = m_xSlice;
        break;
    case CImageSlice::Y:
        m_infoSlice = m_ySlice;
        break;
    case CImageSlice::Z:
        m_infoSlice = m_zSlice;
        break;
    default:
        break;
    }

    if(m_inspector)
    {
        m_xSlice->DrawRect(false);
        m_ySlice->DrawRect(false);
        m_zSlice->DrawRect(false);

        m_infoSlice->DrawRect(true);

        UpdatePixelInfo();
        m_inspector->UpdateData();
    }
}

CImageSliceViewRender::CImageSliceViewRender(CImageSliceView* sliceView) : m_sliceView(sliceView)
{

}

void CImageSliceViewRender::Render(CGLContext& rc)
{
	if (m_sliceView == nullptr) return;
	if (!m_sliceView->isVisible()) return;
	
	// only call the slice view render function
	// if the image model is the active one. 
	CImageModel* img = GetImageModel();
	if (img && (img == m_sliceView->GetImageModel()))
	{
		m_sliceView->RenderSlicers(rc);
	}
}
