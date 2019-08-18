#pragma once
#include "FEMesher.h"

class GRing;

class FEShellRing : public FEMesher
{
public:
	enum { T, NSLICE, NDIV };

public:
	FEShellRing(){}
	FEShellRing(GRing* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return (j%m_ns)*(m_nr+1)+ i; }

protected:
	GRing* m_pobj;

	double	m_t;
	int		m_ns, m_nr;
};
