#pragma once
#include "GItem.h"
#include "GBaseObject.h"
#include "MeshTools/FEMesher.h"
#include "MeshTools/GLMesh.h"
#include "MeshTools/GMaterial.h"
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
// forward declarations
class FEModel;
class FEPart;
class FESurface;
class FENodeSet;

class FECurveMesh;
class FESurfaceMesh;

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
	int GetType() { return m_ntype; }

	// get/set object color
	GLCOLOR GetColor() { return m_col; }
	void SetColor(const GLCOLOR& c) { m_col = c; }

	// collapse the transformation
	void CollapseTransform();

	// serialization
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// --- M E S H I N G ---

	// set the mesher
	void SetFEMesher(FEMesher* pmesher);

	// retrieve the mesher
	virtual FEMesher* GetMesher() { return m_pMesher; }

	// retrieve the FE mesh
	FEMesh* GetFEMesh() { return m_pmesh; }
	const FEMesh* GetFEMesh() const { return m_pmesh; }

	// get the editable mesh
	virtual FEMeshBase* GetEditableMesh() { return 0; }

	// get the editable line mesh
	virtual FELineMesh* GetEditableLineMesh() { return 0; }

	// replace the current mesh
	void ReplaceFEMesh(FEMesh* pm, bool bup = false, bool bdel = false);

	// replace the current surface mesh
	virtual void ReplaceSurfaceMesh(FESurfaceMesh* newMesh);

	// retrieve an FE nodes from a GNode
	FENode* GetFENode(int gid);

	// build the FEMesh
	virtual FEMesh* BuildMesh();

	// delete the mesh
	void DeleteFEMesh() { delete m_pmesh; m_pmesh = 0; }

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
	// set the FE mesh
	void SetFEMesh(FEMesh* pm);

public:
	// --- G R O U P S ---
	int FEParts()    const { return (int)m_pFEPart.size(); }
	int FESurfaces() const { return (int)m_pFESurf.size(); }
	int FEEdgeSets() const { return (int)m_pFEESet.size(); }
	int FENodeSets() const { return (int)m_pFENSet.size(); }

	void AddFEPart(FEPart*    pg) { m_pFEPart.push_back(pg); }
	void AddFESurface(FESurface* pg) { m_pFESurf.push_back(pg); }
	void AddFEEdgeSet(FEEdgeSet* pg) { m_pFEESet.push_back(pg); }
	void AddFENodeSet(FENodeSet* pg) { m_pFENSet.push_back(pg); }

	FEPart*    GetFEPart(int n) { return (n >= 0 && n<(int)m_pFEPart.size() ? m_pFEPart[n] : 0); }
	FESurface* GetFESurface(int n) { return (n >= 0 && n<(int)m_pFESurf.size() ? m_pFESurf[n] : 0); }
	FEEdgeSet* GetFEEdgeSet(int n) { return (n >= 0 && n<(int)m_pFEESet.size() ? m_pFEESet[n] : 0); }
	FENodeSet* GetFENodeSet(int n) { return (n >= 0 && n<(int)m_pFENSet.size() ? m_pFENSet[n] : 0); }

	int RemoveFEPart(FEPart* pg);
	int RemoveFESurface(FESurface* pg);
	int RemoveFEEdgeSet(FEEdgeSet* pg);
	int RemoveFENodeSet(FENodeSet* pg);

	void InsertFEPart(int n, FEPart* pg)    { m_pFEPart.insert(m_pFEPart.begin() + n, pg); }
	void InsertFESurface(int n, FESurface* pg) { m_pFESurf.insert(m_pFESurf.begin() + n, pg); }
	void InsertFEEdgeSet(int n, FEEdgeSet* pg) { m_pFEESet.insert(m_pFEESet.begin() + n, pg); }
	void InsertFENodeSet(int n, FENodeSet* pg) { m_pFENSet.insert(m_pFENSet.begin() + n, pg); }

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
	int	m_ntype;	//!< object type identifier
	GLCOLOR	m_col;	//!< color of object

protected:
	FEMesh*		m_pmesh;	//!< the mesh that this object manages
	FEMesher*	m_pMesher;	//!< the mesher builds the actual mesh
	GLMesh*		m_pGMesh;	//!< the mesh for rendering

	// --- Group Data ---
	//{
	std::vector<FEPart*>	m_pFEPart;
	std::vector<FESurface*>	m_pFESurf;
	std::vector<FEEdgeSet*>	m_pFEESet;
	std::vector<FENodeSet*>	m_pFENSet;
	//}
};
