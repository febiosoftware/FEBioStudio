#pragma once
#include <FSCore/FSObject.h>
#include "FEItemListBuilder.h"

class FENode;
class FEFace;
class FEElement;
class FEMesh;
class GObject;

//-----------------------------------------------------------------------------
// Base class for all groups
//
class FEGroup : public FEItemListBuilder
{
public:
	FEGroup(GObject* po, int ntype);
	virtual ~FEGroup();

	FEMesh* GetMesh();
	void SetGObject(GObject* pm);
	GObject* GetGObject();

	int Type() { return m_ntype; }

	FENodeList* BuildNodeList() { return 0; }
	FEFaceList* BuildFaceList() { return 0; }
	FEElemList* BuildElemList() { return 0; }

public:
	// These functions were added when FEGroup was added to FECore. 
	// The mesh ID is the ID of the GObject that owns the mesh. In FECore
	// GObjects are not known, so a different mechanism was implemented
	// to find the parent object (and mesh) when reading the archive. Now, only the
	// mesh ID (i.e. the object ID) is written and read and a function
	// higher up the call stack needs to find the correct parent mesh. 
	// I hope one day to remove this construction.
//	void SetMeshID(int nid) { m_objID = nid; }
	int GetObjectID() const { return m_objID; }

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

public:
	int m_ntag;

protected:
	GObject*	m_pObj;		//!< pointer to the object this group belongs to
	int			m_objID;	//!< The ID of the object that owns the mesh (only used when reading the IArchive)
};

//-----------------------------------------------------------------------------
// class describing a group of elements
//
class FEPart : public FEGroup
{
public:
	FEPart(GObject* po) : FEGroup(po, FE_PART) {}
	FEPart(GObject* po, const vector<int>&  elset);
	~FEPart(){}

	void CreateFromMesh();

	FEElemList* BuildElemList();
	FENodeList* BuildNodeList();

	FEItemListBuilder* Copy();
	void Copy(FEPart* pg);
};

//-----------------------------------------------------------------------------
// CLASS: FESurface
// class describing a group of faces
//
class FESurface : public FEGroup
{
public:
	FESurface(GObject* po) : FEGroup(po, FE_SURFACE) {}
	FESurface(GObject* po, vector<int>& face);
	~FESurface(){}

	FENodeList* BuildNodeList();
	FEFaceList* BuildFaceList();

	FEItemListBuilder* Copy();
	void Copy(FESurface* pg);
};

//-----------------------------------------------------------------------------
// class describing a group of edges
class FEEdgeSet : public FEGroup
{
public:
	FEEdgeSet(GObject* po) : FEGroup(po, FE_EDGESET) {}
	FEEdgeSet(GObject* po, vector<int>& edge);
	~FEEdgeSet(){}

	FENodeList* BuildNodeList();

	FEItemListBuilder* Copy();
	void Copy(FEEdgeSet* pg);
};

//-----------------------------------------------------------------------------
// CLASS: FENodeSet
// class describing a group of nodes
//
class FENodeSet : public FEGroup
{
public:
	FENodeSet(GObject* po) : FEGroup(po, FE_NODESET) {}
	FENodeSet(GObject* po, const vector<int>& node);
	~FENodeSet(){}

	void CreateFromMesh();

	FEItemListBuilder* Copy();
	void Copy(FENodeSet* pg);

	FENodeList* BuildNodeList();
};
