#pragma once
#include "FEMultiBlockMesh.h"

class GSphere;

class FESphere : public FEMultiBlockMesh
{
public:
	enum { RATIO, NDIV, NSEG, GD, GR, GD2, GR2 };

public:
	FESphere(){}
	FESphere(GSphere* po);
	FEMesh* BuildMesh();

protected:
	GSphere* m_pobj;

	double	m_r;
	int	m_ndiv, m_nseg;
	double	m_gd, m_gr;
	bool	m_bd, m_br;
};
