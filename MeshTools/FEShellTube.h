// FEShellTube.h: interface for the FEShellTube class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FESHELLTUBE_H__6B56FDB4_2521_40C6_BEE9_5103CF279544__INCLUDED_)
#define AFX_FESHELLTUBE_H__6B56FDB4_2521_40C6_BEE9_5103CF279544__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEMesher.h"

class GThinTube;

class FEShellTube : public FEMesher
{
public:
	enum {T, NDIV, NSTACK};

public:
	FEShellTube(){}
	FEShellTube(GThinTube* po);
	FEMesh* BuildMesh();

protected:
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return j*(4*m_nd)+ (i%(4*m_nd)); }

protected:
	GThinTube* m_pobj;

	double	m_t;
	int		m_nd, m_nz;
};


#endif // !defined(AFX_FESHELLTUBE_H__6B56FDB4_2521_40C6_BEE9_5103CF279544__INCLUDED_)
