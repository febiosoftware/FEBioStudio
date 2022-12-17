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
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPixmap>
#include <QTransform>
#include <QGraphicsPixmapItem>
#include <QSlider>
#include <QComboBox>
#include <QToolBar>
#include "MainWindow.h"
#include "Document.h"
#include "GLView.h"
#include <ImageLib/3DImage.h>
#include <PostLib/ImageSlicer.h>
#include <PostLib/ImageModel.h>
#include <ImageLib/3DImage.h>
#include "ImageViewSettings.h"
#include "InputWidgets.h"


#include "ImageSliceView.h"

CImageSlice::CImageSlice(SliceDir sliceDir, bool constAxis, QWidget* extraWidget)
    : m_imgModel(nullptr)
{
    m_sliceDir = sliceDir;

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0,0,0,0);

    m_scene = new QGraphicsScene;
    m_view = new QGraphicsView;
    m_view->setScene(m_scene);

    m_layout->addWidget(m_view);

    QHBoxLayout* sliderLayout = new QHBoxLayout;
    sliderLayout->setContentsMargins(0,0,0,0);

    if(extraWidget)
    {
        sliderLayout->addWidget(extraWidget);
    }

    if(constAxis)
    {
        QString txt;
        switch (m_sliceDir)
        {
        case X:
            txt = "X:";
            break;
        case Y:
            txt = "Y:";
            break;
        case Z:
            txt = "Z:";
            break;
        default:
            break;
        }
        
        sliderLayout->addWidget(new QLabel(txt));
    }
    else
    {
        m_sliceChoice = new QComboBox;
        m_sliceChoice->addItem("X");
        m_sliceChoice->addItem("Y");
        m_sliceChoice->addItem("Z");
        m_sliceChoice->setCurrentIndex(m_sliceDir);

        sliderLayout->addWidget(m_sliceChoice);

        connect(m_sliceChoice, &QComboBox::currentIndexChanged, this, &CImageSlice::on_currentIndexChanged);
    }    

    m_slider = new CIntSlider;
    sliderLayout->addWidget(m_slider);



    m_layout->addLayout(sliderLayout);

    setLayout(m_layout);

    connect(m_slider, &CIntSlider::valueChanged, this, &CImageSlice::on_slider_changed);
}

void CImageSlice::SetImage(Post::CImageModel* imgModel)
{
    if(m_imgModel == imgModel) return;

    m_imgModel = imgModel;

    if(!m_imgModel) return;

    UpdateSliceCount();
}

void CImageSlice::UpdateSliceCount()
{
    C3DImage* img = m_imgModel->Get3DImage();

    int n;
    switch (m_sliceDir)
    {
    case X:
        n = img->Width();
        break;
    case Y:
        n = img->Height();
        break;
    case Z:
        n = img->Depth();
        break;        
    default:
        break;
    }

    m_slider->setRange(0, n-1);
    m_slider->setValue(n/2);
}

void CImageSlice::Update()
{
    m_scene->clear();

    if(!m_imgModel) return;

    int slice = m_slider->getValue();

    C3DImage* img = m_imgModel->Get3DImage();    

    int min = 255 * m_imgModel->GetViewSettings()->GetFloatValue(CImageViewSettings::MIN_INTENSITY);
    int max = 255 * m_imgModel->GetViewSettings()->GetFloatValue(CImageViewSettings::MAX_INTENSITY);

    CImage imgSlice;
    switch (m_sliceDir)
    {
    case X:
        img->GetThresholdedSliceX(imgSlice, slice, min, max);
        break;
    case Y:
        img->GetThresholdedSliceY(imgSlice, slice, min, max);
        break;
    case Z:
        img->GetThresholdedSliceZ(imgSlice, slice, min, max);
        break;
    default:
        break;
    }

    m_slider->setToolTip(QString::number(slice));

    QImage qImg(imgSlice.GetBytes(), imgSlice.Width(), imgSlice.Height(), imgSlice.Width(), QImage::Format::Format_Grayscale8);

    BOX box = m_imgModel->GetBoundingBox();
    double xScale, yScale;

    switch (m_sliceDir)
    {
    case X:
        xScale = box.Height()/img->Height();
        yScale = box.Depth()/img->Depth();
        
        m_scene->setSceneRect(0, 0, box.Height(), box.Depth());
        break;
    case Y:
        xScale = box.Width()/img->Width();
        yScale = box.Depth()/img->Depth();
        
        m_scene->setSceneRect(0, 0, box.Width(), box.Depth());
        break;
    case Z:
        xScale = box.Width()/img->Width();
        yScale = box.Height()/img->Height();

        m_scene->setSceneRect(0, 0, box.Width(), box.Height());
        break;
    default:
        break;
    }

    // Flip the image using QTransform.scale(1,-1)
    QPixmap pixmap = QPixmap::fromImage(qImg).transformed(QTransform().scale(1,-1));
    
    QGraphicsPixmapItem* item = m_scene->addPixmap(pixmap);
    item->setTransform(QTransform().scale(xScale, yScale));

    m_view->fitInView(item, Qt::AspectRatioMode::KeepAspectRatio);

    float sliceOffset = float(slice)/float(m_slider->maximum());
    emit updated(m_sliceDir, sliceOffset);
}

int CImageSlice::GetIndex()
{
    return m_slider->getValue();
}

void CImageSlice::SetIndex(int index)
{
    m_slider->setValue(index);
}

int CImageSlice::GetSliceCount()
{
    return m_slider->maximum() + 1;
}

void CImageSlice::on_slider_changed(int val)
{
    Update();
}

void CImageSlice::on_currentIndexChanged(int index)
{
    bool same = index == m_sliceDir;
    m_sliceDir = (SliceDir)index;

    if(!same)
    {
        UpdateSliceCount();
        Update();
    }
}

void CImageSlice::wheelEvent(QWheelEvent* event)
{
    m_slider->passEvent(event);
}

///////

CImageSliceView::CImageSliceView(CMainWindow* wnd, QWidget* parent)
    : QWidget(parent), m_wnd(wnd), m_imgModel(nullptr), m_xSlicer(nullptr), m_ySlicer(nullptr), m_zSlicer(nullptr)
{
    m_layout = new QGridLayout;

    m_xSlice = new CImageSlice(CImageSlice::X);
    m_ySlice = new CImageSlice(CImageSlice::Y);
    m_zSlice = new CImageSlice(CImageSlice::Z);

    m_layout->addWidget(m_xSlice, 0, 0);
    m_layout->addWidget(m_ySlice, 0, 1);
    m_layout->addWidget(m_zSlice, 1, 0);

    m_glView = new CGLView(m_wnd);
    m_layout->addWidget(m_glView, 1, 1);

    setLayout(m_layout);

    connect(m_xSlice, &CImageSlice::updated, this, &CImageSliceView::SliceUpdated);
    connect(m_ySlice, &CImageSlice::updated, this, &CImageSliceView::SliceUpdated);
    connect(m_zSlice, &CImageSlice::updated, this, &CImageSliceView::SliceUpdated);
}

CImageSliceView::~CImageSliceView()
{
    CleanSlicers();

    delete m_xSlice;
    delete m_ySlice;
    delete m_zSlice;
}

void CImageSliceView::CleanSlicers()
{
    if(m_xSlicer)
    {
        delete m_xSlicer;
        m_xSlicer = nullptr;
    }
    if(m_ySlicer)
    {
        delete m_ySlicer;
        m_ySlicer = nullptr;
    }
    if(m_zSlicer)
    {
        delete m_zSlicer;
        m_zSlicer = nullptr;
    }
}

void CImageSliceView::Update()
{
    CGLDocument* doc = m_wnd->GetGLDocument();

    if(doc)
    {   
        if(doc->GetView()->imgView == CGView::SLICE_VIEW)
        {
            m_xSlice->Update();
            m_ySlice->Update();
            m_zSlice->Update();
        }
    }
}

void CImageSliceView::RenderSlicers(CGLContext& rc)
{
    if(!m_xSlicer || !m_ySlicer || !m_zSlicer) return;
 
    m_xSlicer->Render(rc);
    m_ySlicer->Render(rc);
    m_zSlicer->Render(rc);
}

void CImageSliceView::ModelTreeSelectionChanged(FSObject* obj)
{
    m_imgModel = dynamic_cast<Post::CImageModel*>(obj);

    m_xSlice->SetImage(m_imgModel);
    m_ySlice->SetImage(m_imgModel);
    m_zSlice->SetImage(m_imgModel);

    CleanSlicers();

    if(m_imgModel)
    {
        m_xSlicer = new Post::CImageSlicer(m_imgModel);
        m_ySlicer = new Post::CImageSlicer(m_imgModel);
        m_zSlicer = new Post::CImageSlicer(m_imgModel);
        
        m_xSlicer->SetOrientation(0);
        m_ySlicer->SetOrientation(1);
        m_zSlicer->SetOrientation(2);

        m_xSlicer->Create();
        m_ySlicer->Create();
        m_zSlicer->Create();
    }

    Update();
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
    
    switch (direction)
    {
    case CImageSlice::X:
        if(m_xSlicer)
        {
            m_xSlicer->SetOffset(offset);
            m_xSlicer->Update();
        }
        break;
    case CImageSlice::Y:
        if(m_ySlicer)
        {
            m_ySlicer->SetOffset(offset);
            m_ySlicer->Update();
        }
        break;
    case CImageSlice::Z:
        if(m_zSlicer)
        {
            m_zSlicer->SetOffset(offset);
            m_zSlicer->Update();
        }
        break;
    default:
        break;
    }

    m_glView->repaint();
}
