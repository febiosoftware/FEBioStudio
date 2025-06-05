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
#include "FEMultiBlockMesh.h"

class GBox;

class FEBoxMesher : public FEMultiBlockMesh
{
public:
	// parameters
	enum { CTYPE, RATIO, NX, NY, NZ, NSEG, GX, GY, GZ, GR, GX2, GY2, GZ2, GR2, NELEM };

	// creation methods
	enum { SIMPLE, BUTTERFLY3D, BUTTERFLY2D };

public:
	FEBoxMesher();
	FSMesh* BuildMesh(GObject* po) override;

public:
	void SetResolution(int nx, int ny, int nz);

protected:
	bool BuildMultiBlock() override;

	bool CreateRegularBoxMesh();
	bool CreateButterfly3DMesh();
	bool CreateButterfly2DMesh();

protected:
	FSMesh* CreateRegular();
	FSMesh* CreateButterfly3D();
	FSMesh* CreateButterfly2D();

	FSMesh* CreateRegularHEX  ();
	FSMesh* CreateRegularTET4 ();
	FSMesh* CreateRegularTET10();
	FSMesh* CreateRegularTET15();
	FSMesh* CreateRegularTET20();

protected:
	void BuildHexFaces(FSMesh* pm);
	void BuildTetFaces(FSMesh* pm);
	void BuildEdges(FSMesh* pm);

	int NodeIndex(int i, int j, int k) 
	{
		return i*(m_ny+1)*(m_nz+1) + j*(m_nz+1) + k;
	}

protected:
	GBox*	m_pobj;

	int		m_ctype;
	int		m_nelem;	// element type
	double	m_r;
	int		m_nx, m_ny, m_nz;
	int		m_ns;
	double	m_gx, m_gy, m_gz, m_gr;
	bool	m_bx, m_by, m_bz, m_br;
};
