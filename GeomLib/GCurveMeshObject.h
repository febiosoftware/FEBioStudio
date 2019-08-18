#pragma once
#include "GObject.h"

class FECurveMesh;

// An object build from a FECurveMesh
class GCurveMeshObject : public GObject
{
public:
	// constructor
	GCurveMeshObject(FECurveMesh* pm = 0);

	// updates the GObject data structures based on the curve mesh
	void Update();

	// return the curve mesh
	FECurveMesh* GetCurveMesh();

	// get the mesh of an edge
	// (overridden from base class)
	FECurveMesh* GetFECurveMesh(int edgeId) override;

	// Serialization
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	FELineMesh* GetEditableLineMesh() override;

	void ClearMesh();

private:
	FECurveMesh*	m_curve;
};
