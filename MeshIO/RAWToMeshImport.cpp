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

#include "RAWToMeshImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
#include <MeshLib/FEMeshBuilder.h>

//-----------------------------------------------------------------------------
RAWToMeshImport::RAWToMeshImport(FSProject& prj) : FSFileImport(prj)
{
	m_ntype = 0;
	m_nx = 0;
	m_ny = 0;
	m_nz = 0;

	m_x0 = m_y0 = m_z0 = 0.0;
	m_w = m_h = m_d = 1.0;

	AddChoiceParam(0, "image_type", "Image type")->SetEnumNames(" 8-bit\0 16-bit unsigned\0 64-bit real\0");

	SetActiveGroup("Image dimensions");
	AddIntParam(0, "nx", "width");
	AddIntParam(0, "ny", "height");
	AddIntParam(0, "nz", "depth");

	SetActiveGroup("Physical dimensions");
	AddDoubleParam(0, "x0", "x0");
	AddDoubleParam(0, "y0", "y0");
	AddDoubleParam(0, "z0", "z0");
	AddDoubleParam(0, "w", "width");
	AddDoubleParam(0, "h", "height");
	AddDoubleParam(0, "d", "depth");
}

//-----------------------------------------------------------------------------
RAWToMeshImport::~RAWToMeshImport()
{
	m_nx = m_ny = m_nz = 0;
}

//-----------------------------------------------------------------------------
bool RAWToMeshImport::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_ntype = GetIntValue(0);
		m_nx = GetIntValue(1);
		m_ny = GetIntValue(2);
		m_nz = GetIntValue(3);
		m_x0 = GetFloatValue(4);
		m_y0 = GetFloatValue(5);
		m_z0 = GetFloatValue(6);
		m_w = GetFloatValue(7);
		m_h = GetFloatValue(8);
		m_d = GetFloatValue(9);
	}
	else
	{
		SetIntValue(0, m_ntype);
		SetIntValue(1, m_nx);
		SetIntValue(2, m_ny);
		SetIntValue(3, m_nz);
		SetFloatValue(4, m_x0);
		SetFloatValue(5, m_y0);
		SetFloatValue(6, m_z0);
		SetFloatValue(7, m_w);
		SetFloatValue(8, m_h);
		SetFloatValue(9, m_d);
	}

	return false;
}

//-----------------------------------------------------------------------------
void RAWToMeshImport::SetImageDimensions(int nx, int ny, int nz)
{
	m_nx = nx;
	m_ny = ny;
	m_nz = nz;
}

//-----------------------------------------------------------------------------
void RAWToMeshImport::SetBoxSize(double x0, double y0, double z0, double w, double h, double d)
{
	m_x0 = x0;
	m_y0 = y0;
	m_z0 = z0;

	m_w = w;
	m_h = h;
	m_d = d;
}

//-----------------------------------------------------------------------------
bool RAWToMeshImport::Load(const char* szfile)
{
	// open and read file
	if (Open(szfile, "rb") == false) return false;
	int N = m_nx*m_ny*m_nz;
	unsigned char* pb = new unsigned char[N];
	fread(pb, N, 1, m_fp);
	Close();

	// reindex image so that we know how many gray values are effectively used 
	std::vector<int> bin; bin.assign(256, 0);
	for (int i=0; i<N; ++i) bin[pb[i]]++;
	int n = 0;
	for (int i=0; i<256; ++i) if (bin[i] > 0) bin[i] = n++; else bin[i] = -1;
	for (int i=0; i<N; ++i) { pb[i] = bin[pb[i]]; assert(pb[i] >= 0); }

	// get the FE Model
	FSModel& fem = m_prj.GetFSModel();

	// create a new mesh
	int nodes = (m_nx+1)*(m_ny+1)*(m_nz+1);
	int elems = m_nx*m_ny*m_nz;
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// define the nodal coordinates
	n = 0;
	for (int k=0; k<=m_nz; ++k)
		for (int j=0; j<=m_ny; ++j)
			for (int i=0; i<=m_nx; ++i, ++n)
			{
				vec3d& r = pm->Node(n).r;
				double x = (double) i / (double) m_nx;
				double y = (double) j / (double) m_ny;
				double z = (double) k / (double) m_nz;

				r.x = m_x0 + x*m_w;
				r.y = m_y0 + y*m_h;
				r.z = m_z0 + z*m_d;
			}

	// define the elements
	n = 0;
	for (int k=0; k<m_nz; ++k)
		for (int j=0; j<m_ny; ++j)
			for (int i=0; i<m_nx; ++i, ++n)
			{
				FSElement& elem = pm->Element(n);
				elem.SetType(FE_HEX8);
				elem.m_gid = pb[n];
				elem.m_node[0] = k*(m_nx+1)*(m_ny+1) + j*(m_nx+1) + i;
				elem.m_node[1] = k*(m_nx+1)*(m_ny+1) + j*(m_nx+1) + i+1;
				elem.m_node[2] = k*(m_nx+1)*(m_ny+1) + (j+1)*(m_nx+1) + i+1;
				elem.m_node[3] = k*(m_nx+1)*(m_ny+1) + (j+1)*(m_nx+1) + i;
				elem.m_node[4] = (k+1)*(m_nx+1)*(m_ny+1) + j*(m_nx+1) + i;
				elem.m_node[5] = (k+1)*(m_nx+1)*(m_ny+1) + j*(m_nx+1) + i+1;
				elem.m_node[6] = (k+1)*(m_nx+1)*(m_ny+1) + (j+1)*(m_nx+1) + i+1;
				elem.m_node[7] = (k+1)*(m_nx+1)*(m_ny+1) + (j+1)*(m_nx+1) + i;
			}
	delete [] pb;

	// NOTE: we don't crease the internal surfaces. 
	// For raw images this could otherwise result in a very large number of surfaces!
	FEMeshBuilder meshBuilder(*pm);
	meshBuilder.RebuildMesh(60.0, false, false);

	GMeshObject* po = new GMeshObject(pm);

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
