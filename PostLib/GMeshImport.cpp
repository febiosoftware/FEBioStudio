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
#include "GMeshImport.h"
#include <FSCore/color.h>
#include "FEPostModel.h"
#include <MeshLib/FSMesh.h>
using namespace Post;

GMeshImport::GMeshImport(FEPostModel* fem) : FEFileReader(fem)
{
}

GMeshImport::~GMeshImport(void)
{
}

bool GMeshImport::Load(const char* szfile)
{
	// open the file
	if (Open(szfile, "rt") == false) return false;

	bool bret = true;
	char szline[256] = {0};
	while (fgets(szline, 255, m_fp))
	{
		if      (strncmp(szline, "$Nodes"        ,  6) == 0) bret = ReadNodes();
		else if (strncmp(szline, "$Elements"     ,  9) == 0) bret = ReadElements();

		if (bret == false) return false;
	}

	// close the file
	Close();

	return BuildMesh(*m_fem);
}

bool GMeshImport::ReadNodes()
{
	m_Node.clear();

	char szline[256] = {0};
	fgets(szline, 255, m_fp);
	int nodes = 0;
	int nread = sscanf(szline, "%d", &nodes);
	if (nread != 1) return errf("Error while reading Nodes section");

	m_Node.resize(nodes);

	// read the nodes
	for (int i=0; i<nodes; ++i)
	{
		vec3f& r = m_Node[i].r;
		fgets(szline, 255, m_fp);
		int nread = sscanf(szline, "%*d %g %g %g", &r.x, &r.y, &r.z);
		if (nread != 3) return errf("Error while reading Nodes section");
	}

	// read the end of the mesh format
	fgets(szline, 255, m_fp);
	if (strncmp(szline, "$EndNodes", 9) != 0) return errf("Failed finding EndNodes");

	return true;
}

bool GMeshImport::ReadElements()
{
	m_Elem.clear();

	char szline[256] = {0};
	fgets(szline, 255, m_fp);
	int elems = 0;
	int nread = sscanf(szline, "%d", &elems);
	if (nread != 1) return errf("Error while reading Element section");

	m_Elem.reserve(elems);

	// read the elements
	ELEM el;
	int n[13];
	for (int i=0; i<elems; ++i)
	{
		fgets(szline, 255, m_fp);
		sscanf(szline,"%d%d%d%d%d%d%d%d%d%d%d%d%d",n,n+1,n+2,n+3,n+4,n+5,n+6,n+7,n+8,n+9,n+10,n+11,n+12);

		switch (n[1])
		{
		case 4: // tetrahedron
			el.ntype = FE_TET4;
			el.node[0] = n[ 3 + n[2]    ] - 1;
			el.node[1] = n[ 3 + n[2] + 1] - 1;
			el.node[2] = n[ 3 + n[2] + 2] - 1;
			el.node[3] = n[ 3 + n[2] + 3] - 1;
			m_Elem.push_back(el);
			break;
		case 5:
			el.ntype = FE_HEX8;
			el.node[0] = n[ 3 + n[2]    ] - 1;
			el.node[1] = n[ 3 + n[2] + 1] - 1;
			el.node[2] = n[ 3 + n[2] + 2] - 1;
			el.node[3] = n[ 3 + n[2] + 3] - 1;
			el.node[4] = n[ 3 + n[2] + 4] - 1;
			el.node[5] = n[ 3 + n[2] + 5] - 1;
			el.node[6] = n[ 3 + n[2] + 6] - 1;
			el.node[7] = n[ 3 + n[2] + 7] - 1;
			m_Elem.push_back(el);
			break;
		case 6:
			el.ntype = FE_PENTA6;
			el.node[0] = n[ 3 + n[2]    ] - 1;
			el.node[1] = n[ 3 + n[2] + 1] - 1;
			el.node[2] = n[ 3 + n[2] + 2] - 1;
			el.node[3] = n[ 3 + n[2] + 3] - 1;
			el.node[4] = n[ 3 + n[2] + 4] - 1;
			el.node[5] = n[ 3 + n[2] + 5] - 1;
			m_Elem.push_back(el);
			break;
		}
	}

	// read the end of the mesh format
	fgets(szline, 255, m_fp);
	if (strncmp(szline, "$EndElements", 12) != 0) return errf("Failed finding EndElements");

	return true;
}

bool GMeshImport::BuildMesh(FEPostModel& fem)
{
	int i;

	int nodes = (int)m_Node.size();
	int elems = (int)m_Elem.size();

	if (nodes == 0) return errf("FATAL ERROR: No nodal data defined in file.");
	if (elems == 0) return errf("FATAL ERROR: No element data defined in file.");

	// clear the model
	fem.Clear();

	// add a materials
	Material mat;
	fem.AddMaterial(mat);

	// build the mesh
	FSMesh* pm = new FSMesh;
	pm->Create(nodes, elems);

	// create nodes
	for (i=0; i<nodes; ++i)
	{
		FSNode& n = pm->Node(i);
		NODE& node = m_Node[i];
		n.r.x = node.r.x;
		n.r.y = node.r.y;
		n.r.z = node.r.z;
	}

	// create elements
	for (i=0; i<elems; ++i)
	{
		FSElement& el = static_cast<FSElement&>(pm->ElementRef(i));
		ELEM& E = m_Elem[i];

		el.m_MatID = 0;
		switch (E.ntype)
		{
		case FE_TET4  : el.SetType(FE_TET4); break;
		case FE_HEX8  : el.SetType(FE_HEX8); break;
		case FE_PENTA6: el.SetType(FE_PENTA6); break;
		default:
			assert(false);
			return false;
		}

		el.m_node[0] = E.node[0];
		el.m_node[1] = E.node[1];
		el.m_node[2] = E.node[2];
		el.m_node[3] = E.node[3];
		el.m_node[4] = E.node[4];
		el.m_node[5] = E.node[5];
		el.m_node[6] = E.node[6];
		el.m_node[7] = E.node[7];
	}

	// update the mesh
	fem.AddMesh(pm);
	pm->RebuildMesh();
	fem.UpdateBoundingBox();

	// we need a single state
	FEState* ps = new FEState(0.f, &fem, fem.GetFEMesh(0));
	fem.AddState(ps);

	// clean up
	m_Node.clear();
	m_Elem.clear();

	// we're good!
	return true;
}
