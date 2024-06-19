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

#include "FEFaceEdgeList.h"
#include "FEMeshBase.h"
#include "FEMesh.h"
#include "FESurfaceMesh.h"
#include <assert.h>
using namespace std;

FSNodeNodeTable::FSNodeNodeTable(const FSSurfaceMesh& mesh)
{
	// reset node-node table
	int NN = mesh.Nodes();
	NNT.resize(NN);
	for (int i = 0; i<NN; ++i) NNT[i].clear();

	// loop over all faces
	int NF = mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		const FSFace& face = mesh.Face(i);
		int nf = face.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			int nj = face.n[j];
			for (int k = 0; k<nf; ++k)
			{
				int nk = face.n[k];
				if (nj != nk) NNT[nj].insert(nk);
			}
		}
	}
}

FSNodeNodeTable::FSNodeNodeTable(const FSMesh& mesh, bool surfOnly)
{
	// reset node-node table
	int NN = mesh.Nodes();
	NNT.resize(NN);
	for (int i = 0; i<NN; ++i) NNT[i].clear();

	if (surfOnly)
	{
		// loop over all faces
		int NF = mesh.Faces();
		for (int i = 0; i<NF; ++i)
		{
			const FSFace& f = mesh.Face(i);
			int nf = f.Nodes();
			for (int j = 0; j<nf; ++j)
			{
				int nj = f.n[j];
				for (int k = 0; k<nf; ++k)
				{
					int nk = f.n[k];
					if (nj != nk) NNT[nj].insert(nk);
				}
			}
		}
	}
	else if (mesh.IsType(FE_HEX8))
	{
		const int EHEX[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };
		int NE = mesh.Elements();
		for (int i = 0; i<NE; ++i)
		{
			const FSElement& e = mesh.Element(i);
			for (int j = 0; j<12; ++j)
			{
				int n0 = e.m_node[EHEX[j][0]];
				int n1 = e.m_node[EHEX[j][1]];
				NNT[n0].insert(n1);
				NNT[n1].insert(n0);
			}
		}
	}
	else
	{
		// loop over all elements
		int NE = mesh.Elements();
		for (int i = 0; i<NE; ++i)
		{
			const FSElement& e = mesh.Element(i);
			int ne = e.Nodes();
			for (int j = 0; j<ne; ++j)
			{
				int nj = e.m_node[j];
				for (int k = 0; k<ne; ++k)
				{
					int nk = e.m_node[k];
					if (nj != nk) NNT[nj].insert(nk);
				}
			}
		}
	}
}

FSEdgeList::FSEdgeList(const FSMesh& mesh, bool surfOnly)
{
	ET.clear();

	// build the node-node table
	FSNodeNodeTable NNT(mesh, surfOnly);

	// add all the edges
	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		pair<int, int> edge;
		set<int>& NL = NNT[i];
		if (NL.empty() == false)
		{
			set<int>::iterator it;
			for (it = NL.begin(); it != NL.end(); ++it)
			{
				int m = *it;
				if (m > i)
				{
					edge.first = i;
					edge.second = m;
					ET.push_back(edge);
				}
			}
		}
	}
}

FSEdgeList::FSEdgeList()
{

}

FSEdgeList::FSEdgeList(const FSSurfaceMesh& mesh)
{
	ET.clear();

	// add all the edges
	for (int i = 0; i<mesh.Edges(); ++i)
	{
		const FSEdge& e = mesh.Edge(i);
		pair<int, int> edge;
		edge.first = e.n[0];
		edge.second = e.n[1];
		ET.push_back(edge);
	}
}

void FSEdgeList::BuildFromMeshEdges(FSLineMesh& mesh)
{
	ET.clear();
	// add all the edges
	for (int i = 0; i < mesh.Edges(); ++i)
	{
		const FSEdge& e = mesh.Edge(i);
		pair<int, int> edge;
		edge.first = e.n[0];
		edge.second = e.n[1];
		ET.push_back(edge);
	}
}

FSFaceTable::FSFaceTable(const FSMesh& mesh)
{
	int NE = mesh.Elements();
	vector<int> tag(NE, 0);
	for (int i = 0; i<NE; ++i) tag[i] = i;

	for (int i = 0; i<mesh.Elements(); ++i)
	{
		const FSElement& ei = mesh.Element(i);
		int nf = ei.Faces();
		for (int j = 0; j<nf; ++j)
		{
			if ((ei.m_nbr[j] < 0) || (tag[ ei.m_nbr[j] ] < tag[i]))
			{
				FSFace f = ei.GetFace(j);
				FT.push_back(f);
			}
		}
	}
}


FSFaceEdgeList::FSFaceEdgeList(const FSMeshBase& mesh, const FSEdgeList& ET)
{
	// build a node-edge table
	int NN = mesh.Nodes();
	vector< vector<int> > NET;
	NET.resize(NN);
	for (int i = ET.size() - 1; i >= 0; --i)
	{
		const pair<int, int>& et = ET[i];
		NET[et.first ].push_back(i);
		NET[et.second].push_back(i);
	}

	// loop over all faces
	int NF = mesh.Faces();
	FET.resize(NF);
	for (int i = 0; i<NF; ++i)
	{
		const FSFace& face = mesh.Face(i);
		int ne = face.Edges();
		vector<int>& FETi = FET[i];
		FETi.resize(ne, -1);
		for (int j = 0; j<ne; ++j)
		{
			int n0 = face.n[j];
			int n1 = face.n[(j+1)%ne];
			if (n1 < n0) { int nt = n1; n1 = n0; n0 = nt; }

			vector<int> NETj = NET[n0];
			for (int l = 0; l<(int)NETj.size(); ++l)
			{
				int m0 = ET[ NETj[l] ].first;
				int m1 = ET[ NETj[l] ].second;
				if (((n0 == m0) && (n1 == m1)) || ((n0 == m1) && (n1 == m0)))
				{
					FETi[j] = NETj[l];
					break;
				}
			}

			assert(FETi[j] != -1);
		}
	}
}

//-----------------------------------------------------------------------------
// TODO: This assumes TET4 or HEX8 elements
FSElementEdgeList::FSElementEdgeList(const FSMesh& mesh, const FSEdgeList& ET)
{
	const int ETET[6][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 }, { 0, 3 }, { 1, 3 }, { 2, 3 } };
	const int EHEX[12][2] = { {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7} };
	const int EPENTA[9][2] = { {0,1}, {1,2}, {2,0}, {3,4},{4,5},{5,3},{0,3},{1,4},{2,5}};
	const int ETRI[3][2] = { {0,1}, {1,2}, {2,0} };
	const int EQUAD[4][2] = { {0,1}, {1,2}, {2,3}, {3, 0} };

	// build a node-edge list
	int NN = mesh.Nodes();
	vector<vector<int> > NI(NN);
	for (int i = ET.size() - 1; i >= 0; --i)
	{
		const pair<int, int>& et = ET[i];
		NI[et.first].push_back(i);
		NI[et.second].push_back(i);
	}

	int NE = mesh.Elements();
	EET.resize(NE);
	for (int i = 0; i<NE; ++i)
	{
		const FSElement& el = mesh.Element(i);
		vector<int>& EETi = EET[i];

		int nedges = 0;
		const int(*lut)[2] = nullptr;
		switch (el.Type())
		{
		case FE_TET4  :
		case FE_TET5  : nedges =  6; lut = ETET; break;
		case FE_TET10 : nedges =  6; lut = ETET; break;
		case FE_HEX8  : nedges = 12; lut = EHEX; break;
		case FE_PENTA6: nedges =  9; lut = EPENTA; break;
		case FE_TRI3  : nedges =  3; lut = ETRI; break;
		case FE_QUAD4 : nedges =  4; lut = EQUAD; break;
		default:
			assert(false);
		}
		EETi.assign(nedges, -1);
		for (int j = 0; j<nedges; ++j)
		{
			int n0 = el.m_node[lut[j][0]];
			int n1 = el.m_node[lut[j][1]];

			if (n1 < n0) { int nt = n1; n1 = n0; n0 = nt; }
			int nval = NI[n0].size();
			for (int k = 0; k < nval; ++k)
			{
				int nk = NI[n0][k];
				const std::pair<int, int>& ek = ET[nk];

				if (((n0 == ek.first) && (n1 == ek.second)) ||
					((n0 == ek.second) && (n1 == ek.first)))
				{
					EETi[j] = nk;
					break;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// only works with tet4 or hex8 meshes
FSElementFaceList::FSElementFaceList(const FSMesh& mesh, const FSFaceTable& FT)
{
	// build a node face table for FT to facilitate searching
	vector<vector<int> > NFT; NFT.resize(mesh.Nodes());
	for (int i = 0; i<(int)FT.size(); ++i)
	{
		const FSFace& f = FT[i];
		assert((f.Type() == FE_FACE_TRI3) || (f.Type() == FE_FACE_QUAD4));
		NFT[f.n[0]].push_back(i);
		NFT[f.n[1]].push_back(i);
		NFT[f.n[2]].push_back(i);
		if ((f.Type() == FE_FACE_QUAD4)) NFT[f.n[3]].push_back(i);
	}

	EFT.resize(mesh.Elements());
	for (int i = 0; i<mesh.Elements(); ++i)
	{
		const FSElement& ei = mesh.Element(i);
		vector<int>& EFTi = EFT[i];

		int nf = ei.Faces();
		EFTi.resize(nf);
		for (int j = 0; j<nf; ++j)
		{
			FSFace fj = ei.GetFace(j);
			EFTi[j] = -1;
			vector<int>& nfi = NFT[fj.n[0]];
			for (int k = 0; k<(int)nfi.size(); ++k)
			{
				const FSFace& fk = FT[nfi[k]];
				if (fj == fk)
				{
					EFTi[j] = nfi[k];
					break;
				}
			}
			assert(EFTi[j] != -1);
		}
	}
}

FSFaceFaceList::FSFaceFaceList(const FSMesh& mesh, const FSFaceTable& FT)
{
	// build a node face table for FT to facilitate searching
	vector<vector<int> > NFT; NFT.resize(mesh.Nodes());
	for (int i = 0; i<(int)FT.size(); ++i)
	{
		const FSFace& f = FT[i];
		for (int j=0; j<f.Nodes(); ++j)
			NFT[f.n[j]].push_back(i);
	}

	FFT.resize(mesh.Faces());
	for (int i = 0; i<mesh.Faces(); ++i)
	{
		const FSFace& fi = mesh.Face(i);
		vector<int>& nfi = NFT[fi.n[0]];
		FFT[i] = -1;
		for (int k = 0; k<(int)nfi.size(); ++k)
		{
			const FSFace& fk = FT[nfi[k]];
			if (fi == fk)
			{
				FFT[i] = nfi[k];
				break;
			}
		}
		assert(FFT[i] != -1);
	}
}

FSEdgeIndexList::FSEdgeIndexList(const FSMesh& mesh, const FSEdgeList& ET)
{
	// build a node-edge table for ET to facilitate searching
	vector<vector<int> > NET; NET.resize(mesh.Nodes());
	for (int i = 0; i<(int)ET.size(); ++i)
	{
		const pair<int, int>& edge = ET[i];
		NET[edge.first].push_back(i);
		NET[edge.second].push_back(i);
	}

	EET.resize(mesh.Edges());
	for (int i = 0; i<mesh.Edges(); ++i)
	{
		const FSEdge& ei = mesh.Edge(i);
		vector<int>& nei = NET[ei.n[0]];
		EET[i] = -1;
		for (int k = 0; k<(int)nei.size(); ++k)
		{
			const pair<int, int>& ek = ET[nei[k]];
			if (((ei.n[0] == ek.first) && (ei.n[1] == ek.second)) ||
				((ei.n[1] == ek.first) && (ei.n[0] == ek.second)))
			{
				EET[i] = nei[k];
				break;
			}
		}
		assert(EET[i] != -1);
	}
}

FSEdgeEdgeList::FSEdgeEdgeList(const FSMesh& mesh, int edgeId)
{
	// build a node-edge table to facilitate searching
	vector<vector<int> > NET; NET.resize(mesh.Nodes());
	for (int i = 0; i<mesh.Edges(); ++i)
	{
		const FSEdge& edge = mesh.Edge(i);
		NET[edge.n[0]].push_back(i);
		NET[edge.n[1]].push_back(i);
	}

	EEL.resize(mesh.Edges());
	for (int i = 0; i<mesh.Edges(); ++i)
	{
		const FSEdge& ei = mesh.Edge(i);
		if ((edgeId == -1) || (ei.m_gid == edgeId))
		{
			for (int j = 0; j < 2; ++j)
			{
				vector<int>& nei = NET[ei.n[j]];
				for (int k = 0; k < (int)nei.size(); ++k)
				{
					if (nei[k] != i)
					{
						const FSEdge& ek = mesh.Edge(nei[k]);
						if ((edgeId == -1) || (ek.m_gid == edgeId))
						{
							if ((ek.n[0] == ei.n[0]) || (ek.n[0] == ei.n[1]) ||
								(ek.n[1] == ei.n[0]) || (ek.n[1] == ei.n[1]))
							{
								EEL[i].push_back(nei[k]);
								break;
							}
						}
					}
				}
			}
		}
	}
}


FSEdgeFaceList::FSEdgeFaceList(const FSMesh& mesh)
{
	// build the edge list (surface only)
	FSEdgeList EL(mesh, true);

	// build the face-edge list
	FSFaceEdgeList FEL(mesh, EL);

	// build the edge face list
	int NE = EL.size();
	EFL.resize(NE);

	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		const FSFace& face = mesh.Face(i);

		int ne = (int) FEL[i].size();
		for (int j=0; j<ne; ++j)
		{
			int iedge = FEL[i][j];

			EFL[iedge].push_back(i);
		}
	}
}

FSEdgeFaceList::FSEdgeFaceList(const FSSurfaceMesh& mesh)
{
	// build the edge list
	FSEdgeList EL(mesh);

	// build the face-edge list
	FSFaceEdgeList FEL(mesh, EL);

	// build the edge face list
	int NE = EL.size();
	EFL.resize(NE);

	int NF = mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		const FSFace& face = mesh.Face(i);

		int ne = (int) FEL[i].size();
		for (int j = 0; j<ne; ++j)
		{
			int iedge = FEL[i][j];

			EFL[iedge].push_back(i);
		}
	}
}
