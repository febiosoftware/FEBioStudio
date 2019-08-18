#pragma once
#include "FEMesher.h"

class GTube;
class GTube2;

//-----------------------------------------------------------------------------
class FETube : public FEMesher
{
public:
	enum { NDIV, NSEG, NSTACK, ZZ, ZR, GZ2, GR2 };

public:
	FETube(){}
	FETube(GTube* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j, int k)
	{
		int nd = 4*m_nd;
		return k*(nd*(m_ns+1)) + (j%nd)*(m_ns+1) + i;
	}

protected:
	GTube*	m_pobj;

	int	m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	bool	m_bz, m_br; // double rezoning
};

//-----------------------------------------------------------------------------
class FETube2 : public FEMesher
{
public:
	enum { NDIV, NSEG, NSTACK, ZZ, ZR, GZ2, GR2 };

public:
	FETube2(){}
	FETube2(GTube2* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j, int k)
	{
		int nd = 4*m_nd;
		return k*(nd*(m_ns+1)) + (j%nd)*(m_ns+1) + i;
	}

protected:
	GTube2*	m_pobj;

	int	m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	bool	m_bz, m_br; // double rezoning
};
