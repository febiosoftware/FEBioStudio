#include "stdafx.h"
#include <MeshLib/FEMesh.h>
#include "FEModifier.h"
using namespace std;

const int H2T[64][6][4] = {
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 0 (invalid)
	{{ 0, 1, 3, 4},{ 1, 2, 3, 4},{ 1, 2, 4, 6},{ 1, 4, 5, 6},{ 2, 3, 4, 7},{ 2, 4, 6, 7}}, // case = 1
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 2 (invalid)
	{{ 0, 1, 3, 4},{ 1, 2, 3, 4},{ 1, 2, 4, 5},{ 2, 3, 4, 7},{ 2, 4, 5, 6},{ 2, 4, 6, 7}}, // case = 3
	{{ 0, 1, 3, 5},{ 0, 3, 4, 5},{ 1, 2, 3, 6},{ 1, 3, 5, 6},{ 3, 4, 5, 6},{ 3, 4, 6, 7}}, // case = 4
	{{ 0, 1, 3, 4},{ 1, 2, 3, 4},{ 1, 2, 4, 6},{ 1, 4, 5, 6},{ 2, 3, 4, 6},{ 3, 4, 6, 7}}, // case = 5
	{{ 0, 1, 3, 5},{ 0, 3, 4, 5},{ 1, 2, 3, 5},{ 2, 3, 5, 6},{ 3, 4, 5, 6},{ 3, 4, 6, 7}}, // case = 6
	{{ 0, 1, 3, 4},{ 1, 2, 3, 4},{ 1, 2, 4, 5},{ 2, 3, 4, 6},{ 2, 4, 5, 6},{ 3, 4, 6, 7}}, // case = 7
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 8 (invalid)
	{{ 0, 1, 3, 7},{ 0, 1, 7, 4},{ 1, 2, 3, 7},{ 1, 2, 7, 6},{ 1, 4, 5, 6},{ 1, 4, 6, 7}}, // case = 9
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 10 (invalid)
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 11 (invalid)
	{{ 0, 1, 3, 6},{ 0, 1, 6, 5},{ 0, 3, 7, 6},{ 0, 4, 5, 6},{ 0, 4, 6, 7},{ 1, 2, 3, 6}}, // case = 12
	{{ 0, 1, 3, 6},{ 0, 1, 6, 4},{ 0, 3, 7, 6},{ 0, 4, 6, 7},{ 1, 2, 3, 6},{ 1, 4, 5, 6}}, // case = 13
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 14 (invalid)
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 15 (invalid)
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 16 (invalid)
	{{ 0, 1, 2, 4},{ 0, 2, 3, 4},{ 1, 2, 4, 6},{ 1, 4, 5, 6},{ 2, 3, 4, 7},{ 2, 4, 6, 7}}, // case = 17
	{{ 0, 1, 2, 5},{ 0, 2, 3, 4},{ 0, 2, 4, 5},{ 2, 3, 4, 7},{ 2, 4, 5, 6},{ 2, 4, 6, 7}}, // case = 18
	{{ 0, 1, 2, 4},{ 0, 2, 3, 4},{ 1, 2, 4, 5},{ 2, 3, 4, 7},{ 2, 4, 5, 6},{ 2, 4, 6, 7}}, // case = 19
	{{ 0, 1, 2, 6},{ 0, 1, 6, 5},{ 0, 2, 3, 6},{ 0, 3, 4, 6},{ 0, 4, 5, 6},{ 3, 4, 6, 7}}, // case = 20
	{{ 0, 1, 2, 4},{ 0, 2, 3, 4},{ 1, 2, 4, 6},{ 1, 4, 5, 6},{ 2, 3, 4, 6},{ 3, 4, 6, 7}}, // case = 21
	{{ 0, 1, 2, 5},{ 0, 2, 3, 6},{ 0, 2, 6, 5},{ 0, 3, 4, 6},{ 0, 4, 5, 6},{ 3, 4, 6, 7}}, // case = 22
	{{ 0, 1, 2, 4},{ 0, 2, 3, 4},{ 1, 2, 4, 5},{ 2, 3, 4, 6},{ 2, 4, 5, 6},{ 3, 4, 6, 7}}, // case = 23
	{{ 0, 1, 2, 6},{ 0, 1, 6, 5},{ 0, 2, 3, 7},{ 0, 2, 7, 6},{ 0, 4, 5, 6},{ 0, 4, 6, 7}}, // case = 24
	{{ 0, 1, 2, 6},{ 0, 1, 6, 4},{ 0, 2, 3, 7},{ 0, 2, 7, 6},{ 0, 4, 6, 7},{ 1, 4, 5, 6}}, // case = 25
	{{ 0, 1, 2, 5},{ 0, 2, 3, 7},{ 0, 2, 6, 5},{ 0, 2, 7, 6},{ 0, 4, 5, 6},{ 0, 4, 6, 7}}, // case = 26
	{{ 0, 1, 2, 4},{ 0, 2, 3, 7},{ 0, 2, 7, 4},{ 1, 2, 4, 5},{ 2, 4, 5, 6},{ 2, 4, 6, 7}}, // case = 27
	{{ 0, 1, 2, 6},{ 0, 1, 6, 5},{ 0, 2, 3, 6},{ 0, 3, 7, 6},{ 0, 4, 5, 6},{ 0, 4, 6, 7}}, // case = 28
	{{ 0, 1, 2, 6},{ 0, 1, 6, 4},{ 0, 2, 3, 6},{ 0, 3, 7, 6},{ 0, 4, 6, 7},{ 1, 4, 5, 6}}, // case = 29
	{{ 0, 1, 2, 5},{ 0, 2, 3, 6},{ 0, 2, 6, 5},{ 0, 3, 7, 6},{ 0, 4, 5, 6},{ 0, 4, 6, 7}}, // case = 30
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 31 (invalid)
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 32 (invalid)
	{{ 0, 1, 3, 4},{ 1, 2, 3, 7},{ 1, 2, 7, 6},{ 1, 3, 4, 7},{ 1, 4, 5, 7},{ 1, 5, 6, 7}}, // case = 33
	{{ 0, 1, 3, 5},{ 0, 3, 4, 5},{ 1, 2, 3, 5},{ 2, 3, 5, 7},{ 2, 5, 6, 7},{ 3, 4, 5, 7}}, // case = 34
	{{ 0, 1, 3, 4},{ 1, 2, 3, 4},{ 1, 2, 4, 5},{ 2, 3, 4, 7},{ 2, 4, 5, 7},{ 2, 5, 6, 7}}, // case = 35
	{{ 0, 1, 3, 5},{ 0, 3, 4, 5},{ 1, 2, 3, 6},{ 1, 3, 5, 6},{ 3, 4, 5, 7},{ 3, 5, 6, 7}}, // case = 36
	{{ 0, 1, 3, 4},{ 1, 2, 3, 6},{ 1, 3, 4, 5},{ 1, 3, 5, 6},{ 3, 4, 5, 7},{ 3, 5, 6, 7}}, // case = 37
	{{ 0, 1, 3, 5},{ 0, 3, 4, 5},{ 1, 2, 3, 5},{ 2, 3, 5, 6},{ 3, 4, 5, 7},{ 3, 5, 6, 7}}, // case = 38
	{{ 0, 1, 3, 4},{ 1, 2, 3, 5},{ 1, 3, 4, 5},{ 2, 3, 5, 6},{ 3, 4, 5, 7},{ 3, 5, 6, 7}}, // case = 39
	{{ 0, 1, 3, 7},{ 0, 1, 7, 5},{ 0, 4, 5, 7},{ 1, 2, 3, 7},{ 1, 2, 7, 6},{ 1, 5, 6, 7}}, // case = 40
	{{ 0, 1, 3, 7},{ 0, 1, 7, 4},{ 1, 2, 3, 7},{ 1, 2, 7, 6},{ 1, 4, 5, 7},{ 1, 5, 6, 7}}, // case = 41
	{{ 0, 1, 3, 5},{ 0, 3, 7, 5},{ 0, 4, 5, 7},{ 1, 2, 3, 5},{ 2, 3, 5, 7},{ 2, 5, 6, 7}}, // case = 42
	{{ 0, 1, 3, 7},{ 0, 1, 7, 4},{ 1, 2, 3, 7},{ 1, 2, 7, 5},{ 1, 4, 5, 7},{ 2, 5, 6, 7}}, // case = 43
	{{ 0, 1, 3, 6},{ 0, 1, 6, 5},{ 0, 3, 7, 6},{ 0, 4, 5, 7},{ 0, 5, 6, 7},{ 1, 2, 3, 6}}, // case = 44
	{{ 0, 1, 3, 7},{ 0, 1, 7, 4},{ 1, 2, 3, 6},{ 1, 3, 7, 6},{ 1, 4, 5, 7},{ 1, 5, 6, 7}}, // case = 45
	{{ 0, 1, 3, 5},{ 0, 3, 7, 5},{ 0, 4, 5, 7},{ 1, 2, 3, 5},{ 2, 3, 5, 6},{ 3, 5, 6, 7}}, // case = 46
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 47 (invalid)
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 48 (invalid)
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 49 (invalid)
	{{ 0, 1, 2, 5},{ 0, 2, 3, 5},{ 0, 3, 4, 5},{ 2, 3, 5, 7},{ 2, 5, 6, 7},{ 3, 4, 5, 7}}, // case = 50
	{{ 0, 1, 2, 4},{ 0, 2, 3, 4},{ 1, 2, 4, 5},{ 2, 3, 4, 7},{ 2, 4, 5, 7},{ 2, 5, 6, 7}}, // case = 51
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 52 (invalid)
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 53 (invalid)
	{{ 0, 1, 2, 5},{ 0, 2, 3, 5},{ 0, 3, 4, 5},{ 2, 3, 5, 6},{ 3, 4, 5, 7},{ 3, 5, 6, 7}}, // case = 54
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 55 (invalid)
	{{ 0, 1, 2, 7},{ 0, 1, 7, 5},{ 0, 2, 3, 7},{ 0, 4, 5, 7},{ 1, 2, 7, 6},{ 1, 5, 6, 7}}, // case = 56
	{{ 0, 1, 2, 7},{ 0, 1, 7, 4},{ 0, 2, 3, 7},{ 1, 2, 7, 6},{ 1, 4, 5, 7},{ 1, 5, 6, 7}}, // case = 57
	{{ 0, 1, 2, 5},{ 0, 2, 3, 5},{ 0, 3, 7, 5},{ 0, 4, 5, 7},{ 2, 3, 5, 7},{ 2, 5, 6, 7}}, // case = 58
	{{ 0, 1, 2, 4},{ 0, 2, 3, 7},{ 0, 2, 7, 4},{ 1, 2, 4, 5},{ 2, 4, 5, 7},{ 2, 5, 6, 7}}, // case = 59
	{{ 0, 1, 2, 6},{ 0, 1, 6, 5},{ 0, 2, 3, 6},{ 0, 3, 7, 6},{ 0, 4, 5, 7},{ 0, 5, 6, 7}}, // case = 60
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 61 (invalid)
	{{ 0, 1, 2, 5},{ 0, 2, 3, 5},{ 0, 3, 7, 5},{ 0, 4, 5, 7},{ 2, 3, 5, 6},{ 3, 5, 6, 7}}, // case = 62
	{{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}}, // case = 63 (invalid)
};

//-----------------------------------------------------------------------------
bool cmp_tri(int a[3], int b[3])
{
	if ((a[0]!=b[0])&&(a[0]!=b[1])&&(a[0]!=b[2])) return false;
	if ((a[1]!=b[0])&&(a[1]!=b[1])&&(a[1]!=b[2])) return false;
	if ((a[2]!=b[0])&&(a[2]!=b[1])&&(a[2]!=b[2])) return false;
	return true;
}

//-----------------------------------------------------------------------------
int face_case(FEFace& f1, FEElement& e2)
{
	for (int k=0; k<6; ++k)
	{
		FEFace f2 = e2.GetFace(k);
		if (f1 == f2)
		{
			// we found the face, so split it
			int B0[3], B1[3];
			int n = (e2.m_ntag & (1<<k));
			if (n == 0)
			{
				B0[0] = f2.n[0]; B1[0] = f2.n[2];
				B0[1] = f2.n[1]; B1[1] = f2.n[3];
				B0[2] = f2.n[2]; B1[2] = f2.n[0];
			}
			else
			{
				B0[0] = f2.n[1]; B1[0] = f2.n[3];
				B0[1] = f2.n[2]; B1[1] = f2.n[0];
				B0[2] = f2.n[3]; B1[2] = f2.n[1];
			}

			// now we need to see with which split this correspond of face 1
			int A0[3], A1[3];
			// CASE 0
			A0[0] = f1.n[0]; A1[0] = f1.n[2];
			A0[1] = f1.n[1]; A1[1] = f1.n[3];
			A0[2] = f1.n[2]; A1[2] = f1.n[0];
			if ((cmp_tri(A0,B0)&&cmp_tri(A1,B1))||(cmp_tri(A1,B0)&&cmp_tri(A0,B1))) 
			{
				// case = 0
				// (don't do anything)
				return 0;
			}
			else
			{
				// CASE 1
				A0[0] = f1.n[1]; A1[0] = f1.n[3];
				A0[1] = f1.n[2]; A1[1] = f1.n[0];
				A0[2] = f1.n[3]; A1[2] = f1.n[1];
				if ((cmp_tri(A0,B0)&&cmp_tri(A1,B1))||(cmp_tri(A1,B0)&&cmp_tri(A0,B1)))
				{
					// case = 1
					return 1;
				}
				else return -1;
			}
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
bool case_valid(int n)
{
	if (H2T[n][0][0] == -1) return false; else return true;
}

//-----------------------------------------------------------------------------
// see if a face exists in the new mesh
int FindFace(FEMesh* mesh, int elem, int n[3])
{
	for (int i=0; i<6; ++i)
	{
		FEElement& el = mesh->Element(elem + i);
		for (int j=0; j<4; ++j)
		{
			FEFace fj = el.GetFace(j);
			if (fj.HasNode(n[0]) && fj.HasNode(n[1]) && fj.HasNode(n[2])) return elem + i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
// This algorithm converts a hex mesh into a tet mesh by dividing all hex
// elements into six tet elements. 
FEMesh* FEHex2Tet::Apply(FEMesh* pm)
{
	// make sure all elements are hexes
	if (pm->IsType(FE_HEX8) == false) return 0;

	// Tag all elements
	pm->TagAllElements(-1);
	int NN = pm->Nodes();
	int NE = pm->Elements();
	int NF = pm->Faces();
	int NC = pm->Edges();

	// for each element, find a valid case from the table above
	// case 22 is where all faces are symmetric. This means
	// for a regular mesh all hexes can be divided using case 22. 
	stack<int> S; S.push(0);
	while (!S.empty())
	{
		int i = S.top(); S.pop();
		FEElement& e = pm->Element(i);

		// we need to figure out if any of the neighbors' tags have been set
		int nmask = 0;
		int nface = 0;
		for (int j=0; j<6; ++j)
		{
			int nj = e.m_nbr[j];
			if (nj >= 0)
			{
				FEElement& e2 = pm->Element(nj);
				if (e2.m_ntag == -1) { e2.m_ntag = -2; S.push(nj); } // neigbor has not been processed, so push on stack
				else if (e2.m_ntag >= 0)
				{
					// the neighbor's tag has been set so we cannot choose the case at random
					// we have to figure out the face flags.
					nmask |= (1<<j);
					FEFace f1 = e.GetFace(j);

					int nf = face_case(f1, e2);
					if (nf == -1) return 0;
					if (nf == 1) nface |= (1<<j);
				}
			}
		}

		// set the case
		if (nmask == 0) e.m_ntag = 1;
		else
		{
			// first try case 22 since we that one will always work
			// for regular meshes
			if (case_valid(22) && ((22&nmask) == nface))
			{
				e.m_ntag = 22;
			}
			else
			{
				for (int i=0; i<64; ++i)
				{
					if (case_valid(i) && ((i&nmask) == nface))
					{
						e.m_ntag = i;
						break;
					}
				}
			}
		}

		// if we could not find a valid configurations, we have to abort.
		if (e.m_ntag < 0) return 0;

		// try to find an unprocessed hex if the stack is empty
		if (S.empty())
		{
			for (i=0; i<NE; ++i) if (pm->Element(i).m_ntag == -1) { S.push(i); break; }
		}
	}

	// create a new mesh
	FEMesh* pnew = new FEMesh;
	pnew->Create(NN, 6*NE, 2*NF, NC);

	// copy nodes
	for (int i=0; i<NN; ++i) pnew->Node(i) = pm->Node(i);

	// copy edges
	for (int i = 0; i<NC; ++i) pnew->Edge(i) = pm->Edge(i);

	// using the cases decided above, split each hex into 6 tets.
	int n = 0;
	for (int i=0; i<NE; ++i)
	{
		FEElement& e = pm->Element(i);
		if (e.m_ntag == -1) return 0;
		const int (&TET)[6][4] = H2T[e.m_ntag];
		for (int j=0; j<6; ++j)
		{
			FEElement& ej = pnew->Element(n++);
			ej.SetType(FE_TET4);
			ej.m_gid = e.m_gid;
			ej.m_node[0] = e.m_node[TET[j][0]];
			ej.m_node[1] = e.m_node[TET[j][1]];
			ej.m_node[2] = e.m_node[TET[j][2]];
			ej.m_node[3] = e.m_node[TET[j][3]];
		}
	}
	pnew->UpdateElementNeighbors();

	// split all faces in two
	n = 0;
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = pm->Face(i);
		FEElement& el = pm->Element(f.m_elem[0]);

		// we have to figure out how each face was split by the parent element
		int nf[3] = {f.n[0], f.n[1], f.n[2]};
		int nelem = FindFace(pnew, 6*f.m_elem[0], nf);
		if (nelem != -1)
		{
			FEFace& f1 = pnew->Face(n++);
			f1.SetType(FE_FACE_TRI3);
			f1.m_gid = f.m_gid;
			f1.n[0] = f.n[0];
			f1.n[1] = f.n[1];
			f1.n[2] = f.n[2];

			FEFace& f2 = pnew->Face(n++);
			f2.SetType(FE_FACE_TRI3);
			f2.m_gid = f.m_gid;
			f2.n[0] = f.n[2];
			f2.n[1] = f.n[3];
			f2.n[2] = f.n[0];
			assert(FindFace(pnew, 6*f.m_elem[0], f2.n) != -1);
		}
		else
		{
			FEFace& f1 = pnew->Face(n++);
			f1.SetType(FE_FACE_TRI3);
			f1.m_gid = f.m_gid;
			f1.n[0] = f.n[3];
			f1.n[1] = f.n[0];
			f1.n[2] = f.n[1];
			assert(FindFace(pnew, 6 * f.m_elem[0], f1.n) != -1);

			FEFace& f2 = pnew->Face(n++);
			f2.SetType(FE_FACE_TRI3);
			f2.m_gid = f.m_gid;
			f2.n[0] = f.n[1];
			f2.n[1] = f.n[2];
			f2.n[2] = f.n[3];
			assert(FindFace(pnew, 6 * f.m_elem[0], f2.n) != -1);
		}
	}
	pnew->UpdateFaces();

	return pnew;
}
