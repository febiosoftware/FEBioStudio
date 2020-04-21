#pragma once

namespace Post {

class FEPostModel;

class FEPlotMix
{
public:
	FEPlotMix(void);
	~FEPlotMix(void);

	FEPostModel* Load(const char** szfile, int n);

protected:
	void ClearStates(FEPostModel& fem);
};
}
