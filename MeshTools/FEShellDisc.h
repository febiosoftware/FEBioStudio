#pragma once
#include "FEMesher.h"

class GDisc;

class FEShellDisc : public FEMesher
{
public:
	enum {RATIO, T, NDIV, NSEG };

public:
	FEShellDisc(){}
	FEShellDisc(GDisc* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return i*(m_nd+1)+j; }
	int NodeIndex2(int i, int j);

protected:
	GDisc*	m_pobj;

	double	m_r, m_t;
	int		m_nd, m_nr;
};
