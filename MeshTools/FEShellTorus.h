// FEShellTorus.h: interface for the FEShellTorus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FESHELLTORUS_H__3D9AFA4D_5467_4735_98D0_9AC03A257BCC__INCLUDED_)
#define AFX_FESHELLTORUS_H__3D9AFA4D_5467_4735_98D0_9AC03A257BCC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEMesher.h"

class GTorus;

class FEShellTorus : public FEMesher  
{
public:
	enum { T, NDIV, NSEG };

public:
	FEShellTorus(){}
	FEShellTorus(GTorus* po);
	
	FEMesh* BuildMesh();

protected:
	void BuildNodes(FEMesh* pm);
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	int NodeIndex(int i, int j) { return (i%(4*m_nd))*(4*m_ns) + j%(4*m_ns); }

protected:
	GTorus*		m_pobj;

	double	m_t;
	int		m_nd, m_ns;
};

#endif // !defined(AFX_FESHELLTORUS_H__3D9AFA4D_5467_4735_98D0_9AC03A257BCC__INCLUDED_)
