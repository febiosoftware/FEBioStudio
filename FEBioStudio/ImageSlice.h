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

#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <ImageLib/Image.h>

class QVBoxLayout;
class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QWheelEvent;
class CIntSlider;
class QComboBox;

class CImageModel;


class CustomGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
signals:
    void focusChanged(QPointF point);


protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
};

class CImageSlice : public QWidget
{
    Q_OBJECT

public:
    enum SliceDir
    {
        X, Y, Z
    };

public:
    CImageSlice(SliceDir sliceDir, bool constAxis = true, QWidget* extraWidget = nullptr);

    void SetImage(CImageModel* imgModel);

    void Update();

    int GetIndex();
    void SetIndex(int index);
    int GetSliceCount();

    CImage* GetOriginalSlice() { return &m_orignalSlice; }
    CImage* GetDisplaySlice() { return &m_displaySlice; }

    void DrawRect(bool draw);

signals:
    void updated(int direction, float offset);
    void focusChanged(int direction, QPoint point);

private slots:
    void on_slider_changed(int val);
    void on_currentIndexChanged(int index);
    void on_view_focusChanged(QPointF point);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    void UpdateSliceCount();

    template<class pType>
    void ThresholdAndConvert();;

private:
    CImageModel* m_imgModel;
    CImage m_orignalSlice;
    CImage m_displaySlice;
    
    QVBoxLayout* m_layout;
    QGraphicsScene* m_scene;
    CustomGraphicsView* m_view;
    CIntSlider* m_slider;

    SliceDir m_sliceDir;
    QComboBox* m_sliceChoice;

    QGraphicsRectItem* m_rect;
    QGraphicsPixmapItem* m_imagePixmapItem;

    double m_xScale, m_yScale;
};