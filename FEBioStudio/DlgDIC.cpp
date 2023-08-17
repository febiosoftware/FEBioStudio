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

#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QBoxLayout>
#include <QPushButton>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <ImageLib/ImageModel.h>
#include <ImageLib/3DImage.h>
#include <ImageLib/Image.h>
#include "DlgDIC.h"
#include <iostream>

CROIShape::CROIShape(bool neg)
    : m_neg(neg), m_active(-1)
{

}

bool CROIShape::PointClicked(QPointF point)
{
    int active = -1;

    for(int index = 0; index < points.size(); index++)
    {
        int x = abs(point.x() - points[index].x());
        int y = abs(point.y() - points[index].y());

        if (x <= 3 && y <= 3)
        {
            active = index;
            break;
        }
    }

    SetActive(active);

    return m_active != -1;
}

CROI8Node::CROI8Node(bool neg, QPointF start)
    : CROIShape(neg)
{
    points.push_back(start);

    for(int i = 1; i < 8; i++)
    {
        points.emplace_back(0,0);
    }

    SetActive(7);
}

void CROI8Node::SetActive(int active)
{
    m_active = active;

    switch (m_active)
    {
    case 0:
        m_anchor = points[7];
        break;
    case 1:
        m_anchor = QPointF(-1, points[6].y());
        break;
    case 2:
        m_anchor = points[5];
        break;
    case 3:
        m_anchor = QPointF(points[4].x(), -1);
        break;
    case 4:
        m_anchor = QPointF(points[3].x(), -1);
        break;
    case 5:
        m_anchor = points[2];
        break;
    case 6:
        m_anchor = QPointF(-1, points[1].y());
        break;
    case 7:
        m_anchor = points[0];
        break;
    default:
        break;
    }

}

void CROI8Node::MoveActive(QPointF pos)
{
    if(m_anchor.x() != -1)
    {
        float left, right;

        if(m_anchor.x() <= pos.x())
        {
            left = m_anchor.x();
            right = pos.x();
        }
        else
        {
            left = pos.x();
            right = m_anchor.x();
        }

        float midX = left + (right-left)/2;

        points[0].setX(left);
        points[1].setX(midX);
        points[2].setX(right);
        points[3].setX(left);
        points[4].setX(right);
        points[5].setX(left);
        points[6].setX(midX);
        points[7].setX(right);
    }

   if(m_anchor.y() != -1)
   {
       float top, bottom;

        if(m_anchor.y() <= pos.y())
        {
            top = m_anchor.y();
            bottom = pos.y();
        }
        else
        {
            top = pos.y();
            bottom = m_anchor.y();
        }

        float midY = top + (bottom-top)/2;

        points[0].setY(top);
        points[1].setY(top);
        points[2].setY(top);
        points[3].setY(midY);
        points[4].setY(midY);
        points[5].setY(bottom);
        points[6].setY(bottom);
        points[7].setY(bottom);
   }
}

CROIRect::CROIRect(bool neg, QPointF start)
    : CROI8Node(neg, start)
{
    
}

void CROIRect::Draw(QPainter& painter, CDrawROI* parent)
{
    QPen backup = painter.pen();

    Qt::GlobalColor color;
    if(m_neg)
    {
        color = Qt::red;
    }
    else
    {
        color = Qt::green;
    }

    QPen pen = painter.pen();
    pen.setColor(color);
    painter.setPen(pen);

    painter.drawRect(QRectF(parent->ImageToWidgetCoords(points[0]), parent->ImageToWidgetCoords(points[7])));

    pen.setWidth(6);
    painter.setPen(pen);

    for(auto point : points)
    {
        painter.drawPoint(parent->ImageToWidgetCoords(point));
    }

    painter.setPen(backup);
}

void CROIRect::DrawMask(QPainter& painter, CDrawROI* parent)
{
    Qt::GlobalColor color;
    if(m_neg)
    {
        color = Qt::black;
    }
    else
    {
        color = Qt::white;
    }

    painter.fillRect(QRectF(parent->ImageToWidgetCoords(points[0]), parent->ImageToWidgetCoords(points[7])), color);
}

CROICircle::CROICircle(bool neg, QPointF start)
    : CROI8Node(neg, start)
{

}

void CROICircle::Draw(QPainter& painter, CDrawROI* parent)
{
    QPen backup = painter.pen();

    Qt::GlobalColor color;
    if(m_neg)
    {
        color = Qt::red;
    }
    else
    {
        color = Qt::green;
    }

    QPen pen = painter.pen();
    pen.setColor(color);
    painter.setPen(pen);

    painter.drawEllipse(QRectF(parent->ImageToWidgetCoords(points[0]), parent->ImageToWidgetCoords(points[7])));

    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);
    painter.drawRect(QRectF(parent->ImageToWidgetCoords(points[0]), parent->ImageToWidgetCoords(points[7])));

    pen.setStyle(Qt::SolidLine);
    pen.setWidth(6);
    painter.setPen(pen);

    for(auto point : points)
    {
        painter.drawPoint(parent->ImageToWidgetCoords(point));
    }

    painter.setPen(backup);
}

void CROICircle::DrawMask(QPainter& painter, CDrawROI* parent)
{
    Qt::GlobalColor color;
    if(m_neg)
    {
        color = Qt::black;
    }
    else
    {
        color = Qt::white;
    }

    QBrush backup = painter.brush();
    QPen penBackup = painter.pen();

    painter.setBrush(QBrush(color));
    painter.setPen(QPen(color));
    painter.drawEllipse(QRectF(parent->ImageToWidgetCoords(points[0]), parent->ImageToWidgetCoords(points[7])));

    painter.setBrush(backup);
    painter.setPen(penBackup);
}

CDrawROI::CDrawROI(CImageModel* model)
    : m_model(model), m_newRect(false), m_newCircle(false), m_active(nullptr), m_makeNeg(false)
{
    setMinimumSize(300,300);

    CImage imgSlice;
    m_model->Get3DImage()->GetSliceX(imgSlice, 256);

    m_slice = QPixmap::fromImage(QImage(imgSlice.GetBytes(), imgSlice.Width(), imgSlice.Height(), imgSlice.Width(), QImage::Format::Format_Grayscale8)).transformed(QTransform().scale(1,-1));

    resize(300,300);

}

CDrawROI::~CDrawROI()
{
    for(auto shape : m_shapes)
    {
        delete shape;
    }
}

QSize CDrawROI::PixmapSize()
{
    return m_slice.size();
}

void CDrawROI::GetMask(QPixmap* pix)
{
    QPainter painter(pix);

    painter.fillRect(QRect(0,0,pix->width(), pix->height()), Qt::black);

    for(auto shape : m_shapes)
    {
        shape->DrawMask(painter, this);
    }
}

void CDrawROI::startNewRect()
{
    m_newRect = true;
    m_makeNeg = false;
}

void CDrawROI::startNewRectNeg()
{
    m_newRect = true;
    m_makeNeg = true;
}

void CDrawROI::startNewCircle()
{
    m_newCircle = true;
    m_makeNeg = false;
}

void CDrawROI::startNewCircleNeg()
{
    m_newCircle = true;
    m_makeNeg = true;
}

void CDrawROI::resizeEvent(QResizeEvent* event)
{
    QSize newSize = event->size();
    BOX box = m_model->GetBoundingBox();

    float xScale = box.Height()/newSize.width();
    float yScale = box.Depth()/newSize.height();

    if(xScale > yScale)
    {
        m_scale = box.Depth()/box.Height();

        m_bounds = QRect(0,0,newSize.width(), newSize.width()*m_scale);
    }
    else
    {
        m_scale = box.Height()/box.Depth();

        m_bounds = QRect(0,0,newSize.height()*m_scale, newSize.height());
    }
}

void CDrawROI::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QRect rect(0,0, m_slice.width(), m_slice.height());

    painter.drawPixmap(m_bounds, m_slice);

    for(auto shape : m_shapes)
    {
        shape->Draw(painter, this);
    }
}

void CDrawROI::mousePressEvent(QMouseEvent* event)
{
    QPointF pos = event->localPos();

    if(m_newRect || m_newCircle)
    {
        if(pos.x() < m_bounds.left()) pos.setX(m_bounds.left());
        if(pos.x() > m_bounds.right()) pos.setX(m_bounds.right());
        if(pos.y() < m_bounds.top()) pos.setY(m_bounds.top());
        if(pos.y() > m_bounds.bottom()) pos.setY(m_bounds.bottom());

        if(m_newRect)
        {
            m_shapes.push_back(new CROIRect(m_makeNeg, WidgetToImageCoords(pos)));
        }
        else
        {
            m_shapes.push_back(new CROICircle(m_makeNeg, WidgetToImageCoords(pos)));
        }

        
        m_active = m_shapes[m_shapes.size() - 1];
    }
    else
    {
        for(auto shape : m_shapes)
        {
            if(shape->PointClicked(WidgetToImageCoords(pos)))
            {
                m_active = shape;
                break;
            }
        }
    }

}

void CDrawROI::mouseMoveEvent(QMouseEvent* event)
{
    if(m_active)
    {
        QPointF pos = event->localPos();
        if(pos.x() < m_bounds.left()) pos.setX(m_bounds.left());
        if(pos.x() > m_bounds.right()) pos.setX(m_bounds.right());
        if(pos.y() < m_bounds.top()) pos.setY(m_bounds.top());
        if(pos.y() > m_bounds.bottom()) pos.setY(m_bounds.bottom());

        m_active->MoveActive(WidgetToImageCoords(pos));

        repaint();
    }

    // QPointF pos = event->localPos();
    // if(pos.x() < m_bounds.left()) pos.setX(m_bounds.left());
    // if(pos.x() > m_bounds.right()) pos.setX(m_bounds.right());
    // if(pos.y() < m_bounds.top()) pos.setY(m_bounds.top());
    // if(pos.y() > m_bounds.bottom()) pos.setY(m_bounds.bottom());

    // m_end = WidgetToImageCoords(pos);

    // m_drawRect = true;
    // repaint();
}

void CDrawROI::mouseReleaseEvent(QMouseEvent* event)
{
    m_newRect = false;
    m_newCircle = false;
    m_makeNeg = false;
    m_active = nullptr;

    emit inputDone();
}


QPointF CDrawROI::WidgetToImageCoords(QPointF& widgetPoint)
{
    QPointF imagePoint;
    imagePoint.setX(widgetPoint.x()/m_bounds.width()*m_slice.width());
    imagePoint.setY(widgetPoint.y()/m_bounds.height()*m_slice.height());

    return imagePoint;
}

QPointF CDrawROI::ImageToWidgetCoords(QPointF& imagePoint)
{
    QPointF widgetPoint;
    widgetPoint.setX(imagePoint.x()/m_slice.width()*m_bounds.width());
    widgetPoint.setY(imagePoint.y()/m_slice.height()*m_bounds.height());

    return widgetPoint;
}

CDlgDIC::CDlgDIC(CImageModel* model)
    : m_model(model)
{
    QHBoxLayout* layout = new QHBoxLayout;

    drawROI = new CDrawROI(model);
    layout->addWidget(drawROI);

    QVBoxLayout* buttonLayout = new QVBoxLayout;

    QPushButton* newRect = new QPushButton("Rect +");
    connect(newRect, &QPushButton::clicked, drawROI, &CDrawROI::startNewRect);
    newRect->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    buttonLayout->addWidget(newRect);

    QPushButton* newRectNeg = new QPushButton("Rect -");
    connect(newRectNeg, &QPushButton::clicked, drawROI, &CDrawROI::startNewRectNeg);
    newRectNeg->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    
    buttonLayout->addWidget(newRectNeg);

    QPushButton* newCircle = new QPushButton("Circle +");
    connect(newCircle, &QPushButton::clicked, drawROI, &CDrawROI::startNewCircle);
    newCircle->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    buttonLayout->addWidget(newCircle);

    QPushButton* newCircleNeg = new QPushButton("Circle -");
    connect(newCircleNeg, &QPushButton::clicked, drawROI, &CDrawROI::startNewCircleNeg);
    newCircleNeg->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    buttonLayout->addWidget(newCircleNeg);

    layout->addLayout(buttonLayout);

    layout->addWidget(m_mask = new QGraphicsView);

    setLayout(layout);

    connect(drawROI, &CDrawROI::inputDone, this, &CDlgDIC::inputDone);

    m_maskPix = new QPixmap(drawROI->PixmapSize());
}

CDlgDIC::~CDlgDIC()
{
    delete m_maskPix;
}

void CDlgDIC::inputDone()
{
    drawROI->GetMask(m_maskPix);

    QGraphicsScene* scene = new QGraphicsScene;
    m_mask->setScene(scene);
    
    QGraphicsPixmapItem* item = scene->addPixmap(*m_maskPix);
    m_mask->fitInView(item, Qt::AspectRatioMode::KeepAspectRatio);

}