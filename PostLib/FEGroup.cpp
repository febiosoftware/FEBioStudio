#include "stdafx.h"
#include "FEGroup.h"
#include "FEMesh.h"
#include <string.h>
using namespace Post;

//-----------------------------------------------------------------------------
void FEGroup::SetName(const char* szname)
{
	strcpy(m_szname, szname);
}

//-----------------------------------------------------------------------------
const char* FEGroup::GetName()
{
	return m_szname;
}

//-----------------------------------------------------------------------------
// FEDomain constructor
FEDomain::FEDomain(FEMeshBase *pm)
{
	m_pm = pm;
	m_nmat = -1;
}

//-----------------------------------------------------------------------------
void FEDomain::Reserve(int nelems, int nfaces)
{
	m_Elem.reserve(nelems);
	m_Face.reserve(nfaces);
}

//-----------------------------------------------------------------------------
void FEDomain::SetMatID(int matid)
{
	m_nmat = matid;
}

//-----------------------------------------------------------------------------
FEFace& FEDomain::Face(int n)
{ 
	return m_pm->Face(m_Face[n]); 
}

//-----------------------------------------------------------------------------
FEElement& FEDomain::Element(int n)
{
	return m_pm->Element(m_Elem[n]); 
}

void FEPart::GetNodeList(vector<int>& node, vector<int>& lnode)
{
	FEMeshBase& mesh = *GetMesh();
	int NN = mesh.Nodes();
	int NE = Size();

	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = -1;

	int n = 0, nne = 0;
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(m_Elem[i]);
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
		FEElement& el = mesh.Element(m_Elem[i]);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j)
		{
			int lid = mesh.Node(el.m_node[j]).m_ntag; assert(lid >= 0);
			lnode[nne + j] = lid;
		}
		nne += ne;
	}
}

void FESurface::GetNodeList(vector<int>& node, vector<int>& lnode)
{
	FEMeshBase& mesh = *GetMesh();
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
			if (mesh.Node(face.node[j]).m_ntag == -1) mesh.Node(face.node[j]).m_ntag = n++;
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
			int lid = mesh.Node(face.node[j]).m_ntag; assert(lid >= 0);
			lnode[nnf + j] = lid;
		}
		nnf += nf;
	}
}
