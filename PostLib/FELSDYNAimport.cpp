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
#include "FELSDYNAimport.h"
#include "FEDataManager.h"
#include "FEMeshData_T.h"
#include "FEPostModel.h"
#include "constants.h"

using namespace Post;

FELSDYNAimport::FELSDYNAimport(FEPostModel* fem) : FEFileReader(fem)
{
	m_bdispl = false;
	m_bnresults = false;
	m_bshellthick = false;
	
	m_nltoff = 0;
	m_pm = nullptr;
}

FELSDYNAimport::~FELSDYNAimport()
{

}

char* FELSDYNAimport::get_line(char* szline)
{
	do
	{
		fgets(szline, 255, m_fp);
		if (feof(m_fp)) return 0;
	}
	while (szline[0] == '$');

	char* ch = strrchr(szline, '\n');
	if (ch) *ch = 0;

	ch = strrchr(szline, '\r');
	if (ch) *ch = 0;

	return szline;
}

bool FELSDYNAimport::Load(const char* szfile)
{
	// open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening file.");

	// make sure the first line is a *KEYWORD
	if (get_line(m_szline) == 0) return errf("FATAL ERROR: Unexpected end of file.");
	if (strcmp(m_szline, "*KEYWORD") != 0) return errf("FATAL ERROR: This is not a LSDYNA Keyword file.");

	// clear all data
	m_node.clear();
	m_shell.clear();
	m_solid.clear();

	m_bnresults = false;
	m_bshellthick = false;

	// get the next line
	if (get_line(m_szline) == 0) return errf("FATAL ERROR: Unexpected end of file.");

	// repeat until done
	bool bdone = false;
	do
	{
		if (strcmp(m_szline, "*ELEMENT_SOLID") == 0)
		{
			if (Read_Element_Solid() == false) return errf("FATAL ERROR: error while reading ELEMENT_SOLID section.");
		}
		else if (strcmp(m_szline, "*ELEMENT_SHELL") == 0)
		{
			if (Read_Element_Shell() == false) return errf("FATAL ERROR: error while reading ELEMENT_SHELL section.");
		}
		else if (strcmp(m_szline, "*ELEMENT_SHELL_THICKNESS") == 0)
		{
			if (Read_Element_Shell_Thickness() == false) return errf("FATAL ERROR: error while readin ELEMENT_SHELL_THCICKNESS section.");
			m_bshellthick = true;
		}
		else if (strcmp(m_szline, "*NODE") == 0)
		{
			if (Read_Node() == false) return errf("FATAL ERROR: error while reading NODE section.");
		}
		else if (strcmp(m_szline, "*NODAL_RESULTS") == 0)
		{
			if (Read_Nodal_Results() == false) return errf("FATAL ERROR: error while readin NODAL_RESULTS section.");
			m_bnresults = true;
		}
		else if (strcmp(m_szline, "*END") == 0)
		{
			bdone = true;
		}
		else if (get_line(m_szline) == 0) return errf("FATAL ERROR: unexpected end of file.");
	}
	while (!bdone);

	// close the file
	Close();

	// build the mesh
	return BuildMesh(*m_fem);
}

bool FELSDYNAimport::Read_Element_Solid()
{
	if (get_line(m_szline) == 0) return false;
	int nread;
	ELEMENT_SOLID el;
	while (m_szline[0] != '*')
	{
		nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d%d", &el.id, &el.mid, &el.n[0], &el.n[1], &el.n[2], &el.n[3], &el.n[4], &el.n[5], &el.n[6], &el.n[7]);
		if (nread != 10) return false;

		m_solid.push_back(el);

		if (get_line(m_szline) == 0) return false;
	}

	return true;
}

bool FELSDYNAimport::Read_Element_Shell()
{
	if (get_line(m_szline) == 0) return false;
	int nread;
	ELEMENT_SHELL el;
	while (m_szline[0] != '*')
	{
		nread = sscanf(m_szline, "%d%d%d%d%d%d", &el.id, &el.mid, &el.n[0], &el.n[1], &el.n[2], &el.n[3]);
		if (nread != 6) return false;

		m_shell.push_back(el);

		if (get_line(m_szline) == 0) return false;
	}

	return true;
}

bool FELSDYNAimport::Read_Element_Shell_Thickness()
{
	if (get_line(m_szline) == 0) return false;
	int nread;
	ELEMENT_SHELL el;
	while (m_szline[0] != '*')
	{
		nread = sscanf(m_szline, "%d%d%d%d%d%d", &el.id, &el.mid, &el.n[0], &el.n[1], &el.n[2], &el.n[3]);
		if (nread != 6) return false;

		if (get_line(m_szline) == 0) return false;
		nread = sscanf(m_szline, "%lg%lg%lg%lg", &el.h[0], &el.h[1], &el.h[2], &el.h[3]);
		if (nread != 4) return false;

		m_shell.push_back(el);

		if (get_line(m_szline) == 0) return false;
	}

	return true;
}

bool FELSDYNAimport::Read_Node()
{
	if (get_line(m_szline) == 0) return false;
	int nread;
	NODE n;
	while (m_szline[0] != '*')
	{
		nread = sscanf(m_szline, "%d%lg%lg%lg", &n.id, &n.x, &n.y, &n.z);
		if (nread != 4) return false;

		m_node.push_back(n);

		if (get_line(m_szline) == 0) return false;
	}

	return true;
}

bool FELSDYNAimport::Read_Nodal_Results()
{
	if (get_line(m_szline) == 0) return false;
	int nread;
	list<NODE>::iterator it = m_node.begin();
	while (m_szline[0] != '*')
	{
		if (it == m_node.end()) return false;

		nread = sscanf(m_szline, "%*d%lg", &it->v);
		if (nread != 1) return false;

		++it;
		if (get_line(m_szline) == 0) return false;
	}

	return true;
}

void FELSDYNAimport::BuildMaterials(FEPostModel& fem)
{
	int shells = m_shell.size();
	int solids = m_solid.size();
	int i;

	int nm0 = 9999, nm1 = -9999;
	list<ELEMENT_SOLID>::iterator ih = m_solid.begin();
	for (i=0; i<solids; ++i, ++ih)
	{
		if (ih->mid > nm1) nm1 = ih->mid;
		if (ih->mid < nm0) nm0 = ih->mid;
	}
	list<ELEMENT_SHELL>::iterator is = m_shell.begin();
	for (i=0; i<shells; ++i, ++is)
	{
		if (is->mid > nm1) nm1 = is->mid;
		if (is->mid < nm0) nm0 = is->mid;
	}

	int nmat = nm1 - nm0 + 1;
	for (i=0; i<nmat; ++i)
	{
		Material mat;
		fem.AddMaterial(mat);
	}

	ih = m_solid.begin();
	for (i=0; i<solids; ++i, ++ih) ih->mid -= nm0;
	is = m_shell.begin();
	for (i=0; i<shells; ++i, ++is) is->mid -= nm0;
}

bool FELSDYNAimport::BuildMesh(FEPostModel& fem)
{
	int nodes  = m_node.size();
	int shells = m_shell.size();
	int solids = m_solid.size();

	int elems = shells + solids;

	if (nodes == 0) return errf("FATAL ERROR: No nodal data defined in file.");
	if (elems == 0) return errf("FATAL ERROR: No element data defined in file.");

	fem.Clear();

	// create the materials
	BuildMaterials(fem);

	// build the mesh
	FEPostMesh* pm = m_pm = new FEPostMesh;
	pm->Create(nodes, elems);
	BuildNLT();

	// create nodes
	list<NODE>::iterator in = m_node.begin();
	for (int i=0; i<nodes; ++i, ++in)
	{
		FSNode& n = pm->Node(i);
		n.r.x = (float) in->x;
		n.r.y = (float) in->y;
		n.r.z = (float) in->z;
	}
	fem.AddMesh(pm);

	// create solids
	int ne = 0;
	if (solids > 0)
	{
		list<NODE>::iterator in = m_node.begin();
		list<ELEMENT_SOLID>::iterator ih = m_solid.begin();
		for (int i=0; i<solids; ++i, ++ih)
		{
			FSElement& el = static_cast<FSElement&>(pm->ElementRef(ne++));
			int* n = ih->n;
			if ((n[7] == n[6]) && (n[7]==n[5]) && (n[7]==n[4]) && (n[7]==n[3])) el.SetType(FE_TET4);
			else if ((n[7] == n[6]) && (n[7]==n[5])) el.SetType(FE_PENTA6);
			else el.SetType(FE_HEX8);

			el.m_MatID = ih->mid;

			el.m_node[0] = FindNode(ih->n[0]); if (el.m_node[0] < 0) return false;
			el.m_node[1] = FindNode(ih->n[1]); if (el.m_node[1] < 0) return false;
			el.m_node[2] = FindNode(ih->n[2]); if (el.m_node[2] < 0) return false;
			el.m_node[3] = FindNode(ih->n[3]); if (el.m_node[3] < 0) return false;
			el.m_node[4] = FindNode(ih->n[4]); if (el.m_node[4] < 0) return false;
			el.m_node[5] = FindNode(ih->n[5]); if (el.m_node[5] < 0) return false;
			el.m_node[6] = FindNode(ih->n[6]); if (el.m_node[6] < 0) return false;
			el.m_node[7] = FindNode(ih->n[7]); if (el.m_node[7] < 0) return false;
		}
	}

	// create shells
	if (shells > 0)
	{
		list<NODE>::iterator in = m_node.begin();
		list<ELEMENT_SHELL>::iterator is = m_shell.begin();
		for (int i=0; i<shells; ++i, ++is)
		{
			FSElement& el = static_cast<FSElement&>(pm->ElementRef(ne++));
			el.m_node[0] = FindNode(is->n[0]); if (el.m_node[0] < 0) return false;
			el.m_node[1] = FindNode(is->n[1]); if (el.m_node[1] < 0) return false;
			el.m_node[2] = FindNode(is->n[2]); if (el.m_node[2] < 0) return false;

			el.m_MatID = is->mid;

			if (is->n[3] == is->n[2])
			{
				el.SetType(FE_TRI3);
				el.m_node[3] = el.m_node[2];
			}
			else
			{
				el.SetType(FE_QUAD4);
				el.m_node[3] = FindNode(is->n[3]); if (el.m_node[3] < 0) return false;
			}
		}
	}

	// update the mesh
	m_pm->BuildMesh();
	fem.UpdateBoundingBox();

	// add some data
	FEDataManager* pdm = fem.GetDataManager();
	int ndata[2] = {-1, -1}, nd=0;
	if (m_bnresults)
	{
		pdm->AddDataField(new FEDataField_T<FENodeData<float> >(&fem, EXPORT_DATA), "Nodal Results");
		ndata[0] = nd;	nd++;
	}

	if (m_bdispl)
	{
		pdm->AddDataField(new FEDataField_T<FENodeData<vec3f> >(&fem, EXPORT_DATA), "Displacement");
		ndata[1] = nd; 
		fem.SetDisplacementField(BUILD_FIELD(DATA_CLASS::NODE_DATA, nd, 0));
		nd++;
	}

	if (m_bshellthick)
	{
		pdm->AddDataField(new FEDataField_T<FEElementData<float ,DATA_MULT> >(&fem, EXPORT_DATA), "shell thickness");
	}

	// we need a single state
	FEState* ps = new FEState(0.f, &fem, fem.GetFEMesh(0));
	fem.AddState(ps);

	// add some data
	if (m_bnresults)
	{
		FENodeData<float>& d = dynamic_cast<FENodeData<float>&>(ps->m_Data[ndata[0]]);
		list<NODE>::iterator pn = m_node.begin();
		for (int i=0; i<nodes; ++i, ++pn) d[i] = (float) pn->v;
	}

	if (m_bshellthick)
	{
		int nel8 = m_solid.size();
		int nel2 = 0;	// we don't read beams yet
		ELEMDATA* pd = &ps->m_ELEM[0] + (nel8 + nel2);

		list<ELEMENT_SHELL>::iterator pe = m_shell.begin();
		for (int i=0; i<(int) m_shell.size(); ++i, ++pe)
		{
			double* h = pe->h;
			pd[i].m_h[0] = (float) h[0];
			pd[i].m_h[1] = (float) h[1];
			pd[i].m_h[2] = (float) h[2];
			pd[i].m_h[3] = (float) h[3];
		}

		FEElementData<float,DATA_MULT>& d = dynamic_cast<FEElementData<float,DATA_MULT>&>(ps->m_Data[0]);
		pe = m_shell.begin();
		float h[4];
		for (int i=0; i<(int) m_shell.size(); ++i, ++pe)
		{
			int ne = i;
			h[0] = (float) pe->h[0];
			h[1] = (float) pe->h[1];
			h[2] = (float) pe->h[2];
			h[3] = (float) pe->h[3];
			d.add(ne, 4, h);
		}
	}

	if (m_bdispl)
	{
		FENodeData<vec3f>& d = dynamic_cast<FENodeData<vec3f>&>(ps->m_Data[ndata[1]]);
		for (int i=0; i<nodes; ++i) d[i] = vec3f(0.f, 0.f, 0.f);
	}

	// clean up
	m_node.clear();
	m_shell.clear();
	m_solid.clear();

	// we're good!
	return true;
}

void FELSDYNAimport::BuildNLT()
{
	m_NLT.clear();
	m_nltoff = 0;
	if (m_node.empty()) return;
	const auto& it = m_node.begin();
	int minId = it->id, maxId = it->id;
	for (auto& node : m_node)
	{
		int nodeId = node.id;
		if (nodeId < minId) minId = nodeId;
		if (nodeId > maxId) maxId = nodeId;
	}
	int nsize = maxId - minId + 1;
	m_nltoff = minId;
	m_NLT.assign(nsize, -1);
	int index = 0;
	for (auto& node : m_node)
	{
		int nodeId = node.id;
		m_NLT[nodeId - m_nltoff] = index++;
	}
}

int FELSDYNAimport::FindNode(int id) const noexcept
{
	return m_NLT[id - m_nltoff];
}
