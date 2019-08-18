#pragma once
#include "FEModifier.h"

class FECurveMesh;
class GCurveMeshObject;
class DynamicMesh2D;

class FECurveIntersect2D : public FEModifier
{
public:
	FECurveIntersect2D();

	void SetCurveMesh(GCurveMeshObject* pc);

	FEMesh* Apply(FEMesh* pm);

private:
	void BuildMesh(DynamicMesh2D& dyna, FEMesh* pm);
	FEMesh* BuildFEMesh(DynamicMesh2D& dyna);

private:
	GCurveMeshObject*	m_pc;
};
