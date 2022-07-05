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
copies or substantial portions of the Software.d

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
#include <unordered_map>
#include <cmath>
#include <MeshTools/FENNQuery.h>
#include <FECore/matrix.h>
#include <FECore/sphericalHarmonics.h>
#include <FECore/spherePoints.h>
#include <FEBioLink/FEBioClass.h>
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModel.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/ModelDocument.h>
#include <MeshTools/FEElementData.h>
#include <iostream>

#ifdef HAS_MMG
#include "mmg/mmg3d/libmmg3d.h"
#include <mmg/mmgs/libmmgs.h>
#include <mmg/mmg2d/libmmg2d.h>
#endif

using std::complex;
using std::unordered_map;

#ifdef HAS_ITK

CFiberODF::CFiberODF(CMainWindow* wnd) : CBasicTool(wnd, "Fiber ODF", HAS_APPLY_BUTTON),
    m_wnd(wnd), m_order(16), m_tHigh(2.0), m_tLow(30), m_lengthScale(10), m_hausd(0.05), m_grad(1.3)
{
    addResourceProperty(&m_imgFile, "Image File");
    addIntProperty(&m_order, "Harmonic Order");
    addDoubleProperty(&m_tHigh, "High pass cuttoff");
    addDoubleProperty(&m_tLow, "Low pass cuttoff");

    addDoubleProperty(&m_lengthScale, "Edge length scale");
    addDoubleProperty(&m_hausd, "Hausdorff distance");
    addDoubleProperty(&m_grad, "Remesh gradation");

}

bool CFiberODF::OnApply()
{
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

    // Remove DC component (does not have a direction and does not 
    // constitute fibrillar structures)
    // NOTE: For some reason, this doesn't perfectly match zeroing out the same point in MATLAB
    // and results in a slightly different max value in the ODF
    auto size = img.GetSize();
    std::vector<uint32_t> index = {size[0]/2 + 1, size[1]/2 + 1, size[2]/2 + 1};
    img.SetPixelAsFloat(index, 0);

    // Apply Radial FFT Filter
    fftRadialFilter(img);

    std::vector<double> reduced = std::vector<double>(NPTS,0);
    reduceAmp(img, &reduced);

    // delete image
    img = sitk::Image();

    // qBall algorithm

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

    std::vector<double> ODF(NPTS, 0.0);
    A.mult(B, reduced, ODF);

    // normalize odf
    double gfa = GFA(ODF);
    double min = *std::min_element(ODF.begin(), ODF.end());
    double max = *std::max_element(ODF.begin(), ODF.end());

    double sum = 0;
    for(int index = 0; index < ODF.size(); index++)
    {
        double val = ODF[index] - (min + (max - min)*0.1*gfa);

        if(val < 0)
        {
            ODF[index] = 0;
        }
        else
        {
            ODF[index] = val;
        }
        
        sum += ODF[index];
    }

    for(int index = 0; index < ODF.size(); index++)
    {
        ODF[index] /= sum;
    }

    // Calculate spherical harmonics
    vector<double> sphHarm((*C).columns());   
    B.mult(ODF, sphHarm);

    // Recalc ODF based on spherical harmonics
    (*T).mult(sphHarm, ODF);

    // flattenODF(ODF);

    FSModel* fem = m_wnd->GetModelDocument()->GetFSModel();

    GObject* po = buildMesh();

    po->SetName("ODF");

	// add the object to the model
	fem->GetModel().AddObject(po);

    makeDataField(po, ODF, "ODF");
    
    // Create test material
    FSMaterial* pmat = FEBio::CreateMaterial("solid mixture", fem);
    GMaterial* gmat = new GMaterial(pmat);
    gmat->SetName("FiberODF");

    FSMaterial* pmat2 = FEBio::CreateMaterial("custom fiber distribution", fem);
    pmat2->SetParamVectorDouble("shp_harmonics", sphHarm);

    pmat->AddProperty(0, pmat2);

    fem->AddMaterial(gmat);

    // Calc Gradient
    vector<double> gradient;
    altGradient((GMeshObject*)po, sphHarm, gradient);

    makeDataField(po, gradient, "Gradient");

    // Remesh sphere
    FSMesh* newMesh = Remesh(gradient);
	GMeshObject* po2 = new GMeshObject(newMesh);
    po2->SetName("Remeshed");

	// add the object to the model
	fem->GetModel().AddObject(po2);

    // Create ODF mapping for new sphere

    int nodes = newMesh->Nodes();

    double* xCoords = new double[nodes] {};
    double* yCoords = new double[nodes] {};
    double* zCoords = new double[nodes] {};
    for(int index = 0; index < nodes; index++)
    {
        vec3d vec = newMesh->Node(index).pos();

        xCoords[index] = vec.x;
        yCoords[index] = vec.y;
        zCoords[index] = vec.z;
    }

    theta = new double[nodes] {};
    phi = new double[nodes] {};

    getSphereCoords(nodes, xCoords, yCoords, zCoords, theta, phi);

    T = compSH(m_order, NPTS, theta, phi);

    delete[] xCoords;
    delete[] yCoords;
    delete[] zCoords;
    delete[] theta;
    delete[] phi;

    std::vector<double> newODF(NPTS, 0);  
    (*T).mult(sphHarm, newODF);

    makeDataField(po2, newODF, "ODF");

    return true;
}

void CFiberODF::butterworthFilter(sitk::Image& img)
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

                double rad = sqrt(xPos*xStep*xPos*xStep + yPos*yStep*yPos*yStep + zPos*zStep*zPos*zStep);

                int index = x + y*nx + z*nx*ny;

                data[index] = data[index]/(1 + pow(rad/decent, 2*steepness));
            }
        }
    }
}

sitk::Image CFiberODF::powerSpectrum(sitk::Image& img)
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

void CFiberODF::fftRadialFilter(sitk::Image& img)
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


void CFiberODF::reduceAmp(sitk::Image& img, std::vector<double>* reduced)
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

void CFiberODF::makeDataField(GObject* obj, std::vector<double>& vals, std::string name)
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
    pdata->SetName(name);
    pdata->Create(partList, FEMeshData::DATA_SCALAR, FEMeshData::DATA_MULT);
    mesh->AddMeshDataField(pdata);
    
    double PtArea = 4*M_PI/(NPTS);

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
            pdata->SetValue(i, j, vals[nodeID]/PtArea);
        }
    }
    delete elemList;

    m_wnd->UpdateModel();
}


void CFiberODF::calcGradient(std::vector<double>& sphHarm, std::vector<double>& gradient, std::vector<double>& thetaGrad,std::vector<double>& phiGrad)
{
    // 1/10 degrees
    double diff = M_PI/1800;

    double* theta = new double[NPTS] {};
    double* phi = new double[NPTS] {};

    getSphereCoords(NPTS, XCOORDS, YCOORDS, ZCOORDS, theta, phi);

    std::vector<double> ODF(NPTS, 0);  
    auto T = compSH(m_order, NPTS, theta, phi);
    (*T).mult(sphHarm, ODF);
    // flattenODF(ODF);

    double* newThetaPlus = new double[NPTS] {};
    double* newThetaMinus = new double[NPTS] {};

    double* newPhiPlus = new double[NPTS] {};
    double* newPhiMinus = new double[NPTS] {};

    // double* thetaGrad = new double[NPTS] {};
    // double* phiGrad = new double[NPTS] {};
    thetaGrad.reserve(NPTS);
    phiGrad.reserve(NPTS);

    std::vector<double> temp(NPTS, 0);

    gradient.reserve(NPTS);

    // theta diff
    for(int index = 0; index < NPTS; index++)
    {
        double val = theta[index] + diff;
        // if(val > M_PI) val -= M_PI;
        newThetaPlus[index] = val;

        // val = theta[index] - diff;
        // if(val < 0) val += M_PI;
        // newThetaMinus[index] = val;
    }

    T = compSH(m_order, NPTS, newThetaPlus, phi);
    (*T).mult(sphHarm, temp);
    // flattenODF(temp);

    for(int index = 0; index < NPTS; index++)
    {
        thetaGrad[index] = (ODF[index] - temp[index]);
    }

    // T = compSH(m_order, NPTS, newThetaMinus, phi);
    // (*T).mult(sphHarm, temp);
    // flattenODF(temp);

    // for(int index = 0; index < NPTS; index++)
    // {
    //     thetaGrad[index] += abs(ODF[index] - temp[index]);
    // }

    // phi diff
    for(int index = 0; index < NPTS; index++)
    {
        double val = phi[index] + diff;
        // if(val > M_PI) val -= M_PI;
        newPhiPlus[index] = val;

        // val = phi[index] - diff;
        // if(val < 0) val += M_PI;
        // newPhiPlus[index] = val;
    }

    T = compSH(m_order, NPTS, newPhiPlus, theta);
    (*T).mult(sphHarm, temp);
    // flattenODF(temp);

    for(int index = 0; index < NPTS; index++)
    {
        double val = sin(theta[index]);

        if(val == 0)
        {
            phiGrad[index] = 0;
        }
        else
        {
            phiGrad[index] = (ODF[index] - temp[index])/val;    
        }

        // phiGrad[index] = (ODF[index] - temp[index]);

        
    }

    // T = compSH(m_order, NPTS, newPhiMinus, phi);
    // (*T).mult(sphHarm, temp);
    // flattenODF(temp);

    // for(int index = 0; index < NPTS; index++)
    // {
    //     phiGrad[index] += abs(ODF[index] - temp[index]);
    // }
    
    for(int index = 0; index < NPTS; index++)
    {
        gradient[index] = sqrt(thetaGrad[index]*thetaGrad[index] + phiGrad[index]*phiGrad[index]);
    }

    delete[] theta;
    delete[] phi;

    delete[] newThetaPlus;
    delete[] newThetaMinus;

    delete[] newPhiPlus;
    delete[] newPhiMinus;

    // delete[]  thetaGrad;
    // delete[]  phiGrad;
}

void CFiberODF::flattenODF(std::vector<double>& ODF)
{
    double min = *std::min_element(ODF.begin(), ODF.end());
    double max = *std::max_element(ODF.begin(), ODF.end());
    double step = (max-min)/10;

    for(int index = 0; index < NPTS; index++)
    {
        double val = ODF[index];
        
        double max = 0;
        bool found = false;
        for(int i = 0; i < 9; i++)
        {
            max += step;

            if(val < max)
            {
                ODF[index] = max - step;
                found = true;
                break;
            }
        }

        if(!found) ODF[index] = max;
    }


}

void CFiberODF::altGradient(GMeshObject* mesh, std::vector<double>& sphHarm, std::vector<double>& gradient)
{
    double* theta = new double[NPTS] {};
    double* phi = new double[NPTS] {};

    getSphereCoords(NPTS, XCOORDS, YCOORDS, ZCOORDS, theta, phi);

    std::vector<double> ODF(NPTS, 0);  
    auto T = compSH(m_order, NPTS, theta, phi);
    (*T).mult(sphHarm, ODF);

    delete[] theta;
    delete[] phi;

    gradient.resize(NPTS);
    for(int index = 0; index < NPTS; index++)
    {
        gradient[index] = 0;
    }

    FSMesh* fsMesh = mesh->GetFEMesh();

    std::vector<int> count(NPTS, 0);

    for(int index = 0; index < fsMesh->Elements(); index++)
    {
        FSElement el = fsMesh->Element(index);

        int n0 = el.m_node[0];
        int n1 = el.m_node[1];
        int n2 = el.m_node[2];

        double val0 = ODF[n0];
        double val1 = ODF[n1];
        double val2 = ODF[n2];

        double diff0 = abs(val0 - val1);
        double diff1 = abs(val0 - val2);
        double diff2 = abs(val1 - val2);

        gradient[n0] += diff0 + diff1;
        gradient[n1] += diff0 + diff2;
        gradient[n2] += diff1 + diff2;

        count[n0]++;
        count[n1]++;
        count[n2]++;
    }

    for(int index = 0; index < NPTS; index++)
    {
        gradient[index] /= count[index];
    }

}

void SetError(std::string err)
{
    std::cout << err << std::endl;
}

FSMesh* CFiberODF::Remesh(std::vector<double>& gradient)
{
#ifdef HAS_MMG
	int NN = NPTS;
	int NF = NCON;
    int NC;

    // we only want to remesh half of the sphere, so here we discard 
    // any nodes that have a z coordinate < 0, and any elements defined
    // with those nodes.
    unordered_map<int, int> newNodeIDs;
    int newNodeID = 1;
    for(int index = 0; index < NN; index++)
    {
        if(ZCOORDS[index] >= 0)
        {
            newNodeIDs[index] = newNodeID;
            newNodeID++;
        }
    }

    unordered_map<int, int> newElemIDs;
    int newElemID = 1;
    for(int index = 0; index < NF; index++)
    {
        if(newNodeIDs.count(CONN1[index]-1) == 0 || newNodeIDs.count(CONN2[index]-1) == 0 || newNodeIDs.count(CONN3[index]-1) == 0)
        {
            continue;
        }

        newElemIDs[index] = newElemID;
        newElemID++;
    }

	// build the MMG mesh
	MMG5_pMesh mmgMesh;
	MMG5_pSol  mmgSol;
	mmgMesh = NULL;
	mmgSol = NULL;
	MMGS_Init_mesh(MMG5_ARG_start,
		MMG5_ARG_ppMesh, &mmgMesh,
		MMG5_ARG_ppMet, &mmgSol,
		MMG5_ARG_end);

	// allocate mesh size
	if (MMGS_Set_meshSize(mmgMesh, newNodeID-1, newElemID-1, 0) != 1)
	{
		assert(false);
		SetError("Error in MMGS_Set_meshSize");
		return nullptr;
	}

	// build the MMG mesh
	for (int i = 0; i < NN; ++i)
	{
        if(newNodeIDs.count(i))
        {
            MMGS_Set_vertex(mmgMesh, XCOORDS[i], YCOORDS[i], ZCOORDS[i], 0, newNodeIDs[i]);
        }
	}

	for (int i = 0; i < NF; ++i)
	{
        if(newElemIDs.count(i))
        {
            MMGS_Set_triangle(mmgMesh, newNodeIDs[CONN1[i]-1], newNodeIDs[CONN2[i]-1], newNodeIDs[CONN3[i]-1], 0, newElemIDs[i]);
        }
	}
	
    // Now, we build the "solution", i.e. the target element size.
	// If no elements are selected, we set a homogenous remeshing using the element size parameter.
	// set the "solution", i.e. desired element size
	if (MMGS_Set_solSize(mmgMesh, mmgSol, MMG5_Vertex, newNodeID-1, MMG5_Scalar) != 1)
	{
		assert(false);
		SetError("Error in MMG3D_Set_solSize");
		return nullptr;
	}

    int n0 = CONN1[0]-1;
    int n1 = CONN2[0]-1;

    vec3d pos0(XCOORDS[n0], YCOORDS[n0], ZCOORDS[n0]);
    vec3d pos1(XCOORDS[n1], YCOORDS[n1], ZCOORDS[n1]);

    double minLength = (pos0 - pos1).Length();
    double maxLength = minLength*m_lengthScale;

    double min = *std::min_element(gradient.begin(), gradient.end());
    double max = *std::max_element(gradient.begin(), gradient.end());
    double range = max-min;


    for (int k = 0; k < NN; k++) {
        if(newNodeIDs.count(k))
        {
            double val = (maxLength - minLength)*(1-(gradient[k] - min)/range) + minLength;
            MMGS_Set_scalarSol(mmgSol, val, newNodeIDs[k]);
        }
    }

	// set the control parameters
	MMGS_Set_dparameter(mmgMesh, mmgSol, MMGS_DPARAM_hmin, minLength);
	MMGS_Set_dparameter(mmgMesh, mmgSol, MMGS_DPARAM_hausd, m_hausd);
	MMGS_Set_dparameter(mmgMesh, mmgSol, MMGS_DPARAM_hgrad, m_grad);

	// run the mesher
	int ier = MMGS_mmgslib(mmgMesh, mmgSol);

	if (ier == MMG5_STRONGFAILURE) {
		if (min == 0.0) SetError("Element size cannot be zero.");
		else SetError("MMG was not able to remesh the mesh.");
		return nullptr;
	}
	else if (ier == MMG5_LOWFAILURE)
	{
		SetError("MMG return low failure error");
	}

	// convert back to prv mesh
	FSMesh* newMesh = new FSMesh();

	// get the new mesh sizes
	MMGS_Get_meshSize(mmgMesh, &NN, &NF, &NC);
	newMesh->Create(NN, NF);

	// get the vertex coordinates
	for (int i = 0; i < NN; ++i)
	{
		FSNode& vi = newMesh->Node(i);
		vec3d& ri = vi.r;
		int isCorner = 0;
		MMGS_Get_vertex(mmgMesh, &ri.x, &ri.y, &ri.z, &vi.m_gid, &isCorner, NULL);
		if (isCorner == 0) vi.m_gid = -1;
	}

    // create elements
	for (int i=0; i<NF; ++i)
	{
        FSElement& el = newMesh->Element(i);
        el.SetType(FE_TRI3);
        int* n = el.m_node;
        MMGS_Get_triangle(mmgMesh, n, n + 1, n + 2, &el.m_gid, NULL);
		el.m_node[0]--;
		el.m_node[1]--;
		el.m_node[2]--;
	}

	// Clean up
	MMGS_Free_all(MMG5_ARG_start,
		MMG5_ARG_ppMesh, &mmgMesh, MMG5_ARG_ppMet, &mmgSol,
		MMG5_ARG_end);

    newMesh->RebuildMesh();

	return newMesh;

#else
	SetError("This version does not have MMG support");
	return nullptr;
#endif
}

#endif