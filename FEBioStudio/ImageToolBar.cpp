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

#include <QActionGroup>
#include <QAction>
#include "MainWindow.h"
#include "IconProvider.h"
#include "ImageToolBar.h"
#include <GLLib/GView.h> 
#include "Document.h"
#include "DlgPixelInspector.h"
#include "ImageSliceView.h"
#include "2DImageTimeView.h"

CImageToolBar::CImageToolBar(CMainWindow* wnd)
    : m_wnd(wnd), m_pixelInspector(nullptr)
{
    m_showModelView = new QAction(CIconProvider::GetIcon("mesh"), "Model View");
    m_showModelView->setCheckable(true);

    m_showSliceView = new QAction(CIconProvider::GetIcon("Image"), "Slice View");
    m_showSliceView->setCheckable(true);

    m_show2dImageView = new QAction(CIconProvider::GetIcon("Image", "play"), "Slice Sequence View");
    m_show2dImageView->setCheckable(true);

    QActionGroup* viewGroup = new QActionGroup(this);
    viewGroup->addAction(m_showModelView);
    viewGroup->addAction(m_showSliceView);
    viewGroup->addAction(m_show2dImageView);
    
    m_showModelView->setChecked(true);

    connect(viewGroup, &QActionGroup::triggered, this, &CImageToolBar::on_viewAction_triggered);

    addActions(viewGroup->actions());

    addSeparator();

    m_showPixelInspector = new QAction(CIconProvider::GetIcon("search"), "Pixel Inspector");
    addAction(m_showPixelInspector);
    connect(m_showPixelInspector, &QAction::triggered, this, &CImageToolBar::on_showPixelInspector_triggered);
}

void CImageToolBar::InspectorClosed()
{
    m_pixelInspector = nullptr;
}

void CImageToolBar::on_viewAction_triggered(QAction* action)
{
    CGLDocument* doc = m_wnd->GetGLDocument();

    if(!doc) return;

    if(action == m_showModelView)
    {
        doc->GetView()->imgView = CGView::MODEL_VIEW;

        CloseInspector();
    }
    else if(action == m_showSliceView)
    {
        doc->GetView()->imgView = CGView::SLICE_VIEW;

        if(m_pixelInspector) m_pixelInspector->SetInfoSource(m_wnd->GetImageSliceView());
    }
    else if(action == m_show2dImageView)
    {
        doc->GetView()->imgView = CGView::TIME_VIEW_2D;

        if(m_pixelInspector) m_pixelInspector->SetInfoSource(m_wnd->GetC2DImageTimeView());
    }

    m_wnd->UpdateUIConfig();

    action->setChecked(true);
}

void CImageToolBar::on_showPixelInspector_triggered()
{
    if(!m_pixelInspector)
    {
        if(m_showModelView->isChecked())
        {
            return;
        }
        else if(m_showSliceView->isChecked())
        {
            m_pixelInspector = new CDlgPixelInspector(this, m_wnd->GetImageSliceView());
        }
        else if(m_show2dImageView->isChecked())
        {
            m_pixelInspector = new CDlgPixelInspector(this, m_wnd->GetC2DImageTimeView());
        }

        
    }

    m_pixelInspector->show();
}

void CImageToolBar::CloseInspector()
{
    if(m_pixelInspector)
    {
        m_pixelInspector->close();
    }
}