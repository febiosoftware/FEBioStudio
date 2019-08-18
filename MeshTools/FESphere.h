// FESphere.h: interface for the FESphere class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FESPHERE_H__5C98D7BC_7F6F_4E61_893E_528D13C01885__INCLUDED_)
#define AFX_FESPHERE_H__5C98D7BC_7F6F_4E61_893E_528D13C01885__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEMultiBlockMesh.h"

class GSphere;

class FESphere : public FEMultiBlockMesh
{
public:
	enum { RATIO, NDIV, NSEG, GD, GR, GD2, GR2 };

public:
	FESphere(){}
	FESphere(GSphere* po);
	FEMesh* BuildMesh();

protected:
	GSphere* m_pobj;

	double	m_r;
	int	m_ndiv, m_nseg;
	double	m_gd, m_gr;
	bool	m_bd, m_br;
};

#endif // !defined(AFX_FESPHERE_H__5C98D7BC_7F6F_4E61_893E_528D13C01885__INCLUDED_)
