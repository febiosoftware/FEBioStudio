#pragma once
#include "FEModifier.h"
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
// Splits the selected elements
class FEQuadSplitModifier : public FEModifier
{
	struct DATA {
		FEElement_*	pe;
		int			ncase;
		int			nid;
		int			ntag;
	};

public:
	FEQuadSplitModifier();
	FEMesh* Apply(FEMesh* pm);

protected:
	FEElement_* neighbor(FEElement_* pe, int i) { return (pe->m_nbr[i] >= 0 ? &m_pm->Element(pe->m_nbr[i]) : 0); }

	bool is_split(FEElement_* pe, int i);

	bool can_split(FEElement_* pe, int i);

	bool have_to_split(FEElement_* pe, int i);

protected:
	FEMesh*				m_pm;
	vector<DATA>		m_Data;
};

//-----------------------------------------------------------------------------
// Split tri elements
class FETriSplitModifier : public FEModifier
{
	struct DATA {
		FEElement*	pe;
		int			ncase;
		int			nid;
		int			ntag;
	};

public:
	FETriSplitModifier();
	FEMesh* Apply(FEMesh* pm);

	void SetIterations(int niter) { m_niter = niter; }

protected:
	FEMesh* Split(FEMesh* pm);

	FEElement* neighbor(FEElement* pe, int i) { return (pe->m_nbr[i] >= 0 ? &m_pm->Element(pe->m_nbr[i]) : 0); }

	bool is_split(FEElement* pe, int i);

	bool can_split(FEElement* pe, int i);

	bool have_to_split(FEElement* pe, int i);

protected:
	FEMesh*				m_pm;
	vector<DATA>		m_Data;
	int					m_niter;
};

//-----------------------------------------------------------------------------
// Splits tetrahedral elements
class FETetSplitModifier : public FEModifier
{
public:
	FETetSplitModifier();
	FEMesh* Apply(FEMesh* pm);

protected:
	double	m_tol;	// tolerance for splitting facets
};


//-----------------------------------------------------------------------------
// Splits hexahedral elements
class FEHexSplitModifier : public FEModifier
{
public:
	FEHexSplitModifier();
	FEMesh* Apply(FEMesh* pm);

	void DoSurfaceSmoothing(bool b);

protected:
	double	m_tol;				// tolerance for splitting facets
	bool	m_smoothSurface;	// smooth surface
};
