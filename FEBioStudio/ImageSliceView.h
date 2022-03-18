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

#pragma once

#include <QWidget>

class QVBoxLayout;
class QGraphicsScene;
class QGraphicsView;
class QGridLayout;
class QResizeEvent;
class QWheelEvent;
class CIntSlider;

class CGLView;
class CMainWindow;

namespace Post
{
    class CImageModel;
    class CImageSlicer;
}

class CImageSlice : public QWidget
{
    Q_OBJECT

public:
    enum SliceDir
    {
        X, Y, Z
    };

public:
    CImageSlice(SliceDir sliceDir);

    void SetImage(Post::CImageModel* imgModel);

    void Update();

    int GetIndex();
    void SetIndex(int index);
    int GetSliceCount();

signals:
    void updated(int direction, float offset);

private slots:
    void on_slider_changed(int val);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    Post::CImageModel* m_imgModel;
    
    QVBoxLayout* m_layout;
    QGraphicsScene* m_scene;
    QGraphicsView* m_view;
    CIntSlider* m_slider;

    SliceDir m_sliceDir;

};

class CImageSliceView : public QWidget
{
    Q_OBJECT


public:
    CImageSliceView(CMainWindow* wnd, QWidget* parent = 0);
    ~CImageSliceView();

    void Update();

    Post::CImageModel* GetImageModel() { return m_imgModel; }

    void RenderSlicers(CGLContext& rc);

private:
    void CleanSlicers();

public slots:
    void ModelTreeSelectionChanged(FSObject* obj);
    void SliceUpdated(int direction, float offset);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    CMainWindow* m_wnd;

public:
    QGridLayout* m_layout;

    Post::CImageModel* m_imgModel;

    CImageSlice* m_xSlice;
    CImageSlice* m_ySlice;
    CImageSlice* m_zSlice;

    Post::CImageSlicer* m_xSlicer;
    Post::CImageSlicer* m_ySlicer;
    Post::CImageSlicer* m_zSlicer;

    CGLView* m_glView;
};