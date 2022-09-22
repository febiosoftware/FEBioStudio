// /*This file is part of the FEBio Studio source code and is licensed under the MIT license
// listed below.

// See Copyright-FEBio-Studio.txt for details.

// Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
// the City of New York, and others.

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.*/

#include <QWidget>
#include <QOpenGLWidget>
#include <GLLib/GLMeshRender.h>
#include <GLLib/GLCamera.h>
#include <SimpleITK.h>

class matrix;
class GObject;
class GLTriad;
class GMaterial;

namespace sitk = itk::simple;
using std::string;

class CFiberGLWidget : public QOpenGLWidget
{
public:
    CFiberGLWidget();
    ~CFiberGLWidget();

    void setMesh(GLMesh* mesh);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

	void mousePressEvent(QMouseEvent* ev) override;
	void mouseMoveEvent(QMouseEvent* ev) override;

private:
    GLMesh* m_mesh;
    GLMeshRender m_renderer;
    CGLCamera m_cam;

    GLTriad* m_ptriad;

private:
    int m_x0, m_y0;

};

namespace Ui
{
    class CFiberODFWidget;
}

class CFiberODFWidget : public QWidget
{
    Q_OBJECT

public:
    CFiberODFWidget();
    void setMaterial(GMaterial* mat);

private slots:
    void on_runButton_pressed();

private:
    void butterworthFilter(sitk::Image& img);
    sitk::Image powerSpectrum(sitk::Image& img);
    void fftRadialFilter(sitk::Image& img);
    void reduceAmp(sitk::Image& img, std::vector<double>* reduced);

    std::unique_ptr<matrix> complLapBel_Coef();
    double GFA(std::vector<double> vals);

    GLMesh* buildMesh();

private:
    GMaterial* m_mat;

    double m_tLow;
    double m_tHigh;

    double m_lengthScale;
    double m_hausd;
    double m_grad;

    int m_order;

    std::vector<std::vector<double>*> m_ODFs;

    Ui::CFiberODFWidget* ui;

};