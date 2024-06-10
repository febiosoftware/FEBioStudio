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
#include "FESplitModifier.h"
#include <MeshLib/FEFaceEdgeList.h>
using namespace std;

FEHexSplitModifier::FEHexSplitModifier() : FEModifier("Split")
{
	m_smoothSurface = false;
}

void FEHexSplitModifier::DoSurfaceSmoothing(bool b)
{
	m_smoothSurface = b;
}

FSMesh* FEHexSplitModifier::Apply(FSMesh* pm)
{
	// count the selected elements
	int nsel = pm->CountSelectedElements();
	if (nsel == 0) return RefineMesh(pm);
	else return RefineSelection(pm);
}

FSMesh* FEHexSplitModifier::RefineMesh(FSMesh* pm)
{
	if (pm->IsType(FE_HEX8) == false) return nullptr;

	// build the edge table of the mesh (each edge will add a node)
	FSEdgeList ET(*pm);
	FSElementEdgeList EET(*pm, ET);

	// build the face table (each face will add a node)
	FSFaceTable FT(*pm);
	FSElementFaceList EFL(*pm, FT);

	// get the mesh item counts
	int NN0 = pm->Nodes();
	int NC0 = ET.size();
	int NF0 = FT.size();
	int NE0 = pm->Elements();

	// each node, edge, face, and element will create a new node
	int NN1 = NN0 + NC0 + NF0 + NE0;

	// each element will be split in eight
	int NE1 = 8*NE0;

	// create new mesh
	FSMesh* pmnew = new FSMesh;
	pmnew->Create(NN1, NE1);

	// build face-edge table
	FSFaceEdgeList FET(*pm, ET);

	// assign nodes
	int n = 0;
	for (int i=0; i<NN0; ++i, ++n)
	{
		FSNode& n1 = pmnew->Node(n);
		FSNode& n0 = pm->Node(i);
		n1 = n0;
	}

	for (int i=0; i<NC0; ++i, ++n)
	{
		FSNode& n1 = pmnew->Node(n);

		pair<int,int>& edge = ET[i];
		FSNode& na = pm->Node(edge.first);
		FSNode& nb = pm->Node(edge.second);

		n1.r = (na.r + nb.r)*0.5;
	}

	for (int i=0; i<NF0; ++i, ++n)
	{
		FSNode& n1 = pmnew->Node(n);

		FSFace& face = FT[i];

		vec3d r0 = pm->Node(face.n[0]).r;
		vec3d r1 = pm->Node(face.n[1]).r;
		vec3d r2 = pm->Node(face.n[2]).r;
		vec3d r3 = pm->Node(face.n[3]).r;

		n1.r = (r0 + r1 + r2 + r3)*0.25;
	}

	for (int i=0; i<NE0; ++i, ++n)
	{
		FSNode& n1 = pmnew->Node(n);

		FSElement& el = pm->Element(i);
		vec3d r(0,0,0);
		for (int j=0; j<8; ++j) r += pm->Node(el.m_node[j]).r;
		r *= 0.125;

		n1.r = r;
	}

	const int LUT[8][8] = {
	{  0,  8, 24, 11, 16, 20, 26, 23},
	{  8,  1,  9, 24, 20, 17, 21, 26 },
	{ 11, 24, 10,  3, 23, 26, 22, 19 },
	{ 24,  9,  2, 10, 26, 21, 18, 22 },
	{ 16, 20, 26, 23,  4, 12, 25, 15 },
	{ 20, 17, 21, 26, 12,  5, 13, 25 },
	{ 23, 26, 22, 19, 15, 25, 14,  7 },
	{ 26, 21, 18, 22, 25, 13,  6, 14 }};

	// create new elements
	int m[27];
	for (int i=0; i<NE0; ++i)
	{
		FSElement& el0 = pm->Element(i);
		vector<int>& eel = EET[i];
		vector<int>& fel = EFL[i];

		for (int j=0; j< 8; ++j) m[     j] = el0.m_node[j];
		for (int j=0; j<12; ++j) m[ 8 + j] = NN0 + eel[j];
		for (int j=0; j< 6; ++j) m[20 + j] = NN0 + NC0 + fel[j];
		m[26] = NN0 + NC0 + NF0 + i;

		for (int j=0; j<8; ++j)
		{
			FSElement& el = pmnew->Element(i*8 + j);
			el.m_gid = el0.m_gid;
			el.SetType(FE_HEX8);

			for (int k=0; k<8; ++k) el.m_node[k] = m[LUT[j][k]];
		}
	}

	pmnew->RebuildMesh();

	if (m_smoothSurface)
	{
		// The face table lists all faces (including all internal ones). We only need
		// to loop over exterior faces, so we need to figure out which exterior face
		// corresponds to the face in the face table
		int NF = pm->Faces();
		for (int i=0; i<NF; ++i) 
		{
			FSFace& face = pm->Face(i);
			face.m_ntag = -1;

			for (int j=0; j<NF0; ++j)
			{
				FSFace& fj = FT[j];
				if (fj == face)
				{
					face.m_ntag = j;
					break;
				}
			}

			assert(face.m_ntag != -1);
		}

		for (int i=0; i<pmnew->Nodes(); ++i) pmnew->Node(i).m_ntag = 0;
		vector<vec3d> p(NN0, vec3d(0,0,0));
		for (int i=0; i<NF; ++i)
		{
			FSFace& face = pm->Face(i);
			int ne = face.Edges();
			for (int j=0; j<ne; ++j)
			{
				pmnew->Node(face.n[j]).m_ntag++;
				
				int ej = FET[i][j];
				pair<int, int>& e = ET[ej];
				pmnew->Node(NN0 + ej).m_ntag++;

				p[e.first ] += pmnew->Node(NN0 + ej).r;
				p[e.second] += pmnew->Node(NN0 + ej).r;

				p[face.n[j]] += pmnew->Node(NN0 + NC0 + face.m_ntag).r;
			}
		}

		for (int i=0; i<NN0; ++i)
		{
			FSNode& ni = pmnew->Node(i);
			double m = (double) ni.m_ntag;
			if (m != 0.0)
			{
				ni.r = (p[i]/m + ni.r*(m - 3))/m;
			}
		}

		for (int i = 0; i<NF; ++i)
		{
			FSFace& face = pm->Face(i);
			int ne = face.Edges();
			for (int j = 0; j<ne; ++j)
			{
				int ej = FET[i][j];
				pmnew->Node(NN0 + ej).r += pmnew->Node(NN0 + NC0 + face.m_ntag).r*0.5;
			}
		}

		for (int i=0; i<NC0; ++i) 
		{
			FSNode& node = pmnew->Node(NN0 + i);
			if (node.m_ntag != 0)
			{
				pmnew->Node(NN0 + i).r *= 0.5;
			}
		}

		pmnew->UpdateNormals();
	}

	return pmnew;
}

FSMesh* FEHexSplitModifier::RefineSelection(FSMesh* pm)
{
	// make sure all selected elements are connected
	size_t NN0 = pm->Nodes();
	size_t NE0 = pm->Elements();
	size_t NF0 = pm->Faces();
	size_t NC0 = pm->Edges();
	size_t splitElems = 0;
	pm->TagAllNodes(0);
	pm->TagAllFaces(0);
	pm->TagAllEdges(0);
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsSelected())
		{
			if (!el.IsType(FE_HEX8))
			{
				SetError("Invalid selection.");
				return nullptr;
			}

			el.m_ntag = 1;
			splitElems++;
			for (int j = 0; j < 6; ++j)
			{
				FEElement_* elj = pm->ElementPtr(el.m_nbr[j]);
				if (elj && (!elj->IsSelected()))
				{
					SetError("Invalid selection.");
					return nullptr;
				}
			}

			for (int j = 0; j < 8; ++j) pm->Node(el.m_node[j]).m_ntag = 1;

			for (int j = 0; j < 6; ++j)
			{
				if (el.m_face[j] >= 0) pm->Face(el.m_face[j]).m_ntag = 1;
			}
		}
		else el.m_ntag = 0;
	}

	// build the edge table of the mesh
	// and count how many edges will be split
	FSEdgeList ET(*pm);
	vector<int> edgeTag(ET.size(), 0);
	FSElementEdgeList EET(*pm, ET);
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == 1)
		{
			int ne = EET.Valence(i);
			for (int j = 0; j < ne; ++j) edgeTag[EET.EdgeIndex(i, j)] = 1;
		}
	}
	size_t splitEdges = 0;
	for (int n : edgeTag) splitEdges += n;

	// count how many quads to split
	FSFaceEdgeList FET(*pm, ET);
	size_t splitQuads = 0;
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.m_ntag == 1)
		{
			splitQuads++;
		}
	}

	// count how many edges to split
	size_t splitLines = 0;
	for (int i = 0; i < pm->Edges(); ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if ((pm->Node(edge.n[0]).m_ntag) && (pm->Node(edge.n[1]).m_ntag))
		{
			edge.m_ntag = 1;
			splitLines++;
		}
	}
	FSEdgeIndexList EIL(*pm, ET);

	// build the face table
	FSFaceTable FT(*pm);
	vector<int> faceTag(FT.size(), 0);
	FSElementFaceList EFL(*pm, FT);
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == 1)
		{
			int nf = EFL.Valence(i);
			for (int j = 0; j < nf; ++j) faceTag[EFL.FaceIndex(i, j)] = 1;
		}
	}
	size_t splitFaces = 0;
	for (int n : faceTag) splitFaces += n;
	FSFaceFaceList FFL(*pm, FT);


	// each node and all tagged edges, faces, and elements will create a new node
	int NN1 = NN0 + splitEdges + splitFaces + splitElems;

	// each element will be split in eight
	int NE1 = (NE0 - splitElems) +  8 * splitElems;

	// each face will be split in four
	int NF1 = (NF0 - splitQuads) + 4 * splitQuads;

	// each edge will be split in 2
	int NC1 = (NC0 - splitLines) + 2 * splitLines;

	// create new mesh
	FSMesh* pmnew = new FSMesh;
	pmnew->Create(NN1, NE1, NF1, NC1);

	// assign nodes
	int n = 0;
	for (int i = 0; i < NN0; ++i)
	{
		FSNode& n1 = pmnew->Node(n++);
		FSNode& n0 = pm->Node(i);
		n1 = n0;
	}

	for (int i = 0; i < edgeTag.size(); ++i)
	{
		if (edgeTag[i] == 1)
		{
			edgeTag[i] = n;
			FSNode& n1 = pmnew->Node(n++);

			pair<int, int>& edge = ET[i];
			FSNode& na = pm->Node(edge.first);
			FSNode& nb = pm->Node(edge.second);

			n1.r = (na.r + nb.r) * 0.5;
		}
		else edgeTag[i] = -1;
	}

	for (int i = 0; i < faceTag.size(); ++i)
	{
		if (faceTag[i] == 1)
		{
			faceTag[i] = n;
			FSNode& n1 = pmnew->Node(n++);

			FSFace& face = FT[i];

			vec3d r0 = pm->Node(face.n[0]).r;
			vec3d r1 = pm->Node(face.n[1]).r;
			vec3d r2 = pm->Node(face.n[2]).r;
			vec3d r3 = pm->Node(face.n[3]).r;

			n1.r = (r0 + r1 + r2 + r3) * 0.25;
		}
		else faceTag[i] = -1;
	}

	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == 1)
		{
			el.m_ntag = n;
			FSNode& n1 = pmnew->Node(n++);

			FSElement& el = pm->Element(i);
			vec3d r(0, 0, 0);
			for (int j = 0; j < 8; ++j) r += pm->Node(el.m_node[j]).r;
			r *= 0.125;

			n1.r = r;
		}
		else el.m_ntag = -1;
	}

	const int LUT[8][8] = {
	{  0,  8, 24, 11, 16, 20, 26, 23},
	{  8,  1,  9, 24, 20, 17, 21, 26 },
	{ 11, 24, 10,  3, 23, 26, 22, 19 },
	{ 24,  9,  2, 10, 26, 21, 18, 22 },
	{ 16, 20, 26, 23,  4, 12, 25, 15 },
	{ 20, 17, 21, 26, 12,  5, 13, 25 },
	{ 23, 26, 22, 19, 15, 25, 14,  7 },
	{ 26, 21, 18, 22, 25, 13,  6, 14 } };

	const int FLUT[4][4] = {
	{ 0, 4, 8, 7 },
	{ 4, 1, 5, 8 },
	{ 8, 5, 2, 6 },
	{ 7, 8, 6, 3 }};

	const int ELUT[2][2] = {
	{ 0, 2 },
	{ 2, 1 } };

	// create new elements
	int m[27];
	n = 0;
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el0 = pm->Element(i);
		if (el0.m_ntag < 0)
		{
			FSElement& el = pmnew->Element(n++);
			el.SetType(FE_HEX8);
			el = el0;
		}
		else
		{
			vector<int>& eel = EET[i];
			vector<int>& fel = EFL[i];

			for (int j = 0; j <  8; ++j) m[     j] = el0.m_node[j];
			for (int j = 0; j < 12; ++j) m[ 8 + j] = edgeTag[eel[j]];
			for (int j = 0; j <  6; ++j) m[20 + j] = faceTag[fel[j]];
			m[26] = el0.m_ntag;

			for (int j = 0; j < 8; ++j)
			{
				FSElement& el = pmnew->Element(n++);
				el.m_gid = el0.m_gid;
				el.SetType(FE_HEX8);

				for (int k = 0; k < 8; ++k) el.m_node[k] = m[LUT[j][k]];
			}
		}
	}

	// create new faces
	n = 0;
	for (int i = 0; i < NF0; ++i)
	{
		FSFace& face0 = pm->Face(i);
		if (face0.m_ntag == 0)
		{
			FSFace& face1 = pmnew->Face(n++);
			face1.SetType(FE_FACE_QUAD4);
			face1 = face0;
		}
		else
		{
			vector<int>& fel = FET[i];
			int ffl = FFL[i];

			for (int j = 0; j < 4; ++j) m[j] = face0.n[j];
			for (int j = 0; j < 4; ++j) m[4 + j] = edgeTag[fel[j]];
			m[8] = faceTag[ffl];

			for (int j = 0; j < 4; ++j)
			{
				FSFace& facej = pmnew->Face(n++);
				facej.SetType(FE_FACE_QUAD4);
				facej.m_gid = face0.m_gid;
				facej.m_sid = face0.m_sid;
				for (int k = 0; k < 4; ++k) facej.n[k] = m[FLUT[j][k]];
			}
		}
	}

	// create new edges
	n = 0;
	for (int i = 0; i < NC0; ++i)
	{
		FSEdge& edge0 = pm->Edge(i);
		if (edge0.m_ntag == 0)
		{
			FSEdge& edge1 = pmnew->Edge(n++);
			edge1.SetType(FE_EDGE2);
			edge1 = edge0;
		}
		else
		{
			int eel = EIL[i];
			for (int j = 0; j < 2; ++j) m[j] = edge0.n[j];
			m[2] = edgeTag[eel];

			for (int j = 0; j < 2; ++j)
			{
				FSEdge& edgej = pmnew->Edge(n++);
				edgej.SetType(FE_EDGE2);
				edgej.m_gid = edge0.m_gid;
				edgej.n[0] = m[ELUT[j][0]];
				edgej.n[1] = m[ELUT[j][1]];
			}
		}
	}

	pmnew->BuildMesh();

	// TODO: smooth surface

	return pmnew;
}

//=======================================================================================

FEHex2DSplitModifier::FEHex2DSplitModifier() : FEModifier("Split2D")
{
	m_smoothSurface = false;
}

void FEHex2DSplitModifier::DoSurfaceSmoothing(bool b)
{
	m_smoothSurface = b;
}

FSMesh* FEHex2DSplitModifier::Apply(FSMesh* pm)
{
	// make sure we are dealing with a hex mesh
	if (pm->IsType(FE_HEX8) == false) return 0;

	// we must also check if this is a valid 2D (i.e. single layer) hex mesh.
	// We do this by checking that each element has no neighbor for their top
	// and bottom face (which is assumed to be face 4 and 5).
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if ((el.m_nbr[4] != -1) || (el.m_nbr[5] != -1)) return nullptr;
	}

	// tag all faces that will be split
	pm->TagAllFaces(-1);
	int taggedFaces = 0;
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		el.m_ntag = i;
		assert((el.m_face[4] != -1) && (el.m_face[5] != -1));
		if (el.m_face[4] == -1) return nullptr; else { pm->Face(el.m_face[4]).m_ntag = taggedFaces++;}
		if (el.m_face[5] == -1) return nullptr; else { pm->Face(el.m_face[5]).m_ntag = taggedFaces++;}
	}
	assert(taggedFaces == 2 * pm->Elements());

	// count all edges that will be split
	int edgeCount = 0;
	vector< pair<int, int> > edges;
	vector< vector<int> > EEL(pm->Elements(), vector<int>(8, -1));
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		for (int k=0; k<=1; ++k)
		{
			for (int j = 0; j < 4; ++j)
			{
				int n0 = el.m_node[k*4 + j];
				int n1 = el.m_node[k*4 + (j+1)%4];
				if (el.m_nbr[j] == -1)
				{
					pair<int, int> edge;
					edge.first = n0;
					edge.second = n1;
					edges.push_back(edge);
					EEL[i][k * 4 + j] = edgeCount++;
				}
				else
				{
					int nej = el.m_nbr[j];
					FSElement& ej = pm->Element(nej);
					if (el.m_ntag < ej.m_ntag)
					{
						pair<int, int> edge;
						edge.first = n0;
						edge.second = n1;
						edges.push_back(edge);
						EEL[i][k * 4 + j] = edgeCount++;
					}
					else
					{
						int eid = -1;
						for (int l = 0; l < 4; ++l)
						{
							int m0 = ej.m_node[4 * k + l];
							int m1 = ej.m_node[4 * k + (l+1)%4];

							if (((n0 == m0) && (n1 == m1)) || ((n0 == m1) && (n1 == m0)))
							{
								eid = l + k*4;
								break;
							}
						}
						assert(eid >= 0);

						int edgeId = EEL[nej][eid];
						EEL[i][k * 4 + j] = edgeId;
					}
				}
			}
		}
	}
	assert(edgeCount == (int)edges.size());

	// get the mesh item counts
	int NN0 = pm->Nodes();
	int NE0 = pm->Elements();

	// each node, edge, face, and element will create a new node
	int NN1 = NN0 + edgeCount + taggedFaces;

	// each element will be split in four
	int NE1 = 4 * NE0;

	// create new mesh
	FSMesh* pmnew = new FSMesh;
	pmnew->Create(NN1, NE1);

	// assign nodes
	int n = 0;
	for (int i = 0; i < NN0; ++i, ++n)
	{
		FSNode& n1 = pmnew->Node(n);
		FSNode& n0 = pm->Node(i);
		n1 = n0;
	}

	for (int i = 0; i < edgeCount; ++i, ++n)
	{
		FSNode& n1 = pmnew->Node(n);

		pair<int, int>& edge = edges[i];
		FSNode& na = pm->Node(edge.first);
		FSNode& nb = pm->Node(edge.second);

		n1.r = (na.r + nb.r)*0.5;
	}

	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.m_ntag != -1)
		{
			FSNode& n1 = pmnew->Node(n++);

			vec3d r0 = pm->Node(face.n[0]).r;
			vec3d r1 = pm->Node(face.n[1]).r;
			vec3d r2 = pm->Node(face.n[2]).r;
			vec3d r3 = pm->Node(face.n[3]).r;

			n1.r = (r0 + r1 + r2 + r3)*0.25;
		}
	}

	const int LUT[4][8] = {
	{  0,  8, 16, 11,  4, 12, 17, 15 },
	{  8,  1,  9, 16, 12,  5, 13, 17 },
	{ 11, 16, 10,  3, 15, 17, 14,  7 },
	{ 16,  9,  2, 10, 17, 13,  6, 14 }};

	// create new elements
	int m[18];
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el0 = pm->Element(i);
		vector<int>& eel = EEL[i];

		for (int j = 0; j < 8; ++j) m[j] = el0.m_node[j];
		for (int j = 0; j < 8; ++j) m[8 + j] = NN0 + eel[j];
		for (int j = 0; j < 2; ++j) m[16 + j] = NN0 + edgeCount + 2*i + j;

		for (int j = 0; j < 4; ++j)
		{
			FSElement& el = pmnew->Element(i * 4 + j);
			el.m_gid = el0.m_gid;
			el.SetType(FE_HEX8);

			for (int k = 0; k < 8; ++k) el.m_node[k] = m[LUT[j][k]];
		}
	}

	pmnew->RebuildMesh();
/*
	if (m_smoothSurface)
	{
		// The face table lists all faces (including all internal ones). We only need
		// to loop over exterior faces, so we need to figure out which exterior face
		// corresponds to the face in the face table
		int NF = pm->Faces();
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = pm->Face(i);
			face.m_ntag = -1;

			for (int j = 0; j < NF0; ++j)
			{
				FSFace& fj = FT[j];
				if (fj == face)
				{
					face.m_ntag = j;
					break;
				}
			}

			assert(face.m_ntag != -1);
		}

		for (int i = 0; i < pmnew->Nodes(); ++i) pmnew->Node(i).m_ntag = 0;
		vector<vec3d> p(NN0, vec3d(0, 0, 0));
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = pm->Face(i);
			int ne = face.Edges();
			for (int j = 0; j < ne; ++j)
			{
				pmnew->Node(face.n[j]).m_ntag++;

				int ej = FET[i][j];
				pair<int, int>& e = ET[ej];
				pmnew->Node(NN0 + ej).m_ntag++;

				p[e.first] += pmnew->Node(NN0 + ej).r;
				p[e.second] += pmnew->Node(NN0 + ej).r;

				p[face.n[j]] += pmnew->Node(NN0 + NC0 + face.m_ntag).r;
			}
		}

		for (int i = 0; i < NN0; ++i)
		{
			FSNode& ni = pmnew->Node(i);
			double m = (double)ni.m_ntag;
			if (m != 0.0)
			{
				ni.r = (p[i] / m + ni.r*(m - 3)) / m;
			}
		}

		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = pm->Face(i);
			int ne = face.Edges();
			for (int j = 0; j < ne; ++j)
			{
				int ej = FET[i][j];
				pmnew->Node(NN0 + ej).r += pmnew->Node(NN0 + NC0 + face.m_ntag).r*0.5;
			}
		}

		for (int i = 0; i < NC0; ++i)
		{
			FSNode& node = pmnew->Node(NN0 + i);
			if (node.m_ntag != 0)
			{
				pmnew->Node(NN0 + i).r *= 0.5;
			}
		}

		pmnew->UpdateNormals();
	}
*/
	return pmnew;
}
