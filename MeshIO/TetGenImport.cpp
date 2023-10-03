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

#include "TetGenImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>

//-----------------------------------------------------------------------------
TetGenImport::TetGenImport(FSProject& prj) : FSFileImport(prj)
{
	m_offset = 0;
}

//-----------------------------------------------------------------------------
TetGenImport::~TetGenImport()
{
}

//-----------------------------------------------------------------------------
bool TetGenImport::Load(const char* szfile)
{
	m_Node.clear();
	m_Elem.clear();

	char sznode[256] = {0};
	strcpy(sznode, szfile);
	char* ch = strrchr(sznode, '.');
	if (ch == 0) return false;
	sprintf(ch, ".node");

	// open the node file
	if (Open(sznode, "rt") == false) return false;

	// read the first line
	char szline[256] = {0};
	fgets(szline, 255, m_fp);

	//checking for comments
	sscanf(szline, "%s", ch);
	while(*ch == '#')
	{
		fgets(szline, 255, m_fp);
		sscanf(szline, "%s", ch);
	}

	int nodes, ncoords;
	sscanf(szline, "%d%d", &nodes, &ncoords);
	if (ncoords != 3) return false;

	//taking care of empty lines
	fgets(szline, 255, m_fp);
	int nread = sscanf(szline, "%s", ch);
	while (nread == -1)
	{
		fgets(szline, 255, m_fp);
		nread = sscanf(szline, "%s", ch);
	}

	NODE nd;
	for (int i=0; i<nodes; ++i)
	{		
		sscanf(szline, "%*d%lg%lg%lg", &nd.x, &nd.y, &nd.z);
		m_Node.push_back(nd);
		fgets(szline, 255, m_fp);
	}

	Close();

	// open the element file
	if (Open(szfile, "rt") == false) return false;

	fgets(szline, 255, m_fp);

	//checking for comments
	sscanf(szline, "%s", ch);
	while(*ch == '#')
	{
		fgets(szline, 255, m_fp);
		sscanf(szline, "%s", ch);
	}

	int nelems, npe, natt;
	sscanf(szline, "%d%d%d", &nelems, &npe, &natt);
	if (npe != 4) return false;

	//taking care of empty lines
	// NOTE: Not sure what this is doing here. Commenting it out for now.
/*	fgets(szline, 255, m_fp);
	nread = sscanf(szline, "%s", ch);
	while (nread == -1)
	{
		fgets(szline, 255, m_fp);
		nread = sscanf(szline, "%s", ch);
	}
*/
	ELEM el;
	int att, nid;
	for (int i=0; i<nelems; ++i)
	{
		fgets(szline, 255, m_fp);
		int nread = sscanf(szline, "%d%d%d%d%d%d", &nid, &el.node[0], &el.node[1], &el.node[2], &el.node[3], &att);
		if ((natt>0) && (nread == 5)) el.att = att; else el.att = 0; 

		// we use the first ID to see if the file is zero or one based
		if (i == 0)
		{
			if ((nid != 0) && (nid != 1)) return false;
			m_offset = nid;
		}
		m_Elem.push_back(el);
	}

	Close();

	return BuildMesh(m_prj.GetFSModel());
}

//-----------------------------------------------------------------------------
bool TetGenImport::BuildMesh(FSModel& fem)
{
	// counts
	int nodes = (int) m_Node.size();
	int elems = (int) m_Elem.size();

	// create a new mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// create the nodes
	for (int i=0; i<nodes; ++i)
	{
		NODE& nd = m_Node[i];
		pm->Node(i).r = vec3d(nd.x, nd.y, nd.z);
	}

	// create the elements
	for (int i=0; i<elems; ++i)
	{
		ELEM& tet = m_Elem[i];
		FSElement& el = pm->Element(i);

		el.SetType(FE_TET4);
		el.m_node[0] = tet.node[0]-m_offset;
		el.m_node[1] = tet.node[1]-m_offset;
		el.m_node[2] = tet.node[2]-m_offset;
		el.m_node[3] = tet.node[3]-m_offset;

		el.m_gid = tet.att;
	}

	// update the mesh
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
