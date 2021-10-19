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

#include <QGridLayout>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPixmap>
#include <QTransform>
#include <QGraphicsPixmapItem>
#include <QSlider>
#include <QToolBar>
#include "MainWindow.h"
#include "ImageDocument.h"
#include "GLView.h"
#include <ImageLib/3DImage.h>
#include <PostLib/ImageSlicer.h>
#include <PostLib/ImageModel.h>
#include <ImageLib/3DImage.h>


#include "ImageSliceView.h"

CImageSlice::CImageSlice(CMainWindow* wnd, SliceDir sliceDir)
    : m_wnd(wnd), m_imgModel(nullptr)
{
    m_sliceDir = sliceDir;

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0,0,0,0);

    m_scene = new QGraphicsScene;
    m_view = new QGraphicsView;
    m_view->setScene(m_scene);

    m_layout->addWidget(m_view);

    m_slider = new QSlider;
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setTickPosition(QSlider::TicksBelow);

    connect(m_slider, &QSlider::valueChanged, this, &CImageSlice::on_slider_changed);

    m_layout->addWidget(m_slider);

    setLayout(m_layout);
}

void CImageSlice::SetImage(Post::CImageModel* imgModel)
{
    if(m_imgModel == imgModel) return;

    m_imgModel = imgModel;

    C3DImage* img = imgModel->GetImageSource()->Get3DImage();

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

    int m = n / 100 + 1;
    m_slider->setTickInterval(m);

    m_slider->setSingleStep(1);
    m_slider->setPageStep(m);
}

void CImageSlice::Update()
{
    if(!m_imgModel) return;

    int slice = m_slider->value();

    CImageDocument* doc = m_wnd->GetImageDocument();
    float sliceOffset = float(slice)/float(m_slider->maximum());

    C3DImage* img = m_imgModel->GetImageSource()->Get3DImage();    

    CImage imgSlice;
    QString text;
    switch (m_sliceDir)
    {
    case X:
        img->GetSliceX(imgSlice, slice);
        text = "X";
        
        if(doc)
        {
            doc->GetXSlicer()->SetOffset(sliceOffset);
            doc->GetXSlicer()->Update();
        }
        break;
    case Y:
        img->GetSliceY(imgSlice, slice);
        text = "Y";
        
        if(doc)
        {
            doc->GetYSlicer()->SetOffset(sliceOffset);
            doc->GetYSlicer()->Update();
        }
        break;
    case Z:
        img->GetSliceZ(imgSlice, slice);
        text = "Z";
        
        if(doc)
        {
            doc->GetZSlicer()->SetOffset(sliceOffset);
            doc->GetZSlicer()->Update();
        }
        break;
    default:
        break;
    }

    m_slider->setToolTip(QString::number(slice));

    m_scene->clear();

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

    QGraphicsTextItem* textItem = new QGraphicsTextItem();
    textItem->setPos(0,0);
    textItem->setPlainText(text += QString(": %1").arg(slice));
    m_scene->addItem(textItem);

    m_view->fitInView(item, Qt::AspectRatioMode::KeepAspectRatio);

    emit updated();
}

void CImageSlice::on_slider_changed(int val)
{
    Update();
}

void CImageSlice::wheelEvent(QWheelEvent* event)
{
    m_slider->event(event);
}

///////

CImageSliceView::CImageSliceView(CMainWindow* wnd, QWidget* parent)
    : QWidget(parent), m_wnd(wnd)
{
    m_layout = new QGridLayout;

    m_xSlice = new CImageSlice(wnd, CImageSlice::X);
    m_ySlice = new CImageSlice(wnd, CImageSlice::Y);
    m_zSlice = new CImageSlice(wnd, CImageSlice::Z);

    m_layout->addWidget(m_xSlice, 0, 0);
    m_layout->addWidget(m_ySlice, 0, 1);
    m_layout->addWidget(m_zSlice, 1, 0);

    m_glView = new CGLView(m_wnd);
    m_layout->addWidget(m_glView, 1, 1);

    setLayout(m_layout);

    connect(m_xSlice, &CImageSlice::updated, this, &CImageSliceView::RepaintGLView);
    connect(m_ySlice, &CImageSlice::updated, this, &CImageSliceView::RepaintGLView);
    connect(m_zSlice, &CImageSlice::updated, this, &CImageSliceView::RepaintGLView);
}

void CImageSliceView::Update()
{
    m_xSlice->Update();
    m_ySlice->Update();
    m_zSlice->Update();
}

void CImageSliceView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    Update();
}

void CImageSliceView::UpdateImage()
{
    CImageDocument* doc = dynamic_cast<CImageDocument*>(m_wnd->GetDocument());

    if(doc)
    {
        Post::CImageModel* img = doc->GetActiveModel();

        if(!img) return;
        
        m_xSlice->SetImage(img);
        m_ySlice->SetImage(img);
        m_zSlice->SetImage(img);
    }

    Update();
}

void CImageSliceView::RepaintGLView()
{
    m_glView->repaint();
}