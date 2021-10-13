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
#include <QGraphicsPixmapItem>
#include <QSlider>
#include <QToolBar>
#include "MainWindow.h"
#include "ImageDocument.h"
#include <ImageLib/3DImage.h>


#include "ImageSliceView.h"

CImageSlice::CImageSlice(SliceDir sliceDir)
    : m_img(nullptr)
{
    m_sliceDir = sliceDir;

    m_layout = new QVBoxLayout;

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

void CImageSlice::SetImage(C3DImage* img)
{
    m_img = img;

    int n;
    switch (m_sliceDir)
    {
    case X:
        n = m_img->Width();
        break;
    case Y:
        n = m_img->Height();
        break;
    case Z:
        n = m_img->Depth();
        break;        
    default:
        break;
    }
    
    m_slider->setRange(0, n-1);

    int m = n / 100 + 1;
    m_slider->setTickInterval(m);

    m_slider->setSingleStep(1);
    m_slider->setPageStep(m);
}

void CImageSlice::Update()
{
    if(!m_img) return;

    int slice = m_slider->value();

    CImage img;
    QString text;
    switch (m_sliceDir)
    {
    case X:
        m_img->GetSliceX(img, slice);
        text = "X";
        break;
    case Y:
        m_img->GetSliceY(img, slice);
        text = "Y";
        break;
    case Z:
        m_img->GetSliceZ(img, slice);
        text = "Z";
        break;
    
    default:
        break;
    }

    m_slider->setToolTip(QString::number(slice));

    QImage qImg(img.GetBytes(), img.Width(), img.Height(), img.Width(), QImage::Format::Format_Grayscale8);

    QPixmap pixmap = QPixmap::fromImage(qImg);
    m_scene->setSceneRect(0, 0, img.Width(), img.Height());
    QGraphicsPixmapItem* item = m_scene->addPixmap(pixmap);

    QGraphicsTextItem* textItem = new QGraphicsTextItem();
    textItem->setPos(0,0);
    textItem->setPlainText(text += QString(": %1").arg(slice));
    m_scene->addItem(textItem);

    m_view->fitInView(item, Qt::AspectRatioMode::KeepAspectRatio);
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
    layout = new QGridLayout;

    xSlice = new CImageSlice(CImageSlice::X);
    ySlice = new CImageSlice(CImageSlice::Y);
    zSlice = new CImageSlice(CImageSlice::Z);

    layout->addWidget(xSlice, 0, 0);
    layout->addWidget(ySlice, 0, 1);
    layout->addWidget(zSlice, 1, 0);
    layout->addWidget(new QWidget, 1, 1);

    setLayout(layout);
}

void CImageSliceView::Update()
{
    xSlice->Update();
    ySlice->Update();
    zSlice->Update();
}

void CImageSliceView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    Update();
}

void CImageSliceView::SetImage()
{
    CImageDocument* doc = dynamic_cast<CImageDocument*>(m_wnd->GetDocument());

    if(doc)
    {
        if(!doc->ImageModels()) return;
        
        C3DImage* img = doc->GetImageModel(0)->GetImageSource()->Get3DImage();

        xSlice->SetImage(img);
        ySlice->SetImage(img);
        zSlice->SetImage(img);
    }

    Update();
}