#pragma once
#include "PostLib/GLObject.h"

namespace Post {

class CGLModel;

//-----------------------------------------------------------------------------
// Base class for classes that map model data to a mesh attribute, e.g. nodal position, color, etc.
class CGLDataMap :	public CGLObject
{
public:
	CGLDataMap(CGLModel* po);
	~CGLDataMap(void);

	CGLModel* GetModel();

protected:
	CGLModel*	m_pgl;
};
}
