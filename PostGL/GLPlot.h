#pragma once
#include "PostLib/FEModel.h"
#include "PostLib/ColorMap.h"
#include <GLLib/GLTexture1D.h>
#include "PostLib/GLObject.h"
#include "PostLib/DataMap.h"

namespace Post {

class CGLModel;

class CGLPlot : public CGLVisual
{
protected:
	struct SUBELEMENT
	{
		float   vf[8];		// vector values
		float    h[8][8];	// shapefunctions
	};

public:
	CGLPlot(CGLModel* po = 0);
	virtual ~CGLPlot();

	virtual void UpdateTexture() {}
};
}
