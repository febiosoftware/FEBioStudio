#pragma once
#include "FEModifier.h"

class FEExtrudeFaces : public FEModifier
{
public:
	FEExtrudeFaces();

	FEMesh* Apply(FEMesh* pm);
	FEMesh* Apply(FEGroup* pg);

	void SetExtrusionDistance(double D);

protected:
	void Extrude(FEMesh* pm, vector<int>& faceList);
};
