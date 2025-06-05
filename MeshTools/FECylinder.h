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

class GCylinder;
class GCylinder2;

class FECylinder : public FEMultiBlockMesh
{
public:
	enum {RATIO, NDIV, NSEG, NSTACK, ZZ, GR, CTYPE, GZ2, GR2, ELEM_TYPE};

	// creation types
	enum { BUTTERFLY, WEDGED };

public:
	FECylinder();

	FSMesh* BuildMesh(GObject* po) override;

	bool BuildMultiBlock() override;

protected:
	FSMesh* BuildButterfly();
	FSMesh* BuildWedged();

	void BuildWedgedFaces(FSMesh* pm);
	void BuildWedgedEdges(FSMesh* pm);
	void BuildWedgesNodes(FSMesh* pm);

	int NodeIndex(int i, int j, int k) 
	{
		if (j==0) return i*(m_nd*m_ns+1);
		else return i*(m_nd*m_ns+1) + 1+(j-1)*m_nd + k%m_nd; 
	}

protected:
	GCylinder*	m_pobj;

	double	m_r;
	int		m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	int		m_ctype;
	bool	m_bz, m_br;
};

class FECylinder2 : public FEMultiBlockMesh
{
public:
	enum {RATIO, NDIV, NSEG, NSTACK, ZZ, GR, CTYPE, GZ2, GR2};

	// creation types
	enum { BUTTERFLY, WEDGED };

public:
	FECylinder2();
	FSMesh* BuildMesh(GObject* op) override;

protected:
	FSMesh* BuildButterfly();
	FSMesh* BuildWedged();

	void BuildWedgedFaces(FSMesh* pm);
	void BuildWedgedEdges(FSMesh* pm);
	void BuildWedgesNodes(FSMesh* pm);

	int NodeIndex(int i, int j, int k) 
	{
		if (j==0) return i*(m_nd*m_ns+1);
		else return i*(m_nd*m_ns+1) + 1+(j-1)*m_nd + k%m_nd; 
	}

protected:
	GCylinder2*	m_pobj;

	double	m_r;
	int		m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	int		m_ctype;
	bool	m_bz, m_br;
};
