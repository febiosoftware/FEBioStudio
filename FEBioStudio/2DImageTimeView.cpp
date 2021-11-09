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

#include <QBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QTimer>
#include <QSpinBox>
#include "MainWindow.h"
#include "ImageDocument.h"
#include "IconProvider.h"
#include "ImageSliceView.h"
#include "2DImageTimeView.h"

class Ui::C2DImageTimeView
{
public:
    QAction* actionPlayPause;
    CImageSlice* slice;

    QSpinBox* interval;
    
    QTimer* timer;

    bool running;
public:
    void setup(::C2DImageTimeView* parent)
    {
        m_parent = parent;
        running = false;

        QVBoxLayout* layout = new QVBoxLayout;

        QToolBar* toolbar = new QToolBar;

        actionPlayPause = new QAction;
        actionPlayPause->setIcon(CIconProvider::GetIcon("play"));
        actionPlayPause->setObjectName("actionPlayPause");
        toolbar->addAction(actionPlayPause);

        interval = new QSpinBox;
        interval->setMinimum(1);
        interval->setMaximum(9999);
        interval->setValue(300);
        toolbar->addWidget(interval);

        layout->addWidget(toolbar);

        slice = new CImageSlice(CImageSlice::Z);

        layout->addWidget(slice);

        parent->setLayout(layout);

        timer = new QTimer;
        timer->setParent(m_parent);
        timer->setObjectName("timer");
    }

private:
    ::C2DImageTimeView* m_parent;

};


C2DImageTimeView::C2DImageTimeView(CMainWindow* wnd)
    : m_wnd(wnd), ui(new Ui::C2DImageTimeView)
{
    ui->setup(this);

    connect(ui->actionPlayPause, &QAction::triggered, this, &C2DImageTimeView::on_actionPlayPause_triggered);
    connect(ui->timer, &QTimer::timeout, this, &C2DImageTimeView::on_timer_timeout);
}

void C2DImageTimeView::UpdateImage()
{
    CImageDocument* doc = dynamic_cast<CImageDocument*>(m_wnd->GetDocument());

    if(doc)
    {
        Post::CImageModel* img = doc->GetActiveModel();

        if(!img) return;
        
        ui->slice->SetImage(img);
    }

    ui->slice->Update();
}


void C2DImageTimeView::on_actionPlayPause_triggered()
{
    if(ui->running)
    {
        ui->timer->stop();
        ui->actionPlayPause->setIcon(CIconProvider::GetIcon("play"));
        ui->running = false;
    }
    else
    {
        ui->timer->start(ui->interval->value());
        ui->actionPlayPause->setIcon(CIconProvider::GetIcon("pause"));
        ui->running = true;
    }
    
}

void C2DImageTimeView::on_timer_timeout()
{
    int index = ui->slice->GetIndex() + 1;

    if(index == ui->slice->GetSliceCount())
    {
        index = 0;
    }

    ui->slice->SetIndex(index);
}