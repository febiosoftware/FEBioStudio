#pragma once
#include "FEMultiBlockMesh.h"

class GBox;

class FEBox : public FEMultiBlockMesh
{
public:
	// parameters
	enum { CTYPE, RATIO, NX, NY, NZ, NSEG, GX, GY, GZ, GR, GX2, GY2, GZ2, GR2, NELEM };

	// creation methods
	enum { SIMPLE, BUTTERFLY3D, BUTTERFLY2D };

public:
	FEBox(){}
	FEBox(GBox* po);
	FEMesh* BuildMesh();

protected:
	FEMesh* CreateRegular();
	FEMesh* CreateButterfly3D();
	FEMesh* CreateButterfly2D();

	FEMesh* CreateRegularHEX8 ();
	FEMesh* CreateRegularHEX20();
	FEMesh* CreateRegularHEX27();
	FEMesh* CreateRegularTET4 ();
	FEMesh* CreateRegularTET10();
	FEMesh* CreateRegularTET15();
	FEMesh* CreateRegularTET20();

protected:
	void BuildHexFaces(FEMesh* pm);
	void BuildTetFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	void BuildHex20Faces(FEMesh* pm, vector<int>& LUT);
	void BuildHex20Edges(FEMesh* pm, vector<int>& LUT);

	void BuildHex27Faces(FEMesh* pm);
	void BuildHex27Edges(FEMesh* pm);

	int NodeIndex(int i, int j, int k) 
	{
		return i*(m_ny+1)*(m_nz+1) + j*(m_nz+1) + k;
	}

	int NodeIndex2(int i, int j, int k, vector<int>& LUT)
	{
		int m = k + j*(2*m_nz+1) + i*(2*m_ny+1)*(2*m_nz+1);
		assert(LUT[m] != -1);
		return LUT[m];
	}

	int NodeIndex3(int i, int j, int k) 
	{
		return i*(2*m_ny+1)*(2*m_nz+1) + j*(2*m_nz+1) + k;
	}

protected:
	GBox*	m_pobj;

	int		m_ctype;
	int		m_nelem;	// element type
	double	m_r;
	int		m_nx, m_ny, m_nz;
	int		m_ns;
	double	m_gx, m_gy, m_gz, m_gr;
	bool	m_bx, m_by, m_bz, m_br;
};
