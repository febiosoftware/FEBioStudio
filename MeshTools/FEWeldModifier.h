#pragma once
#include "FEModifier.h"
#include "FESurfaceModifier.h"

//-----------------------------------------------------------------------------
//! This class implements a FE modifier that welds nodes from a surface mesh
class FEWeldNodes : public FEModifier
{
public:
	//! constructor
	FEWeldNodes();

	//! Apply the weld modifier
	FEMesh* Apply(FEMesh* pm);

	void SetThreshold(double d);

protected:
	void UpdateNodes   (FEMesh* pm);
	void UpdateElements(FEMesh* pm);
	void UpdateFaces   (FEMesh* pm);
	void UpdateEdges   (FEMesh* pm);

private:
	vector<int>	m_order;
};

//-----------------------------------------------------------------------------
//! This class implements a FE modifier that welds nodes from a surface mesh
class FEWeldSurfaceNodes : public FESurfaceModifier
{
public:
	//! constructor
	FEWeldSurfaceNodes();

	//! Apply the weld modifier
	FESurfaceMesh* Apply(FESurfaceMesh* pm) override;

	void SetThreshold(double d);

protected:
	void UpdateNodes(FESurfaceMesh* pm);
	void UpdateFaces(FESurfaceMesh* pm);

private:
	vector<int>	m_order;
};
