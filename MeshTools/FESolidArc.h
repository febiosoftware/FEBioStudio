#pragma once
#include "FEMesher.h"

class GSolidArc;

class FESolidArc : public FEMesher
{
public:
	enum { NDIV, NSEG, NSTACK, ZZ, ZR, GZ2, GR2 };

public:
	FESolidArc(){}
	FESolidArc(GSolidArc* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j, int k)
	{
		int nd = 4*m_nd;
		return k*((m_nd+1)*(m_ns+1)) + j*(m_ns+1) + i;
	}

protected:
	GSolidArc*	m_pobj;

	int	m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	bool	m_bz, m_br; // double rezoning
};
