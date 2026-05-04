#pragma once
#include "FEModelComponent.h"

class FSCoreStudy : public FSModelComponent
{
public:
	FSCoreStudy(FSModel* fem);

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};