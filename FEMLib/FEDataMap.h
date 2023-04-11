#pragma once
#include "FEBase.h"

class FEComponent;

//-------------------------------------------------------------------
// Class that represents a data map, which is mapped to a model parameter
class FEDataMapGenerator : public FEBase
{
public:
	enum Type {
		NODE_DATA_GENERATOR = 1,
		ELEM_DATA_GENERATOR = 2,
		FACE_DATA_GENERATOR = 3
	};

public:
	FEDataMapGenerator();

	int Type() const { return m_type; }

public:
	unsigned int	m_type;
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

class FESurfaceConstVec3d : public FEDataMapGenerator
{
public:
	FESurfaceConstVec3d();

	vec3d Value() const;
	void SetValue(const vec3d& v);
};
