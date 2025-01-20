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
#include "FEExtrudeFaces.h"
#include <MeshLib/FSMeshBuilder.h>
using namespace std;

FEExtrudeFaces::FEExtrudeFaces() : FEModifier("Extrude faces")
{
	AddDoubleParam(1.0, "D", "Distance");
	AddIntParam(1, "N", "Segments");
	AddBoolParam(false, "L", "Use local normal");
	AddDoubleParam(1, "mesh bias");
	AddBoolParam(false, "symmetric mesh bias");

	m_map = nullptr;
}

void FEExtrudeFaces::SetExtrusionDistance(double D)
{
	SetFloatValue(0, D);
}

void FEExtrudeFaces::SetSegments(int n)
{
	SetIntValue(1, n);
}

void FEExtrudeFaces::SetUseNormalLocal(bool b)
{
	SetBoolValue(2, b);
}

void FEExtrudeFaces::SetMeshBiasFactor(double g)
{
	SetFloatValue(3, g);
}

void FEExtrudeFaces::SetSymmetricBias(bool b)
{
	SetBoolValue(4, b);
}

void FEExtrudeFaces::SetNodalMap(FSNodeData* map)
{
	m_map = map;
}

FSMesh* FEExtrudeFaces::Apply(FSGroup* pg)
{
	if (pg->Type() != FE_SURFACE)
	{
		FEModifier::SetError("Invalid selection");
		return 0;
	}

	if (pg->size() == 0)
	{
		FEModifier::SetError("Empty selection");
		return 0;
	}

	vector<int> faceList;
	for (FSItemListBuilder::Iterator it = pg->begin(); it != pg->end(); ++it)
	{
		faceList.push_back(*it);
	}

	FSMesh* pm = pg->GetMesh();
	FSMesh* pnm = new FSMesh(*pm);
	Extrude(pnm, faceList);

	// let's try to map the user selections
	pnm->MapFENodeSets(pm);
	pnm->MapFEElemSets(pm);
	pnm->MapFESurfaces(pm);

	return pnm;
}

FSMesh* FEExtrudeFaces::Apply(FSMesh* pm) 
{
	vector<int> faceList;
	for (int i=0; i<pm->Faces(); ++i)
	{
		if (pm->Face(i).IsSelected()) faceList.push_back(i);
	}	

	FSMesh* pnm = new FSMesh(*pm);
	if (!Extrude(pnm, faceList))
	{
		delete pnm;
		pnm = nullptr;
	}

	return pnm;
}

bool FEExtrudeFaces::Extrude(FSMesh* pm, vector<int>& faceList)
{
	// let's mark the nodes that need to be copied
	pm->TagAllNodes(-1);
	for (int i = 0; i<pm->Nodes(); ++i) pm->Node(i).m_ntag = -1;

	bool linear = true;
	int ne1 = 0;
	for (int i=0; i < (int)faceList.size(); ++i)
	{
		FSFace& face = pm->Face(faceList[i]);
		int n = face.Nodes();

		// extrusion is only allowed on tris and quads!
		if ((n != 4) && (n != 3) && (n != 9) && (n != 8) && (n != 6)) return false;

		if ((n == 9) || (n == 8) || (n == 6)) linear = false;

		for (int j = 0; j<n; ++j) pm->Node(face.n[j]).m_ntag = 1;
		++ne1;
	}

	// make sure there was something selected
	if (ne1 == 0) return false;

	// count the nodes
	int n0 = pm->Nodes();
	int nn = 0;
	for (int i = 0; i < n0; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.m_ntag > 0)
		{
			node.m_ntag = nn;
			++nn;
		}
		else node.m_ntag = -1;
	}

	double dist = GetFloatValue(0);
	int nseg = GetIntValue(1);
	bool bloc = GetBoolValue(2);

	double gd = GetFloatValue(3);
	bool bd = GetBoolValue(4);

	// cache the node IDs for now.
	vector<int> nodeIDs(n0);
	for (int i = 0; i < n0; ++i) nodeIDs[i] = pm->Node(i).m_nid;

	// allocate room for new nodes
	if (linear) pm->Create(n0 + nseg*nn, 0);
	else pm->Create(n0 + nseg*nn * 2, 0);   // nn/2 wasteful nodes

	// reallocating the nodes removes node IDs, so let's set those back
	for (int i = 0; i < n0; ++i) pm->Node(i).m_nid = nodeIDs[i];

	// we use the nodal position to store the extrusion direction
	vector<vec3d> ed; ed.assign(nn, vec3d(0, 0, 0));

	// find the extrusion directions
	for (int i=0; i <(int)faceList.size(); ++i)
	{
		FSFace& face = pm->Face(faceList[i]);
		int n = face.Nodes();
		for (int j = 0; j<n; ++j)
		{
			int n1 = pm->Node(face.n[j]).m_ntag;
			ed[n1] += to_vec3d(face.m_nn[j]);
		}
	}

	if (!bloc)
	{
		// calculate average normal
		vec3d n(0, 0, 0);
		for (int i = 0; i<nn; ++i) n += ed[i];
		for (int i = 0; i<nn; ++i) ed[i] = n;
	}

	// make sure all directional vectors are normalized
	for (int i = 0; i<nn; ++i) ed[i].Normalize();

	// apply nodal map
	if (m_map)
	{
		FSNodeSet* nset = dynamic_cast<FSNodeSet*>(m_map->GetItemList());
		if (nset == nullptr) return error("Invalid nodal map specified.");

		vector<int> tag(n0, -1);
		for (int i = 0; i < n0; ++i)
			if (pm->Node(i).m_ntag >= 0)
			{
				tag[i] = pm->Node(i).m_ntag;
			}

		if (m_map->GetDataType() == DATA_SCALAR)
		{
			auto it = nset->begin();
			for (int i = 0; i < nset->size(); ++i, ++it)
			{
				int m = tag[*it];
				if (m == -1) return error("Invalid nodal map specified.");

				double v = m_map->get(i);
				ed[m] *= v;
			}
		}
		else if (m_map->GetDataType() == DATA_VEC3)
		{
			auto it = nset->begin();
			for (int i = 0; i < nset->size(); ++i, ++it)
			{
				int m = tag[*it];
				if (m == -1) return error("Invalid nodal map specified.");

				vec3d v = m_map->getVec3d(i);
				ed[m] = v;
			}
		}
		else return error("Invalid nodal data map specified.");
	}

	double fd = gd;
	gd = 1;
	if (bd)
	{
		gd = 2; if (nseg % 2) gd += fd;
		for (int i = 0; i < nseg / 2 - 1; ++i) gd = fd * gd + 2;
		gd = dist / gd;
	}
	else
	{
		for (int i = 0; i < nseg - 1; ++i) gd = fd * gd + 1;
		gd = dist / gd;
	}

	// extrude the nodes
	double dd = gd;
	double d = 0;
	for (int l = 1; l <= nseg; ++l)
	{
		d += dd;
		dd *= fd;
		if (bd && ((l-1) == nseg / 2 - 1))
		{
			if (nseg % 2 == 0) dd /= fd;
			fd = 1.0 / fd;
		}

		for (int i = 0; i<n0; ++i)
		{
			FSNode& node = pm->Node(i);
			if (node.m_ntag >= 0)
			{
				FSNode& node2 = pm->Node(n0 + (l - 1)*nn + node.m_ntag);
				node2.r = node.r + ed[node.m_ntag] * d;
				node2.m_ntag = node.m_ntag;
			}
		}
	}

	// reassign nodal IDs to the top layer
	for (int i = 0; i < n0; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.m_ntag >= 0)
		{
			FSNode& node2 = pm->Node(n0 + (nseg - 1) * nn + node.m_ntag);
//			node2.m_nid = node.m_nid;
			node.m_nid = -1;
		}
	}

	// add mid-layer nodes for quadratic elements
	if (!linear) 
	{
		dd = gd;
		d = 0;
		for (int l = 1; l <= nseg; ++l)
		{
			for (int i = 0; i<n0; ++i)
			{
				FSNode& node = pm->Node(i);
				if (node.m_ntag >= 0)
				{
					FSNode& node2 = pm->Node(n0 + nseg*nn + (l - 1)*nn + node.m_ntag);
					node2.r = node.r + ed[node.m_ntag] * (d  + dd* 0.5);
					node2.m_ntag = node.m_ntag;
				}
			}

			d += dd;
			dd *= fd;
			if (bd && ((l - 1) == nseg / 2 - 1))
			{
				if (nseg % 2 == 0) dd /= fd;
				fd = 1.0 / fd;
			}
		}
	}

	// get the largest element group number
	int nid = 0;
	for (int i = 0; i<pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_gid > nid) nid = el.m_gid;
	}
	nid++;

	int ne0 = pm->Elements();

	// cache the element IDs for now.
	vector<int> elemIDs(ne0);
	for (int i = 0; i < ne0; ++i) elemIDs[i] = pm->Element(i).m_nid;

	// create new elements
	pm->Create(0, ne0 + nseg*ne1);

	// adding elements can reset their IDs so let's set them back
	for (int i = 0; i < ne0; ++i) pm->Element(i).m_nid = elemIDs[i];

	int n = ne0;
	for (int l = 1; l <= nseg; ++l)
	{
		for (int i = 0; i <(int)faceList.size(); ++i)
		{
			FSFace& face = pm->Face(faceList[i]);

			FSElement& el = pm->Element(n);

			if (face.Nodes() == 3)
			{
				el.SetType(FE_PENTA6);
				el.m_gid = nid;

				el.m_node[0] = face.n[0];
				el.m_node[1] = face.n[1];
				el.m_node[2] = face.n[2];

				el.m_node[3] = n0 + (l - 1)*nn + pm->Node(face.n[0]).m_ntag;
				el.m_node[4] = n0 + (l - 1)*nn + pm->Node(face.n[1]).m_ntag;
				el.m_node[5] = n0 + (l - 1)*nn + pm->Node(face.n[2]).m_ntag;

				// move the face
				face.n[0] = el.m_node[3];
				face.n[1] = el.m_node[4];
				face.n[2] = el.m_node[5];
				face.m_elem[0].eid = n;
				face.m_elem[1].eid = face.m_elem[2].eid = -1;

				++n;
			}
			else if (face.Nodes() == 4)
			{
				el.SetType(FE_HEX8);
				el.m_gid = nid;

				el.m_node[0] = face.n[0];
				el.m_node[1] = face.n[1];
				el.m_node[2] = face.n[2];
				el.m_node[3] = face.n[3];

				el.m_node[4] = n0 + (l - 1)*nn + pm->Node(face.n[0]).m_ntag;
				el.m_node[5] = n0 + (l - 1)*nn + pm->Node(face.n[1]).m_ntag;
				el.m_node[6] = n0 + (l - 1)*nn + pm->Node(face.n[2]).m_ntag;
				el.m_node[7] = n0 + (l - 1)*nn + pm->Node(face.n[3]).m_ntag;

				// move the face
				face.n[0] = el.m_node[4];
				face.n[1] = el.m_node[5];
				face.n[2] = el.m_node[6];
				face.n[3] = el.m_node[7];
				face.m_elem[0].eid = n;
				face.m_elem[1].eid = face.m_elem[2].eid = -1;

				++n;
			}
			else if (face.Nodes() == 6) {
				el.SetType(FE_PENTA15);
				el.m_gid = nid;

				el.m_node[0] = face.n[0];
				el.m_node[1] = face.n[1];
				el.m_node[2] = face.n[2];

				el.m_node[3] = n0 + (l - 1)*nn + pm->Node(face.n[0]).m_ntag;
				el.m_node[4] = n0 + (l - 1)*nn + pm->Node(face.n[1]).m_ntag;
				el.m_node[5] = n0 + (l - 1)*nn + pm->Node(face.n[2]).m_ntag;

				el.m_node[6] = face.n[3];
				el.m_node[7] = face.n[4];
				el.m_node[8] = face.n[5];

				el.m_node[9] = n0 + (l - 1)*nn + pm->Node(face.n[3]).m_ntag;
				el.m_node[10] = n0 + (l - 1)*nn + pm->Node(face.n[4]).m_ntag;
				el.m_node[11] = n0 + (l - 1)*nn + pm->Node(face.n[5]).m_ntag;

				el.m_node[12] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[0]).m_ntag;
				el.m_node[13] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[1]).m_ntag;
				el.m_node[14] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[2]).m_ntag;

				// move the face
				face.n[0] = el.m_node[3];
				face.n[1] = el.m_node[4];
				face.n[2] = el.m_node[5];
				face.n[3] = el.m_node[9];
				face.n[4] = el.m_node[10];
				face.n[5] = el.m_node[11];
				face.m_elem[0].eid = n;
				face.m_elem[1].eid = face.m_elem[2].eid = -1;
                
                ++n;
			}
			else if (face.Nodes() == 8) {
				el.SetType(FE_HEX20);
				el.m_gid = nid;

				el.m_node[0] = face.n[0];
				el.m_node[1] = face.n[1];
				el.m_node[2] = face.n[2];
				el.m_node[3] = face.n[3];

				el.m_node[4] = n0 + (l - 1)*nn + pm->Node(face.n[0]).m_ntag;
				el.m_node[5] = n0 + (l - 1)*nn + pm->Node(face.n[1]).m_ntag;
				el.m_node[6] = n0 + (l - 1)*nn + pm->Node(face.n[2]).m_ntag;
				el.m_node[7] = n0 + (l - 1)*nn + pm->Node(face.n[3]).m_ntag;

				el.m_node[8] = face.n[4];
				el.m_node[9] = face.n[5];
				el.m_node[10] = face.n[6];
				el.m_node[11] = face.n[7];

				el.m_node[12] = n0 + (l - 1)*nn + pm->Node(face.n[4]).m_ntag;
				el.m_node[13] = n0 + (l - 1)*nn + pm->Node(face.n[5]).m_ntag;
				el.m_node[14] = n0 + (l - 1)*nn + pm->Node(face.n[6]).m_ntag;
				el.m_node[15] = n0 + (l - 1)*nn + pm->Node(face.n[7]).m_ntag;

				el.m_node[16] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[0]).m_ntag;
				el.m_node[17] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[1]).m_ntag;
				el.m_node[18] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[2]).m_ntag;
				el.m_node[19] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[3]).m_ntag;

				// move the face
				face.n[0] = el.m_node[4];
				face.n[1] = el.m_node[5];
				face.n[2] = el.m_node[6];
				face.n[3] = el.m_node[7];
				face.n[4] = el.m_node[12];
				face.n[5] = el.m_node[13];
				face.n[6] = el.m_node[14];
				face.n[7] = el.m_node[15];
				face.m_elem[0].eid = n;
				face.m_elem[1].eid = face.m_elem[2].eid = -1;
                
                ++n;
			}
			else if (face.Nodes() == 9) {
				el.SetType(FE_HEX27);
				el.m_gid = nid;

				el.m_node[0] = face.n[0];
				el.m_node[1] = face.n[1];
				el.m_node[2] = face.n[2];
				el.m_node[3] = face.n[3];

				el.m_node[4] = n0 + (l - 1)*nn + pm->Node(face.n[0]).m_ntag;
				el.m_node[5] = n0 + (l - 1)*nn + pm->Node(face.n[1]).m_ntag;
				el.m_node[6] = n0 + (l - 1)*nn + pm->Node(face.n[2]).m_ntag;
				el.m_node[7] = n0 + (l - 1)*nn + pm->Node(face.n[3]).m_ntag;

				el.m_node[8] = face.n[4];
				el.m_node[9] = face.n[5];
				el.m_node[10] = face.n[6];
				el.m_node[11] = face.n[7];

				el.m_node[12] = n0 + (l - 1)*nn + pm->Node(face.n[4]).m_ntag;
				el.m_node[13] = n0 + (l - 1)*nn + pm->Node(face.n[5]).m_ntag;
				el.m_node[14] = n0 + (l - 1)*nn + pm->Node(face.n[6]).m_ntag;
				el.m_node[15] = n0 + (l - 1)*nn + pm->Node(face.n[7]).m_ntag;

				el.m_node[16] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[0]).m_ntag;
				el.m_node[17] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[1]).m_ntag;
				el.m_node[18] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[2]).m_ntag;
				el.m_node[19] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[3]).m_ntag;

				el.m_node[20] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[4]).m_ntag;
				el.m_node[21] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[5]).m_ntag;
				el.m_node[22] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[6]).m_ntag;
				el.m_node[23] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[7]).m_ntag;

				el.m_node[24] = face.n[8];
				el.m_node[25] = n0 + (l - 1)*nn + pm->Node(face.n[8]).m_ntag;
				el.m_node[26] = n0 + nseg*nn + (l - 1)*nn + pm->Node(face.n[8]).m_ntag;

				// move the face
				face.n[0] = el.m_node[4];
				face.n[1] = el.m_node[5];
				face.n[2] = el.m_node[6];
				face.n[3] = el.m_node[7];
				face.n[4] = el.m_node[12];
				face.n[5] = el.m_node[13];
				face.n[6] = el.m_node[14];
				face.n[7] = el.m_node[15];
				face.n[8] = el.m_node[25];
				face.m_elem[0].eid = n;
				face.m_elem[1].eid = face.m_elem[2].eid = -1;
                
                ++n;
			}
		}
	}

	// rebuilding the mesh is going to scramble the nodal (geometry) IDs. 
	// let's keep track of them so we can restore them later
	int np = pm->CountNodePartitions();
	vector<int> gid(pm->Nodes(), -1);
	for (int i = 0; i < n0; ++i)
	{
		FSNode& node = pm->Node(i);
		gid[i] = node.m_gid;
	}

	// rebuild the object
	pm->RebuildMesh();

	// let's restore GIDs
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		if ((gid[i] == -1) && (node.m_gid >= 0))
		{
			// this is a newly partitioned node
			node.m_gid = np++;
		}
		else if (gid[i] >= 0)
		{
			node.m_gid = gid[i];
		}
	}

	// reselect the new faces
	pm->ClearFaceSelection();
	for (int i=0; i<n0; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.m_ntag >= 0)
		{
			FSNode& nj = pm->Node(n0 + (nseg - 1)*nn + node.m_ntag);
			nj.m_ntag = -2;
		}
	}
	for (int i=0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		int nf = face.Nodes();
		bool bsel = true;
		for (int j=0; j<nf; ++j)
		{
			if (pm->Node(face.n[j]).m_ntag != -2)
			{
				bsel = false;
				break;
			}
		}
		if (bsel) face.Select();
	}

	// delete extraneous nodes (if any)
	FSMeshBuilder meshBuilder(*pm);
	meshBuilder.RemoveIsolatedNodes();

	return true;
}
