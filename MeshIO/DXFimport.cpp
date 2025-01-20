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
#include "DXFimport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
using namespace std;

#define ABS(x) ((x) > 0 ? (x) : -(x))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DXFimport::DXFimport(FSProject& prj) : FSFileImport(prj)
{	
}

DXFimport::~DXFimport()
{
	list<POLYFACE*>::iterator po = m_Obj.begin();
	for (int i=0; i<(int) m_Obj.size(); ++i, ++po) delete *po;
	m_Obj.clear();
}

bool DXFimport::SearchFor(const char* sz)
{
	bool bFound = false;

	while (fgets(m_szline, 255, m_fp))
	{
		if (strstr(m_szline, sz))
		{
			bFound = true;
			break;
		}
	}

	return bFound;
}

//-----------------------------------------------------------------------------
bool DXFimport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();
	m_pfem = &fem;

	// open the file
	if (Open(szfile, "rt") == false) return false;

	int i;

	while (fgets(m_szline, 255, m_fp))
	{
		bool bret = true;
		if      (strncmp(m_szline, "POLYLINE", 8) == 0) bret = read_POLYLINE(); 
		else if (strncmp(m_szline, "3DFACE"  , 6) == 0) bret = read_3DFACE  ();

		if (bret == false) return false;
	}

	// conver the 3D faces to a polyface
	if (m_Face.size()) MakePolyFace(m_Face);

	if (m_Obj.size() == 0) return errf("No geometry found in DXF file.");

	list<POLYFACE*>::iterator pi = m_Obj.begin();
	for (int n=0; n<(int) m_Obj.size(); ++n, ++pi)
	{
		int nodes = (*pi)->m_Node.size();
		int elems = (*pi)->m_Face.size();

		FSMesh* pm = new FSMesh();
		pm->Create(nodes, elems);

		// create nodes
		list<NODE>::iterator in = (*pi)->m_Node.begin();
		FSNode* pn = pm->NodePtr();
		for (i=0; i<nodes; ++i, ++pn, ++in)
		{
			pn->r.x = in->x;
			pn->r.y = in->y;
			pn->r.z = in->z;
		}

		// create elements
		list<FACE>::iterator is = (*pi)->m_Face.begin();
		for (i=0; i<elems; ++i, ++is)
		{
			FEElement_* pe = pm->ElementPtr(i);

			pe->SetType(is->nodes == 3 ? FE_TRI3 : FE_QUAD4);
			pe->m_node[0] = is->a;
			pe->m_node[1] = is->b;
			pe->m_node[2] = is->c;
			pe->m_node[3] = (is->nodes == 3? is->c : is->d);
			pe->m_gid = 0;
		}

		// update the mesh
		pm->RebuildMesh();
		GMeshObject* po = new GMeshObject(pm);

		char sz[256];
		sprintf(sz, "Object%2d", n+1);
		po->SetName(sz);

		// add the object to the model
		fem.GetModel().AddObject(po);
	}

	return true;
}

bool DXFimport::read_POLYLINE()
{
	bool ret;

	POLYFACE* po = new POLYFACE;
	m_Obj.push_back(po);

	// find the number of nodes
	int nn;
	ret = SearchFor("71");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%d", &nn);

	// find the number of elements
	int ne;
	ret = SearchFor("72");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%d", &ne);

	// read in the nodes
	int i;
	NODE node;
	for (i=0; i<nn; i++)
	{
		ret = SearchFor("VERTEX");
		ret = SearchFor("10");
		fgets(m_szline, MAX_LINE, m_fp);
		sscanf(m_szline, "%lg", &node.x);
		fgets(m_szline, MAX_LINE, m_fp);
		fgets(m_szline, MAX_LINE, m_fp);
		sscanf(m_szline, "%lg", &node.y);
		fgets(m_szline, MAX_LINE, m_fp);
		fgets(m_szline, MAX_LINE, m_fp);
		sscanf(m_szline, "%lg", &node.z);

		po->m_Node.push_back(node);
	}
	
	// read in the faces
	int id;
	FACE face;
	for (i=0; i<ne; i++)
	{
		ret = SearchFor("VERTEX");
		ret = SearchFor("71");
		face.nodes = 3;

		// 71
		fgets(m_szline, MAX_LINE, m_fp);
		sscanf(m_szline, "%d", &face.a);

		// 72
		fgets(m_szline, MAX_LINE, m_fp);
		fgets(m_szline, MAX_LINE, m_fp);
		sscanf(m_szline, "%d", &face.b);

		// 73
		fgets(m_szline, MAX_LINE, m_fp);
		fgets(m_szline, MAX_LINE, m_fp);
		sscanf(m_szline, "%d", &face.c);

		// 74 (optional)
		fgets(m_szline, MAX_LINE, m_fp);
		id = atoi(m_szline);
		if (id == 74)
		{
			face.nodes = 4;
			fgets(m_szline, MAX_LINE, m_fp);
			sscanf(m_szline, "%d", &face.d);
		}

		face.a = ABS(face.a) - 1;
		face.b = ABS(face.b) - 1;
		face.c = ABS(face.c) - 1;
		face.d = ABS(face.d) - 1;

		po->m_Face.push_back(face);
	}

	return true;
}

bool DXFimport::read_3DFACE()
{
	bool ret;
	FACE3D f;

	ret = SearchFor("10");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[0].x);
	ret = SearchFor("20");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[0].y);
	ret = SearchFor("30");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[0].z);

	ret = SearchFor("11");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[1].x);
	ret = SearchFor("21");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[1].y);
	ret = SearchFor("31");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[1].z);

	ret = SearchFor("12");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[2].x);
	ret = SearchFor("22");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[2].y);
	ret = SearchFor("32");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[2].z);

	ret = SearchFor("13");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[3].x);
	ret = SearchFor("23");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[3].y);
	ret = SearchFor("33");
	fgets(m_szline, MAX_LINE, m_fp);
	sscanf(m_szline, "%lg", &f.r[3].z);

	if (f.r[3].x == f.r[2].x) f.nn = 3; else f.nn = 4;

	m_Face.push_back(f);

	return true;
}

void DXFimport::MakePolyFace(std::list<FACE3D> &Face)
{
	// create a new polyface
	POLYFACE* po = new POLYFACE;

	BOX b;

	// loop over all 3D faces
	int nn = 0;
	list<FACE3D>::iterator pf = Face.begin();
	for (int i=0; i<(int) Face.size(); ++i, ++pf)
	{
		FACE f;
		f.nodes = pf->nn;
		for (int j=0; j<pf->nn; ++j)
		{
			vec3d r = pf->r[j];

			// find the node index
			int n = -1;

			if (nn == 0) b = BOX(r, r);

			if (b.IsInside(r))
			{
				list<NODE>::reverse_iterator pn = po->m_Node.rbegin();
				for (int k=nn-1; k>=0; --k, ++pn) 
				{
					NODE& p = *pn;
					if (((r.x-p.x)*(r.x-p.x) + (r.y-p.y)*(r.y-p.y) + (r.z-p.z)*(r.z-p.z)) < 1.0e-6) { n = k; break; }
				}
			}

			if (n == -1)
			{
				NODE node;
				node.x = r.x;
				node.y = r.y;
				node.z = r.z;
				po->m_Node.push_back(node);
				n = nn++;

				b += r;
			}

			switch (j)
			{
			case 0: f.a = n; break;
			case 1: f.b = n; break;
			case 2: f.c = n; break;
			case 3: f.d = n; break;
			}
		}

		po->m_Face.push_back(f);
	}

	m_Obj.push_back(po);
}
