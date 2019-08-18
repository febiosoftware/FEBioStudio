#pragma once
#include "FEMesher.h"

class GOCCObject;

class NetGenMesher : public FEMesher
{
public:
	enum {
		GRANULARITY,
		USELOCALH,
		GRADING,
		MAXELEMSIZE,
		NROPT2D,
		NROPT3D,
		SECONDORDER
	};

public:
	NetGenMesher();
	NetGenMesher(GOCCObject* po);

	FEMesh*	BuildMesh() override;

	MeshingProgress Progress() override;

	void Terminate() override;

private:
	GOCCObject*	m_occ;
};
