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
#include "Image.h"
#include <string>
#include <FSCore/box.h>

//-----------------------------------------------------------------------------
// A class for representing 3D image stacks
class C3DImage
{

public:
	C3DImage();
	virtual ~C3DImage();
	void CleanUp();

	virtual bool Create(int nx, int ny, int nz, uint8_t* data = nullptr, int pixelType = CImage::UINT_8);

	int Width () { return m_cx; }
	int Height() { return m_cy; }
	int Depth () { return m_cz; }
    int PixelType() { return m_pixelType; }
	std::string PixelTypeString();
    int BPS() const { return m_bps; }
    bool IsRGB();

    virtual BOX GetBoundingBox() { return m_box; }
    virtual void SetBoundingBox(BOX& box) { m_box = box; }

    virtual mat3d GetOrientation() { return m_orientation; }
    virtual void SetOrientation(mat3d& orientation) { m_orientation = orientation; }

	uint8_t& GetByte(int i, int j, int k) { return m_pb[m_cx*(k*m_cy + j) + i]; }
    
    double Value(int i, int j, int k, int channel = 0);
	double Value(double fx, double fy, int nz, int channel = 0);
	double Peek(double fx, double fy, double fz, int channel = 0);

    double ValueAtGlobalPos(vec3d pos, int channel = 0);

	void GetSliceX(CImage& im, int n);
	void GetSliceY(CImage& im, int n);
	void GetSliceZ(CImage& im, int n);

	void GetSampledSliceX(CImage& im, double f);
	void GetSampledSliceY(CImage& im, double f);
	void GetSampledSliceZ(CImage& im, double f);

	uint8_t* GetBytes() { return m_pb; }
	void SetBytes(uint8_t* bytes) {m_pb = bytes; }

    void GetMinMax(double& min, double& max, bool recalc = true);

	void Zero();

public:
    bool ExportRAW(const std::string& filename);
    bool ExportSITK(const std::string& filename);

private:
    template <class pType> 
    void CopySliceX(pType* dest, int n, int channels = 1);

    template <class pType> 
    void CopySliceY(pType* dest, int n, int channels = 1);

    template <class pType> 
    void CopySliceZ(pType* dest, int n, int channels = 1);

    template <class pType> 
    void CopySampledSliceX(pType* dest, double f, int channels = 1);

    template <class pType> 
    void CopySampledSliceY(pType* dest, double f, int channels = 1);

    template <class pType> 
    void CopySampledSliceZ(pType* dest, double f, int channels = 1);

    template <class pType>
    void CalcMinMaxValue();

    template <class pType>
    void ZeroTemplate(int channels = 1);

protected:
	uint8_t*	m_pb;	// image data
	int		m_cx, m_cy, m_cz; // pixel dimensions
    int     m_pixelType; // pixel representation
	int		m_bps;	// bytes per sample

    double m_minValue, m_maxValue;

    BOX     m_box; // physical bounds
    mat3d m_orientation; // rotation matrix
};

