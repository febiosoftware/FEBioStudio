#pragma once
#include "PostLib/FEModel.h"
#include "PostLib/ColorMap.h"
#include <GLLib/GLTexture1D.h>
#include "PostLib/GLObject.h"
#include "PostLib/DataMap.h"

class GLLegendBar;

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

	virtual void UpdateTexture();

	void SetRenderOrder(int renderOrder);
	int GetRenderOrder() const;

private:
	int	m_renderOrder;
};

class CGLLegendPlot : public CGLPlot
{
public:
	CGLLegendPlot(CGLModel* po = 0);
	virtual ~CGLLegendPlot();

	void SetLegendBar(GLLegendBar* bar);
	GLLegendBar* GetLegendBar();

	void ChangeName(const std::string& name) override;

	bool ShowLegend() const;
	void ShowLegend(bool b);

private:
	GLLegendBar*	m_pbar;
};

}
