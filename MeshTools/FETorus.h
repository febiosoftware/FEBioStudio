// FETorus.h: interface for the FETorus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FETORUS_H__C9A00B89_4CA3_4190_AB43_A4261079F556__INCLUDED_)
#define AFX_FETORUS_H__C9A00B89_4CA3_4190_AB43_A4261079F556__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEMesher.h"

class GTorus;

class FETorus : public FEMesher
{
public:
	enum { NDIV, NSEG };

public:
	FETorus(){}
	FETorus(GTorus* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j)
	{
		int ns = 4*m_ns;
		int nd = 2*m_nd;
		i = (i+nd/2 - 1)%(4*nd);
		int n = (j%ns)*((nd+1)*(nd+1)+nd*4*nd) + (nd+1)*(nd+1) + (nd-1)*4*nd + i;
		return n;
	}

protected:
	GTorus* m_pobj;
	int	m_nd, m_ns;
};

#endif // !defined(AFX_FETORUS_H__C9A00B89_4CA3_4190_AB43_A4261079F556__INCLUDED_)
