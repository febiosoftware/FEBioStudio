#pragma once
#include "FEModifier.h"

//-----------------------------------------------------------------------------
//! This class implements a FE modifier that welds nodes from a surface mesh
class FEWeldNodes : public FEModifier
{
public:
	//! constructor
	FEWeldNodes();

	//! Apply the weld modifier
	FEMesh* Apply(FEMesh* pm);

	void SetThreshold(double d);

protected:
	void UpdateNodes   (FEMesh* pm);
	void UpdateElements(FEMesh* pm);
	void UpdateFaces   (FEMesh* pm);
	void UpdateEdges   (FEMesh* pm);

private:
	vector<int>	m_order;
};
