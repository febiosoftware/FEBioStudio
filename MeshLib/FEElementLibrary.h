#pragma once
#include "FEElement.h"
#include <vector>

//=============================================================================
class FEElementLibrary
{
public:
	static void InitLibrary();

	static const FEElemTraits* GetTraits(int type);

private:
	FEElementLibrary() {}
	FEElementLibrary(const FEElementLibrary&) {}

	static void addElement(int ntype, int nshape, int nclass, int nodes, int faces, int edges);

private:
	static vector<FEElemTraits>	m_lib;
};

