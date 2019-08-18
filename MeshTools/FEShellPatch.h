// FEShellPatch.h: interface for the FEShellPatch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FESHELLPATCH_H__70AA0BE1_5F71_402B_82AB_BEE83BE46DBA__INCLUDED_)
#define AFX_FESHELLPATCH_H__70AA0BE1_5F71_402B_82AB_BEE83BE46DBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEMesher.h"

class GPatch;

class FEShellPatch : public FEMesher
{
public:
	enum { T, NX, NY };

public:
	FEShellPatch(){}
	FEShellPatch(GPatch* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return i*(m_ny+1) + j; }

protected:
	GPatch* m_pobj;

	double	m_t;
	int		m_nx, m_ny;
};

#endif // !defined(AFX_FESHELLPATCH_H__70AA0BE1_5F71_402B_82AB_BEE83BE46DBA__INCLUDED_)
