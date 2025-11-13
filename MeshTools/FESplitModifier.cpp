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
#include <MeshLib/FSNodeElementList.h>
#include <MeshLib/FSFaceEdgeList.h>
using namespace std;

//-----------------------------------------------------------------------------
FEQuadSplitModifier::FEQuadSplitModifier() : FEModifier("Split")
{
	m_pm = 0;
}

//-----------------------------------------------------------------------------
// This function determines wether edge i is already split in the neighbor element
bool FEQuadSplitModifier::is_split(FSElement* pe, int i)
{
	FSElement* pi = neighbor(pe, i);
	if (pi == 0) return false;
	if (pi->m_ntag <= 0) return false;
	for (int j=0; j<4; ++j)
	{
		FSElement* pj = neighbor(pi, j);
		if (pj == pe) { return (pi->m_ntag & (1 << j)) != 0; }
	}
	return false;
}

//-----------------------------------------------------------------------------
// This function determines if an edge could be split without breaking connectivity
// An edge can be split if it does not have a neighbor, the neighbor is undetermined
// or the neighbor has the corresponding edge split
bool FEQuadSplitModifier::can_split(FSElement* pe, int i)
{
	FSElement* pi = neighbor(pe, i);
	if ((pi == 0) || (pi->m_ntag == -1)) return true;
	return is_split(pe, i);
}

//-----------------------------------------------------------------------------
// This function determines if this edge has to be split because it is split
// in the neighbor element
bool FEQuadSplitModifier::have_to_split(FSElement* pe, int i)
{
	FSElement* pi = neighbor(pe, i);
	if ((pi == 0) || (pi->m_ntag <= 0)) return false;
	return is_split(pe, i);
}

//-----------------------------------------------------------------------------
// This splits the selected elements and possibly adjacent elements to maintain
// connectivity. This is done by assigning an integer to each element that encodes
// which edges have to be split. For instance, a code of 15 means all elements have
// to be split. Currently, this algorithm is restriced in that only combinations
// of edges can be split: 15 (all edges), 3, 6, 9, 12 (two adjacent edges). 
FSMesh* FEQuadSplitModifier::Apply(FSMesh* pm)
{
	if (pm->IsType(FE_QUAD4) == false) return nullptr;

	m_pm = pm;
	int N0 = pm->Nodes();
	int E0 = pm->Elements();

	// tag all elements
	int nsel = pm->CountSelectedElements();
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement* pe = &pm->Element(i);
		pe->m_ntag = (pe->IsSelected() || (nsel == 0) ? 15 : -2);
	}

	FSNodeElementList NEL; NEL.Build(pm);
	list<FSElement*> EL;

	// tag the 1-neighborhood. These elements may have to 
	// be split as well in order to maintain connectivity.
	for (int i=0; i<pm->Elements(); ++i)
	{
		FSElement* pe = &pm->Element(i);
		if (pe->IsSelected())
		{
			for (int j=0; j<4; ++j)
			{
				int n0 = pe->m_node[j];
				int nval = NEL.Valence(n0);
				for (int k=0; k<nval; ++k)
				{
					FSElement* pk = dynamic_cast<FSElement*>(NEL.Element(n0, k));
					assert(pk);
					if (pk->m_ntag == -2) 
					{
						pk->m_ntag = -1;
						EL.push_back(pk);
					}
				}
			}
		}
	}

	// next, eliminate elements that we know can be split
	bool bdone;
	do
	{
		bdone = true;
		// see if we can find an element that we know how to split
		list<FSElement*>::iterator it;
		for (it = EL.begin(); it != EL.end(); ++it)
		{
			FSElement* pe = *it;
			assert(pe->m_ntag == -1);

			// first check if we have to split this element a particular way
			int nhave = 0;
			for (int i=0; i<4; ++i)
			{
				if (have_to_split(pe, i)) nhave |= (1 << i);
			}

			if ((nhave == 3) || (nhave == 6) || (nhave == 9) || (nhave == 12) || (nhave == 15))
			{
				pe->m_ntag = nhave;
				EL.erase(it);
				bdone = false;
				break;
			}

			// then see if we cannot split this element
			int ncan = 0;
			for (int i=0; i<4; ++i)
			{
				if (can_split(pe, i)) ncan |= (1 << i);
			}

			// if we have to split an edge, and we can only split a certain way, we split
			if (nhave != 0)
			{
				if ((ncan == 3) || (ncan == 6) || (ncan == 9) || (ncan == 12) || (ncan == 15))
				{
					pe->m_ntag = ncan;
					EL.erase(it);
					bdone = false;
					break;
				}

				// check if we can theoretically split this element
				if ((nhave == ncan) || (ncan == 10) || (ncan == 5))
				{
					// nope, we cannot
					// add the 1-neighborhood and hope for the best
					int NE = (int) EL.size();
					for (int i=0; i<4; ++i)
					{
						int ni = pe->m_node[i];
						int nval = NEL.Valence(ni);
						for (int k=0; k<nval; ++k)
						{
							FSElement* pk = dynamic_cast<FSElement*>(NEL.Element(ni, k));
							assert(pk);
							if ((pk != pe) && (pk->m_ntag == -2))
							{
								pk->m_ntag = -1;
								EL.push_back(pk);
							}
						}
					}
					assert((int) EL.size() > NE);
					bdone = false;
					break;
				}

				if ((nhave == 1) || (nhave == 2) || (nhave == 4) || (nhave == 8))
				{
					int nsel = nhave | (nhave == 8 ? 1 : (nhave << 1)) | (nhave == 1 ? 8 : (nhave >> 1));
					nsel = nsel & ncan;
					if ((nsel == 3) || (nsel == 6) || (nsel == 9) || (nsel == 12))
					{
						pe->m_ntag = nsel;
						EL.erase(it);
						bdone = false;
						break;
					}
				}
			}

			if ((ncan == 0) || (ncan == 1) || (ncan == 2) || (ncan == 4) || (ncan == 8) || (ncan == 5) || (ncan == 10))
			{
				pe->m_ntag = 0;
				EL.erase(it);
				bdone = false;
				break;
			}
		}

		if (bdone && (EL.empty() == false))
		{
			// if we get here, we did not find any element that we know how to split
			// In this case, we just pick one, split it and hope for the best
			FSElement* pe = *EL.begin();
			int ncase = 0;
			for (int i=0; i<4; ++i)
			{
				if (can_split(pe, i)) ncase |= (1 << i);
			}

			if      (ncase == 0) pe->m_ntag = 0;
			else if ((ncase &  3) == 3) pe->m_ntag =  3;
			else if ((ncase &  6) == 6) pe->m_ntag =  6;
			else if ((ncase &  9) == 9) pe->m_ntag =  9;
			else if ((ncase & 12) ==12) pe->m_ntag = 12;
			else assert(false);

			EL.erase(EL.begin());
			bdone = EL.empty();
		}
	}
	while (bdone == false);

	// tag elements that won't get split
	EL.clear();
	for (int i=0; i<pm->Elements(); ++i)
	{
		FSElement* pe = &pm->Element(i);
		if (pe->m_ntag == 0) 
			pe->m_ntag = -1;
		else if (pe->m_ntag > 0) 
			EL.push_back(pe);
	}

	// build the data
	m_Data.clear();
	list<FSElement*>::iterator it;
	for (it = EL.begin(); it != EL.end(); ++it)
	{
		FSElement* pe = *it;
		DATA d;
		d.pe = pe;
		d.ncase = pe->m_ntag;
		d.nid = (int) m_Data.size();
		d.ntag = 0;
		pe->m_ntag = d.nid;
		m_Data.push_back(d);
	}

	// count interior nodes
	int nfn = (int) EL.size();

	// count edges nodes for all elements
	int nen = 0;
	int NE = (int)m_Data.size();
	for (int i=0; i<NE; ++i)
	{
		DATA& di = m_Data[i];
		FSElement* pe = di.pe;
		for (int j=0; j<4; ++j)
		{
			if (di.ncase & (1 << j))
			{
				FSElement* pj = neighbor(pe, j);
				if ((pj == 0) || (m_Data[pj->m_ntag].ntag == 0)) nen++;
			}
		}
		di.ntag = 1;
	}

	// flip all tags again
	for (int i=0; i<NE; ++i) m_Data[i].ntag = 0;

	// count number of elements
	int nel = 0;
	for (int i=0; i<NE; ++i)
	{
		DATA& di = m_Data[i];
		if (di.ncase == 15) nel += 4;
		else 
			nel += 3;
	}

	// create the new mesh
	int NN1 = N0 + nfn + nen;
	int NE1 = E0 + nel - (int) EL.size();
	int NF1 = NE1;
	FSMesh* pnew = new FSMesh;
	pnew->Create(NN1, NE1, NF1);

	// copy old nodes
	for (int i=0; i<N0; ++i) pnew->Node(i) = pm->Node(i);

	// add the new face nodes
	it = EL.begin();
	for (int i=0; i<nfn; ++i, ++it)
	{
		FSElement* pe = (*it);
		vec3d r(0,0,0);
		for (int j=0; j<4; ++j) r += pm->Node(pe->m_node[j]).r*0.25;

		FSNode& n0 = pnew->Node(N0 + i);
		n0.r = r;
	}

	// add the new edge nodes
	nen = 0;
	vector<int>	edn(NE*4);
	for (int i=0; i<NE; ++i)
	{
		DATA& di = m_Data[i];
		FSElement* pe = di.pe;
		for (int j=0; j<4; ++j)
		{
			if (di.ncase & (1 << j))
			{
				FSElement* pj = neighbor(pe, j);
				if ((pj == 0) || (m_Data[pj->m_ntag].ntag == 0)) 
				{
					FSNode& n0 = pm->Node(pe->m_node[j      ]);
					FSNode& n1 = pm->Node(pe->m_node[(j+1)%4]);
					vec3d r = (n0.r + n1.r)*0.5;

					pnew->Node(N0 + nfn + nen).r = r;
					edn[i*4 + j] = N0 + nfn + nen;
					nen++;
				}
				else
				{
					DATA& dj = m_Data[pj->m_ntag];
					for (int k=0; k<4; ++k)
					{
						int nk = pj->m_nbr[k];
						FSElement* pk = (nk >= 0 ? &pm->Element(nk) : 0);
						if (pk == pe)
						{
							edn[i*4 + j] = edn[pj->m_ntag*4 + k];
							assert(edn[i*4 + j] >= 0);
							break;
						}
					}

				}
			}
			else edn[4*i + j] = -1;
		}
		di.ntag = 1;
	}

	// add the non-selected elements
	int ne = 0;
	for (int i=0; i<E0; ++i)
	{
		FSElement& en = pnew->Element(ne);
		FSElement& eo = pm->Element(i);
		if (eo.m_ntag < 0) { 
			en = eo; ne++; }
	}

	// split the elements
	for (int i=0; i<NE; ++i)
	{
		DATA& di = m_Data[i];
		FSElement& el = *di.pe;
		switch (di.ncase)
		{
		case 3:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				FSElement& e2 = pnew->Element(ne++); e2 = el;
				int* en = &edn[4*i];

				e0.m_node[0] = el.m_node[0]; e1.m_node[0] = en[0]       ; e2.m_node[0] = N0 + i      ;
				e0.m_node[1] = en[0]       ; e1.m_node[1] = el.m_node[1]; e2.m_node[1] = en[1]       ;
				e0.m_node[2] = N0 + i      ; e1.m_node[2] = en[1]       ; e2.m_node[2] = el.m_node[2];
				e0.m_node[3] = el.m_node[3]; e1.m_node[3] = N0 + i      ; e2.m_node[3] = el.m_node[3];
			}
			break;
		case 6:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				FSElement& e2 = pnew->Element(ne++); e2 = el;
				int* en = &edn[4*i];

				e0.m_node[0] = el.m_node[0]; e1.m_node[0] = N0 + i      ; e2.m_node[0] = el.m_node[0];
				e0.m_node[1] = el.m_node[1]; e1.m_node[1] = en[1]       ; e2.m_node[1] = N0 + i      ;
				e0.m_node[2] = en[1]       ; e1.m_node[2] = el.m_node[2]; e2.m_node[2] = en[2];
				e0.m_node[3] = N0 + i      ; e1.m_node[3] = en[2]       ; e2.m_node[3] = el.m_node[3];
			}
			break;
		case 9:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				FSElement& e2 = pnew->Element(ne++); e2 = el;
				int* en = &edn[4*i];

				e0.m_node[0] = el.m_node[0]; e1.m_node[0] = en[0]       ; e2.m_node[0] = en[3]       ;
				e0.m_node[1] = en[0]       ; e1.m_node[1] = el.m_node[1]; e2.m_node[1] = N0 + i      ;
				e0.m_node[2] = N0 + i      ; e1.m_node[2] = el.m_node[2]; e2.m_node[2] = el.m_node[2];
				e0.m_node[3] = en[3]       ; e1.m_node[3] = N0 + i      ; e2.m_node[3] = el.m_node[3];
			}
			break;
		case 12:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				FSElement& e2 = pnew->Element(ne++); e2 = el;
				int* en = &edn[4*i];

				e0.m_node[0] = el.m_node[0]; e1.m_node[0] = N0 + i      ; e2.m_node[0] = en[3]       ;
				e0.m_node[1] = el.m_node[1]; e1.m_node[1] = el.m_node[1]; e2.m_node[1] = N0 + i      ;
				e0.m_node[2] = N0 + i      ; e1.m_node[2] = el.m_node[2]; e2.m_node[2] = en[2]       ;
				e0.m_node[3] = en[3]       ; e1.m_node[3] = en[2]       ; e2.m_node[3] = el.m_node[3];
			}
			break;
		case 15:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				FSElement& e2 = pnew->Element(ne++); e2 = el;
				FSElement& e3 = pnew->Element(ne++); e3 = el;
				int* en = &edn[4*i];

				e0.m_node[0] = el.m_node[0]; e1.m_node[0] = en[0]       ; e2.m_node[0] = N0 + i      ; e3.m_node[0] = en[3]       ;
				e0.m_node[1] = en[0]       ; e1.m_node[1] = el.m_node[1]; e2.m_node[1] = en[1]       ; e3.m_node[1] = N0 + i      ;
				e0.m_node[2] = N0 + i      ; e1.m_node[2] = en[1]       ; e2.m_node[2] = el.m_node[2]; e3.m_node[2] = en[2]       ;
				e0.m_node[3] = en[3]       ; e1.m_node[3] = N0 + i      ; e2.m_node[3] = en[2]       ; e3.m_node[3] = el.m_node[3];
			}
			break;
		default:
			assert(false);
		}
	}

	// create the faces
	for (int i=0; i<NF1; ++i)
	{
		FSElement& el = pnew->Element(i);
		FSFace& f = pnew->Face(i);
		f.SetType(FE_FACE_QUAD4);
		f.n[0] = el.m_node[0];
		f.n[1] = el.m_node[1];
		f.n[2] = el.m_node[2];
		f.n[3] = el.m_node[3];
		f.m_gid = 0;
	}

	// clean up
	m_Data.clear();

	// update the new mesh
	pnew->RebuildMesh();

	// All done
	return pnew;
}

//=============================================================================
FETriSplitModifier::FETriSplitModifier() : FEModifier("Split")
{
	m_pm = 0;
	m_niter = 1;
}

//-----------------------------------------------------------------------------
// This function determines wether edge i is already split in the neighbor element
bool FETriSplitModifier::is_split(FSElement* pe, int i)
{
	FSElement* pi = neighbor(pe, i);
	if (pi == 0) return false;
	if (pi->m_ntag <= 0) return false;
	for (int j=0; j<4; ++j)
	{
		FSElement* pj = neighbor(pi, j);
		if (pj == pe) { return (pi->m_ntag & (1 << j)) != 0; }
	}
	return false;
}

//-----------------------------------------------------------------------------
// This function determines if an edge could be split without breaking connectivity
// An edge can be split if it does not have a neighbor, the neighbor is undetermined
// or the neighbor has the corresponding edge split
bool FETriSplitModifier::can_split(FSElement* pe, int i)
{
	FSElement* pi = neighbor(pe, i);
	if ((pi == 0) || (pi->m_ntag == -1)) return true;
	return is_split(pe, i);
}

//-----------------------------------------------------------------------------
// This function determines if this edge has to be split because it is split
// in the neighbor element
bool FETriSplitModifier::have_to_split(FSElement* pe, int i)
{
	FSElement* pi = neighbor(pe, i);
	if ((pi == 0) || (pi->m_ntag <= 0)) return false;
	return is_split(pe, i);
}

FSMesh* FETriSplitModifier::Apply(FSMesh* pm)
{
	if (m_niter <= 0)
	{
		FEModifier::SetError("Invalid number of iterations");
		return 0;
	}

	FSMesh* newMesh = 0;
	for (int i=0; i<m_niter; ++i)
	{
		newMesh = Split(pm);

		if (i < m_niter - 1)
		{
			if (i != 0) delete pm;
			pm = newMesh;
		}
	}
	return newMesh;
}

FSMesh* FETriSplitModifier::Split(FSMesh* pm)
{
	if (pm == nullptr) return nullptr;
	if (pm->IsType(FE_TRI3) == false) return nullptr;

	m_pm = pm;
	int N0 = pm->Nodes();
	int E0 = pm->Elements();

	// tag all elements
	int nsel = pm->CountSelectedElements();
	for (int i=0; i<pm->Elements(); ++i) 
	{
		FSElement* pe = &pm->Element(i);
		pe->m_ntag = (pe->IsSelected() || (nsel == 0) ? 7 : 0);
	}

	// build the element edge list
	EdgeList EL(*pm);
	FSElementEdgeList EEL(*pm, EL);
	vector<int> edgeTags(EL.size(), 0);

	// tag the cases
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == 7)
		{
			vector<int>& edl = EEL[i]; assert(edl.size() == 3);
			edgeTags[edl[0]] = 1;
			edgeTags[edl[1]] = 1;
			edgeTags[edl[2]] = 1;
		}
	}

	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == 0)
		{
			vector<int>& edl = EEL[i]; assert(edl.size() == 3);
			int ncase = 0;
			if (edgeTags[edl[0]] == 1) ncase += 1;
			if (edgeTags[edl[1]] == 1) ncase += 2;
			if (edgeTags[edl[2]] == 1) ncase += 4;
			el.m_ntag = ncase;
		}
	}

	// tag the edges
	int nen = 0;
	for (int i = 0; i < edgeTags.size(); ++i)
	{
		if (edgeTags[i] == 1) edgeTags[i] = nen++;
		else edgeTags[i] = -1;
	}

	// count number of elements
	int NE1 = 0;
	for (int i=0; i<E0; ++i)
	{
		FSElement& el = pm->Element(i);
		int n = el.m_ntag;
		if (n > 0)
		{
			if (n == 7) NE1 += 4;
			else if ((n == 3) || (n == 5) || (n == 6)) NE1 += 3;
			else NE1 += 2;
		}
		else NE1++; // elements that are not split
	}

	// create the new mesh
	int NN1 = N0 + nen;
	int NF1 = NE1;
	FSMesh* pnew = new FSMesh;
	pnew->Create(NN1, NE1, NF1);

	// copy old nodes
	for (int i=0; i<N0; ++i) pnew->Node(i) = pm->Node(i);

	// add the new edge nodes
	nen = 0;
	for (int i=0; i<EL.size(); ++i)
	{
		if (edgeTags[i] >= 0)
		{
			std::pair<int, int> edge = EL[i];

			FSNode& n0 = pm->Node(edge.first);
			FSNode& n1 = pm->Node(edge.second);
			vec3d r = (n0.r + n1.r)*0.5;

			pnew->Node(N0 + nen).r = r;
			nen++;
		}
	}

	// split the elements
	int ne = 0;
	for (int i=0; i<E0; ++i)
	{
		FSElement& el = pm->Element(i);

		vector<int>& edl = EEL[i];
		int en[3];
		en[0] = N0 + edgeTags[edl[0]];
		en[1] = N0 + edgeTags[edl[1]];
		en[2] = N0 + edgeTags[edl[2]];
		int m[6] = { el.m_node[0], el.m_node[1], el.m_node[2], en[0], en[1], en[2] };

		switch (el.m_ntag)
		{	
		case 0:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
			}
			break;
		case 1:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				e0.m_node[0] = m[0]; e0.m_node[1] = m[3]; e0.m_node[2] = m[2];
				e1.m_node[0] = m[3]; e1.m_node[1] = m[1]; e1.m_node[2] = m[2];
			}
			break;
		case 2:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				e0.m_node[0] = m[0]; e0.m_node[1] = m[1]; e0.m_node[2] = m[4];
				e1.m_node[0] = m[4]; e1.m_node[1] = m[2]; e1.m_node[2] = m[0];
			}
			break;
		case 3:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				FSElement& e2 = pnew->Element(ne++); e2 = el;
				e0.m_node[0] = m[0]; e0.m_node[1] = m[3]; e0.m_node[2] = m[4];
				e1.m_node[0] = m[1]; e1.m_node[1] = m[4]; e1.m_node[2] = m[3];
				e2.m_node[0] = m[0]; e2.m_node[1] = m[4]; e2.m_node[2] = m[2];
			}
			break;
		case 4:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				e0.m_node[0] = m[0]; e0.m_node[1] = m[1]; e0.m_node[2] = m[5];
				e1.m_node[0] = m[1]; e1.m_node[1] = m[2]; e1.m_node[2] = m[5];
			}
			break;
		case 5:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				FSElement& e2 = pnew->Element(ne++); e2 = el;
				e0.m_node[0] = m[0]; e0.m_node[1] = m[3]; e0.m_node[2] = m[5];
				e1.m_node[0] = m[1]; e1.m_node[1] = m[5]; e1.m_node[2] = m[3];
				e2.m_node[0] = m[1]; e2.m_node[1] = m[2]; e2.m_node[2] = m[5];
			}
			break;
		case 6:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				FSElement& e2 = pnew->Element(ne++); e2 = el;
				e0.m_node[0] = m[0]; e0.m_node[1] = m[1]; e0.m_node[2] = m[5];
				e1.m_node[0] = m[1]; e1.m_node[1] = m[4]; e1.m_node[2] = m[5];
				e2.m_node[0] = m[5]; e2.m_node[1] = m[4]; e2.m_node[2] = m[2];
			}
			break;
		case 7:
			{
				FSElement& e0 = pnew->Element(ne++); e0 = el;
				FSElement& e1 = pnew->Element(ne++); e1 = el;
				FSElement& e2 = pnew->Element(ne++); e2 = el;
				FSElement& e3 = pnew->Element(ne++); e3 = el;
				e0.m_node[0] = m[0]; e0.m_node[1] = m[3]; e0.m_node[2] = m[5];
				e1.m_node[0] = m[1]; e1.m_node[1] = m[4]; e1.m_node[2] = m[3];
				e2.m_node[0] = m[2]; e2.m_node[1] = m[5]; e2.m_node[2] = m[4];
				e3.m_node[0] = m[3]; e3.m_node[1] = m[4]; e3.m_node[2] = m[5];
			}
			break;
		default:
			assert(false);
		}
	}
	assert(ne == NE1);

	// create the faces
/*	for (int i=0; i<NF1; ++i)
	{
		FSElement& el = pnew->Element(i);
		FSFace& f = pnew->Face(i);
		f.SetType(FE_FACE_TRI3);
		f.n[0] = el.m_node[0];
		f.n[1] = el.m_node[1];
		f.n[2] = el.m_node[2];
		f.m_gid = 0;
	}
*/
	// update the new mesh
	pnew->GenerateNodalIDs();
	pnew->GenerateElementIDs();
	pnew->RebuildMesh();

	// All done
	return pnew;
}
