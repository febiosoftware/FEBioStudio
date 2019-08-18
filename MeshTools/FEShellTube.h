#pragma once
#include "FEMesher.h"

class GThinTube;

class FEShellTube : public FEMesher
{
public:
	enum {T, NDIV, NSTACK};

public:
	FEShellTube(){}
	FEShellTube(GThinTube* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return j*(4*m_nd)+ (i%(4*m_nd)); }

protected:
	GThinTube* m_pobj;

	double	m_t;
	int		m_nd, m_nz;
};
