#pragma once
#include <FSCore/FSObject.h>

class GObject;
class FEMesh;

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

class FEMesher : public FSObject
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

public:
	static FEMesher* Create(GObject* po, int classType);
};
