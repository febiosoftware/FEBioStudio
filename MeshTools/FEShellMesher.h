#pragma once
#include "FEMesher.h"

class FEShellMesher : public FEMesher
{
public:
	FEShellMesher();
	FEShellMesher(GObject* po);

	// build the mesh
	FEMesh*	BuildMesh() override;

private:
	GObject*	m_po;
};
