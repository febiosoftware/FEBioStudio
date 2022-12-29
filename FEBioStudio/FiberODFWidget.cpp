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

#include "FiberODFWidget.h"
#include <QOpenGLFunctions>
#include <QStackedWidget>
#include <QLabel>
#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>
#include <complex>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <FEAMR/sphericalHarmonics.h>
#include <FEAMR/spherePoints.h>
#include <MeshTools/FENNQuery.h>
#include <MeshTools/GLMesh.h>
#include <GL/glu.h>
#include <PostLib/ColorMap.h>
#include <GLWLib/GLWidget.h>
#include <MeshTools/GMaterial.h>
#include <FEBioLink/FEBioClass.h>
#include <FEMLib/FECoreMaterial.h>
#include <FECore/fecore_enum.h>

#include <iostream>

using std::vector;
using std::complex;

CFiberGLWidget::CFiberGLWidget() : m_mesh(nullptr)
{
    setMouseTracking(true);
}

CFiberGLWidget::~CFiberGLWidget()
{
    if(m_mesh) delete m_mesh;
}

void CFiberGLWidget::setMesh(GLMesh* mesh)
{
    m_mesh = mesh;
}

void CFiberGLWidget::initializeGL()
{
    GLfloat amb1[] = { .09f, .09f, .09f, 1.f };
	GLfloat dif1[] = { .8f, .8f, .8f, 1.f };

	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDepthFunc(GL_LEQUAL);

	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//	glShadeModel(GL_FLAT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.5f);

	// enable lighting and set default options
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif1);

	glEnable(GL_POLYGON_OFFSET_FILL);

	// enable color tracking for diffuse color of materials
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glPointSize(7.0f);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);


    glClearColor(0.3, 0.3, 0.3, 1);

    m_ptriad = new GLTriad(0, 0, 50, 50);
	m_ptriad->align(GLW_ALIGN_LEFT | GLW_ALIGN_BOTTOM);
}

void CFiberGLWidget::resizeGL(int w, int h)
{
    QOpenGLWidget::resizeGL(w, h);
}

void CFiberGLWidget::paintGL()
{
    // set the projection Matrix to ortho2d so we can draw some stuff on the screen
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.2, 1.2, -1.2, 1.2, 0.01, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    m_cam.Transform();

    if(m_mesh)
    {
        m_renderer.RenderGLMesh(m_mesh);

        // glColor3ub(0,0,0);
        // m_renderer.RenderGLMeshLines(m_mesh);
    }

    // render the GL widgets
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    m_ptriad->setOrientation(m_cam.GetOrientation());
    m_ptriad->draw(&painter);

    QOpenGLWidget::paintGL();
}

void CFiberGLWidget::mousePressEvent(QMouseEvent* ev)
{
    int x = ev->x();
	int y = ev->y();

    m_x0 = ev->pos().x();
	m_y0 = ev->pos().y();


    ev->accept();
}

void CFiberGLWidget::mouseMoveEvent(QMouseEvent* ev)
{
    bool but1 = (ev->buttons() & Qt::LeftButton);
    if(!but1) return;

    bool balt   = (ev->modifiers() & Qt::AltModifier     ? true : false);

    int x = ev->pos().x();
	int y = ev->pos().y();

    if (balt)
    {
        quatd qz = quatd((y - m_y0)*0.01f, vec3d(0, 0, 1));
        m_cam.Orbit(qz);
    }
    else
    {
        quatd qx = quatd((y - m_y0)*0.01f, vec3d(1, 0, 0));
        quatd qy = quatd((x - m_x0)*0.01f, vec3d(0, 1, 0));

        m_cam.Orbit(qx);
        m_cam.Orbit(qy);
    }

    m_x0 = x;
    m_y0 = y;

    repaint();

    m_cam.Update(true);

    ev->accept();

}


//-------------------------------------------------------------------------------
class Ui::CFiberODFWidget
{
public:
    QStackedWidget* stack;
    
    QWidget* firstPage;
    QLineEdit* imagePath;
    QSpinBox* xDivisions;
    QSpinBox* yDivisions;
    QSpinBox* zDivisions;
    QPushButton* runButton;

    QWidget* secondPage;
    CFiberGLWidget* glWidget;

public:
    void setupUI(::CFiberODFWidget* parent)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        stack = new QStackedWidget;

        firstPage = new QWidget;
        QVBoxLayout* firstPageLayout = new QVBoxLayout;
        firstPageLayout->addWidget(imagePath = new QLineEdit("Run"));
        firstPageLayout->addWidget(runButton = new QPushButton("Run"));

        QHBoxLayout* xLayout = new QHBoxLayout;
        xLayout->addWidget(new QLabel("Number of X Divisions"));
        xDivisions = new QSpinBox;
        xDivisions->setValue(1);
        xDivisions->setRange(1,99);
        xLayout->addWidget(xDivisions);
        firstPageLayout->addLayout(xLayout);

        QHBoxLayout* yLayout = new QHBoxLayout;
        yLayout->addWidget(new QLabel("Number of Y Divisions"));
        yDivisions = new QSpinBox;
        yDivisions->setValue(1);
        yDivisions->setRange(1,99);
        yLayout->addWidget(yDivisions);
        firstPageLayout->addLayout(yLayout);

        QHBoxLayout* zLayout = new QHBoxLayout;
        zLayout->addWidget(new QLabel("Number of Z Divisions"));
        zDivisions = new QSpinBox;
        zDivisions->setValue(1);
        zDivisions->setRange(1,99);
        zLayout->addWidget(zDivisions);
        firstPageLayout->addLayout(zLayout);

        firstPage->setLayout(firstPageLayout);
        stack->addWidget(firstPage);

        secondPage = new QWidget;
        QVBoxLayout* secondPageLayout = new QVBoxLayout;
        secondPageLayout->addWidget(glWidget = new CFiberGLWidget);
        secondPage->setLayout(secondPageLayout);
        stack->addWidget(secondPage);

        layout->addWidget(stack);

        parent->setLayout(layout);

    }

};

CFiberODFWidget::CFiberODFWidget()
    : m_mat(nullptr), m_order(16), m_tHigh(2.0), m_tLow(30), m_lengthScale(10), m_hausd(0.05), 
        m_grad(1.3), ui(new Ui::CFiberODFWidget)
{
    ui->setupUI(this);

    connect(ui->runButton, &QPushButton::pressed, this, &CFiberODFWidget::on_runButton_pressed);
}

void CFiberODFWidget::setMaterial(GMaterial* mat)
{
    m_mat = mat;
}


void CFiberODFWidget::on_runButton_pressed()
{
    sitk::Image img = sitk::ReadImage(ui->imagePath->text().toStdString());
    img = sitk::Cast(img, sitk::sitkUInt32);

    int xDiv = ui->xDivisions->value();
    int yDiv = ui->yDivisions->value();
    int zDiv = ui->zDivisions->value();

    auto size = img.GetSize();

    unsigned int xDivSize = size[0]/xDiv;
    unsigned int yDivSize = size[1]/yDiv;
    unsigned int zDivSize = size[2]/zDiv;

    auto spacing = img.GetSpacing();
    auto origin = img.GetOrigin();

    std::cout << "Origin: " << origin[0] << ", " << origin[1] << ", " << origin[2] << std::endl;
    std::cout << "Size: " << size[0]*spacing[0] << ", " << size[1]*spacing[1] << ", " << size[2]*spacing[2] << std::endl;

    int xDivSizePhys = size[0]*spacing[0]/(xDiv+1);
    int yDivSizePhys = size[1]*spacing[1]/(yDiv+1);
    int zDivSizePhys = size[2]*spacing[2]/(zDiv+1);

    sitk::ExtractImageFilter extractFilter;
    extractFilter.SetSize(std::vector<unsigned int> {xDivSize, yDivSize, zDivSize});

    // preprocessing for qBall algorithm

    auto C = complLapBel_Coef();

    double* theta = new double[NPTS] {};
    double* phi = new double[NPTS] {};

    getSphereCoords(NPTS, XCOORDS, YCOORDS, ZCOORDS, theta, phi);

    auto T = compSH(m_order, NPTS, theta, phi);

    delete[] theta;
    delete[] phi;

    matrix transposeT = T->transpose();

    matrix A = (*T)*(*C);
    matrix B = (transposeT*(*T)).inverse()*transposeT;

    int currentX = 1;
    int currentY = 1;
    int currentZ = 1;
    int currentLoop = 0;
    while(true)
    {
        extractFilter.SetIndex(std::vector<int> {(int)xDivSize*(currentX - 1), (int)yDivSize*(currentY - 1), (int)zDivSize*(currentZ - 1)});
        sitk::Image current = extractFilter.Execute(img);
        
        // sitk::ImageFileWriter writer;
        // QString name = QString("/home/mherron/Desktop/test%1.tif").arg(currentLoop++);
        // writer.SetFileName(name.toStdString());
        // writer.Execute(current);

        // Apply Butterworth filter
        butterworthFilter(current);
        
        current = sitk::Cast(current, sitk::sitkFloat32);
        
        // Apply FFT and FFT shift
        current = sitk::FFTPad(current);
        current = sitk::ForwardFFT(current);
        current = sitk::FFTShift(current);

        // Obtain Power Spectrum
        current = powerSpectrum(current);

        // Remove DC component (does not have a direction and does not 
        // constitute fibrillar structures)
        // NOTE: For some reason, this doesn't perfectly match zeroing out the same point in MATLAB
        // and results in a slightly different max value in the ODF
        auto currentSize = current.GetSize();
        std::vector<uint32_t> index {currentSize[0]/2 + 1, currentSize[1]/2 + 1, currentSize[2]/2 + 1};
        current.SetPixelAsFloat(index, 0);

        // Apply Radial FFT Filter
        fftRadialFilter(current);

        std::vector<double> reduced = std::vector<double>(NPTS,0);
        reduceAmp(current, &reduced);

        // delete image
        current = sitk::Image();
        
        // Apply qBall algorithm
        std::vector<double>* ODF = new std::vector<double>(NPTS, 0.0);

        A.mult(B, reduced, *ODF);

        // normalize odf
        double gfa = GFA(*ODF);
        double min = *std::min_element(ODF->begin(), ODF->end());
        double max = *std::max_element(ODF->begin(), ODF->end());

        double sum = 0;
        for(int index = 0; index < ODF->size(); index++)
        {
            double val = (*ODF)[index] - (min + (max - min)*0.1*gfa);

            if(val < 0)
            {
                (*ODF)[index] = 0;
            }
            else
            {
                (*ODF)[index] = val;
            }
            
            sum += (*ODF)[index];
        }

        for(int index = 0; index < ODF->size(); index++)
        {
            (*ODF)[index] /= sum;
        }

        // Calculate spherical harmonics
        vector<double> sphHarm((*C).columns());   
        B.mult(*ODF, sphHarm);

        m_ODFs.push_back(ODF);

        vec3d position(xDivSizePhys*currentX + origin[0], yDivSizePhys*currentY + origin[1], zDivSizePhys*currentZ + origin[2]);

        std::cout << "X: " << currentX << " Y: " << currentY << " Z: " << currentZ << std::endl;
        std::cout << "Spherical Harmonics:" << std::endl;
        for(int index = 0; index < sphHarm.size() - 1; index++)
        {
            std::cout << sphHarm[index] << ",";
        }
        std::cout << sphHarm[sphHarm.size() - 1] << std::endl;

        std::cout << "Position:" << std::endl;
        std::cout << position.x << "," << position.y << "," << position.z << std::endl;

        // add spherical harmonics and position to material
        int classID = FEBio::GetClassId(FECLASS_ID, "fiber-odf");
        FSModelComponent* fiberODF = FEBio::CreateClass(classID,  m_mat->GetModel());
        fiberODF->GetParam("shp_harmonics")->SetVectorDoubleValue(sphHarm);
        fiberODF->GetParam("position")->SetVec3dValue(position);
        // fiberODF->AddVectorDoubleParam(sphHarm);
        // fiberODF->AddVecParam(position);
        m_mat->GetMaterialProperties()->GetProperty(0).AddComponent(fiberODF);

        // Recalc ODF based on spherical harmonics
        // (*T).mult(sphHarm, *m_ODF);

        // ui->glWidget->setMesh(buildMesh());

        if((currentX == xDiv) && (currentY == yDiv) && (currentZ == zDiv)) break;

        currentZ++;
        if(currentZ > zDiv)
        {
            currentZ = 1;
            currentY++;
        }

        if(currentY > yDiv)
        {
            currentY = 1;
            currentX++;
        }
    }

    

    // // Calc Gradient
    // vector<double> gradient;
    // altGradient(m_order, sphHarm, gradient);

    // makeDataField(po, gradient, "Gradient");

    // // Remesh sphere
    // vector<vec3d> nodePos;
    // vector<vec3i> elems;
    // remesh(gradient, m_lengthScale, m_hausd, m_grad, nodePos, elems);

    // FSMesh* newMesh = new FSMesh();

	// // get the new mesh sizes
    // int NN = nodePos.size();
    // int NF = elems.size();
	// newMesh->Create(NN, NF);

	// // get the vertex coordinates
	// for (int i = 0; i < NN; ++i)
	// {
	// 	FSNode& vi = newMesh->Node(i);
	// 	vi.r = nodePos[i];
	// }

    // // create elements
	// for (int i=0; i<NF; ++i)
	// {
    //     FSElement& el = newMesh->Element(i);
    //     el.SetType(FE_TRI3);
    //     int* n = el.m_node;
	// 	el.m_node[0] = elems[i].x;
	// 	el.m_node[1] = elems[i].y;
	// 	el.m_node[2] = elems[i].z;
	// }

    // newMesh->RebuildMesh();

	// GMeshObject* po2 = new GMeshObject(newMesh);
    // po2->SetName("Remeshed");

	// // add the object to the model
	// fem->GetModel().AddObject(po2);

    // // Create ODF mapping for new sphere

    // int nodes = newMesh->Nodes();

    // double* xCoords = new double[nodes] {};
    // double* yCoords = new double[nodes] {};
    // double* zCoords = new double[nodes] {};
    // for(int index = 0; index < nodes; index++)
    // {
    //     vec3d vec = newMesh->Node(index).pos();

    //     xCoords[index] = vec.x;
    //     yCoords[index] = vec.y;
    //     zCoords[index] = vec.z;
    // }

    // theta = new double[nodes] {};
    // phi = new double[nodes] {};

    // getSphereCoords(nodes, xCoords, yCoords, zCoords, theta, phi);

    // T = compSH(m_order, nodes, theta, phi);

    // delete[] xCoords;
    // delete[] yCoords;
    // delete[] zCoords;
    // delete[] theta;
    // delete[] phi;

    // std::vector<double> newODF(nodes, 0);  
    // (*T).mult(sphHarm, newODF);

    // makeDataField(po2, newODF, "ODF");

    ui->stack->setCurrentIndex(1);
}

void CFiberODFWidget::butterworthFilter(sitk::Image& img)
{
    double fraction = 0.2;
    double steepness = 10;

    uint32_t* data = img.GetBufferAsUInt32();

    int nx = img.GetSize()[0];
    int ny = img.GetSize()[1];
    int nz = img.GetSize()[2];

    double xStep = img.GetSpacing()[0];
    double yStep = img.GetSpacing()[1];
    double zStep = img.GetSpacing()[2];

    double height = nx*xStep;
    double width = ny*yStep;
    double depth = nz*zStep;
    double radMax = std::min({height, width, depth})/2;

    radMax = 1;

    double decent = radMax - radMax * fraction;

    #pragma omp parallel for
    for(int z = 0; z < nz; z++)
    {
        for(int y = 0; y < ny; y++)
        {
            for(int x = 0; x < nx; x++)
            {
                int xPos = x - nx/2;
                int yPos = y - ny/2;
                int zPos = z - nz/2;

                int xPercent = xPos/nx;
                int yPercent = yPos/ny;
                int zPercent = zPos/nz;

                // double rad = sqrt(xPos*xStep*xPos*xStep + yPos*yStep*yPos*yStep + zPos*zStep*zPos*zStep);
                double rad = sqrt(xPercent*xPercent + yPercent*yPercent + zPercent*zPercent);
                // double rad = xPercent*yPercent*zPercent/3;

                int index = x + y*nx + z*nx*ny;

                data[index] = data[index]/(1 + pow(rad/decent, 2*steepness));
            }
        }
    }
}

sitk::Image CFiberODFWidget::powerSpectrum(sitk::Image& img)
{
    int nx = img.GetSize()[0];
    int ny = img.GetSize()[1];
    int nz = img.GetSize()[2];

    sitk::Image PS(nx, ny, nz, sitk::sitkFloat32);
    float* data = PS.GetBufferAsFloat();

    #pragma omp parallel for
    for(uint32_t x = 0; x <nx; x++)
    {
        for(uint32_t y = 0; y < ny; y++)
        {
            for(uint32_t z = 0; z < nz; z++)
            {
                std::vector<uint32_t> index = {x,y,z};
                
                complex<float> val = img.GetPixelAsComplexFloat32(index);

                float ps = abs(val);

                int newIndex = x + y*nx + z*nx*ny;

                data[newIndex] = ps*ps;
            }
        }
    }

    return PS;
}

void CFiberODFWidget::fftRadialFilter(sitk::Image& img)
{
    float fLow = 1/m_tLow;
    float fHigh = 1/m_tHigh;

    int nx = img.GetSize()[0];
    int ny = img.GetSize()[1];
    int nz = img.GetSize()[2];

    int minSize = std::min({nx, ny, nz})/2;
    
    double xStep = img.GetSpacing()[0];
    double yStep = img.GetSpacing()[1];
    double zStep = img.GetSpacing()[2];

    float* data = img.GetBufferAsFloat();

    #pragma omp parallel for
    for(int z = 0; z < nz; z++)
    {
        for(int y = 0; y < ny; y++)
        {
            for(int x = 0; x < nx; x++)
            {
                int xPos = x - nx/2;
                int yPos = y - ny/2;
                int zPos = z - nz/2;

                double rad = sqrt(xPos*xStep*xPos*xStep + yPos*yStep*yPos*yStep + zPos*zStep*zPos*zStep);

                double val = rad/minSize;

                int index = x + y*nx + z*nx*ny;

                if(val < fLow || val > fHigh)
                {
                   data[index] = 0;
                }
            }
        }
    }
}


void CFiberODFWidget::reduceAmp(sitk::Image& img, std::vector<double>* reduced)
{
    std::vector<vec3d> points;
    points.reserve(NPTS);
    
    for (int index = 0; index < NPTS; index++)
    {
        points.emplace_back(XCOORDS[index], YCOORDS[index], ZCOORDS[index]);
    }

    float* data = img.GetBufferAsFloat();

    int nx = img.GetSize()[0];
    int ny = img.GetSize()[1];
    int nz = img.GetSize()[2];

    double xStep = img.GetSpacing()[0];
    double yStep = img.GetSpacing()[1];
    double zStep = img.GetSpacing()[2];

    #pragma omp parallel shared(img, points)
    {
        FSNNQuery query(&points);
        query.Init();
        
        std::vector<double> tmp(NPTS, 0.0);

        #pragma omp for
        for (int z = 0; z < nz; z++)
        {
            for (int y = 0; y < ny; y++)
            {
                for (int x = 0; x < nx; x++)
                {
                    int xPos = (x - nx / 2);
                    int yPos = (y - ny / 2);
                    int zPos = (z - nz / 2);

                    double realX = xPos*xStep;
                    double realY = yPos*yStep;
                    double realZ = zPos*zStep;
                    
                    double rad = sqrt(realX*realX + realY*realY + realZ*realZ);
                    
                    if(rad == 0) continue;

                    int closestIndex = query.Find(vec3d(realX/rad, realY/rad, realZ/rad));
                    
                    int index = x + y*nx + z*nx*ny;
                    
                    tmp[closestIndex] += data[index];
                }
            }
        }
        
        for (int i = 0; i < NPTS; ++i)
        {
            #pragma omp critical
            (*reduced)[i] += tmp[i];
        }
    }
}

enum NUMTYPE { REALTYPE, IMAGTYPE, COMPLEXTYPE };

double fact(int val)
{
    double ans = 1;
    
    for(int i = 1; i <= val; i++)
    {
        ans *= i;
    } 
    
    return ans;
}

std::unique_ptr<matrix> CFiberODFWidget::complLapBel_Coef()
{
    int size = (m_order+1)*(m_order+2)/2;
    std::unique_ptr<matrix> out = std::make_unique<matrix>(size, size);
    out->fill(0,0,size,size,0.0);

    for(int k = 0; k <= m_order; k+=2)
    {
        for(int m = -k; m <= k; m++)
        {
            int j = k*(k+1)/2 + m;

            double prod1 = 1;
            for(int index = 3; index < k; index+=2)
            {
                prod1 *= index;
            }

            double prod2 = 1;
            for(int index = 2; index <= k; index+=2)
            {
                prod2 *= index;
            }

            (*out)[j][j] = (pow(-1, k/2))*prod1/prod2*2*M_PI;
        }

    }

    return std::move(out);
}

double CFiberODFWidget::GFA(std::vector<double> vals)
{
    // Standard deviation and root mean square
    double mean = 0;
    for(auto val : vals)
    {
        mean += val;
    }
    mean /= vals.size();

    double stdDev = 0;
    double rms = 0;
    for(auto val : vals)
    {
        double diff = val - mean;
        stdDev += diff*diff;
        
        rms += val*val;
    }
    stdDev = sqrt(stdDev/vals.size());
    rms = sqrt(rms/vals.size());

    return stdDev/rms;
}

GLMesh* CFiberODFWidget::buildMesh()
{
    // Post::CColorMap map;
    // map.jet();

    // double min = *std::min_element(m_ODF->begin(), m_ODF->end());
    // double max = *std::max_element(m_ODF->begin(), m_ODF->end());
    // double scale = 1/(max - min);

    // GLMesh* pm = new GLMesh();
	// pm->Create(NPTS, NCON);

	// // create nodes
	// for (int i=0; i<NPTS; ++i)
	// {
	// 	auto& node = pm->Node(i);
	// 	node.r = vec3d(XCOORDS[i], YCOORDS[i], ZCOORDS[i]);
	// }

	// // create elements
	// for (int i=0; i<NCON; ++i)
	// {
    //     auto& el = pm->Face(i);
    //     el.n[0] = CONN1[i]-1;
    //     el.n[1] = CONN2[i]-1;
    //     el.n[2] = CONN3[i]-1;

    //     el.c[0] = map.map((*m_ODF)[el.n[0]]*scale);
    //     el.c[1] = map.map((*m_ODF)[el.n[1]]*scale);
    //     el.c[2] = map.map((*m_ODF)[el.n[2]]*scale);
	// }

	// update the mesh
	// pm->Update();

	// return pm;

    return nullptr;
}