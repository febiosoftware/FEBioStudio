#pragma once
#include "GLDataMap.h"
#include <vector>

namespace Post {

//-----------------------------------------------------------------------------
// This datamap maps vector data to nodal displacements
class CGLDisplacementMap : public CGLDataMap
{
	enum { DATA_FIELD, SCALE };

public:
	CGLDisplacementMap(CGLModel* po);

	void Update(int ntime, float dt, bool breset) override;
	void Activate(bool b) override;

	void UpdateState(int ntime, bool breset = false);

	float GetScale() { return m_scl; }
	void SetScale(float f) { m_scl = f; }

public:
	void UpdateData(bool bsave = true) override;

	void UpdateNodes();

public:
	float				m_scl;		//!< displacement scale factor
	std::vector<vec3f>	m_du;		//!< nodal displacements
	std::vector<int>	m_ntag;
};
}
