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
#include "FEMesher.h"

class GSphere;

class FEShellSphere : public FEMesher
{
public:
	enum {T, NDIV};

public:
	FEShellSphere();
	virtual ~FEShellSphere();

	FSMesh* BuildMesh(GObject* po) override;

protected:
	int GetElementID(int i, int j, int n1, int n2, int n3, int n4)
	{
		int N = m_nd;
		int n = 2*i/N + 2*(2*j/N);
		switch (n)
		{
		case 0: return n1;
		case 1: return n2;
		case 2: return n3;
		case 3: return n4;
		}
		assert(false);
		return -1;
	}

	void BuildFaces(FSMesh* pm);
	void BuildEdges(FSMesh* pm);

	int NodeIndex(int i, int j, int k);

protected:
	GSphere*	m_pobj;

	double	m_t;
	int		m_nd;
};
