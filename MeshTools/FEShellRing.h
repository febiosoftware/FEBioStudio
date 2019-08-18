// FEShellRing.h: interface for the FEShellRing class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FESHELLRING_H__C90362D6_8501_4CCE_9933_27B0BB73FBEA__INCLUDED_)
#define AFX_FESHELLRING_H__C90362D6_8501_4CCE_9933_27B0BB73FBEA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEMesher.h"

class GRing;

class FEShellRing : public FEMesher
{
public:
	enum { T, NSLICE, NDIV };

public:
	FEShellRing(){}
	FEShellRing(GRing* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return (j%m_ns)*(m_nr+1)+ i; }

protected:
	GRing* m_pobj;

	double	m_t;
	int		m_ns, m_nr;
};

#endif // !defined(AFX_FESHELLRING_H__C90362D6_8501_4CCE_9933_27B0BB73FBEA__INCLUDED_)
