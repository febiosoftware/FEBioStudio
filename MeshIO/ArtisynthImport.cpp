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
#include "ArtisynthImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>

ArtiSynthImport::ArtiSynthImport(FSProject& prj) : FSFileImport(prj)
{

}

ArtiSynthImport::~ArtiSynthImport()
{

}

bool ArtiSynthImport::Load(const char* szfile)
{
	string filename = szfile;
	size_t n = filename.rfind('.');
	if (n == string::npos) return false;

	string nodeFile = filename.substr(0, n) + ".node";
	string elemFile = filename.substr(0, n) + ".elem";

	if (!readElems(elemFile)) return false;
	if (!readNodes(nodeFile)) return false;

	FSMesh* mesh = BuildMesh();
	if (mesh)
	{
		GObject* po = new GMeshObject(mesh);

		char szname[256];
		FileTitle(szname);
		po->SetName(szname);

		GModel& mdl = m_prj.GetFSModel().GetModel();
		mdl.AddObject(po);
		return true;
	}

	return false;
}

bool ArtiSynthImport::readNodes(const std::string& nodeFile)
{
	if (!Open(nodeFile.c_str(), "rt")) return false;

	// read the first line
	m_Node.reserve(1024);
	while (!feof(m_fp) && !ferror(m_fp))
	{
		char szline[256] = { 0 };
		fgets(szline, 255, m_fp);

		int nid;
		double x, y, z;
		int nread = sscanf(szline, "%d%lg%lg%lg", &nid, &x, &y, &z);
		if (nread == 4)
		{
			m_Node.push_back({ nid, x, y, z });
		}
	}
	Close();
	return true;
}

bool ArtiSynthImport::readElems(const std::string& elemFile)
{
	if (!Open(elemFile.c_str(), "rt")) return false;

	m_Elem.reserve(1024);
	while (!feof(m_fp) && !ferror(m_fp))
	{
		char szline[256] = { 0 };
		fgets(szline, 255, m_fp);

		ELEM e;
		int nread = sscanf(szline, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d", e.node, e.node+1, e.node+2, e.node+3, e.node+4, e.node+5, e.node+6, e.node+7, e.tmp, e.tmp+1, e.tmp+2, e.tmp+3, e.tmp + 4, &e.eid);
		if (nread == 14)
		{
			m_Elem.push_back(e);
		}
	}
	Close();
	return true;
}

FSMesh* ArtiSynthImport::BuildMesh()
{
	// counts
	int nodes = (int)m_Node.size();
	int elems = (int)m_Elem.size();

	// create a new mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// create the nodes
	int imax = -1, imin = -1;
	for (int i = 0; i < nodes; ++i)
	{
		NODE& nd = m_Node[i];
		FSNode& node = pm->Node(i);
		if ((nd.nid > imax) || (imax == -1)) imax = nd.nid;
		if ((nd.nid < imin) || (imin == -1)) imin = nd.nid;
		node.m_nid = nd.nid;
		node.r = vec3d(nd.x, nd.y, nd.z);
	}

	int nsize = imax - imin + 1;
	std::vector<int> lut(nsize, -1);
	for (int i = 0; i < nodes; ++i)
	{
		NODE& nd = m_Node[i];
		lut[nd.nid - imin] = i;
	}

	// create the elements
	for (int i = 0; i < elems; ++i)
	{
		ELEM& elem = m_Elem[i];
		FSElement& el = pm->Element(i);

		if ((elem.node[6] == 0) && (elem.node[7] == 0))
		{
			if ((elem.node[4] == 0) && (elem.node[5] == 0))
				el.SetType(FE_TET4);
			else
				el.SetType(FE_PENTA6);
		}
		else
			el.SetType(FE_HEX8);

		for (int j=0; j<el.Nodes(); ++j)
			el.m_node[j] = lut[elem.node[j] - imin];

		el.m_gid = 0;
		el.m_nid = elem.eid;
	}

	// update the mesh
	pm->RebuildMesh();

	// clean up
	m_Node.clear();
	m_Elem.clear();

	// we're good!
	return pm;
}
