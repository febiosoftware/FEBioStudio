// FEHollowSphere.h: interface for the FEHollowSphere class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEHOLLOWSPHERE_H__C62597A0_DACC_4A4B_B597_1413B2757330__INCLUDED_)
#define AFX_FEHOLLOWSPHERE_H__C62597A0_DACC_4A4B_B597_1413B2757330__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEMultiBlockMesh.h"

class GHollowSphere;

class FEHollowSphere : public FEMultiBlockMesh
{
public:
	enum { NDIV, NSEG, GR, GR2 };

public:
	FEHollowSphere(){}
	FEHollowSphere(GHollowSphere* po);
	FEMesh* BuildMesh();

protected:
	GHollowSphere*	m_pobj;
	int	m_nd, m_ns;
	double	m_gr;
	bool	m_br;
};

#endif // !defined(AFX_FEHOLLOWSPHERE_H__C62597A0_DACC_4A4B_B597_1413B2757330__INCLUDED_)
