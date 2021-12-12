#pragma once
#include "FEBase.h"

class FSModelComponent;

//-------------------------------------------------------------------
// Class that represents a data map, which is mapped to a model parameter
class FSDataMapGenerator : public FSBase
{
public:
	FSDataMapGenerator();

public:
	std::string		m_var;			// var value
	std::string		m_generator;	// generator name
	std::string		m_elset;		// element set
};

class FSSurfaceToSurfaceMap : public FSDataMapGenerator
{
public:
	FSSurfaceToSurfaceMap();

	void SetBottomSurface(const std::string& surfName);
	void SetTopSurface   (const std::string& surfName);

	std::string GetBottomSurface() const;
	std::string GetTopSurface() const;
};
