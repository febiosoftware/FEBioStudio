#pragma once
#include "FEItem.h"
#include "math3d.h"

namespace Post {
//-----------------------------------------------------------------------------
// Class describing a node of the mesh
class FENode : public FEItem
{
public:
	FENode() { m_tex = 0.f; }

public:
	vec3f	m_r0;	// initial coordinates of node // TODO: I would like to remove this variable
	vec3f	m_rt;	// current coordinates of node
	bool	m_bext;	// interior or exterior node
	float	m_tex;	// nodal texture coordinate
};
}
