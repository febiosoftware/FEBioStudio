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
#include "NASTRANimport.h"
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEMultiMaterial.h>
#include <GeomLib/GModel.h>
using namespace std;
extern GLColor col[];

//-----------------------------------------------------------------------------
bool NASTRANimport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();

	// clear all data
	m_Node.clear();
	m_Elem.clear();

	// try to open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening file %s\n", szfile);

	// parse the file
	if (ParseFile() == false)
	{
		Close();
		return false;
	}

	// don't forget to close the file
	Close();

	// build the mesh
	return BuildMesh(fem);
}

//-----------------------------------------------------------------------------
char* NASTRANimport::get_line(char* szline)
{
	do
	{
		fgets(szline, 255, m_fp);
		if (feof(m_fp)) return 0;
	}
	while (szline[0] == '$');

	char* ch = strrchr(szline, '\n');
	if (ch) *ch = 0;

	return szline;
}

//-----------------------------------------------------------------------------
bool NASTRANimport::read_card(NASTRANimport::CARD& c, char* szline)
{
	// see if it's free or long format
	char* ch = strchr(szline, ',');
	if (ch)
	{
		char* ch1 = szline;
		*ch = 0;
		strcpy(c.szitem[0], ch1); ch1 = ch+1;
		c.nitems = 1;

		// free format
		// TODO: Read continuation
		while (ch1 && *ch1)
		{
			ch = strchr(ch1, ',');
			if (ch) *ch = 0;
			strcpy(c.szitem[c.nitems++], ch1);
			if (ch) ch1 = ch + 1; else ch1 = 0;
		}
	}
	else
	{
		// long format
		int L = strlen(szline);

		// copy first field
		int l = (L < 8 ? L : 8);
		strncpy(c.szitem[0], szline, l); c.szitem[0][l] = 0;
		c.nitems = 1;

		// make sure there is more work to do
		if (L <= 8) return true;

		// see if it's small or large field format.
		// This is indicated by an asterisk at the end of the string in field 1
		char* szfield = c.szitem[0];

		// remove spaces
		l = 7;
		while ((l>=0) && (szfield[l] == ' ')) { szfield[l] = 0; l--; }
		if (l < 0) return false;

		bool blarge = false;
		if (szfield[l] == '*') { szfield[l] = 0; blarge = true; }

		int fieldSize = 8;
		if (blarge) fieldSize = 16;

		// start reading the next fields
		ch = szline + 8;
		L -= 8;
		while (L > 0)
		{
			char* szfield = c.szitem[c.nitems];

			int l = (L < fieldSize ? L : fieldSize);
			strncpy(szfield, ch, l); szfield[l] = 0;

			if ((strchr(szfield, '*')))
			{
				// read next line
				if (get_line(ch) == 0) return false;

				int L = strlen(ch) - 8;
				if (L > 0) ch += 8;
			}
			else
			{
				L -= fieldSize;
				ch += fieldSize;
				c.nitems++;
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
bool NASTRANimport::ParseFile()
{
	char szline[512] = {0};
	bool bvalid = false;
	while (get_line(szline))
	{
		if (strstr(szline, "BEGIN BULK"))
		{
			bvalid = true;
			break;
		}
	}

	// make sure we found the BEGIN BULK item
	if (bvalid == false) return false;

	CARD c = {0};
	while (get_line(szline))
	{
		if (strstr(szline, "ENDDATA")) break;

		// parse the line
		if (read_card(c, szline) == false) return false;

		// process the card
		if (szcmp(c.szitem[0], "GRID") == 0)
		{
			if (read_GRID(c) == false) return errf("Error reading GRID data.");
		}
		else if (szcmp(c.szitem[0], "CTETRA") == 0)
		{
			if (read_CTETRA(c) == false) return errf("Error reading CTETRA data.");
		}
		else if (szcmp(c.szitem[0], "PSOLID") == 0)
		{
			if (read_PSOLID(c) == false) return errf("Error reading PSOLID data.");
		}
		else if (szcmp(c.szitem[0], "MAT1") == 0)
		{
			if (read_MAT1(c) == false) return errf("Error reading MAT1 data.");
		}
		else if (szcmp(c.szitem[0], "CHexa") == 0)
		{
			if (read_CHEXA(c) == false) return errf("Error reading MAT1 data.");
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool NASTRANimport::read_GRID(NASTRANimport::CARD &c)
{
	if (c.nitems < 6) return false;

	GRID n;
	n.x = atof(c.szitem[3]);
	n.y = atof(c.szitem[4]);
	n.z = atof(c.szitem[5]);
	m_Node.push_back(n);
	return true;
}

//-----------------------------------------------------------------------------
bool NASTRANimport::read_CTETRA(NASTRANimport::CARD &c)
{
	if (c.nitems < 7) return false;

	ELEM e;
	e.pid = atoi(c.szitem[2]);
	e.nn = 4;
	e.n[0] = atoi(c.szitem[3]);
	e.n[1] = atoi(c.szitem[4]);
	e.n[2] = atoi(c.szitem[5]);
	e.n[3] = atoi(c.szitem[6]);
	m_Elem.push_back(e);
	return true;
}

//-----------------------------------------------------------------------------
bool NASTRANimport::read_CHEXA(NASTRANimport::CARD &c)
{
	if (c.nitems < 7) return false;

	ELEM e;
	e.pid = atoi(c.szitem[2]);
	e.nn = 8;
	e.n[0] = atoi(c.szitem[ 3]);
	e.n[1] = atoi(c.szitem[ 4]);
	e.n[2] = atoi(c.szitem[ 5]);
	e.n[3] = atoi(c.szitem[ 6]);
	e.n[4] = atoi(c.szitem[ 7]);
	e.n[5] = atoi(c.szitem[ 8]);
	e.n[6] = atoi(c.szitem[ 9]);
	e.n[7] = atoi(c.szitem[10]);
	m_Elem.push_back(e);
	return true;
}

//-----------------------------------------------------------------------------
bool NASTRANimport::read_PSOLID(NASTRANimport::CARD &c)
{
	if (c.nitems < 3) return false;

	PSOLID p;
	p.pid = atoi(c.szitem[1]);
	p.mid = atoi(c.szitem[2]);
	m_Part.push_back(p);
	return true;
}

//-----------------------------------------------------------------------------
bool NASTRANimport::read_MAT1(NASTRANimport::CARD &c)
{
	if (c.nitems < 6) return false;

	MAT1 m;
	m.mid = atoi(c.szitem[1]);
	m.E = atof(c.szitem[2]);
	m.v = atof(c.szitem[4]);
	m.d = atof(c.szitem[5]);
	m_Mat.push_back(m);
	return true;
}

//-----------------------------------------------------------------------------

bool NASTRANimport::BuildMesh(FSModel& fem)
{
	int i, n;

	// get the counts
	int nodes  = (int) m_Node.size();
	int elems  = (int) m_Elem.size();
	int nparts = (int) m_Part.size();

	// create a new mesh
	FSMesh* pm = new FSMesh;
	pm->Create(nodes, elems);

	// create nodes
	list<GRID>::iterator in = m_Node.begin();
	FSNode* pn = pm->NodePtr();
	for (i=0; i<nodes; ++i, ++pn, ++in)
	{
		pn->r.x = in->x;
		pn->r.y = in->y;
		pn->r.z = in->z;
	}

	// create elements
	list<ELEM>::iterator ih = m_Elem.begin();
	for (i=0; i<elems; ++i, ++ih)
	{
		FEElement_* pe = pm->ElementPtr(i);

		pe->SetType(ih->nn == 4? FE_TET4 : FE_HEX8);
		for (int j=0; j<ih->nn; ++j) pe->m_node[j] = ih->n[j]-1;
	}

	// update the mesh
	pm->RebuildMesh();

	GMeshObject* po = new GMeshObject(pm);

	// create parts
	if (nparts > 0)
	{
		char szname[256] = {0};
		list<PSOLID>::iterator ip = m_Part.begin();
		for (i=0; i<nparts; ++i, ++ip)
		{
			FSElemSet* pg = new FSElemSet(pm);

			sprintf(szname, "Part%02d", i+1);
			pg->SetName(szname);

			list<ELEM>::iterator ib = m_Elem.begin();
			for (n=0; n<elems; ++n, ++ib) if (ib->pid == ip->pid) pg->add(n);

			pm->AddFEElemSet(pg);
		}
	}

	// create materials
	if (m_Mat.size())
	{
		list<MAT1>::iterator im = m_Mat.begin();
		for (i=0; i<(int) m_Mat.size(); ++i, ++im)
		{
			MAT1& mat = *im;
			FSIsotropicElastic* pmat = new FSIsotropicElastic(&fem);
			pmat->SetFloatValue(FSIsotropicElastic::MP_DENSITY, mat.d);
			pmat->SetFloatValue(FSIsotropicElastic::MP_E, mat.E);
			pmat->SetFloatValue(FSIsotropicElastic::MP_v, mat.v);

			GMaterial* pgm = new GMaterial(pmat);
			pgm->SetColor(col[i%16]);
			fem.AddMaterial(pgm);

			// see if there is a part that has this material
/*			list<PSOLID>::iterator ip = m_Part.begin();
			for (int j=0; j<nparts; ++j, ++ip)
			{
				PSOLID& p = *ip;
				if (p.mid == mat.mid) pm->Part(j)->SetMaterial(pmat);
			}
*/
		}
	}

	// clean up
	m_Node.clear();
	m_Elem.clear();

	// if we get here we are good to go!
	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
