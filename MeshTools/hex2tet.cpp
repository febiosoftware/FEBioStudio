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
#include <MeshLib/FSMesh.h>
#include "FEModifier.h"
//using namespace std;

//-----------------------------------------------------------------------------
// lookup table into HT:
// - first index = start index into HT
// - second index = configurations for corresponding case

const int HTC[8][2] = {
	{0, 1}, {1, 2}, {3, 2}, {5, 1}, {6, 2}, {8, 1}, {9, 1}, {10, 1}
	};

const int HT[11][6][4] = {
	// case 0
	{ { 0, 1, 2, 5},{ 0, 2, 7, 5},{ 0, 2, 3, 7},{ 0, 5, 7, 4},{ 2, 7, 5, 6},{-1,-1,-1,-1} },

	// case 1 - 2 configs
	{ { 0, 5, 7, 4},{ 0, 7, 2, 3},{ 0, 1, 6, 5},{ 0, 6, 7, 5},{ 0, 7, 6, 2},{ 1, 0, 6, 2} },
	{ { 0, 5, 7, 4},{ 0, 7, 2, 3},{ 1, 5, 7, 0},{ 0, 1, 2, 7},{ 1, 6, 7, 5},{ 2, 6, 7, 1} },

	// case 2 - 2 configs
	{ { 0, 5, 7, 4},{0, 1, 2, 5},{0, 5, 6, 7},{0, 2, 6, 5},{0, 6, 3, 7},{0, 2, 3, 6} },
	{ { 0, 5, 7, 4},{0, 1, 2, 5},{0, 2, 3, 5},{2, 6, 3, 5},{0, 5, 3, 7},{3, 6, 7, 5} },

	// case 3
	{ {0, 5, 7, 4},{0, 5, 6, 7},{0, 6, 3, 7},{0, 1, 6, 5},{0, 1, 2, 6},{0, 2, 3, 6} },

	// case 4 - 2 configs
	{ {0, 1, 2, 5},{0, 2, 3, 7},{0, 2, 6, 5},{0, 5, 6, 4},{0, 6, 7, 4},{0, 6, 2, 7} },
	{ {0, 1, 2, 5},{0, 2, 3, 7},{0, 2, 4, 5},{2, 4, 5, 6},{0, 4, 2, 7},{2, 4, 6, 7} },

	// case 5
	{ {0, 2, 3, 7},{0, 5, 6, 4},{0, 1, 2, 6},{0, 1, 6, 5},{4, 7, 6, 0},{0, 7, 6, 2} },

	// case 6
	{ {0, 1, 2, 5},{0, 2, 6, 5},{0, 6, 2, 3},{0, 6, 7, 4},{0, 6, 3, 7},{0, 5, 6, 4} },

	// case 7
	{ {0, 1, 6, 5},{0, 6, 1, 2},{0, 5, 6, 4},{0, 6, 7, 4},{0, 6, 2, 3},{0, 6, 3, 7} }
};

//-----------------------------------------------------------------------------
// This algorithm converts a hex mesh into a tet mesh by dividing all hex
// elements into six tet elements. 
FSMesh* FEHex2Tet::Apply(FSMesh* pm)
{
	// get the mesh metrics
	int NN0 = pm->Nodes();
	int NE0 = pm->Elements();
	int NF0 = pm->Faces();

	// Make sure this mesh only has hexes and elements that are not connected to hexes
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);

		// make sure this is a solid
		if (el.IsSolid() == false) return nullptr;

		// make sure any non-hex element is not connected to a hex
		if (el.Type() != FE_HEX8)
		{
			// make sure this is not connected to a hex
			for (int j = 0; j < el.Faces(); ++j)
			{
				FSElement_* pne = pm->ElementPtr(el.m_nbr[j]);
				if ((pne != nullptr) && (pne->Type() == FE_HEX8)) return nullptr;
			}
		}
	}

	// create the new mesh
	FSMesh* mesh = new FSMesh;

	// allocate nodes and edges, since they are the same
	mesh->Create(NN0, 0);

	// copy nodes
	for (int i = 0; i < NN0; ++i)
	{
		mesh->Node(i) = pm->Node(i);
	}

	// tag all the faces to split
	pm->TagAllFaces(0);

	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsType(FE_HEX8))
		{
			for (int j = 0; j < 6; ++j)
			{
				int fj = el.m_face[j];
				if (fj >= 0) pm->Face(fj).m_ntag = 1;
			}
		}
	}

	// each quad will be split in two
	int NF1 = 0;
	for (int i = 0; i < NF0; ++i)
	{
		FSFace& fs = pm->Face(i);
		if (fs.m_ntag == 1)
		{
			assert(fs.Type() == FE_FACE_QUAD4);
			NF1 += 2;
		}
		else NF1++;
	}

	int fc = 0;
	mesh->Create(0, 0, NF1);
	for (int i = 0; i < NF0; ++i)
	{
		FSFace& fs = pm->Face(i);

		if (fs.m_ntag == 0)
		{
			FSFace& f0 = mesh->Face(fc++);
			f0 = fs;
		}
		else
		{
			// find the lowest vertex index
			int l = 0;
			for (int j = 1; j < 4; ++j)
			{
				if (fs.n[j] < fs.n[l]) l = j;
			}

			// split at this lowest vertex
			FSFace& f0 = mesh->Face(fc++);
			f0.m_gid = fs.m_gid;
			f0.SetType(FE_FACE_TRI3);
			f0.n[0] = fs.n[(l + 0) % 4];
			f0.n[1] = fs.n[(l + 1) % 4];
			f0.n[2] = fs.n[(l + 2) % 4];

			FSFace& f1 = mesh->Face(fc++);
			f1.SetType(FE_FACE_TRI3);
			f1.m_gid = fs.m_gid;
			f1.n[0] = fs.n[(l + 2) % 4];
			f1.n[1] = fs.n[(l + 3) % 4];
			f1.n[2] = fs.n[(l + 0) % 4];
		}
	}

	// node look-up table after rotating cube to have lowest vertex at pivot
	int LT[8][8] = {
		{ 0, 1, 2, 3, 4, 5, 6, 7},
		{ 1, 0, 4, 5, 2, 3, 7, 6 },
		{ 2, 1, 5, 6, 3, 0, 4, 7 },
		{ 3, 0, 1, 2, 7, 4, 5, 6 },
		{ 4, 0, 3, 7, 5, 1, 2, 6 },
		{ 5, 1, 0, 4, 6, 2, 3, 7 },
		{ 6, 2, 1, 5, 7, 3, 0, 4 },
		{ 7, 3, 2, 6, 4, 0, 1, 5 }
	};

	// The six element faces
	// Note that the first three faces all start at node 0, and
	// these faces will always be split by the diagonal that goes through node 0
	int FT[6][4] = {
		{ 0, 3, 2, 1 },
		{ 0, 1, 5, 4 },
		{ 0, 4, 7, 3 },
		{ 1, 2, 6, 5 },
		{ 2, 3, 7, 6 },
		{ 4, 5, 6, 7 }
	};

	// Figure out how to split each element
	int NE1 = 0;
	std::vector<int> tag(NE0, -1);
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& els = pm->Element(i);

		if (els.Type() == FE_HEX8)
		{
			int* n = els.m_node;

			// find the lowest element vertex
			int l = 0;
			for (int j = 1; j < 8; ++j)
			{
				if (n[j] < n[l]) l = j;
			}
			int* L = LT[l];

			// calculate the case by inspecting the last 3 faces
			// (we already know how to split the first three faces)
			int ncase = 0;
			for (int j = 3; j < 6; ++j)
			{
				int* F = FT[j];

				// get the lowest index for this face
				int l0 = 0;
				for (int k = 1; k < 4; ++k)
				{
					if (n[L[F[k]]] < n[L[F[l0]]]) l0 = k;
				}
				int l1 = (l0 + 2) % 4;

				// if the diagonal (l0, l1) constains node 6, we tag it
				if ((F[l0] == 6) || (F[l1] == 6)) ncase |= (1 << (j - 3));
			}

			// case 0 gives us five tets, all other cases give 6
			if (ncase == 0) NE1 += 5; else NE1 += 6;
			tag[i] = ncase;
		}
		else NE1++;
	}

	// allocate the new elements
	mesh->Create(0, NE1);
	int ec = 0;
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& els = pm->Element(i);
		int* n = els.m_node;

		if (els.Type() != FE_HEX8)
		{
			assert(tag[i] == -1);
			FSElement& eld = mesh->Element(ec++);
			eld = els;
		}
		else
		{
			// find the lowest element vertex
			int l = 0;
			for (int j = 0; j < 8; ++j)
			{
				if (n[j] < n[l]) l = j;
			}
			int* L = LT[l];

			// calculate the case by inspecting the last 3 faces
			// (we already know how to split the first three faces)
			int ncase = tag[i];

			// get the number of configurations and the start index
			// TODO: for cases with multiple configs, pick the one with the best tet qualities
			int nc = 1;// HTC[ncase][1];
			int c0 = HTC[ncase][0];

			// get the tets
			for (int k = 0; k < nc; ++k)
			{
				const int(*f)[4] = HT[c0 + k];
				int nt = (ncase == 0 ? 5 : 6);
				for (int j = 0; j < nt; ++j)
				{
					FSElement& tet = mesh->Element(ec++);
					tet.SetType(FE_TET4);
					tet.m_gid = els.m_gid;

					tet.m_node[0] = n[L[f[j][0]]];
					tet.m_node[1] = n[L[f[j][1]]];
					tet.m_node[2] = n[L[f[j][2]]];
					tet.m_node[3] = n[L[f[j][3]]];
				}
			}
		}
	}

	mesh->BuildMesh();

	return mesh;
}
