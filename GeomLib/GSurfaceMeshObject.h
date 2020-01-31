#pragma once
#include "GObject.h"
#include <MeshLib/FESurfaceMesh.h>

class tetgenio;

class GSurfaceMeshObject : public GObject
{
public:
	// create a new surface mesh object
	GSurfaceMeshObject(FESurfaceMesh* pm = 0);

	// create a new surface mesh object from a (meshed) object
	// this extracts the surface from the object
	GSurfaceMeshObject(GObject* po);

	// build the mesh
	FEMesh* BuildMesh() override;

	// update mesh for rendering
	void BuildGMesh() override;

	// update data structures
	void Update();

	// default mesher
	FEMesher* CreateDefaultMesher() override;

	// return the surface mesh
	FESurfaceMesh* GetSurfaceMesh();
	const FESurfaceMesh* GetSurfaceMesh() const;

	FEMeshBase* GetEditableMesh() override { return GetSurfaceMesh(); }
	FELineMesh* GetEditableLineMesh() override { return GetSurfaceMesh(); }

	// get the mesh of an edge curve
	FECurveMesh* GetFECurveMesh(int edgeId) override;

	void ReplaceSurfaceMesh(FESurfaceMesh* newMesh) override;

	// serialization
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	// attach another surface mesh object
	void Attach(const GSurfaceMeshObject* po, bool weld, double weldTolerance);

private:
	// Move this elsewhere or refactor
	bool build_tetgen_plc(FEMesh* pm, tetgenio& in);

private:
	void UpdateEdges();
	void UpdateNodes();
	void UpdateSurfaces();

private:
	FESurfaceMesh*	m_surfmesh;
};
