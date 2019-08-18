#pragma once
#include "FEModifier.h"

//-----------------------------------------------------------------------------
class tetgenio;

//-----------------------------------------------------------------------------
//! This modifier creates a tet mesh using TetGen.
class FETetGenModifier : public FEModifier
{
public:
	FETetGenModifier();

	FEMesh* Apply(FEMesh* pm);
	FEMesh* Apply(FEGroup* pg);

private:
	bool build_tetgen_plc   (FEMesh* pm, tetgenio& in);
	bool build_tetgen_remesh(FEMesh* pm, tetgenio& in);

	FEMesh* CreateMesh(FEMesh* pm);
	FEMesh* RefineMesh(FEMesh* pm);

public:
	double	m_tol;

	bool	m_bremesh;

	// hole list
	vector<vec3d>	m_hole;
};
