// FENNQuery.h: interface for the FENNQuery class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FENNQUERY_H__97ED0D65_BAD0_4D77_8D66_BDEB7D073930__INCLUDED_)
#define AFX_FENNQUERY_H__97ED0D65_BAD0_4D77_8D66_BDEB7D073930__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MathLib/math3d.h>
#include <vector>

//-----------------------------------------------------------------------------
//! This class is a helper class to locate the neirest neighbour on a surface

class FENNQuery  
{
public:
	struct NODE
	{
		int		i;	// index of node
		vec3d	r;	// position of node
		double	d1;	// distance to pivot 1
		double	d2;	// distance to pivot 2
	};

public:
	FENNQuery(std::vector<vec3d>* ps = 0);
	virtual ~FENNQuery();

	//! initialize search structures
	void Init();

	//! attach to a surface
	void Attach(std::vector<vec3d>* ps) { m_ps = ps; }

	//! find the neirest neighbour of r
	int Find(vec3d x);	

protected:
	int FindRadius(double r);

protected:
	std::vector<vec3d>*	m_ps;	//!< the node array to search
	std::vector<NODE>	m_bk;	// BK tree

	vec3d	m_q1;	// pivot 1
	vec3d	m_q2;	// pivot 2

	int		m_imin;	// last found index
};

#endif // !defined(AFX_FENNQUERY_H__97ED0D65_BAD0_4D77_8D66_BDEB7D073930__INCLUDED_)
