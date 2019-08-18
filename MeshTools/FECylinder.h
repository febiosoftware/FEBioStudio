#pragma once
#include "FEMultiBlockMesh.h"

class GCylinder;
class GCylinder2;

class FECylinder : public FEMultiBlockMesh
{
public:
	enum {RATIO, NDIV, NSEG, NSTACK, ZZ, GR, CTYPE, GZ2, GR2};

	// creation types
	enum { BUTTERFLY, WEDGED };

public:
	FECylinder(){}
	FECylinder(GCylinder* po);
	FEMesh* BuildMesh();

protected:
	FEMesh* BuildButterfly();
	FEMesh* BuildWedged();

	void BuildWedgedFaces(FEMesh* pm);
	void BuildWedgedEdges(FEMesh* pm);
	void BuildWedgesNodes(FEMesh* pm);

	int NodeIndex(int i, int j, int k) 
	{
		if (j==0) return i*(m_nd*m_ns+1);
		else return i*(m_nd*m_ns+1) + 1+(j-1)*m_nd + k%m_nd; 
	}

protected:
	GCylinder*	m_pobj;

	double	m_r;
	int		m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	int		m_ctype;
	bool	m_bz, m_br;
};

class FECylinder2 : public FEMultiBlockMesh
{
public:
	enum {RATIO, NDIV, NSEG, NSTACK, ZZ, GR, CTYPE, GZ2, GR2};

	// creation types
	enum { BUTTERFLY, WEDGED };

public:
	FECylinder2(){}
	FECylinder2(GCylinder2* po);
	FEMesh* BuildMesh();

protected:
	FEMesh* BuildButterfly();
	FEMesh* BuildWedged();

	void BuildWedgedFaces(FEMesh* pm);
	void BuildWedgedEdges(FEMesh* pm);
	void BuildWedgesNodes(FEMesh* pm);

	int NodeIndex(int i, int j, int k) 
	{
		if (j==0) return i*(m_nd*m_ns+1);
		else return i*(m_nd*m_ns+1) + 1+(j-1)*m_nd + k%m_nd; 
	}

protected:
	GCylinder2*	m_pobj;

	double	m_r;
	int		m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	int		m_ctype;
	bool	m_bz, m_br;
};
