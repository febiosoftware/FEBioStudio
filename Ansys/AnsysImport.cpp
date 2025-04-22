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

#include "AnsysImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
using namespace std;

AnsysImport::AnsysImport(FSProject& prj) : FSFileImport(prj)
{
}

AnsysImport::~AnsysImport(void)
{
}

//-----------------------------------------------------------------------------
bool AnsysImport::Load(const char* szfile)
{
	m_szline[0] = 0;

	// open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening file %s", szfile);

	// read the first line
	fgets(m_szline, 255, m_fp);

	// parse the file
	while (!feof(m_fp) && !ferror(m_fp))
	{
		if (strstr(m_szline, "SFE") || strstr(m_szline, "sfe"))
		{
			// skip this
			fgets(m_szline, 255, m_fp);
		}
		if (strstr(m_szline, "NBLOCK") || strstr(m_szline, "nblock"))
		{
			if (read_NBLOCK() == false) return errf("Error while reading NBLOCK");
		}
		else if (strstr(m_szline, "EBLOCK")|| strstr(m_szline, "eblock"))
		{
			if (read_EBLOCK() == false) return errf("Error while reading EBLOCK");
		}
		else fgets(m_szline, 255, m_fp);
	}

	// close the file
	Close();

	// build the mesh
	return BuildMesh(m_prj.GetFSModel());
}

//-----------------------------------------------------------------------------
bool AnsysImport::read_NBLOCK()
{
	// see if the SOLID field is defined
	bool bSolid = ((strstr(m_szline, "SOLID") != 0) || (strstr(m_szline, "solid") != 0));

	// find the number of nodes
	int nodes = 0;
	char* ch = strrchr(m_szline, ',');
	if (ch) nodes = atoi(ch+1);
	if (nodes <= 0) return false;

	// skip format line
	fgets(m_szline, 255, m_fp);

	// start reading nodes
	float x,y,z;
	for (int i=0; i<nodes; ++i)
	{
		NODE n;
		fgets(m_szline, 255, m_fp);
		x = y = z = 0.f;
		if (bSolid)
			  sscanf(m_szline, "%d%*d%*d%g%g%g", &n.nid, &x, &y, &z);
		else
			sscanf(m_szline, "%d%g%g%g", &n.nid, &x, &y, &z);

		if (n.nid == -1) break;

		n.r = vec3d(x,y,z);
		m_Node.push_back(n);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool AnsysImport::read_EBLOCK()
{
	// find the number of nodes
	int elems = 0;
	char* ch = strrchr(m_szline, ',');
	if (ch) elems = atoi(ch + 1);
	if (elems <= 0) return false;

	// read format line
	fgets(m_szline, 255, m_fp);
	// process
	ch = m_szline;
	if (*ch != '(') return false;
	ch++;
	char* ch2 = ch;
	while (isdigit(*ch2)) ch2++;
	*ch2 = 0;
	int n0 = atoi(ch); 
	ch = ch2+1; ch2 = ch;
	while (isdigit(*ch2)) ch2++;
	*ch2 = 0;
	int n1 = atoi(ch);

	// start reading elements
	for (int i=0; i<elems; ++i)
	{
		ELEM e;
		int* n = e.n;
		fgets(m_szline, 255, m_fp);
		int np = sscanf(m_szline, "%d%*d%*d%*d%*d%*d%*d%*d%d%*d%d%d%d%d%d%d%d%d%d", &e.mid, &e.nn, &e.eid, n,n+1,n+2,n+3,n+4,n+5,n+6,n+7);
		if ((e.nn == 4) || (e.nn == 8)) m_Elem.push_back(e);
		if (e.nn == 10)
		{
			fgets(m_szline, 255, m_fp);
			sscanf(m_szline, "%d%d", n+8, n+9);
			m_Elem.push_back(e);
		}
		else if (e.nn == 20)
		{
			fgets(m_szline, 255, m_fp);
			sscanf(m_szline, "%d%d%d%d%d%d%d%d%d%d%d%d", n+8, n+9, n+10, n+11, n+12, n+13, n+14, n+15, n+16, n+17, n+18, n+19);
			m_Elem.push_back(e);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool AnsysImport::BuildMesh(FSModel &fem)
{
	int i;

	// calculate the mesh size
	int nodes = (int)m_Node.size();
	int elems = (int)m_Elem.size();

	if (nodes == 0) return errf("FATAL ERROR: No nodal data defined in file.");
	if (elems == 0) return errf("FATAL ERROR: No element data defined in file.");

	// create a new mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// create nodes
	list<NODE>::iterator in = m_Node.begin();
	FSNode* pn = pm->NodePtr();
	int imin = 0, imax = 0;
	for (i=0; i<nodes; ++i, ++pn, ++in)
	{
		in->n = i;
		pn->r = in->r;

		if (i==0 || (in->nid < imin)) imin = in->nid;
		if (i==0 || (in->nid > imax)) imax = in->nid;
	}

	// create node lookup table
	int nsize = imax - imin + 1;
	std::vector<int> NLT;
	NLT.resize(nsize);
	for (i=0; i<nsize; ++i) NLT[i] = -1;
	in = m_Node.begin();
	for (i=0; i<nodes; ++i, ++in) NLT[ in->nid - imin] = i;

	in = m_Node.begin();
	list<ELEM>::iterator ih = m_Elem.begin();
	for (i=0; i<elems; ++i, ++ih)
	{
		FSElement_* pe = pm->ElementPtr(i);

		pe->m_gid = ih->mid;
		ih->tag = i;
		int* n = ih->n;
		if (ih->nn == 8)
		{
			if ((n[7] == n[6]) && (n[7]==n[5]) && (n[7]==n[4]) && (n[3]==n[2]))
			{
				pe->SetType(FE_TET4);
				pe->m_node[0] = NLT[ n[0] - imin]; if (pe->m_node[0] < 0) return false;
				pe->m_node[1] = NLT[ n[1] - imin]; if (pe->m_node[1] < 0) return false;
				pe->m_node[2] = NLT[ n[2] - imin]; if (pe->m_node[2] < 0) return false;
				pe->m_node[3] = NLT[ n[4] - imin]; if (pe->m_node[3] < 0) return false;
			}
			else if ((n[7] == n[6]) && (n[7] == n[5]) && (n[7] == n[4]))
			{
				pe->SetType(FE_PYRA5);
				pe->m_node[0] = NLT[n[0] - imin]; if (pe->m_node[0] < 0) return false;
				pe->m_node[1] = NLT[n[1] - imin]; if (pe->m_node[1] < 0) return false;
				pe->m_node[2] = NLT[n[2] - imin]; if (pe->m_node[2] < 0) return false;
				pe->m_node[3] = NLT[n[3] - imin]; if (pe->m_node[3] < 0) return false;
				pe->m_node[4] = NLT[n[4] - imin]; if (pe->m_node[4] < 0) return false;
			}
			else if ((n[7] == n[6]) && (n[7] == n[5]))
			{
				pe->SetType(FE_PENTA6);
				pe->m_node[0] = NLT[ n[0] - imin]; if (pe->m_node[0] < 0) return false;
				pe->m_node[1] = NLT[ n[1] - imin]; if (pe->m_node[1] < 0) return false;
				pe->m_node[2] = NLT[ n[2] - imin]; if (pe->m_node[2] < 0) return false;
				pe->m_node[3] = NLT[ n[3] - imin]; if (pe->m_node[3] < 0) return false;
				pe->m_node[4] = NLT[ n[4] - imin]; if (pe->m_node[4] < 0) return false;
				pe->m_node[5] = NLT[ n[5] - imin]; if (pe->m_node[5] < 0) return false;
			}
			else if ((n[2]==n[3]) && (n[7]==n[6]))
			{
				pe->SetType(FE_PENTA6);
				pe->m_node[0] = NLT[n[0] - imin]; if (pe->m_node[0] < 0) return false;
				pe->m_node[1] = NLT[n[1] - imin]; if (pe->m_node[1] < 0) return false;
				pe->m_node[2] = NLT[n[2] - imin]; if (pe->m_node[2] < 0) return false;
				pe->m_node[3] = NLT[n[4] - imin]; if (pe->m_node[3] < 0) return false;
				pe->m_node[4] = NLT[n[5] - imin]; if (pe->m_node[4] < 0) return false;
				pe->m_node[5] = NLT[n[6] - imin]; if (pe->m_node[5] < 0) return false;
			}
			else 
			{
				pe->SetType(FE_HEX8);
				pe->m_node[0] = NLT[ n[0] - imin]; if (pe->m_node[0] < 0) return false;
				pe->m_node[1] = NLT[ n[1] - imin]; if (pe->m_node[1] < 0) return false;
				pe->m_node[2] = NLT[ n[2] - imin]; if (pe->m_node[2] < 0) return false;
				pe->m_node[3] = NLT[ n[3] - imin]; if (pe->m_node[3] < 0) return false;
				pe->m_node[4] = NLT[ n[4] - imin]; if (pe->m_node[4] < 0) return false;
				pe->m_node[5] = NLT[ n[5] - imin]; if (pe->m_node[5] < 0) return false;
				pe->m_node[6] = NLT[ n[6] - imin]; if (pe->m_node[6] < 0) return false;
				pe->m_node[7] = NLT[ n[7] - imin]; if (pe->m_node[7] < 0) return false;
			}
		}
		else if (ih->nn == 4)
		{
			if (n[3] == n[2]) pe->SetType(FE_TRI3); else pe->SetType(FE_QUAD4);
			pe->m_node[0] = NLT[ n[0] - imin]; if (pe->m_node[0] < 0) return false;
			pe->m_node[1] = NLT[ n[1] - imin]; if (pe->m_node[1] < 0) return false;
			pe->m_node[2] = NLT[ n[2] - imin]; if (pe->m_node[2] < 0) return false;
			pe->m_node[3] = NLT[ n[3] - imin]; if (pe->m_node[3] < 0) return false;

			pe->m_h[0] = 0;
			pe->m_h[1] = 0;
			pe->m_h[2] = 0;
			pe->m_h[3] = 0;
		}
		else if (ih->nn == 10)
		{
			pe->SetType(FE_TET10);
			pe->m_node[0] = NLT[n[0] - imin]; if (pe->m_node[0] < 0) return false;
			pe->m_node[1] = NLT[n[1] - imin]; if (pe->m_node[1] < 0) return false;
			pe->m_node[2] = NLT[n[2] - imin]; if (pe->m_node[2] < 0) return false;
			pe->m_node[3] = NLT[n[3] - imin]; if (pe->m_node[3] < 0) return false;
			pe->m_node[4] = NLT[n[4] - imin]; if (pe->m_node[4] < 0) return false;
			pe->m_node[5] = NLT[n[5] - imin]; if (pe->m_node[5] < 0) return false;
			pe->m_node[6] = NLT[n[6] - imin]; if (pe->m_node[6] < 0) return false;
			pe->m_node[7] = NLT[n[7] - imin]; if (pe->m_node[7] < 0) return false;
			pe->m_node[8] = NLT[n[8] - imin]; if (pe->m_node[8] < 0) return false;
			pe->m_node[9] = NLT[n[9] - imin]; if (pe->m_node[9] < 0) return false;
		}
        else if (ih->nn == 13)
        {
            pe->SetType(FE_PYRA13);
            pe->m_node[0] = NLT[n[0] - imin]; if (pe->m_node[0] < 0) return false;
            pe->m_node[1] = NLT[n[1] - imin]; if (pe->m_node[1] < 0) return false;
            pe->m_node[2] = NLT[n[2] - imin]; if (pe->m_node[2] < 0) return false;
            pe->m_node[3] = NLT[n[3] - imin]; if (pe->m_node[3] < 0) return false;
            pe->m_node[4] = NLT[n[4] - imin]; if (pe->m_node[4] < 0) return false;
            pe->m_node[5] = NLT[n[5] - imin]; if (pe->m_node[5] < 0) return false;
            pe->m_node[6] = NLT[n[6] - imin]; if (pe->m_node[6] < 0) return false;
            pe->m_node[7] = NLT[n[7] - imin]; if (pe->m_node[7] < 0) return false;
            pe->m_node[8] = NLT[n[8] - imin]; if (pe->m_node[8] < 0) return false;
            pe->m_node[9] = NLT[n[9] - imin]; if (pe->m_node[9] < 0) return false;
            pe->m_node[10] = NLT[n[10] - imin]; if (pe->m_node[10] < 0) return false;
            pe->m_node[11] = NLT[n[11] - imin]; if (pe->m_node[11] < 0) return false;
            pe->m_node[12] = NLT[n[12] - imin]; if (pe->m_node[12] < 0) return false;
        }
		else if (ih->nn == 20)
		{
			pe->SetType(FE_HEX20);
			pe->m_node[0] = NLT[n[0] - imin]; if (pe->m_node[0] < 0) return false;
			pe->m_node[1] = NLT[n[1] - imin]; if (pe->m_node[1] < 0) return false;
			pe->m_node[2] = NLT[n[2] - imin]; if (pe->m_node[2] < 0) return false;
			pe->m_node[3] = NLT[n[3] - imin]; if (pe->m_node[3] < 0) return false;
			pe->m_node[4] = NLT[n[4] - imin]; if (pe->m_node[4] < 0) return false;
			pe->m_node[5] = NLT[n[5] - imin]; if (pe->m_node[5] < 0) return false;
			pe->m_node[6] = NLT[n[6] - imin]; if (pe->m_node[6] < 0) return false;
			pe->m_node[7] = NLT[n[7] - imin]; if (pe->m_node[7] < 0) return false;
			pe->m_node[8] = NLT[n[8] - imin]; if (pe->m_node[8] < 0) return false;
			pe->m_node[9] = NLT[n[9] - imin]; if (pe->m_node[9] < 0) return false;
			pe->m_node[10] = NLT[n[10] - imin]; if (pe->m_node[10] < 0) return false;
			pe->m_node[11] = NLT[n[11] - imin]; if (pe->m_node[11] < 0) return false;
			pe->m_node[12] = NLT[n[12] - imin]; if (pe->m_node[12] < 0) return false;
			pe->m_node[13] = NLT[n[13] - imin]; if (pe->m_node[13] < 0) return false;
			pe->m_node[14] = NLT[n[14] - imin]; if (pe->m_node[14] < 0) return false;
			pe->m_node[15] = NLT[n[15] - imin]; if (pe->m_node[15] < 0) return false;
			pe->m_node[16] = NLT[n[16] - imin]; if (pe->m_node[16] < 0) return false;
			pe->m_node[17] = NLT[n[17] - imin]; if (pe->m_node[17] < 0) return false;
			pe->m_node[18] = NLT[n[18] - imin]; if (pe->m_node[18] < 0) return false;
			pe->m_node[19] = NLT[n[19] - imin]; if (pe->m_node[19] < 0) return false;
		}
	}

	// update the mesh
	pm->UpdateElementPartitions();
	pm->RebuildMesh();
	GObject* po = new GMeshObject(pm);

	// clean up
	m_Node.clear();
	m_Elem.clear();

	// if we get here we are good to go!
	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	// we're good!
	return true;
}
