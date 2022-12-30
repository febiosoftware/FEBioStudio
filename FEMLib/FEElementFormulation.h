#pragma once
#include "FEModelComponent.h"

class FSModel;

class FEElementFormulation : public FSModelComponent
{
public:
	FEElementFormulation(FSModel*);
};

class FESolidFormulation : public FEElementFormulation 
{
public: 
	FESolidFormulation(FSModel*);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;
};

class FEShellFormulation : public FEElementFormulation 
{
public: 
	FEShellFormulation(FSModel*);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;
};

class FEBeamFormulation : public FEElementFormulation
{
public:
	FEBeamFormulation(FSModel*);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;
};
