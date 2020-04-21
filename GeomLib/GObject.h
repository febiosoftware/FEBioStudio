#pragma once
#include "GBaseObject.h"
#include <FSCore/box.h>
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
// forward declarations
class FEPart;
class FESurface;
class FEEdgeSet;
class FENodeSet;
class FECurveMesh;
class FESurfaceMesh;
class FEMesher;
class FEMesh;
class FEMeshBase;
class FELineMesh;
class FENode;
class FEGroup;
class GLMesh;
class GObject;

//-----------------------------------------------------------------------------
// some exceptions that may be thrown in case of throuble
class GObjectException
{
public:
	GObjectException(GObject* po, const char* szerr) : m_po(po), m_szerr(szerr){}
	const char* ErrorMsg() { return m_szerr; }

	GObject* GetGObject() const { return m_po; }

protected:
	const char* m_szerr;
	GObject* m_po;
};

//-----------------------------------------------------------------------------
// GObject is the base class for all geometry objects
//
class GObject : public GBaseObject
{
	class Imp;

public:
	// --- C R E A T I O N ---

	// con/de-structor
	GObject(int ntype);
	virtual ~GObject(void);

	// create the object
	virtual void Create() {}

	// create a clone of this object
	virtual GObject* Clone() { return 0; }

	// Copy the geometry from another object
	// This does not copy the mesh from the passed object.
	// If a mesh exists then this mesh is deleted.
	virtual void Copy(GObject* po);

	// return type of Object
	int GetType() const;

	// get/set object color
	GLColor GetColor() const;
	void SetColor(const GLColor& c);

	// collapse the transformation
	void CollapseTransform();

	// serialization
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// --- M E S H I N G ---

	// set the mesher
	void SetFEMesher(FEMesher* pmesher);

	// retrieve the mesher
	virtual FEMesher* GetFEMesher();

	// create a default mesher
	virtual FEMesher* CreateDefaultMesher();

	// retrieve the FE mesh
	FEMesh* GetFEMesh();
	const FEMesh* GetFEMesh() const;

	// get the editable mesh
	virtual FEMeshBase* GetEditableMesh() { return 0; }

	// get the editable line mesh
	virtual FELineMesh* GetEditableLineMesh() { return 0; }

	// replace the current mesh
	void ReplaceFEMesh(FEMesh* pm, bool bup = false, bool bdel = false);

	// set the FE mesh
	void SetFEMesh(FEMesh* pm);

	// replace the current surface mesh
	virtual void ReplaceSurfaceMesh(FESurfaceMesh* newMesh);

	// retrieve an FE nodes from a GNode
	FENode* GetFENode(int gid);

	// build the FEMesh
	virtual FEMesh* BuildMesh();

	// delete the mesh
	void DeleteFEMesh();

	// update data
	bool Update(bool b =  true);

	// update GNodes (only GMeshObject uses this).
	void UpdateGNodes();

	// update FE mesh
	virtual void UpdateMesh() {}

	// get the mesh of an edge curve
	virtual FECurveMesh* GetFECurveMesh(int edgeId);

	// --- G E O M E T R Y ---

	// assign a material to all parts of this object
	void AssignMaterial(int matid);

	// assign a material to a part
	virtual void AssignMaterial(int partid, int matid);

	// render the geometry of the object (not the FE mesh)
//	virtual void Render(GLCanvas* pc);

	// build the render mesh
	virtual void BuildGMesh();

	// get the render mesh
	GLMesh*	GetRenderMesh();

	// get the local bounding box 
	BOX GetLocalBox() const;

	// get the global bounding box
	BOX GetGlobalBox() const;

public:
	// show the object
	void Show();

	// hide the object
	void Hide();

	// show (or hide) a part by index
	void ShowPart(int n, bool bshow);

	// show (or hide) a part by reference
	void ShowPart(GPart& part, bool bshow);

	// show all parts
	void ShowAllParts();

	// update the visibility of items (i.e. surfaces, edges, nodes, and mesh items)
	void UpdateItemVisibility();

	// is called whenever the selection has changed (default does nothing)
	virtual void UpdateSelection();

public:
	bool IsFaceVisible(const GFace* pf) const;

protected: // helper functions for building the GMesh
	void BuildFacePolygon     (GLMesh* glmesh, GFace& f);
	void BuildFaceExtrude     (GLMesh* glmesh, GFace& f);
	void BuildFaceQuad        (GLMesh* glmesh, GFace& f);
	void BuildFaceRevolve     (GLMesh* glmesh, GFace& f);
	void BuildFaceRevolveWedge(GLMesh* glmesh, GFace& f);
	void BuildEdgeMesh        (GLMesh* glmesh, GEdge& e);

protected:
	// set the render mesh
	void SetRenderMesh(GLMesh* mesh);

public:
	// --- G R O U P S ---
	int FEParts()    const;
	int FESurfaces() const;
	int FEEdgeSets() const;
	int FENodeSets() const;

	void AddFEPart   (FEPart*    pg);
	void AddFESurface(FESurface* pg);
	void AddFEEdgeSet(FEEdgeSet* pg);
	void AddFENodeSet(FENodeSet* pg);

	FEPart*    GetFEPart   (int n);
	FESurface* GetFESurface(int n);
	FEEdgeSet* GetFEEdgeSet(int n);
	FENodeSet* GetFENodeSet(int n);

	int RemoveFEPart(FEPart* pg);
	int RemoveFESurface(FESurface* pg);
	int RemoveFEEdgeSet(FEEdgeSet* pg);
	int RemoveFENodeSet(FENodeSet* pg);

	void InsertFEPart   (int n, FEPart*    pg);
	void InsertFESurface(int n, FESurface* pg);
	void InsertFEEdgeSet(int n, FEEdgeSet* pg);
	void InsertFENodeSet(int n, FENodeSet* pg);

	FEGroup* FindFEGroup(int nid);

	FESurface* FindFESurface(const string& szname);
	FENodeSet* FindFENodeSet(const string& szname);

	void ClearFEParts();
	void ClearFESurfaces();
	void ClearFEEdgeSets();
	void ClearFENodeSets();
	void ClearFEGroups();

	void RemoveEmptyFEGroups();

	GNode* FindNodeFromTag(int ntag);

private:
	Imp*	imp;
};
