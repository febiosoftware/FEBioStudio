/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2023 University of Utah, The Trustees of Columbia University in
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

#include <QMouseEvent>
#include <QBoxLayout>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QPen>
#include <QPixmap>
#include <QTransform>
#include <QGraphicsPixmapItem>
#include <QSlider>
#include <QComboBox>
#include <QToolBar>
#include <ImageLib/ImageModel.h>
#include <ImageLib/3DImage.h>
#include <CUILib/InputWidgets.h>

#include "ImageSlice.h"

void CustomGraphicsView::mousePressEvent(QMouseEvent* event)
{
    emit focusChanged(mapToScene(event->pos()));
}

void CustomGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    emit focusChanged(mapToScene(event->pos()));
}

CImageSlice::CImageSlice(SliceDir sliceDir, bool constAxis, QWidget* extraWidget)
    : m_imgModel(nullptr), m_xScale(1), m_yScale(1)
{
    m_sliceDir = sliceDir;

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0,0,0,0);

    m_scene = new QGraphicsScene;
    m_view = new CustomGraphicsView;
    m_view->setScene(m_scene);
    
    m_imagePixmapItem = new QGraphicsPixmapItem;
    m_imagePixmapItem->setVisible(false);
    m_scene->addItem(m_imagePixmapItem);

    m_rect = new QGraphicsRectItem;
    QPen pen(QColorConstants::Red);
    pen.setCosmetic(true);
    m_rect->setPen(pen);
    m_rect->setVisible(false);
    m_scene->addItem(m_rect);

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
    connect(m_view, &CustomGraphicsView::focusChanged, this, &CImageSlice::on_view_focusChanged);
}

void CImageSlice::SetImage(CImageModel* imgModel)
{
    if(m_imgModel == imgModel) return;

    m_imgModel = imgModel;

    if(!m_imgModel)
    {
        m_orignalSlice.Clear();
        m_displaySlice.Clear();
        return;
    }

    UpdateSliceCount();
}

template<class pType> void CImageSlice::ThresholdAndConvert()
{
    size_t N  = m_orignalSlice.Width() * m_orignalSlice.Height();
    if(m_orignalSlice.IsRGB())
    {
        N *= 3;
    }

    pType* imgData = (pType*)m_imgModel->Get3DImage()->GetBytes();

    double min, max;
    m_imgModel->Get3DImage()->GetMinMax(min, max, false);
    
    double minThresh = m_imgModel->GetViewSettings()->GetFloatValue(CImageViewSettings::MIN_INTENSITY);
    double maxThresh = m_imgModel->GetViewSettings()->GetFloatValue(CImageViewSettings::MAX_INTENSITY);

    if(m_orignalSlice.IsRGB())
    {
        m_displaySlice.Create(m_orignalSlice.Width(), m_orignalSlice.Height(), nullptr, CImage::UINT_RGB8);
    }
    else
    {
        m_displaySlice.Create(m_orignalSlice.Width(), m_orignalSlice.Height());
    }
    
    
    pType* data = (pType*)m_orignalSlice.GetBytes();
    uint8_t* outData = m_displaySlice.GetBytes();

    for(int i = 0; i < N; i++)
    {
        double val = (((double)data[i])-min)/(max-min);

        if(val < minThresh)
        {
            outData[i] = 0;
        }
        else if (val > maxThresh)
        {
            outData[i] = 255;
        }
        else
        {
            outData[i] = 255*(val - minThresh)/(maxThresh-minThresh);
        }
    }
}

void CImageSlice::UpdateSliceCount()
{
    C3DImage* img = m_imgModel->Get3DImage();
	if (img == nullptr) return;

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
    m_imagePixmapItem->setVisible(false);

    if(!m_imgModel) return;

    int slice = m_slider->getValue();

    m_slider->setToolTip(QString::number(slice));

    C3DImage* img = m_imgModel->Get3DImage();
	if (img == nullptr) return;

    switch (m_sliceDir)
    {
    case X:
        img->GetSliceX(m_orignalSlice, slice);
        break;
    case Y:
        img->GetSliceY(m_orignalSlice, slice);
        break;
    case Z:
        img->GetSliceZ(m_orignalSlice, slice);
        break;
    default:
        break;
    }

    switch (img->PixelType())
    {
    case CImage::UINT_8:
        ThresholdAndConvert<uint8_t>();
        break;
    case CImage::INT_8:
        ThresholdAndConvert<int8_t>();
        break;
    case CImage::UINT_16:
        ThresholdAndConvert<uint16_t>();
        break;
    case CImage::INT_16:
        ThresholdAndConvert<int16_t>();
        break;
    case CImage::UINT_32:
        ThresholdAndConvert<uint32_t>();
        break;
    case CImage::INT_32:
        ThresholdAndConvert<int32_t>();
        break;
    case CImage::UINT_RGB8:
        ThresholdAndConvert<uint8_t>();
        break;
    case CImage::INT_RGB8:
        ThresholdAndConvert<int8_t>();
        break;
    case CImage::UINT_RGB16:
        ThresholdAndConvert<uint16_t>();
        break;
    case CImage::INT_RGB16:
        ThresholdAndConvert<int16_t>();
        break;
    case CImage::REAL_32:
        ThresholdAndConvert<float>();
        break;
    case CImage::REAL_64:
        ThresholdAndConvert<double>();
        break;
    default:
        assert(false);
    }

    std::unique_ptr<QImage> qImg;
    
    if(m_orignalSlice.IsRGB())
    {
        qImg = std::make_unique<QImage>(m_displaySlice.GetBytes(), m_displaySlice.Width(), m_displaySlice.Height(), 
            m_displaySlice.Width()*3, QImage::Format::Format_RGB888);
    }
    else
    {
        qImg = std::make_unique<QImage>(m_displaySlice.GetBytes(), m_displaySlice.Width(), m_displaySlice.Height(), 
            m_displaySlice.Width(), QImage::Format::Format_Grayscale8);
    }

    BOX box = m_imgModel->GetBoundingBox();

    switch (m_sliceDir)
    {
    case X:
        m_xScale = box.Height()/img->Height();
        m_yScale = box.Depth()/img->Depth();
        
        m_scene->setSceneRect(0, 0, box.Height(), box.Depth());
        break;
    case Y:
        m_xScale = box.Width()/img->Width();
        m_yScale = box.Depth()/img->Depth();
        
        m_scene->setSceneRect(0, 0, box.Width(), box.Depth());
        break;
    case Z:
        m_xScale = box.Width()/img->Width();
        m_yScale = box.Height()/img->Height();

        m_scene->setSceneRect(0, 0, box.Width(), box.Height());
        break;
    default:
        break;
    }

    // Flip the image using QTransform.scale(1,-1)
    QPixmap pixmap = QPixmap::fromImage(*qImg).transformed(QTransform().scale(1,-1));
    
    m_imagePixmapItem->setPixmap(pixmap);
    m_imagePixmapItem->setTransform(QTransform().scale(m_xScale, m_yScale));
    m_imagePixmapItem->setVisible(true);

    m_rect->setTransform(QTransform().scale(m_xScale, m_yScale));

    m_view->fitInView(m_imagePixmapItem, Qt::AspectRatioMode::KeepAspectRatio);

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

void CImageSlice::DrawRect(bool draw)
{
    m_rect->setVisible(draw);
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

void CImageSlice::on_view_focusChanged(QPointF point)
{
    m_rect->setRect((int)(point.x()/m_xScale) - 3, (int)(point.y()/m_yScale) - 3, 7, 7);

    // I'm honestly not sure why, but we need to add one to the y-coordinate in order to get
    // the box lined up correctly.
    emit focusChanged(m_sliceDir, QPoint(point.x()/m_xScale, point.y()/m_yScale+1));
}

void CImageSlice::wheelEvent(QWheelEvent* event)
{
    m_slider->passEvent(event);
}