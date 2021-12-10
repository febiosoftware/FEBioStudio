#pragma once
#include "FEBase.h"

class FEComponent;

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

	void SetBottomSurface(const std::string& surfName);
	void SetTopSurface   (const std::string& surfName);

	std::string GetBottomSurface() const;
	std::string GetTopSurface() const;
};
