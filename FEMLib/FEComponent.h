#pragma once
#include "FEBase.h"
#include "FEDataMap.h"

//-----------------------------------------------------------------------------
// Base class for components that are applied to item lists.
// The parameters of these components can be mapped.
class FEComponent : public FEBase
{
public:
	FEComponent();

	FEDataMap* CreateMap(const std::string& paramName, Param_Type type);

	int DataMaps() const { return (int) m_map.size(); }

	FEDataMap* GetDataMap(int i) { return m_map[i]; }

	void DeleteMap(FEDataMap* map);

private:
	std::vector<FEDataMap*>	m_map;
};
