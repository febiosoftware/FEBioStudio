#include "FEComponent.h"

FEComponent::FEComponent()
{
}

FEDataMap* FEComponent::CreateMap(const std::string& paramName, Param_Type type)
{
	FEDataMap* map = new FEDataMap(this, paramName, type);
	m_map.push_back(map);
	return map;
}

void FEComponent::DeleteMap(FEDataMap* map)
{
	for (int i=0; i<(int)m_map.size(); ++i)
	{
		FEDataMap* map_i = m_map[i];
		if (map_i == map)
		{
			m_map.erase(m_map.begin() + i);
			delete map;
			return;
		}
	}

	assert(false);
}
