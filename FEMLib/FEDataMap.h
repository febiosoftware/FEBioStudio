#pragma once
#include "FEBase.h"

class FEModelComponent;

//-------------------------------------------------------------------
// Class that represents a data map, which is mapped to a model parameter
class FEDataMapGenerator : public FEBase
{
public:
	FEDataMapGenerator();

public:
	std::string		m_var;			// var value
	std::string		m_generator;	// generator name
	std::string		m_elset;		// element set
};

class FESurfaceToSurfaceMap : public FEDataMapGenerator
{
public:
	FESurfaceToSurfaceMap();

public:
	std::string		m_bottomSurface;
	std::string		m_topSurface;
	FELoadCurve		m_points;
};
