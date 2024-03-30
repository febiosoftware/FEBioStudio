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
#include <QToolButton>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QTimer>
#include <QSpinBox>
#include "MainWindow.h"
#include "Document.h"
#include "IconProvider.h"
#include "ImageSlice.h"
#include <ImageLib/3DImage.h>
#include "2DImageTimeView.h"
#include "DlgPixelInspector.h"

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
        toolbar->setContentsMargins(0,0,0,0);

        actionPlayPause = new QAction;
        actionPlayPause->setIcon(CIconProvider::GetIcon("play"));
        actionPlayPause->setObjectName("actionPlayPause");

        toolbar->addAction(actionPlayPause);
        toolbar->addSeparator();
        
        toolbar->addWidget(new QLabel("Inverval (ms):"));

        interval = new QSpinBox;
        interval->setMinimum(1);
        interval->setMaximum(9999);
        interval->setValue(40);
        toolbar->addWidget(interval);

        toolbar->addSeparator();

        toolbar->addWidget(new QLabel("Slice Direction:"));

        slice = new CImageSlice(CImageSlice::Z, false, toolbar);

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

    m_infoSlice = ui->slice;

    connect(ui->actionPlayPause, &QAction::triggered, this, &C2DImageTimeView::on_actionPlayPause_triggered);
    connect(ui->timer, &QTimer::timeout, this, &C2DImageTimeView::on_timer_timeout);
    connect(ui->interval, &QSpinBox::valueChanged, this, &C2DImageTimeView::on_interval_valueChanged);
    connect(ui->slice, &CImageSlice::focusChanged, this, &C2DImageTimeView::on_slice_clicked);
}

void C2DImageTimeView::Update()
{
    CGLDocument* doc = m_wnd->GetGLDocument();

    if(doc)
    {   
        if(doc->GetUIViewMode() == CGLDocument::TIME_VIEW_2D)
        {
            ui->slice->Update();
        }
    }
}

void C2DImageTimeView::ModelTreeSelectionChanged(FSObject* obj)
{
    CImageModel* img = dynamic_cast<CImageModel*>(obj);

    // Forces recalc of min and max values on the image
    if(img && img->Get3DImage())
    {
        double min, max;
        img->Get3DImage()->GetMinMax(min, max);
    }

    ui->slice->SetImage(img);

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
    CGLDocument* doc = m_wnd->GetGLDocument();

    if(doc)
    {   
        if(doc->GetUIViewMode() == CGLDocument::TIME_VIEW_2D)
        {
            int index = ui->slice->GetIndex() + 1;

            if(index == ui->slice->GetSliceCount())
            {
                index = 0;
            }

            ui->slice->SetIndex(index);
            
            if(m_inspector)
            {
                UpdatePixelInfo();
                m_inspector->UpdateData();
            }

            return;
        }
    }

    // stop the animation if this view isn't visible
    on_actionPlayPause_triggered();
}

void C2DImageTimeView::on_interval_valueChanged()
{
    if(ui->running)
    {
        ui->timer->stop();
        ui->timer->start(ui->interval->value());
    }
}

void C2DImageTimeView::on_slice_clicked(int direction, QPoint pos)
{
    m_infoPoint = pos;

    if(m_inspector)
    {
        UpdatePixelInfo();
        m_inspector->UpdateData();
    }
}