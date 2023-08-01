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

#include "MeshImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
using namespace std;

MeshImport::MeshImport(FSProject& prj) : FSFileImport(prj)
{
	m_bread_surface = false;
}

MeshImport::~MeshImport(void)
{
}

void MeshImport::ReadSurface(bool b)
{
	m_bread_surface = b;
}

bool MeshImport::Load(const char* szfile)
{
	if (Open(szfile, "rt") == false) return false;
	char szline[256] = {0};

	while (fgets(szline, 255, m_fp))
	{
		if (strncmp(szline, "Vertices"  ,  8) == 0) ReadNodes(m_fp);
		if (strncmp(szline, "Hexahedra" ,  9) == 0) ReadHex  (m_fp);
		if (strncmp(szline, "Tetrahedra", 10) == 0) ReadTet  (m_fp);
		if (strncmp(szline, "Triangles" ,  9) == 0) ReadTri  (m_fp);
	}

	Close();

	BuildMesh(m_prj);

	return true;
}

void MeshImport::ReadNodes(FILE* fp)
{
	char szline[256] = {0};
	fgets(szline, 255, fp);
	int nodes;
	sscanf(szline, "%d", &nodes);
	m_Node.resize(nodes);
	for (int i=0; i<nodes; ++i)
	{
		NODE& n = m_Node[i];
		fgets(szline, 255, fp);
		sscanf(szline, "%g %g %g", &n.x, &n.y, &n.z);
	}
}

void MeshImport::ReadHex(FILE* fp)
{
	char szline[256] = {0};
	fgets(szline, 255, fp);
	int elems;
	sscanf(szline, "%d", &elems);
	ELEM e;
	e.ntype = 0;
	for (int i=0; i<elems; ++i)
	{
		fgets(szline, 255, fp);
		sscanf(szline, "%d%d%d%d%d%d%d%d%d", &e.n[0], &e.n[1], &e.n[2], &e.n[3], &e.n[4], &e.n[5], &e.n[6], &e.n[7], &e.npid);
		m_Elem.push_back(e);
	}
}

void MeshImport::ReadTet(FILE* fp)
{
	char szline[256] = {0};
	fgets(szline, 255, fp);
	int elems;
	sscanf(szline, "%d", &elems);
	ELEM e;
	e.ntype = 1;
	for (int i=0; i<elems; ++i)
	{
		fgets(szline, 255, fp);
		sscanf(szline, "%d%d%d%d%d", &e.n[0], &e.n[1], &e.n[2], &e.n[3], &e.npid);
		m_Elem.push_back(e);
	}
}

void MeshImport::ReadTri(FILE* fp)
{
	char szline[256] = {0};
	fgets(szline, 255, fp);
	int elems;
	sscanf(szline, "%d", &elems);
	ELEM e;
	e.ntype = 2;
	for (int i=0; i<elems; ++i)
	{
		fgets(szline, 255, fp);
		sscanf(szline, "%d%d%d%d", &e.n[0], &e.n[1], &e.n[2], &e.npid);
		if (m_bread_surface) m_Elem.push_back(e);
	}
}

void MeshImport::BuildMesh(FSProject& prj)
{
	FSModel& fem = prj.GetFSModel();

	int i;
	int nodes = m_Node.size();
	int elems = m_Elem.size();

	// create a new mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// create the nodes
	for (i=0; i<nodes; ++i)
	{
		NODE& N = m_Node[i];
		FSNode& n = pm->Node(i);
		n.r = vec3d(N.x, N.y, N.z);
	}

	// create elements 
	for (i=0; i<elems; ++i)
	{
		ELEM& E = m_Elem[i];
		FSElement& e = pm->Element(i);
		for (int j=0; j<8; ++j) e.m_node[j] = E.n[j] - 1;
		switch(E.ntype)
		{
		case 0: e.SetType(FE_HEX8); break;
		case 1: e.SetType(FE_TET4); break;
		case 2: e.SetType(FE_TRI3); break;
		}
	}

	// see how many parts there are
	int nmin = m_Elem[0].npid, nmax = nmin;
	for (i=0; i<elems; ++i)
	{
		ELEM& E = m_Elem[i];
		if (E.npid < nmin) nmin = E.npid;
		if (E.npid > nmax) nmax = E.npid;
	}
	int nparts = nmax - nmin+1;

	for (int i=0; i<elems; ++i)
	{
		ELEM& E = m_Elem[i];
		FSElement& el = pm->Element(i);
		el.m_gid = E.npid - nmin;
	}

	pm->RebuildMesh();

	GMeshObject* po = new GMeshObject(pm);
	if (nparts > 1)
	{
		vector<FSElemSet*> P(nparts);
		char sz[256] = {0};
		for (i=0; i<nparts; ++i)
		{
			P[i] = new FSElemSet(po);
			sprintf(sz, "Part%02d", i+1);
			P[i]->SetName(sz);
		}

		for (i=0; i<elems; ++i) 
		{
			int n = m_Elem[i].npid - nmin;
			P[n]->add(i);
		}

		for (i=0; i<nparts; ++i)
		{
			if (P[i]->size()) po->AddFEElemSet(P[i]);
			else delete P[i];
		}
	}

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);
}
