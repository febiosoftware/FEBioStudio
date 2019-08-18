#pragma once
#include <GeomLib/GObject.h>

//-----------------------------------------------------------------------------
class GSketch;

//-----------------------------------------------------------------------------
class GPLCObject : public GObject
{
public:
	GPLCObject();
	~GPLCObject(void){}

	void Create(GSketch& s);

	FEMesh* BuildMesh();
};
