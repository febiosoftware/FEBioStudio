#pragma once
#include <FEMLib/FEModelComponent.h>

class FSModel;

class FEElementFormulation : public FSModelComponent
{
public:
	FEElementFormulation(FSModel*);
};

class FESolidFormulation : public FEElementFormulation 
{
public: FESolidFormulation(FSModel*);
};

class FEShellFormulation : public FEElementFormulation 
{
public: FEShellFormulation(FSModel*);
};
