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

#include "HMASCIIimport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
using namespace std;

HMASCIIimport::HMASCIIimport(FSProject& prj) : FSFileImport(prj)
{
}

HMASCIIimport::~HMASCIIimport(void)
{
}

void HMASCIIimport::Clear()
{
	m_Node.clear();
	m_Elem.clear();
	m_Part.clear();
}

int parse_line(char* szline, char* argv[])
{
	if (szline[0] != '*') return -1;
	char* sz = szline;
	char* ch = strchr(szline, '(');
	if (ch == 0) return -1;
	*ch++ = 0;
	sz = ch;
	int nargs = 0;
	while (*ch != 0)
	{
		if ((*ch == ',') || (*ch == ')'))
		{
			argv[nargs++] = sz;
			*ch++ = 0; sz = ch;
		}
		else ch++;
	}
	return nargs;	
}

bool HMASCIIimport::Load(const char* szfile)
{
	FSProject& prj = m_prj;

	char szline[256] = {0};

	// try to open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening file %s", szfile);

	// make sure the lists are cleared
	Clear();

	// get the coordinates and elements
	char* argv[32];
	int nargs;
	double x, y, z;
	int np = -1, nid, nline = 0;
	while (fgets(szline, 256, m_fp))
	{
		if (strstr(szline, "*node") != 0)
		{
			nargs = parse_line(szline, argv); if (nargs == -1) return false;
			nid = atoi(argv[0]);
			x = atof(argv[1]);
			y = atof(argv[2]);
			z = atof(argv[3]);
			NODE n = {vec3d(x, y, z), nid};
			m_Node.push_back(n);
		}
		if (strstr(szline, "*component") != 0)
		{
			COMPONENT p;
			char* cl = strchr(szline, '"');
			char* cr = strrchr(szline, '"');
			if (cl && cr && (cr > cl+1))
			{
				*cr = 0;
				strncpy(p.szname, cl+1, cr-cl);
			}
			else 
			{
				sprintf(p.szname, "noname");
			}
			m_Part.push_back(p);
			++np;
		}
		else if (strstr(szline, "*tria3") != 0)
		{
			nargs = parse_line(szline, argv); if (nargs == -1) return false;
			ELEM e;
			e.nid = atoi(argv[0]);
			e.npid = np;
			e.ntype = 3;
			e.node[0] = atoi(argv[2]);
			e.node[1] = atoi(argv[3]);
			e.node[2] = atoi(argv[4]);
			m_Elem.push_back(e);
		}
		else if (strstr(szline, "*quad4") != 0)
		{
			nargs = parse_line(szline, argv); if (nargs == -1) return false;
			ELEM e;
			e.nid = atoi(argv[0]);
			e.npid = np;
			e.ntype = 5;
			e.node[0] = atoi(argv[2]);
			e.node[1] = atoi(argv[3]);
			e.node[2] = atoi(argv[4]);
			e.node[3] = atoi(argv[5]);
			m_Elem.push_back(e);
		}
		else if (strstr(szline, "*tetra4") != 0)
		{
			nargs = parse_line(szline, argv); if (nargs == -1) return false;
			ELEM e;
			e.nid = atoi(argv[0]);
			e.npid = np;
			e.ntype = 4;
			e.node[0] = atoi(argv[2]);
			e.node[1] = atoi(argv[3]);
			e.node[2] = atoi(argv[4]);
			e.node[3] = atoi(argv[5]);
			m_Elem.push_back(e);
		}
		else if (strstr(szline, "*hexa8") != 0)
		{
			nargs = parse_line(szline, argv); if (nargs == -1) return false;
			ELEM e;
			e.nid = atoi(argv[0]);
			e.npid = np;
			e.ntype = 8;
			e.node[0] = atoi(argv[2]);
			e.node[1] = atoi(argv[3]);
			e.node[2] = atoi(argv[4]);
			e.node[3] = atoi(argv[5]);
			e.node[4] = atoi(argv[6]);
			e.node[5] = atoi(argv[7]);
			e.node[6] = atoi(argv[8]);
			e.node[7] = atoi(argv[9]);
			m_Elem.push_back(e);
		}

		nline++;
	}

	// close the file
	Close();

	return BuildMesh(prj.GetFSModel());
}

//-----------------------------------------------------------------------------

bool HMASCIIimport::BuildMesh(FSModel& fem)
{
	// make sure there is something
	if (m_Node.empty() || m_Elem.empty()) return false;

	// count items
	int nodes = (int)m_Node.size();
	int elems = (int)m_Elem.size();
	int parts = (int)m_Part.size();

	// create the mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// find the node ID ranges
	list<NODE>::iterator in = m_Node.begin();
	int i, j;
	int nmin = in->nid, nmax = nmin;
	for (i=0; i<nodes; ++i, ++in)
	{
		int nid = in->nid;
		if (nid > nmax) nmax = nid;
		if (nid < nmin) nmin = nid;
	}

	// create the node look-up table
	int NS = nmax - nmin + 1;
	vector<int> NLT; NLT.assign(NS, -1);
	in = m_Node.begin();
	for (i=0; i<nodes; ++i, ++in)
	{
		int nid = in->nid;
		NLT[nid - nmin] = i;
	}

	// create the nodes
	in = m_Node.begin();
	for (i=0; i<nodes; ++i, in++)
	{
		FSNode& n = pm->Node(i);
		n.r = in->r;
	}

	// create elements
	list<ELEM>::iterator ie = m_Elem.begin();
	for (i=0; i<elems; ++i, ++ie)
	{
		FSElement& e = pm->Element(i);
		e.m_gid = 0;
		int nn = -1;
		switch (ie->ntype)
		{
		case 3: e.SetType(FE_TRI3); nn = 3; break;
		case 4: e.SetType(FE_TET4); nn = 4; break;
		case 5: e.SetType(FE_QUAD4); nn = 4; break;
		case 8: e.SetType(FE_HEX8); nn = 8; break;
		default:
			assert(false);
		}
		for (j=0; j<nn; ++j) e.m_node[j] = NLT[ie->node[j] - nmin];
		for (j=nn; j<8; ++j) e.m_node[j] = NLT[ie->node[nn-1] - nmin];
	}

	// update the mesh
	pm->RebuildMesh();

	// create the geometry
	GMeshObject* po = new GMeshObject(pm);

	// create parts
	if (parts > 0)
	{
		list<COMPONENT>::iterator ip = m_Part.begin();
		for (int i=0; i<parts; ++i, ++ip)
		{
			FSElemSet* pg = new FSElemSet(pm);

			pg->SetName(ip->szname);

			list<ELEM>::iterator ib = m_Elem.begin();
			for (int j=0; j<elems; ++j, ++ib) if (ib->npid == i) pg->add(j);

			po->AddFEElemSet(pg);
		}
	}

	// cleanup
	Clear();

	// if we get here we are good to go!
	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
