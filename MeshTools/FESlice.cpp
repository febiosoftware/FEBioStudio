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
#include "FESlice.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>

FESlice::FESlice(GObject& o) : FEMultiBlockMesh(o)
{
	m_nd = m_ns = 4;
	m_nz = 8;
	m_gz = 1;
	m_gr = 1;
	m_ctype = 0;
	m_bz = false;
	m_br = false;

	AddIntParam(m_nd, "nd", "Slices");
	AddIntParam(m_ns, "ns", "Segments");
	AddIntParam(m_nz, "nz", "Stacks");

	AddDoubleParam(m_gz, "gz", "Z-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");

	AddBoolParam(m_bz, "bz", "Z-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");
}

//-----------------------------------------------------------------------------
int FESlice::NodeIndex(int i, int j, int k) 
{
	int nid = -1;
	if (j==0) nid = i*((m_nd+1)*m_ns+1);
	else nid = i*((m_nd+1)*m_ns+1) + 1+(j-1)*(m_nd+1) + k; 
	assert(nid < (m_nz+1)*(1 + (m_nd+1)*m_ns));
	return nid;
}

//-----------------------------------------------------------------------------
FSMesh* FESlice::BuildMesh()
{
	GSlice* po = dynamic_cast<GSlice*>(&m_o);
	if (po == nullptr) return nullptr;

	int i, j, k;

	// get the object parameters
	ParamBlock& param = po->GetParamBlock();
	double R1 = param.GetFloatValue(GSlice::RADIUS);
	double h  = param.GetFloatValue(GSlice::HEIGHT);
	double w  = param.GetFloatValue(GSlice::ANGLE );
	w *= PI/180.0;

	// get the mesh parameters
	m_gz = GetFloatValue(ZZ);
	m_gr = GetFloatValue(GR);
	m_nd = GetIntValue(NSLICE);
	m_ns = GetIntValue(NLOOP);
	m_nz = GetIntValue(NSTACK);
	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_ns < 1) m_ns = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_ns == 1) { m_gr = 1; m_br = false; }

	// calculate storage
	int nodes = (m_nz+1)*(1 + (m_nd+1)*m_ns);
	int elems = m_nz*m_ns*m_nd;
	int faces = 2*m_ns*m_nd + m_nz*m_nd + 2*m_ns*m_nz;
	int edges = 2*m_nd + 3*m_nz + 4*m_ns;

	// create mesh
	FSMesh* pm = new FSMesh;
	pm->Create(nodes, elems, faces, edges);

	// --- A. Create the nodes ---
	FSNode* pn = pm->NodePtr();
	double x, y, z, R;

	double gz = 1;
	double gr = 1;

	double fz = m_gz;
	double fr = m_gr;

	if (m_bz)
	{
		gz = 2; if (m_nz%2) gz += fz;
		for (i=0; i<m_nz/2-1; ++i) gz = fz*gz+2;
		gz = h / gz;
	}
	else 
	{
		for (i=0; i<m_nz-1; ++i) gz = fz*gz+1; 
		gz = h / gz;
	}

	if (m_br)
	{
		gr = 2; if (m_ns%2) gr += fr;
		for (i=0; i<m_ns/2-1; ++i) gr = fr*gr+2;
		gr = R1 / gr;
	}
	else 
	{
		for (i=0; i<m_ns-1; ++i) gr = fr*gr+1; 
		gr = R1 / gr;
	}

	double dz = gz;
	z = 0;
	for (i=0; i<=m_nz; ++i)
	{
		double dr = gr;
		R = 0;

		// create the center node
		pn->r = vec3d(0, 0, z);
		pn->m_gid = -1;
		++pn;

		R += dr;
		dr *= fr;
		if (m_br && (m_ns == 2))
		{
			dr /= fr;
			fr = 1.0/fr;
		}

		// create the other nodes
		for (j=0; j<m_ns; ++j)
		{
			for (k=0; k<=m_nd; ++k)
			{
				x = R*cos(k*w / (m_nd));
				y = R*sin(k*w / (m_nd));

				pn->r = vec3d(x, y, z);
				pn->m_gid = -1;
				pn++;
			}

			R += dr;
			dr *= fr;
			if (m_br && (j+1 == m_ns/2-1))
			{
				if (m_ns%2 == 0) dr /= fr;
				fr = 1.0/fr;
			}
		}
		if (m_br) fr = 1.0/fr;

		z += dz;
		dz *= fz;
		if (m_bz && (i == m_nz/2-1))
		{
			if (m_nz%2 == 0) dz /= fz;
			fz = 1.0/fz;
		}
	}

	pm->Node(NodeIndex(0,    0, 0      )).m_gid = 0;
	pm->Node(NodeIndex(0, m_ns, 0      )).m_gid = 1;
	pm->Node(NodeIndex(0, m_ns, m_nd   )).m_gid = 2;
	pm->Node(NodeIndex(m_nz, 0,    0   )).m_gid = 3;
	pm->Node(NodeIndex(m_nz, m_ns, 0   )).m_gid = 4;
	pm->Node(NodeIndex(m_nz, m_ns, m_nd)).m_gid = 5;

	// --- B. Create the elements ---

	// create the inner wedge elements
	int eid = 0;
	int nlevel = 1+(m_nd+1)*m_ns;
	for (i=0; i<m_nz; i++)
	{
		// wedge elements
		for (k=0; k<m_nd; ++k)
		{
			FSElement_* ph = pm->ElementPtr(eid++);

			ph->SetType(FE_PENTA6);
			ph->m_gid = 0;
			ph->m_node[0] = i*nlevel;
			ph->m_node[1] = i*nlevel + 1 + k;
			ph->m_node[2] = i*nlevel + 1 + k+1;

			ph->m_node[3] = (i+1)*nlevel;
			ph->m_node[4] = (i+1)*nlevel + 1 + k;
			ph->m_node[5] = (i+1)*nlevel + 1 + k+1;
		}

		// hex elements
		for (j=1; j<m_ns; ++j)
		{
			for (k=0; k<m_nd; ++k)
			{
				FSElement_* ph = pm->ElementPtr(eid++);

				ph->SetType(FE_HEX8);
				ph->m_gid = 0;

				ph->m_node[0] = i*nlevel + 1 + (j-1)*(m_nd+1) + k;
				ph->m_node[1] = i*nlevel + 1 + (j  )*(m_nd+1) + k;
				ph->m_node[2] = i*nlevel + 1 + (j  )*(m_nd+1) + k+1;
				ph->m_node[3] = i*nlevel + 1 + (j-1)*(m_nd+1) + k+1;

				ph->m_node[4] = (i+1)*nlevel + 1 + (j-1)*(m_nd+1) + k;
				ph->m_node[5] = (i+1)*nlevel + 1 + (j  )*(m_nd+1) + k;
				ph->m_node[6] = (i+1)*nlevel + 1 + (j  )*(m_nd+1) + k+1;
				ph->m_node[7] = (i+1)*nlevel + 1 + (j-1)*(m_nd+1) + k+1;
			}
		}
	}

	// --- C. Create faces ---
	// side faces
	FSFace* pf = pm->FacePtr();
	for (i=0; i<m_nz; ++i)
	{
		for (k=0; k<m_ns; ++k, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
			pf->m_gid = 0;
			pf->n[0] = NodeIndex(i  , k  , 0);
			pf->n[1] = NodeIndex(i  , k+1, 0);
			pf->n[2] = NodeIndex(i+1, k+1, 0);
			pf->n[3] = NodeIndex(i+1, k  , 0);
		}
	}

	for (i=0; i<m_nz; ++i)
	{
		for (k=0; k<m_nd; ++k, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
			pf->m_gid = 1;
			pf->n[0] = NodeIndex(i  , m_ns, k  );
			pf->n[1] = NodeIndex(i  , m_ns, k+1);
			pf->n[2] = NodeIndex(i+1, m_ns, k+1);
			pf->n[3] = NodeIndex(i+1, m_ns, k  );
		}
	}

	for (i=0; i<m_nz; ++i)
	{
		for (k=0; k<m_ns; ++k, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
			pf->m_gid = 2;
			pf->n[0] = NodeIndex(i+1, k  , m_nd);
			pf->n[1] = NodeIndex(i+1, k+1, m_nd);
			pf->n[2] = NodeIndex(i  , k+1, m_nd);
			pf->n[3] = NodeIndex(i  , k  , m_nd);
		}
	}

	// bottom faces
	for (k=0; k<m_nd; ++k)
	{
		pf->SetType(FE_FACE_TRI3);
		pf->m_gid = 3;
		pf->n[0] = NodeIndex(0, 0, 0);
		pf->n[1] = NodeIndex(0, 1, k+1);
		pf->n[2] = NodeIndex(0, 1, k);
		pf->n[3] = pf->n[2];
		++pf;

		for (j=1; j<m_ns; ++j, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
			pf->m_gid = 3;
			pf->n[0] = NodeIndex(0,j  ,k+1);
			pf->n[1] = NodeIndex(0,j+1,k+1);
			pf->n[2] = NodeIndex(0,j+1,k  );
			pf->n[3] = NodeIndex(0,j  ,k  );
		}
	}

	// top faces
	for (k=0; k<m_nd; ++k)
	{
		pf->SetType(FE_FACE_TRI3);
		pf->m_gid = 4;
		pf->n[0] = NodeIndex(m_nz, 0, 0);
		pf->n[1] = NodeIndex(m_nz, 1, k);
		pf->n[2] = NodeIndex(m_nz, 1, k+1);
		pf->n[3] = pf->n[2];
		++pf;

		for (j=1; j<m_ns; ++j, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
			pf->m_gid = 4;
			pf->n[0] = NodeIndex(m_nz,j  ,k  );
			pf->n[1] = NodeIndex(m_nz,j+1,k  );
			pf->n[2] = NodeIndex(m_nz,j+1,k+1);
			pf->n[3] = NodeIndex(m_nz,j  ,k+1);
		}
	}

	// --- D. Create edges ---
	FSEdge* pe = pm->EdgePtr();
	for (k=0; k<m_ns; ++k, ++pe)
	{
		pe->SetType(FE_EDGE2);
		pe->m_gid = 0;
		pe->n[0] = NodeIndex(0, k  , 0);
		pe->n[1] = NodeIndex(0, k+1, 0);
	}

	for (k=0; k<m_nd; ++k, ++pe)
	{
		pe->SetType(FE_EDGE2); 
		pe->m_gid = 1;
		pe->n[0] = NodeIndex(0, m_ns, k);
		pe->n[1] = NodeIndex(0, m_ns, k+1);
	}

	for (k=0; k<m_ns; ++k, ++pe)
	{
		pe->SetType(FE_EDGE2); 
		pe->m_gid = 2;
		pe->n[0] = NodeIndex(0, m_ns-k  , m_nd);
		pe->n[1] = NodeIndex(0, m_ns-k-1, m_nd);
	}

	for (k=0; k<m_ns; ++k, ++pe)
	{
		pe->SetType(FE_EDGE2);
		pe->m_gid = 3;
		pe->n[0] = NodeIndex(m_nz, k  , 0);
		pe->n[1] = NodeIndex(m_nz, k+1, 0);
	}

	for (k=0; k<m_nd; ++k, ++pe)
	{
		pe->SetType(FE_EDGE2);
		pe->m_gid = 4;
		pe->n[0] = NodeIndex(m_nz, m_ns, k);
		pe->n[1] = NodeIndex(m_nz, m_ns, k+1);
	}

	for (k=0; k<m_ns; ++k, ++pe)
	{
		pe->SetType(FE_EDGE2);
		pe->m_gid = 5;
		pe->n[0] = NodeIndex(m_nz, m_ns-k  , m_nd);
		pe->n[1] = NodeIndex(m_nz, m_ns-k-1, m_nd);
	}

	for (i=0; i<m_nz; ++i, ++pe)
	{
		pe->SetType(FE_EDGE2);
		pe->m_gid = 6;
		pe->n[0] = NodeIndex(i  , 0, 0);
		pe->n[1] = NodeIndex(i+1, 0, 0);
	}

	for (k=0; k<2; ++k)
	{
		for (i=0; i<m_nz; ++i, ++pe)
		{
			pe->SetType(FE_EDGE2); 
			pe->m_gid = 7 + k;
			pe->n[0] = NodeIndex(i  , m_ns, k*m_nd);
			pe->n[1] = NodeIndex(i+1, m_ns, k*m_nd);
		}
	}

	pm->BuildMesh();

	return pm;
}
