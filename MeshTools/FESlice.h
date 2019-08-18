#pragma once
#include "FEMultiBlockMesh.h"
class GSlice;

class FESlice : public FEMultiBlockMesh
{
public:
	enum { NSLICE, NLOOP, NSTACK, ZZ, GR, GZ2, GR2};

public:
	FESlice(){}
	FESlice(GSlice* po);
	FEMesh* BuildMesh();

protected:
	void BuildWedgedFaces(FEMesh* pm);
	void BuildWedgedEdges(FEMesh* pm);
	void BuildWedgesNodes(FEMesh* pm);

	int NodeIndex(int i, int j, int k);

protected:
	GSlice*	m_pobj;

	int		m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	int		m_ctype;
	bool	m_bz, m_br;
};
