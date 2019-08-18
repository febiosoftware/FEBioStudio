#pragma once
#include <GeomLib/GObject.h>

//-----------------------------------------------------------------------------
//! The GMultiBox class is the geometry that is used to create a multi-block mesh. 
//! A multi-block mesh is basically a mesh that is composed of rectangular blocks. 
//! The user can edit the number of partitions by splitting edges, faces and parts.
//
class GMultiBox : public GObject
{
public:
	//! constructor
	GMultiBox();
	GMultiBox(GObject* po);

	//! update geometry
	bool Update(bool b = true);

	// build an FE mesh
	FEMesh* BuildMesh();

	// update the FE mesh
	void UpdateFEMesh();

public:
	// serialization
	void Save(OArchive& ar);
	void Load(IArchive& ar);

protected:
	//! create geometry
	void Create();
};
