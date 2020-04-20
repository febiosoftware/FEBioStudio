#include "stdafx.h"
#include "FEGroup.h"
#include "FEPostMesh.h"
#include <string.h>

//-----------------------------------------------------------------------------
// FEDomain constructor
Post::FEDomain::FEDomain(Post::FEPostMesh *pm)
{
	m_pm = pm;
	m_nmat = -1;
}

//-----------------------------------------------------------------------------
void Post::FEDomain::Reserve(int nelems, int nfaces)
{
	m_Elem.reserve(nelems);
	m_Face.reserve(nfaces);
}

//-----------------------------------------------------------------------------
void Post::FEDomain::SetMatID(int matid)
{
	m_nmat = matid;
}

//-----------------------------------------------------------------------------
FEFace& Post::FEDomain::Face(int n)
{ 
	return m_pm->Face(m_Face[n]); 
}

//-----------------------------------------------------------------------------
FEElement_& Post::FEDomain::Element(int n)
{
	return m_pm->ElementRef(m_Elem[n]);
}

void Post::FEPart::GetNodeList(vector<int>& node, vector<int>& lnode)
{
	FECoreMesh& mesh = *GetMesh();
	int NN = mesh.Nodes();
	int NE = Size();

	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = -1;

	int n = 0, nne = 0;
	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(m_Elem[i]);
		int ne = el.Nodes();
		nne += ne;
		for (int j=0; j<ne; ++j)
		{
			if (mesh.Node(el.m_node[j]).m_ntag == -1) mesh.Node(el.m_node[j]).m_ntag = n++;
		}
	}

	node.resize(n);
	for (int i=0; i<NN; ++i)
		if (mesh.Node(i).m_ntag >= 0) node[mesh.Node(i).m_ntag] = i;

	lnode.resize(nne); nne = 0;
	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(m_Elem[i]);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j)
		{
			int lid = mesh.Node(el.m_node[j]).m_ntag; assert(lid >= 0);
			lnode[nne + j] = lid;
		}
		nne += ne;
	}
}

void Post::FESurface::GetNodeList(vector<int>& node, vector<int>& lnode)
{
	FECoreMesh& mesh = *GetMesh();
	int NN = mesh.Nodes();
	int NF = Size();

	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = -1;

	int n = 0, nnf = 0;
	for (int i=0; i<NF; ++i)
	{
		FEFace& face = mesh.Face(m_Face[i]);
		int nf = face.Nodes();
		nnf += nf;
		for (int j=0; j<nf; ++j)
		{
			if (mesh.Node(face.n[j]).m_ntag == -1) mesh.Node(face.n[j]).m_ntag = n++;
		}
	}

	node.resize(n);
	for (int i=0; i<NN; ++i)
		if (mesh.Node(i).m_ntag >= 0) node[mesh.Node(i).m_ntag] = i;

	lnode.resize(nnf); nnf = 0;
	for (int i=0; i<NF; ++i)
	{
		FEFace& face = mesh.Face(m_Face[i]);
		int nf = face.Nodes();
		for (int j=0; j<nf; ++j)
		{
			int lid = mesh.Node(face.n[j]).m_ntag; assert(lid >= 0);
			lnode[nnf + j] = lid;
		}
		nnf += nf;
	}
}
