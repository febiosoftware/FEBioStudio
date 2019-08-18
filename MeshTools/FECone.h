#pragma once
#include "FEMultiBlockMesh.h"

class GCone;

class FECone : public FEMultiBlockMesh
{
public:
	enum { RB, NDIV, NSEG, NSTACK, GZ, GR, GZ2, GR2 };

public:
	FECone(){}
	FECone(GCone* po);
	FEMesh* BuildMesh();

protected:
	GCone*	m_pobj;

	double	m_Rb;
	int		m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	bool	m_bz, m_br;
};
