#pragma once
#include "FEMultiBlockMesh.h"
class GQuartDogBone;

class FEQuartDogBone : public FEMultiBlockMesh
{
public:
	enum {N_X, N_Y, N_Z};

public:
	FEQuartDogBone(){}
	FEQuartDogBone(GQuartDogBone* po);
	FEMesh* BuildMesh();

protected:
	GQuartDogBone* m_pobj;

};
