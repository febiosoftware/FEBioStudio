#pragma
#include "FEMesher.h"

//-----------------------------------------------------------------------------
class FEMesh;
class GObject;


//-----------------------------------------------------------------------------
// Class that implements an advancing front mesh generation algorithm in 2D.
// This algorithm generates triangular meshes.
//
class FEAdvancingFrontMesher2D : public FEMesher
{
public:
	// constructor
	FEAdvancingFrontMesher2D(GObject* po);

	// generate the mesh
	FEMesh* BuildMesh();

protected:
	GObject*	m_obj;
};
