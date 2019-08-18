#pragma once
#include "FEMultiBlockMesh.h"

class GCylinderInBox;

class FECylinderInBox : public FEMultiBlockMesh
{
public:
	enum {NX, NY, NZ, NR, GZ, GR, BZ, BR, NELEM};

public:
	FECylinderInBox();
	FECylinderInBox(GCylinderInBox* po);
	FEMesh* BuildMesh();

protected:
	int		m_nelem;
	int		m_nx, m_ny, m_nz, m_nr;
	double	m_gz, m_gr;
	bool	m_bz, m_br;

	GCylinderInBox*	m_po;
};
