#pragma once
#include "GObject.h"

//-----------------------------------------------------------------------------
class FESurfaceMesh;

//-----------------------------------------------------------------------------
// The GMeshObject defines the geometry of a "naked" FE mesh. That is, a mesh
// that does not correspond to a geometry object. There are two ways to create
// GMeshObjects. The first is to import a FE model from a file. Most FE models imported 
// are considered naked. The second is by converting a geometry object that has a mesh
// to a GMeshObject. 
// 
// This object defines the geometry (such as parts, faces, 
// edges, nodes) using this FE geometry. This has one major limitation and that
// is that when the user changes the mesh, it can not be assumed that the geometry
// stays the same and therefore all geometry dependant quantities (e.g. boundary 
// conditions) need to be removed. The GMeshObject is the only object for which
// the user can define their own partitions.

class GMeshObject : public GObject
{
public:
	// Constructor for creating a GMeshObject from a "naked" mesh
	GMeshObject(FEMesh* pm);

	// Constructor for creating a GMeshObject from a "naked" surface mesh 
	// (this creates a shell mesh)
	GMeshObject(FESurfaceMesh* pm);

	// Constructor for converting a GObject to a GMeshObject
	GMeshObject(GObject* po);

	// update geometry information
	bool Update(bool b = true) override;

	int MakeGNode(int n);

	int AddNode(vec3d r);

	virtual FEMesh* BuildMesh() override;

	// serialization
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	// cloning
	GObject* Clone() override;

	FEMeshBase* GetEditableMesh() override;
	FELineMesh* GetEditableLineMesh() override;

	// detach an element selection
	GMeshObject* DetachSelection();

public:
	void Attach(GObject* po, bool bweld, double tol);

	void DeletePart(GPart* pg);

protected:
	void BuildGMesh() override;

protected:
	void UpdateParts();
	void UpdateSurfaces();
	void UpdateEdges();
	void UpdateNodes();
};

GMeshObject* ExtractSelection(GObject* po);
