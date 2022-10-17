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

class GTube;
class GTube2;

//-----------------------------------------------------------------------------
class FETube : public FEMultiBlockMesh
{
public:
	enum { NDIV, NSEG, NSTACK, ZZ, ZR, GZ2, GR2, ELEM_TYPE };

public:
	FETube(){}
	FETube(GTube* po);
	FSMesh* BuildMesh();

	FSMesh* BuildMeshLegacy();
	FSMesh* BuildMultiBlockMesh();

protected:
	void BuildFaces(FSMesh* pm);
	void BuildEdges(FSMesh* pm);

	int NodeIndex(int i, int j, int k)
	{
		int nd = 4*m_nd;
		return k*(nd*(m_ns+1)) + (j%nd)*(m_ns+1) + i;
	}

	bool BuildMultiBlock() override;

protected:
	GTube*	m_pobj;

	int	m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	bool	m_bz, m_br; // double rezoning
};

//-----------------------------------------------------------------------------
class FETube2 : public FEMesher
{
public:
	enum { NDIV, NSEG, NSTACK, ZZ, ZR, GZ2, GR2 };

public:
	FETube2(){}
	FETube2(GTube2* po);
	FSMesh* BuildMesh();

protected:
	void BuildFaces(FSMesh* pm);
	void BuildEdges(FSMesh* pm);

	int NodeIndex(int i, int j, int k)
	{
		int nd = 4*m_nd;
		return k*(nd*(m_ns+1)) + (j%nd)*(m_ns+1) + i;
	}

protected:
	GTube2*	m_pobj;

	int	m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	bool	m_bz, m_br; // double rezoning
};
