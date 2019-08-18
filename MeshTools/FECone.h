// FECone.h: interface for the FECone class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FECONE_H__09C067BD_3377_4ABD_8359_0DA8E311E3DA__INCLUDED_)
#define AFX_FECONE_H__09C067BD_3377_4ABD_8359_0DA8E311E3DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEMultiBlockMesh.h"

class GCone;

class FECone : public FEMultiBlockMesh
{
public:
	enum { RB, NDIV, NSEG, NSTACK, GZ, GR, GZ2, GR2 };

public:
	FECone(){}
	FECone(GCone* po);
	FEMesh* BuildMesh();

protected:
	GCone*	m_pobj;

	double	m_Rb;
	int		m_nd, m_ns, m_nz;
	double	m_gz, m_gr;
	bool	m_bz, m_br;
};

#endif // !defined(AFX_FECONE_H__09C067BD_3377_4ABD_8359_0DA8E311E3DA__INCLUDED_)
