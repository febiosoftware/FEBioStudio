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

#include "stdafx.h"
#include "FERAWImageReader.h"
#include "FEMeshData_T.h"
#include "FEPostModel.h"

using namespace Post;

//-----------------------------------------------------------------------------
FERAWImageReader::FERAWImageReader(FEPostModel* fem) : FEFileReader(fem)
{
	m_ops.nx = 0;
	m_ops.ny = 0;
	m_ops.nz = 0;
	m_ops.nformat = -1;
}

//-----------------------------------------------------------------------------
bool FERAWImageReader::Load(const char* szfile)
{
	if ((m_ops.nx <= 0) || (m_ops.ny <= 0) || (m_ops.nz <= 0)) return errf("Invalid image dimensions");
	if (m_ops.nformat != 0) return errf("Invalid image format");

	// open the file
	if (Open(szfile, "rb") == false) return errf("Failed opening file.");

	// add one material to the scene
	Material mat;
	m_fem->AddMaterial(mat);

	// figure out the mesh dimensions
	int NN = m_ops.nx * m_ops.ny * m_ops.nz;
	int NE = 0;
	int dim = 0;
	if (m_ops.nz == 1) { dim = 2; NE = (m_ops.nx-1)*(m_ops.ny-1); }
	else { dim = 3; NE = (m_ops.nx-1)*(m_ops.ny-1)*(m_ops.nz - 1); }

	// create a new mesh
	FSMesh* pm = new FSMesh;

	try
	{
		pm->Create(NN, NE);
	}
	catch (std::bad_alloc)
	{
		return errf("Insufficient memory for importing image data");
	}
	catch (...)
	{
		return errf("Unkwown exception in FERAWImageReader::Load");
	}
	m_fem->AddMesh(pm);

	// scale parameters
	double dx = m_ops.wx / (float) m_ops.nx;
	double dy = m_ops.wy / (float) m_ops.ny;
	double dz = m_ops.wz / (float) m_ops.nz;

	// position the nodes
	int n = 0;
	for (int k=0; k<m_ops.nz; ++k)
		for (int j=0; j<m_ops.ny; ++j)
			for (int i=0; i<m_ops.nx; ++i)
			{
				FSNode& nd = pm->Node(n++);
				nd.r.x = (double) i * dx;
				nd.r.y = (double) j * dy;
				nd.r.z = (double) k * dz;
			}

	if (dim == 2)
	{
		// create a 2D mesh
		n = 0;
		for (int j=0; j<m_ops.ny-1; ++j)
			for (int i=0; i<m_ops.nx-1; ++i)
			{
				FSElement& e = pm->Element(n++);
				e.SetType(FE_QUAD4);
				e.m_node[0] = j*(m_ops.nx) + i;
				e.m_node[1] = j*(m_ops.nx) + i+1;
				e.m_node[2] = (j+1)*(m_ops.nx) + i+1;
				e.m_node[3] = (j+1)*(m_ops.nx) + i;
			}
	}
	else
	{
		// create a 3D mesh
		n = 0;
		for (int k=0; k<m_ops.nz-1; ++k)
			for (int j=0; j<m_ops.ny-1; ++j)
				for (int i=0; i<m_ops.nx-1; ++i)
				{
					FSElement& e = pm->Element(n++);
					e.SetType(FE_HEX8);
					e.m_node[0] = k*(m_ops.nx*m_ops.ny) + j*(m_ops.nx) + i;
					e.m_node[1] = k*(m_ops.nx*m_ops.ny) + j*(m_ops.nx) + i+1;
					e.m_node[2] = k*(m_ops.nx*m_ops.ny) + (j+1)*(m_ops.nx) + i+1;
					e.m_node[3] = k*(m_ops.nx*m_ops.ny) + (j+1)*(m_ops.nx) + i;
					e.m_node[4] = (k+1)*(m_ops.nx*m_ops.ny) + j*(m_ops.nx) + i;
					e.m_node[5] = (k+1)*(m_ops.nx*m_ops.ny) + j*(m_ops.nx) + i+1;
					e.m_node[6] = (k+1)*(m_ops.nx*m_ops.ny) + (j+1)*(m_ops.nx) + i+1;
					e.m_node[7] = (k+1)*(m_ops.nx*m_ops.ny) + (j+1)*(m_ops.nx) + i;
				}
	}

	// update the mesh
	pm->RebuildMesh();
	m_fem->UpdateBoundingBox();

	// Add a data field
	m_fem->AddDataField(new FEDataField_T<FENodeData<float> >(m_fem, EXPORT_DATA), "Image");

	// we need a single state
	FEState* ps = new FEState(0.f, m_fem, m_fem->GetFEMesh(0));
	m_fem->AddState(ps);

	// add the image data
	FENodeData<float>& d = dynamic_cast<FENodeData<float>&>(ps->m_Data[0]);
	unsigned char* pb = new unsigned char[ m_ops.nx*m_ops.ny ];
	n = 0;
	for (int k=0; k<m_ops.nz; ++k)
	{
		fread(pb, sizeof(unsigned char), m_ops.nx*m_ops.ny, m_fp);
		for (int j=0; j<m_ops.ny; ++j)
		{
			for (int i=0; i<m_ops.nx; ++i)
			{
				d[n++] = (float) pb[j*m_ops.nx + i];
			}
		}
	}
	delete [] pb;

	Close();

	return true;
}
