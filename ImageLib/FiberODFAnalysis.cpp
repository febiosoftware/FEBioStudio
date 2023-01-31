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

#include "FiberODFAnalysis.h"
#include <qopengl.h>
#include <GL/glu.h>
#include <PostLib/ImageModel.h>
#include <FEAMR/sphericalHarmonics.h>
#include <FEAMR/spherePoints.h>
#include <MeshTools/FENNQuery.h>
#include <PostLib/ColorMap.h>
#include <MeshLib/GLMesh.h>
#include <GLLib/GLCamera.h>
#include <complex>
#include <sstream>
#include "ImageSITK.h"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

namespace sitk = itk::simple;
using std::vector;
using std::complex;

enum { ORDER,  T_LOW, T_HIGH, XDIV, YDIV, ZDIV, DISP, MESHLINES, REMESHED, RADIAL};

CODF::CODF() : m_odf(NPTS, 0.0) {};

CFiberODFAnalysis::CFiberODFAnalysis(Post::CImageModel* img)
    : CImageAnalysis(img), m_lengthScale(10), m_hausd(0.05), 
        m_grad(1.3)
{
    static int n = 1;
	std::stringstream ss;
	ss << "Fiber ODF Analysis" << n++;
	SetName(ss.str());

    AddIntParam(16, "Harmonic Order");
    AddDoubleParam(30, "Low Freq Cutoff (pixels)");
    AddDoubleParam(2.0, "High Freq Cutoff (pixels)");
    AddIntParam(1, "X Divisions");
    AddIntParam(1, "Y Divisions");
    AddIntParam(1, "Z Divisions");
    AddBoolParam(true, "Display in graphics view");
    AddBoolParam(false, "Render Mesh Lines");
    AddBoolParam(false, "Show Remeshed ODF");
    AddBoolParam(false, "Show Radial Mesh");
}

CFiberODFAnalysis::~CFiberODFAnalysis()
{
    clear();
}

void CFiberODFAnalysis::clear()
{
    for(auto odf : m_ODFs)
    {
        delete odf;
    }
    m_ODFs.clear();
}

void CFiberODFAnalysis::run()
{
    clear();
	resetProgress();
	setCurrentTask("Starting ODF Analysis ...");

    CImageSITK* imgSITK = dynamic_cast<CImageSITK*>(m_img->Get3DImage());
    if(!imgSITK) return;

    sitk::Image img = sitk::Cast(imgSITK->GetSItkImage(), sitk::sitkUInt32);

    int xDiv = GetIntValue(XDIV);
    int yDiv = GetIntValue(YDIV);
    int zDiv = GetIntValue(ZDIV);

    auto size = img.GetSize();

    unsigned int xDivSize = size[0]/xDiv;
    unsigned int yDivSize = size[1]/yDiv;
    unsigned int zDivSize = size[2]/zDiv;

    auto spacing = img.GetSpacing();
    auto origin = img.GetOrigin();

    std::cout << "Origin: " << origin[0] << ", " << origin[1] << ", " << origin[2] << std::endl;
    std::cout << "Size: " << size[0]*spacing[0] << ", " << size[1]*spacing[1] << ", " << size[2]*spacing[2] << std::endl;

    double xDivSizePhys = size[0]*spacing[0]/xDiv;
    double yDivSizePhys = size[1]*spacing[1]/yDiv;
    double zDivSizePhys = size[2]*spacing[2]/zDiv;
    double radius = std::min({xDivSizePhys, yDivSizePhys, zDivSizePhys})*0.375;

    sitk::ExtractImageFilter extractFilter;
    extractFilter.SetSize(std::vector<unsigned int> {xDivSize, yDivSize, zDivSize});

    // preprocessing for qBall algorithm

    auto C = complLapBel_Coef();

    double* theta = new double[NPTS] {};
    double* phi = new double[NPTS] {};

    getSphereCoords(NPTS, XCOORDS, YCOORDS, ZCOORDS, theta, phi);

    auto T = compSH(GetIntValue(ORDER), NPTS, theta, phi);

    delete[] theta;
    delete[] phi;

    matrix transposeT = T->transpose();

    matrix A = (*T)*(*C);
    matrix B = (transposeT*(*T)).inverse()*transposeT;

	m_totalSteps = xDiv * yDiv * zDiv;

    int currentX = 0;
    int currentY = 0;
    int currentZ = 0;
    int currentLoop = 0;
	m_progress = 0;
	setCurrentTask("Building ODFs ...");
	while(true)
    {
		// check progress
		m_stepsCompleted = currentZ + currentY * zDiv + currentX * zDiv * yDiv;

		// see if user cancelled
		if (IsCancelled())
		{
			clear();
			return;
		}

		std::stringstream ss;
		ss << "Building ODFS (" << m_stepsCompleted + 1 << "/" << m_totalSteps << ")...";
		m_task = ss.str();
		setCurrentTask(m_task.c_str(), m_progress);

        extractFilter.SetIndex(std::vector<int> {(int)xDivSize*currentX, (int)yDivSize*currentY, (int)zDivSize*currentZ});
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
        
		// see if user cancelled
		if (IsCancelled())
		{
			clear();
			return;
		}

		// Apply qBall algorithm
		CODF* odf = new CODF;
		// odf->m_odf.reserve(NPTS);

        A.mult(B, reduced, odf->m_odf);
		updateProgress(0.75);

        // normalize odf
        double gfa = GFA(odf->m_odf);
        double min = *std::min_element(odf->m_odf.begin(), odf->m_odf.end());
        double max = *std::max_element(odf->m_odf.begin(), odf->m_odf.end());

        double sum = 0;
        for(int index = 0; index < odf->m_odf.size(); index++)
        {
            double val = (odf->m_odf)[index] - (min + (max - min)*0.1*gfa);

            if(val < 0)
            {
                odf->m_odf[index] = 0;
            }
            else
            {
                odf->m_odf[index] = val;
            }
            
            sum += odf->m_odf[index];
        }

        for(int index = 0; index < odf->m_odf.size(); index++)
        {
            odf->m_odf[index] /= sum;
        }

        // Calculate spherical harmonics
        odf->m_sphHarmonics.resize(C->columns());   
        B.mult(odf->m_odf, odf->m_sphHarmonics);

        odf->m_position = vec3d(size[0]*spacing[0]/(xDiv*2)*(currentX*2+1) + origin[0], size[1]*spacing[1]/(yDiv*2)*(currentY*2+1) + origin[1], size[2]*spacing[2]/(zDiv*2)*(currentZ*2+1) + origin[2]);
        odf->m_radius = radius;
        m_ODFs.push_back(odf);

        std::cout << "X: " << currentX << " Y: " << currentY << " Z: " << currentZ << std::endl;
        std::cout << "Spherical Harmonics:" << std::endl;
        for(int index = 0; index < odf->m_sphHarmonics.size() - 1; index++)
        {
            std::cout << odf->m_sphHarmonics[index] << ",";
        }
        std::cout << odf->m_sphHarmonics[odf->m_sphHarmonics.size() - 1] << std::endl;

        std::cout << "Position:" << std::endl;
        std::cout << odf->m_position.x << "," << odf->m_position.y << "," << odf->m_position.z << std::endl;

        // Recalc ODF based on spherical harmonics
        (*T).mult(odf->m_sphHarmonics, odf->m_odf);

        // ui->glWidget->setMesh(buildMesh());

        if((currentX + 1 == xDiv) && (currentY + 1 == yDiv) && (currentZ + 1 == zDiv)) break;

        currentZ++;
        if(currentZ + 1> zDiv)
        {
            currentZ = 0;
            currentY++;
        }

        if(currentY + 1 > yDiv)
        {
            currentY = 0;
            currentX++;
        }
		updateProgress(1.0);
    }
	setCurrentTask("Building meshes ...", 100);

    buildMeshes();
}

void CFiberODFAnalysis::render(CGLCamera* cam)
{
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_COLOR_MATERIAL);
    GLfloat spc[4] = { 0, 0, 0, 1.f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spc);

    bool remeshed = GetBoolValue(REMESHED);
    bool meshLines = GetBoolValue(MESHLINES);

    for(auto odf : m_ODFs)
    {
        glPushMatrix();
        glTranslated(odf->m_position.x, odf->m_position.y, odf->m_position.z);
        glScaled(odf->m_radius, odf->m_radius, odf->m_radius);
        
        GLMesh* mesh;

        bool radial = showRadial();

        if(renderRemeshed())
        {
            if(radial)
            {
                mesh = &odf->m_radialRemesh;
            }
            else
            {
                mesh = &odf->m_remesh;
            }
        }
        else
        {
            if(radial)
            {
                mesh = &odf->m_radialMesh;
            }
            else
            {
                mesh = &odf->m_mesh;
            }
        }

        m_render.RenderGLMesh(mesh);

        glPopMatrix();

        if(meshLines)
        {
            cam->LineDrawMode(true);
            cam->Transform();

            glPushMatrix();
            glTranslated(odf->m_position.x, odf->m_position.y, odf->m_position.z);
            glScaled(odf->m_radius, odf->m_radius, odf->m_radius);

            glColor3f(0,0,0);

            m_render.RenderGLMeshLines(mesh);

             glPopMatrix();

            cam->LineDrawMode(false);
            cam->Transform();
        }
        
        // glPopMatrix();
    }

    glPopAttrib();
}

// void CFiberODFAnalysis::render()
// {

// }

bool CFiberODFAnalysis::display()
{
    return GetBoolValue(DISP);
}

CODF* CFiberODFAnalysis::GetODF(int i)
{
    try
    {
        return m_ODFs.at(i);
    }
    catch(const std::exception& e)
    {
        return nullptr;
    }
}

bool CFiberODFAnalysis::renderRemeshed()
{
    return GetBoolValue(REMESHED);
}

bool CFiberODFAnalysis::renderMeshLines()
{
    return GetBoolValue(MESHLINES);
}

bool CFiberODFAnalysis::showRadial()
{
    return GetBoolValue(RADIAL);
}

void CFiberODFAnalysis::butterworthFilter(sitk::Image& img)
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

sitk::Image CFiberODFAnalysis::powerSpectrum(sitk::Image& img)
{
    int nx = img.GetSize()[0];
    int ny = img.GetSize()[1];
    int nz = img.GetSize()[2];

    sitk::Image PS(nx, ny, nz, sitk::sitkFloat32);
    float* data = PS.GetBufferAsFloat();

    #pragma omp parallel for
    for(int x = 0; x <nx; x++)
    {
        for(int y = 0; y < ny; y++)
        {
            for(int z = 0; z < nz; z++)
            {
                std::vector<uint32_t> index = {(unsigned int)x,(unsigned int)y,(unsigned int)z};
                
                complex<float> val = img.GetPixelAsComplexFloat32(index);

                float ps = abs(val);

                int newIndex = x + y*nx + z*nx*ny;

                data[newIndex] = ps*ps;
            }
        }
    }

    return PS;
}

void CFiberODFAnalysis::fftRadialFilter(sitk::Image& img)
{
    float fLow = 1/GetFloatValue(T_LOW);
    float fHigh = 1/GetFloatValue(T_HIGH);

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

void CFiberODFAnalysis::updateProgress(double f)
{
	m_progress = 100.0 * (m_stepsCompleted + f) / m_totalSteps;
	setProgress(m_progress);
}

void CFiberODFAnalysis::reduceAmp(sitk::Image& img, std::vector<double>* reduced)
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

	double zcount = 0;

    #pragma omp parallel shared(img, points)
    {
        FSNNQuery query(&points);
        query.Init();
        
        std::vector<double> tmp(NPTS, 0.0);

        #pragma omp for schedule(dynamic)
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

			#pragma omp critical
			{
				zcount++;
				updateProgress(0.5*zcount / nz);
			}
        }
        
		#pragma omp for
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

std::unique_ptr<matrix> CFiberODFAnalysis::complLapBel_Coef()
{
    int order = GetIntValue(ORDER);

    int size = (order+1)*(order+2)/2;
    std::unique_ptr<matrix> out = std::make_unique<matrix>(size, size);
    out->fill(0,0,size,size,0.0);

    for(int k = 0; k <= order; k+=2)
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

double CFiberODFAnalysis::GFA(std::vector<double>& vals)
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

void CFiberODFAnalysis::buildMeshes()
{
    Post::CColorMap map;
    map.jet();

    for(auto odf : m_ODFs)
    {
        double min = *std::min_element(odf->m_odf.begin(), odf->m_odf.end());
        double max = *std::max_element(odf->m_odf.begin(), odf->m_odf.end());
        double scale = 1/(max - min);

        GLMesh* pm = &odf->m_mesh;
        pm->Create(NPTS, NCON);

        GLMesh* radial = &odf->m_radialMesh;
        radial->Create(NPTS, NCON);

        // create nodes
        for (int i=0; i<NPTS; ++i)
        {
            auto& node = pm->Node(i);
            node.r = vec3d(XCOORDS[i], YCOORDS[i], ZCOORDS[i]);

            auto& radialNode = radial->Node(i);
            double val = (odf->m_odf[i]-min)*scale;
            radialNode.r = vec3d(XCOORDS[i]*val, YCOORDS[i]*val, ZCOORDS[i]*val);
        }

        // create elements
        for (int i=0; i<NCON; ++i)
        {
            auto& el = pm->Face(i);
            el.n[0] = CONN1[i]-1;
            el.n[1] = CONN2[i]-1;
            el.n[2] = CONN3[i]-1;

            el.c[0] = map.map(odf->m_odf[el.n[0]]*scale);
            el.c[1] = map.map(odf->m_odf[el.n[1]]*scale);
            el.c[2] = map.map(odf->m_odf[el.n[2]]*scale);

            auto& radialEl = radial->Face(i);
            radialEl.n[0] = CONN1[i]-1;
            radialEl.n[1] = CONN2[i]-1;
            radialEl.n[2] = CONN3[i]-1;

            radialEl.c[0] = map.map(odf->m_odf[radialEl.n[0]]*scale);
            radialEl.c[1] = map.map(odf->m_odf[radialEl.n[1]]*scale);
            radialEl.c[2] = map.map(odf->m_odf[radialEl.n[2]]*scale);
        }

        // update the mesh
        pm->Update();
        radial->Update();

        // remesh sphere
        remeshSphere(odf);
    }
}

void CFiberODFAnalysis::remeshSphere(CODF* odf)
{
    // Calc Gradient
    vector<double> gradient;
    // altGradient(GetIntValue(ORDER), odf->m_sphHarmonics, gradient);
    altGradient(GetIntValue(ORDER), odf->m_odf, gradient);

    // Remesh sphere
    vector<vec3d> nodePos;
    vector<vec3i> elems;
    remeshFull(gradient, m_lengthScale, m_hausd, m_grad, nodePos, elems);

    GLMesh* mesh = &odf->m_remesh;
	// get the new mesh sizes
    int NN = nodePos.size();
    int NF = elems.size();
	mesh->Create(NN, NF);

	// get the vertex coordinates
	for (int i = 0; i < NN; ++i)
	{
		auto& node = mesh->Node(i);
		node.r = nodePos[i];
	}

    double* xCoords = new double[NN] {};
    double* yCoords = new double[NN] {};
    double* zCoords = new double[NN] {};
    for(int index = 0; index < NN; index++)
    {
        vec3d vec = mesh->Node(index).r;

        xCoords[index] = vec.x;
        yCoords[index] = vec.y;
        zCoords[index] = vec.z;
    }

    double* theta = new double[NN] {};
    double* phi = new double[NN] {};

    getSphereCoords(NN, xCoords, yCoords, zCoords, theta, phi);

    auto T = compSH(GetIntValue(ORDER), NN, theta, phi);

    delete[] xCoords;
    delete[] yCoords;
    delete[] zCoords;
    delete[] theta;
    delete[] phi;

    std::vector<double> newODF(NN, 0);  
    (*T).mult(odf->m_sphHarmonics, newODF);

    double min = *std::min_element(newODF.begin(), newODF.end());
    double max = *std::max_element(newODF.begin(), newODF.end());
    double scale = 1/(max - min);

    Post::CColorMap map;
    map.jet();

    // create elements
	for (int i=0; i<NF; ++i)
	{
        auto& el = mesh->Face(i);
        el.n[0] = elems[i].x;
        el.n[1] = elems[i].y;
        el.n[2] = elems[i].z;

        el.c[0] = map.map(newODF[el.n[0]]*scale);
        el.c[1] = map.map(newODF[el.n[1]]*scale);
        el.c[2] = map.map(newODF[el.n[2]]*scale);
	}

    mesh->Update();

    // create radial mesh
    GLMesh* radial = &odf->m_radialRemesh;
    radial->Create(NN, NF);

	// get the vertex coordinates
	for (int i = 0; i < NN; ++i)
    {
        vec3d vec = mesh->Node(i).r;

        auto& radialNode = radial->Node(i);
        double val = (newODF[i]-min)*scale;
        radialNode.r = vec3d(vec.x*val, vec.y*val, vec.z*val);
    }

    // create elements
    for (int i=0; i<NF; ++i)
    {
        auto& radialEl = radial->Face(i);
        radialEl.n[0] = elems[i].x;
        radialEl.n[1] = elems[i].y;
        radialEl.n[2] = elems[i].z;

        radialEl.c[0] = map.map(newODF[radialEl.n[0]]*scale);
        radialEl.c[1] = map.map(newODF[radialEl.n[1]]*scale);
        radialEl.c[2] = map.map(newODF[radialEl.n[2]]*scale);
    }

    radial->Update();
}