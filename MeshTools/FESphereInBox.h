#pragma once
#include "FEMultiBlockMesh.h"

class GSphereInBox;

class FESphereInBox : public FEMultiBlockMesh
{
public:
	enum {NX, NY, NZ, NR, GR, BR};

public:
	FESphereInBox();
	FESphereInBox(GSphereInBox* po);
	FEMesh* BuildMesh();

protected:
	GSphereInBox*	m_po;
};
