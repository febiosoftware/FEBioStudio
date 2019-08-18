#pragma once
#include "FEMultiBlockMesh.h"

class GTruncatedEllipsoid;

class FETruncatedEllipsoid : public FEMultiBlockMesh
{
public:
	enum {NSLICE, NSTACK, NDIV, GR, GR2};

public:
	FETruncatedEllipsoid(){}
	FETruncatedEllipsoid(GTruncatedEllipsoid* po);
	FEMesh* BuildMesh();

protected:
	GTruncatedEllipsoid*	m_pobj;
	int	m_ns, m_nz, m_nr;
	double	m_gr;
	bool	m_br;
};

