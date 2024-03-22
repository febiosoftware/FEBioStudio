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

#include <QDialog>
#include <QWidget>

class QGraphicsView;
class QPixmap;
class CDrawROI;
class CImageModel;

class CROIShape
{
public:
    CROIShape(bool neg);

    virtual void SetActive(int active) = 0;
    virtual void MoveActive(QPointF pos) = 0;
    virtual void Draw(QPainter& painter, CDrawROI* parent) = 0;
    virtual void DrawMask(QPainter& painter, CDrawROI* parent) = 0;
    bool PointClicked(QPointF point);
    

protected:
    bool m_neg;
    std::vector<QPointF> points;
    QPointF m_anchor;
    int m_active;

};

class CROI8Node : public CROIShape
{
public:
    CROI8Node(bool neg, QPointF start);

    void SetActive(int active);
    void MoveActive(QPointF pos);
};

class CROIRect : public CROI8Node
{
public:
    CROIRect(bool neg, QPointF start);

    void Draw(QPainter& painter, CDrawROI* parent);
    void DrawMask(QPainter& painter, CDrawROI* parent);

};

class CROICircle : public CROI8Node
{
public:
    CROICircle(bool neg, QPointF start);

    void Draw(QPainter& painter, CDrawROI* parent);
    void DrawMask(QPainter& painter, CDrawROI* parent);

};


class CDrawROI : public QWidget
{
    Q_OBJECT

public:
    CDrawROI(CImageModel* model);
    ~CDrawROI();

    QSize PixmapSize();
    void GetMask(QPixmap* pix);

    QPointF WidgetToImageCoords(QPointF& widgetPoint);
    QPointF ImageToWidgetCoords(QPointF& imagePoint);

public slots:
    void startNewRect();
    void startNewRectNeg();
    void startNewCircle();
    void startNewCircleNeg();

signals:
    void inputDone();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    CImageModel* m_model;
    QPixmap m_slice;
    QRect m_bounds;
    float m_scale;

    bool m_newRect;
    bool m_newCircle;
    bool m_makeNeg;

    std::vector<CROIShape*> m_shapes;
    CROIShape* m_active;

};


class CDlgDIC : public QDialog
{
    Q_OBJECT

public:
    CDlgDIC(CImageModel* model);
    ~CDlgDIC();

public slots:
    void inputDone();

private:
    CImageModel* m_model;
    CDrawROI* drawROI;
    QGraphicsView* m_mask;
    QPixmap* m_maskPix;
};