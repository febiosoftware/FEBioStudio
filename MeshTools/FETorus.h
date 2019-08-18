#pragma once
#include "FEMesher.h"

class GTorus;

class FETorus : public FEMesher
{
public:
	enum { NDIV, NSEG };

public:
	FETorus(){}
	FETorus(GTorus* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j)
	{
		int ns = 4*m_ns;
		int nd = 2*m_nd;
		i = (i+nd/2 - 1)%(4*nd);
		int n = (j%ns)*((nd+1)*(nd+1)+nd*4*nd) + (nd+1)*(nd+1) + (nd-1)*4*nd + i;
		return n;
	}

protected:
	GTorus* m_pobj;
	int	m_nd, m_ns;
};
