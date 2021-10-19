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
#include "ImageDocument.h"
#include <GLLib/GView.h> 

CImageToolBar::CImageToolBar(CMainWindow* wnd)
    : m_wnd(wnd)
{
    m_showGLView = new QAction(CIconProvider::GetIcon("volrender"), "GL View");
    m_showGLView->setCheckable(true);
    
    m_showSliceView = new QAction(CIconProvider::GetIcon("Image"), "Slice View");
    m_showSliceView->setCheckable(true);

    QActionGroup* viewGroup = new QActionGroup(this);
    viewGroup->addAction(m_showGLView);
    viewGroup->addAction(m_showSliceView);
    
    m_showGLView->setChecked(true);

    connect(viewGroup, &QActionGroup::triggered, this, &CImageToolBar::on_viewAction_triggered);

    addActions(viewGroup->actions());
}

void CImageToolBar::on_viewAction_triggered(QAction* action)
{
    CImageDocument* doc = m_wnd->GetImageDocument();

    if(!doc) return;

    if(action == m_showGLView) 
    {
        doc->GetView()->imgView = CGView::GL_VIEW;
    }
    else if(action == m_showSliceView)
    {
        doc->GetView()->imgView = CGView::SLICE_VIEW;
    }

    m_wnd->UpdateUIConfig();

    action->setChecked(true);
}