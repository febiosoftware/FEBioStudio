#pragma once
#include "FEModel.h"

namespace Post {

class FEPlotMix
{
public:
	FEPlotMix(void);
	~FEPlotMix(void);

	FEModel* Load(const char** szfile, int n);

protected:
	void ClearStates(FEModel& fem);
};
}
