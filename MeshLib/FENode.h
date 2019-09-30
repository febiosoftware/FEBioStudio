#pragma once
#include "FEItem.h"
#include "MathLib/math3d.h"

//-----------------------------------------------------------------------------
// The FENode class stores the nodal data.
//
class FENode : public FEItem
{
public:
	// constructor
	FENode();

	// copy constructor
	FENode(const FENode& n);

	// assignment operator
	void operator = (const FENode& n);

	// set/get position
	void pos(const vec3d& p) { r = p; }
	const vec3d& pos() const { return r; }

public:
	vec3d	r;			// nodal position
};
