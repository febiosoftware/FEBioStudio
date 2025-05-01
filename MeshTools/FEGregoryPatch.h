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

class FEGregoryPatch :	public FEMesher
{
public:
	enum {W, H, NX, NY, MX, MY};

	class GNode
	{
	public:
		vec3d	m_r;	// node position
		vec3d	m_n;	// node normal
	};

	class GPatch
	{
	public:
		int		m_node[4];	// vertex indices
		vec3d	m_ye[4][2];	// edge control points
		vec3d	m_yi[4][2];	// interior control points
	};

public:
	FEGregoryPatch(void);
	~FEGregoryPatch(void);

	FSMesh* BuildMesh(GObject* po) override;

	GNode&  GetNode(int i) { return m_GNode[i]; }
	FSMesh* BuildFEMesh();
	void BuildPatchData();

protected:
	void BuildPatches();
	void BuildFaces(FSMesh* pm);

	vec3d EvalPatch(GPatch& p, double r, double s);

	int NodeIndex(int i, int j) { return j*(m_nx*m_mx+1) + i; }

protected:
	std::vector<GNode>	m_GNode;
	std::vector<GPatch>	m_GPatch;

	double	m_w, m_h;
	int	m_nx, m_ny;
	int	m_mx, m_my;
};
