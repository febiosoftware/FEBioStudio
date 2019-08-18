#pragma once
#include "FEMultiBlockMesh.h"

class GHollowSphere;

class FEHollowSphere : public FEMultiBlockMesh
{
public:
	enum { NDIV, NSEG, GR, GR2 };

public:
	FEHollowSphere(){}
	FEHollowSphere(GHollowSphere* po);
	FEMesh* BuildMesh();

protected:
	GHollowSphere*	m_pobj;
	int	m_nd, m_ns;
	double	m_gr;
	bool	m_br;
};
