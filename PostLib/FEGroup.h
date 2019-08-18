#pragma once
#include "FEElement.h"
#include <vector>
using namespace std;

namespace Post {
//-----------------------------------------------------------------------------
// forward declaration of the mesh class
class FEMeshBase;

//-----------------------------------------------------------------------------
// Base class that describes a group of mesh items. 
class FEGroup
{
public:
	FEGroup(FEMeshBase* pm) { m_pm = pm; m_szname[0] = 0; }
	virtual ~FEGroup(void) {}

	const char* GetName();
	void SetName(const char* szname);

	FEMeshBase* GetMesh() const { return m_pm; }

protected:
	FEMeshBase*	m_pm;	// pointer to the parent mesh
	char	m_szname[64];
};

//-----------------------------------------------------------------------------
// A doman is an internal organization of elements. A domain is created for each material.
class FEDomain
{
public:
	FEDomain(FEMeshBase* pm);

	void SetMatID(int matid);
	int GetMatID() const { return m_nmat; }

	int Type() const { return m_ntype; }

	int Faces() { return (int) m_Face.size(); }
	FEFace& Face(int n);

	int Elements() { return (int) m_Elem.size(); }
	FEElement& Element(int n);

	void Reserve(int nelems, int nfaces);

	void AddElement(int n) { m_Elem.push_back(n); }
	void AddFace   (int n) { m_Face.push_back(n); }

protected:
	FEMeshBase*	m_pm;
	int			m_nmat;	// material index
	int			m_ntype;
	vector<int>	m_Face;	// face indices 
	vector<int>	m_Elem;	// element indices
};

//-----------------------------------------------------------------------------
// Class that describes a group of elements
class FEPart : public FEGroup
{
public:
	FEPart(FEMeshBase* pm) : FEGroup(pm) {}

	int Size() const { return (int) m_Elem.size(); }

	void GetNodeList(vector<int>& node, vector<int>& lnode);

public:
	vector<int>	m_Elem;	// element indices
};

//-------------------------------------------------------------------------
// Class that describes a group of faces
class FESurface : public FEGroup
{
public:
	FESurface(FEMeshBase* pm) : FEGroup(pm) {}

	int Size() const { return (int) m_Face.size(); }

	void GetNodeList(vector<int>& node, vector<int>& lnode);

public:
	vector<int>	m_Face;	// face indices
};

//-------------------------------------------------------------------------
//! Class that defines a node set
class FENodeSet : public FEGroup
{
public:
	FENodeSet(FEMeshBase* pm) : FEGroup(pm){}

public:
	vector<int>	m_Node;
};
}
