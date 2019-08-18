#pragma once
#include "FESurfaceModifier.h"

class FEEdgeCollapse : public FESurfaceModifier
{
public:
	FEEdgeCollapse();

	FESurfaceMesh* Apply(FESurfaceMesh* mesh);

	void SetTolerance(double tol) { m_tol = tol; }

private:
	double	m_tol;
};
