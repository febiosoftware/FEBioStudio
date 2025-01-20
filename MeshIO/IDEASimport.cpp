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

#include "IDEASimport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
#include <vector>
using namespace std;

IDEASimport::IDEASimport(FSProject& prj) : FSFileImport(prj)
{
}

IDEASimport::~IDEASimport()
{
}

bool IDEASimport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();
	m_pfem = &fem;

	// open the file 
	if (Open(szfile, "rt") == false) return errf("Failed opening IDEAS file %s", szfile);

	char sz[256] = {0};

	do
	{
		// find the next dataset
		if (fgets(sz, 255, m_fp) == 0) break;
		if ((sz[4] == '-') && (sz[5] == '1'))
		{
			// get the dataset number
			if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered in IDEAS file %s", szfile);
			int nid = atoi(sz);
			bool bend = false;
			bool bret = true;
			switch (nid)
			{
			case 151: bret = ReadHeader(bend); break;
			case 2411: bret = ReadNodes(bend); break;
			case 2412: bret = ReadElements(bend); break;
			}

			// check for return sign
			if (bret == false) return false;

			// if the end tag was not read in the dataset reader
			// we find it here
			if (bend == false)
			{
				int l;
				do
				{
					if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered in IDEAS file %s", szfile);
					l = (int)strlen(sz);
				}
				while ((l<6) || (sz[4] != '-') || (sz[5] != '1'));
			}
		}
		else return errf("Error encountered in IDEAS file %s", szfile);
	}
	while (1);

	// close the file
	Close();

	// build the mesh
	if (!BuildMesh(fem)) return false;

	char szname[256];
	strcpy(szname, szfile);
	char* ch = strrchr(szname, '.');
	if (ch) *ch = 0;
	ch = strrchr(szname, '\\');
	if (ch == 0) ch = strrchr(szname, '/');
	if (ch == 0) ch = szname; else ++ch;
	fem.GetModel().Object(fem.GetModel().Objects()-1)->SetName(ch);

	return true;
}

bool IDEASimport::BuildMesh(FSModel& fem)
{
	int i, j;

	// count mesh items
	int nodes = (int)m_Node.size();
	int elems = (int)m_Elem.size();

	// create new mesh
	FSMesh* pm = new FSMesh;
	pm->Create(nodes, elems);

	// create the node-lookup table
	list<NODE>::iterator in = m_Node.begin();
	int imin = in->id;
	int imax = in->id;
	for (i=0; i<nodes; ++i, ++in)
	{
		if (in->id < imin) imin = in->id;
		if (in->id > imax) imax = in->id;
	}

	int nsize = imax - imin + 1;
	std::vector<int> NLT; NLT.resize(nsize);
	for (i=0; i<nsize; ++i) NLT[i] = -1;

	in = m_Node.begin();
	for (i=0; i<nodes; ++i, ++in) NLT[in->id - imin] = i;

	// create the nodes
	in = m_Node.begin();
	FSNode* pn = pm->NodePtr();
	for (i=0; i<nodes; ++i, ++in, ++pn)
	{
		pn->r.x = in->x;
		pn->r.y = in->y;
		pn->r.z = in->z;
	}

	// create the elements
	list<ELEMENT>::iterator ie = m_Elem.begin();
	int ne;
	for (i=0; i<elems; ++i, ++ie)
	{
		FEElement_* pe = pm->ElementPtr(i);

		switch (ie->ntype)
		{
		case  44: pe->SetType(FE_QUAD4 ); break;
		case  91: pe->SetType(FE_TRI3  ); break;
		case  94: pe->SetType(FE_QUAD4 ); break;
		case 111: pe->SetType(FE_TET4  ); break;
		case 112: pe->SetType(FE_PENTA6); break;
        case 113: pe->SetType(FE_PENTA15); break;
        case 115:
            {
                if (ie->nn == 8) pe->SetType(FE_HEX8);
                else if (ie->nn == 5) pe->SetType(FE_PYRA5);
            }
            break;
        case 116:
            {
                if (ie->nn == 27) pe->SetType(FE_HEX27 );
                else if (ie->nn == 20) pe->SetType(FE_HEX20);
                else if (ie->nn == 13) pe->SetType(FE_PYRA13);
            }
            break;
        case 118: pe->SetType(FE_TET10 ); break;
		default:
			delete pm;
			return false;
		}

		// all elements are assigned to the same group
		pe->m_gid = 0;

		// read the nodes
		ne = pe->Nodes();
		for (j=0; j<ne; ++j) pe->m_node[j] = NLT[ie->n[j] - imin];
	}

	// update the mesh
	pm->RebuildMesh();

	// if we get here we are good to go!
	fem.GetModel().AddObject(new GMeshObject(pm));

	// clean up
	m_Node.clear();
	m_Elem.clear();

	return true;
}

bool IDEASimport::ReadHeader(bool& bend)
{
	char sz[256] = {0};

	// get the model name
	if (fgets(sz, 255, m_fp) == 0) return errf("Error while reading header dataset");

	// we did not read the end tag
	bend = false;

	// no errors encountered
	return true;
}

bool IDEASimport::ReadNodes(bool& bend)
{
	char sz[256] = {0};

	NODE node;

	do
	{
		// read the first record
		if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Nodes dataset (2411)");

		// see if this is the end tag
		if ((sz[4] == '-') && (sz[5] == '1')) break;

		// read the node label
		sscanf(sz, "%d", &node.id);

		// read the second record
		if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Nodes dataset (2411)");

		// replace 'D' with 'E'
		char* ch = sz;
		while (ch = strchr(ch, 'D')) *ch = 'E';

		// read the nodal coordinates
		sscanf(sz, "%lg %lg %lg", &node.x, &node.y, &node.z);

		// add the node to the list
		m_Node.push_back(node);
	}
	while (1);

	// we did find the end tag
	bend = true;

	// no errors encountered
	return true;
}

bool IDEASimport::ReadElements(bool& bend)
{
	char sz[256] = {0};
    int junk;

	ELEMENT el;
	int* n = el.n;

	do
	{
		// read the first record
		if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Element dataset (2412)");

		if ((sz[4] == '-') && (sz[5] == '1')) break;

		// read the element id and type
		sscanf(sz, "%d %d %d %d %d %d", &el.id, &el.ntype, &junk, &junk, &junk, &el.nn);

		// read the second record
		if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Element dataset (2412)");
		sscanf(sz, "%d %d %d %d %d %d %d %d", &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7]);

		// read the second record
		switch (el.ntype)
		{
		case 44:	// plane stress linear quadrilateral
		case 91:	// linear triangular element
		case 94:	// linear quadrilateral element
		case 111:	// linear tetrahedral element
		case 112:	// linear wedge element
		case 115:	// linear brick element
			{
				// add the element to the list
				m_Elem.push_back(el);
			}
			break;
        case 113:    // 15-node wedge element
            {
                // read the next record
                if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Element dataset (2412)");
                sscanf(sz, "%d %d %d %d %d %d %d", &n[8], &n[12], &n[13], &n[14], &n[9], &n[10], &n[11]);
                // add the element to the list
                m_Elem.push_back(el);
            }
            break;
        case 116:
            {
                // 27-node brick element
                if (el.nn == 27) {
                    // rescan second record
                    sscanf(sz, "%d %d %d %d %d %d %d %d", &n[4], &n[16], &n[0], &n[11], &n[3], &n[19], &n[7], &n[15]);
                    // read the next record
                    if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Element dataset (2412)");
                    sscanf(sz, "%d %d %d %d %d %d %d %d", &n[12], &n[8], &n[10], &n[14], &n[5], &n[17], &n[1], &n[9]);
                    // read the next record
                    if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Element dataset (2412)");
                    sscanf(sz, "%d %d %d %d %d %d %d %d", &n[2], &n[18], &n[6], &n[13], &n[26], &n[23], &n[21], &n[20]);
                    // read the next record
                    if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Element dataset (2412)");
                    sscanf(sz, "%d %d %d", &n[24], &n[22], &n[25]);
                }
                // 13-node pyramid element
                else if (el.nn == 13) {
                    // read the next record
                    if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Element dataset (2412)");
                    sscanf(sz, "%d %d %d %d %d", &n[8], &n[9], &n[10], &n[11], &n[12]);
                }
                // add the element to the list
                m_Elem.push_back(el);
            }
                break;
        case 118:    // 10-node tet element
            {
                // rescan second record
                sscanf(sz, "%d %d %d %d %d %d %d %d", &n[0], &n[4], &n[1], &n[5], &n[2], &n[6], &n[7], &n[8]);
                // read the next record
                if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Element dataset (2412)");
                sscanf(sz, "%d %d", &n[9], &n[3]);
                // add the element to the list
                m_Elem.push_back(el);
            }
                break;
		case 11:	// rod elements
		case 21:	// linear beam
		case 22:	// tapered beam
		case 23:	// curved beam
		case 24:	// parabolic beam
			{
				// read third record
				if (fgets(sz, 255, m_fp) == 0) return errf("Error encountered while reading Element dataset (2412)");
			}
			break;
		}
	}
	while (1);

	// we did read the end tag
	bend = true;

	// no errors encountered
	return true;
}
