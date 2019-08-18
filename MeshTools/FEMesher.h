#pragma once

#include "FEObject.h"
#include "MeshLib/FEMesh.h"

struct MeshingProgress
{
	double		percent;
	const char*	task;

	MeshingProgress()
	{
		percent = 0.0;
		task = 0;
	}
};

//-----------------------------------------------------------------------------
// The FEMesher class takes a geometry object and converts it to a finite
// element mesh.
//

class FEMesher : public FEObject
{
	enum {PARAMS};

public:
	// constructor
	FEMesher();

	// desctructor
	virtual ~FEMesher();

	// build the mesh
	virtual FEMesh*	BuildMesh() = 0;

	// save/load
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// return progress
	virtual MeshingProgress Progress();
	virtual void Terminate();
};
