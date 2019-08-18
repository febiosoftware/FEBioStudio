// FEShellDisc.h: interface for the FEShellDisc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FESHELLDISC_H__6D5169AF_267B_4A5D_ADEB_E829F601CDD6__INCLUDED_)
#define AFX_FESHELLDISC_H__6D5169AF_267B_4A5D_ADEB_E829F601CDD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEMesher.h"

class GDisc;

class FEShellDisc : public FEMesher
{
public:
	enum {RATIO, T, NDIV, NSEG };

public:
	FEShellDisc(){}
	FEShellDisc(GDisc* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return i*(m_nd+1)+j; }
	int NodeIndex2(int i, int j);

protected:
	GDisc*	m_pobj;

	double	m_r, m_t;
	int		m_nd, m_nr;
};

#endif // !defined(AFX_FESHELLDISC_H__6D5169AF_267B_4A5D_ADEB_E829F601CDD6__INCLUDED_)
