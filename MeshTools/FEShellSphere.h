#pragma once
#include "FEMesher.h"

class GSphere;

class FEShellSphere : public FEMesher
{
public:
	enum {T, NDIV};

public:
	FEShellSphere(){}
	FEShellSphere(GSphere* po);
	virtual ~FEShellSphere();

	FEMesh* BuildMesh();

protected:
	int GetElementID(int i, int j, int n1, int n2, int n3, int n4)
	{
		int N = m_nd;
		int n = 2*i/N + 2*(2*j/N);
		switch (n)
		{
		case 0: return n1;
		case 1: return n2;
		case 2: return n3;
		case 3: return n4;
		}
		assert(false);
		return -1;
	}

	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j, int k);

protected:
	GSphere*	m_pobj;

	double	m_t;
	int		m_nd;
};
