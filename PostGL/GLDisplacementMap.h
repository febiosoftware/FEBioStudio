#pragma once
#include "GLDataMap.h"
#include <vector>

namespace Post {

//-----------------------------------------------------------------------------
// This datamap maps vector data to nodal displacements
class CGLDisplacementMap : public CGLDataMap
{
public:
	CGLDisplacementMap(CGLModel* po);

	void Update(int ntime, float dt, bool breset);
	void Activate(bool b);

	void UpdateState(int ntime, bool breset = false);

	float GetScale() { return m_scl; }
	void SetScale(float f) { m_scl = f; }

	CPropertyList* propertyList();

public:
	float				m_scl;		//!< displacement scale factor
	std::vector<int>	m_ntag;
};
}
