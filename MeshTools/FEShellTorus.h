#pragma once
#include "FEMesher.h"

class GTorus;

class FEShellTorus : public FEMesher  
{
public:
	enum { T, NDIV, NSEG };

public:
	FEShellTorus(){}
	FEShellTorus(GTorus* po);
	
	FEMesh* BuildMesh();

protected:
	void BuildNodes(FEMesh* pm);
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return (i%(4*m_nd))*(4*m_ns) + j%(4*m_ns); }

protected:
	GTorus*		m_pobj;

	double	m_t;
	int		m_nd, m_ns;
};
