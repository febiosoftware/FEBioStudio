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

using std::complex;

#ifdef HAS_ITK

CFiberODF::CFiberODF(string& inFile, string& outFile)
    : m_inFile(inFile), m_outFile(outFile), m_order(0)
{

}

void CFiberODF::Apply()
{
    sitk::ImageFileReader reader;
    reader.SetFileName(m_inFile);

    sitk::Image img = reader.Execute();

    if(img.GetPixelID() != sitk::sitkUInt16)
    {
        sitk::RescaleIntensityImageFilter rescaleFiler;
        rescaleFiler.SetOutputMinimum(0);
        rescaleFiler.SetOutputMaximum(65536);
        img = rescaleFiler.Execute(img);

        sitk::CastImageFilter castFilter;
        castFilter.SetOutputPixelType(sitk::sitkUInt16);
        img = castFilter.Execute(img);
    }

    // Apply Butterworth filter
    butterworthFilter(img);

    // Apply FFT and FFT shift
    sitk::ForwardFFTImageFilter fft;
    img = fft.Execute(img);

    sitk::FFTShiftImageFilter fftShift;
    img = fftShift.Execute(img);

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

    std::vector<double> ODF;
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
    auto size = img.GetSize();

    sitk::Image PS(size[0], size[1], size[2], sitk::sitkFloat32);

    for(uint32_t x = 0; x < size[0]; x++)
    {
        for(uint32_t y = 0; y < size[1]; y++)
        {
            for(uint32_t z = 0; z < size[2]; z++)
            {
                std::vector<uint32_t> index = {x,y,z};
                
                complex<float> val = img.GetPixelAsComplexFloat32(index);

                float ps = abs(val);

                PS.SetPixelAsFloat(index, ps*ps);
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

    int index = 0;
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

                if(val < fLow || val > fHigh)
                {
                   data[index] = 0;
                }

                index++;
            }
        }
    }
}


void CFiberODF::reduceAmp(sitk::Image img, std::vector<double>* reduced)
{
    std::vector<vec3d> points;
    points.reserve(NPTS);

    for(int index = 0; index < NPTS; index++)
    {
        points.emplace_back(XCOORDS[NPTS], YCOORDS[NPTS], ZCOORDS[NPTS]);
    }

    FSNNQuery query(&points);

    double* data = img.GetBufferAsDouble();
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
                int xPos = x - nx/2;
                int yPos = y - ny/2;
                int zPos = z - nz/2;

                double rad = sqrt(xPos*xStep*xPos*xStep + yPos*yStep*yPos*yStep + zPos*zStep*zPos*zStep);

                int closestIndex = query.Find(vec3d(xPos/rad, yPos/rad, zPos/rad));

                (*reduced)[closestIndex] += data[index];

                index++;

            }

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

std::unique_ptr<matrix> CFiberODF::compSH(int size, double* theta, double*phi)
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
    if(order < 0) order = abs(order);

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

    double returnVal = normalization*std::assoc_legendre(degree, order, cos(theta))*pow(-1, degree)*e;

    if(order < 0 && order%2==1)
    {
        returnVal *= -1;
    }

    return returnVal;
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

#endif