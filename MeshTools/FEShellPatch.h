#pragma once
#include "FEMesher.h"

class GPatch;
class GCylindricalPatch;

class FEShellPatch : public FEMesher
{
public:
	enum { T, NX, NY };

public:
	FEShellPatch(){}
	FEShellPatch(GPatch* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return i*(m_ny+1) + j; }

protected:
	GPatch* m_pobj;

	double	m_t;
	int		m_nx, m_ny;
};

class FECylndricalPatch : public FEMesher
{
public:
	enum { T, NX, NY };

public:
	FECylndricalPatch() {}
	FECylndricalPatch(GCylindricalPatch* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return i*(m_ny + 1) + j; }

protected:
	GCylindricalPatch* m_pobj;

	double	m_t;
	int		m_nx, m_ny;
};
