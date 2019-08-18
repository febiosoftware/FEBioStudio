#pragma once
#include "GMesh.h"
#include "MeshLib/FEElement.h"

//-----------------------------------------------------------------------------
// This class adds rendering capabilities to the GMesh class
//
class GLMesh : public GMesh
{
public:
	GLMesh(void);
	GLMesh(GLMesh& m);
	~GLMesh(void);
};
