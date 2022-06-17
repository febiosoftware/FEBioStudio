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

#include "FiberODF.h"
#include <algorithm>
#include <complex>
#include <vector>
#include <cmath>
#include "FESpherePts.h"
#include <MeshTools/FENNQuery.h>
#include <FECore/matrix.h>
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModel.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/ModelDocument.h>
#include <MeshTools/FEElementData.h>

using std::complex;

#ifdef HAS_ITK

CFiberODF::CFiberODF(CMainWindow* wnd) : CBasicTool(wnd, "Fiber ODF", HAS_APPLY_BUTTON),
    m_wnd(wnd), m_order(16), m_tHigh(2.0), m_tLow(30)
{
    addResourceProperty(&m_imgFile, "Image File");
    addIntProperty(&m_order, "Harmonic Order");
    addDoubleProperty(&m_tHigh, "High pass cuttoff");
    addDoubleProperty(&m_tLow, "Low pass cuttoff");
}

bool CFiberODF::OnApply()
{
    // sitk::ImageFileReader reader;
    // reader.SetFileName(m_imgFile.toStdString());

    // sitk::Image img = reader.Execute();

    // if(img.GetPixelID() != sitk::sitkUInt16)
    // {
        // sitk::RescaleIntensityImageFilter rescaleFiler;
        // rescaleFiler.SetOutputMinimum(0);
        // rescaleFiler.SetOutputMaximum(65536);
        // img = rescaleFiler.Execute(img);

        // sitk::CastImageFilter castFilter;
        // castFilter.SetOutputPixelType(sitk::sitkUInt32);
        // img = castFilter.Execute(img);

        // img = sitk::Cast(img, sitk::sitkUInt32);
    // }

    sitk::Image img = sitk::ReadImage(m_imgFile.toStdString());
    img = sitk::Cast(img, sitk::sitkUInt32);

    // Apply Butterworth filter
    butterworthFilter(img);

    img = sitk::Cast(img, sitk::sitkFloat32);

    // Apply FFT and FFT shift
    img = sitk::FFTPad(img);
    img = sitk::ForwardFFT(img);
    img = sitk::FFTShift(img);

    // Obtain Power Spectrum
    img = powerSpectrum(img);

    // Remove DC componenet (does not have a direction and does not 
    // constitute fibrillar structures)
    auto size = img.GetSize();
    std::vector<uint32_t> index = {size[0]/2 + 1, size[1]/2 + 1, size[2]/2 + 1};
    img.SetPixelAsFloat(index, 0);

    // Apply Radial FFT Filter
    fftRadialFilter(img);

    std::vector<double> reduced = std::vector<double>(NPTS,0);
    reduceAmp(img, &reduced);

    // delete image
    img = sitk::Image();

    // get spherical coordinates
    double* theta = new double[NPTS] {};
    for(int index = 0; index < NPTS; index++)
    {
        theta[index] = acos(ZCOORDS[index]);
    }

    double* phi = new double[NPTS] {};
    for(int index = 0; index < NPTS; index++)
    {
        double x = XCOORDS[index];
        double y = YCOORDS[index];

        if(x > 0)
        {
            phi[index] = atan(y/x);
        }
        else if(x < 0 && y >= 0)
        {
            phi[index] = atan(y/x) + M_PI;
        }
        else if(x < 0 && y < 0)
        {
            phi[index] = atan(y/x) - M_PI;
        }
        else if(x == 0 && y > 0)
        {
            phi[index] = M_PI/2;
        }
        else if(x == 0 && y < 0)
        {
            phi[index] = -M_PI/2;
        }
    }

    // qBall algorithm

    auto C = complLapBel_Coef();
    auto T = compSH(NPTS, theta, phi);

    auto transposeT = T->transpose();

    auto A = (*T)*(*C)*((transposeT*(*T)).inverse()*transposeT);

    std::vector<double> ODF(NPTS, 0.0);
    A.mult(reduced, ODF);

    delete[] theta;
    delete[] phi;

    // normalize odf
    double gfa = GFA(ODF);
    double min = *std::min_element(ODF.begin(), ODF.end());
    double max = *std::max_element(ODF.begin(), ODF.end());

    double sum = 0;
    for(int index = 0; index < ODF.size(); index++)
    {
        ODF[index] = ODF[index] - min + (max - min)*0.1*gfa;
        sum += ODF[index];
    }

    for(int index = 0; index < ODF.size(); index++)
    {
        ODF[index] /= sum;
    }

    FSModel* fem = m_wnd->GetModelDocument()->GetFSModel();

    GObject* po = buildMesh();

    po->SetName("ODF");

	// add the object to the model
	fem->GetModel().AddObject(po);

    makeDataField(po, ODF);

    return true;
}

void CFiberODF::butterworthFilter(sitk::Image img)
{
    double fraction = 0.2;
    double steepness = 10;

    double height = img.GetSize()[0]*img.GetSpacing()[0];
    double width = img.GetSize()[1]*img.GetSpacing()[1];
    double depth = img.GetSize()[2]*img.GetSpacing()[2];
    double radMax = std::min({height, width, depth});

    double decent = radMax - radMax * fraction;

    uint32_t* data = img.GetBufferAsUInt32();

    int nx = img.GetSize()[0];
    int ny = img.GetSize()[1];
    int nz = img.GetSize()[2];

    double xStep = img.GetSpacing()[0];
    double yStep = img.GetSpacing()[1];
    double zStep = img.GetSpacing()[2];

    int index = 0;
    for(int z = 0; z < nz; z++)
    {
        for(int y = 0; y < ny; y++)
        {
            for(int x = 0; x < nx; x++)
            {
                int xPos = abs(x - nx/2);
                int yPos = abs(y - ny/2);
                int zPos = abs(z - nz/2);

                double rad = sqrt(xPos*xStep*xPos*xStep + yPos*yStep*yPos*yStep + zPos*zStep*zPos*zStep);

                data[index] = data[index]/(1 + pow(rad/decent, 2*steepness));

                index++;
            }
        }
    }
}

sitk::Image CFiberODF::powerSpectrum(sitk::Image img)
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

void CFiberODF::fftRadialFilter(sitk::Image img)
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


void CFiberODF::reduceAmp(sitk::Image img, std::vector<double>* reduced)
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
                    int xPos = x - nx / 2;
                    int yPos = y - ny / 2;
                    int zPos = z - nz / 2;
                    
                    double rad = sqrt(xPos * xStep * xPos * xStep + yPos * yStep * yPos * yStep + zPos * zStep * zPos * zStep);
                    
                    if(rad == 0) continue;

                    int closestIndex = query.Find(vec3d(xPos / rad, yPos / rad, zPos / rad));
                    // int closestIndex = 0;
                    
                    int index = x + y * nx + z * nx * ny;
                    
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

std::unique_ptr<matrix> CFiberODF::compSH(int size, double* theta, double* phi)
{
    int numCols = (m_order+1)*(m_order+2)/2;
    std::unique_ptr<matrix> out = std::make_unique<matrix>(size, numCols);
    out->fill(0,0,size, numCols, 0.0);

    for(int k = 0; k <= m_order; k+=2)
    {
        for(int m = -k; m <= k; m++)
        {
            int j = (k*k + k + 2)/2 + m - 1;

            int numType = COMPLEXTYPE;
            double factor = 1;
            if(m < 0)
            {
                numType = REALTYPE;
                factor = sqrt(2);
            }
            else if(m > 0)
            {
                numType = IMAGTYPE;
                factor = sqrt(2);
            }

            for(int index = 0; index < size; index++)
            {
                (*out)[index][j] = factor*harmonicY(k, m, theta[index], phi[index], numType);
            }
        }
    }

    return std::move(out);
}

double CFiberODF::harmonicY(int degree, int order, double theta, double phi, int numType)
{
    // Will be true if order is both positive and odd
    // In that case, we need to negate the answer to match the output
    // in Adam's MATLAB code.
    int negate = 1;
    if(order % 2 == 1) negate = -1;
    
    order = abs(order);

    double a = (2*degree+1)/(4*M_PI);
    double b = fact(degree-order)/fact(degree+order);

    double normalization = sqrt(a*b);

    double e;
    switch (numType)
    {
    case REALTYPE:
        e = cos(order*phi);
        break;
    case IMAGTYPE:
        e = sin(order*phi);
        break;
    case COMPLEXTYPE:
        e = 1;
        break;
    
    default:
        break;
    }

    return normalization*std::assoc_legendre(degree, order, cos(theta))*pow(-1, degree)*e*negate;
}

std::unique_ptr<matrix> CFiberODF::complLapBel_Coef()
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

double CFiberODF::GFA(std::vector<double> vals)
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

GObject* CFiberODF::buildMesh()
{
    FSMesh* pm = new FSMesh();
	pm->Create(NPTS, NCON);

	// create nodes
	for (int i=0; i<NPTS; ++i)
	{
		FSNode& node = pm->Node(i);
		node.pos(vec3d(XCOORDS[i], YCOORDS[i], ZCOORDS[i]));
	}

	// create elements
	for (int i=0; i<NCON; ++i)
	{
        FSElement& el = pm->Element(i);
        el.SetType(FE_TRI3);
        el.m_gid = 0;
        el.m_node[0] = CONN1[i]-1;
        el.m_node[1] = CONN2[i]-1;
        el.m_node[2] = CONN3[i]-1;
	}

	// update the mesh
	pm->RebuildMesh();
	GMeshObject* po = new GMeshObject(pm);

	return po;
}

void CFiberODF::makeDataField(GObject* obj, std::vector<double>& vals)
{
    // create element data
    int parts = obj->Parts();
    vector<int> partList;
    for (int i = 0; i < parts; ++i)
    {
        partList.push_back(i);
    }

    FSMesh* mesh = obj->GetFEMesh();
    FEPartData* pdata = new FEPartData(mesh);
    pdata->SetName("ODF");
    pdata->Create(partList, FEMeshData::DATA_SCALAR, FEMeshData::DATA_MULT);
    mesh->AddMeshDataField(pdata);

    FEElemList* elemList = pdata->BuildElemList();
    int NE = elemList->Size();
    auto it = elemList->First();
    for (int i = 0; i < NE; ++i, ++it)
    {
        FEElement_& el = *it->m_pi;
        int ne = el.Nodes();
        for (int j = 0; j < ne; ++j)
        {
            int nodeID = el.m_node[j];
            pdata->SetValue(i, j, vals[nodeID]);
        }
    }
    delete elemList;

    m_wnd->UpdateModel();
}

#endif