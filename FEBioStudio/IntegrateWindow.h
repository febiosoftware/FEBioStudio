#pragma once
#include <QMainWindow>
#include <vector>
#include "Document.h"
#include "GraphWindow.h"

class CMainWindow;
class CPlotData;

namespace Post {
	class FEPostMesh;
	class FEState;
	class CGLPlaneCutPlot;
}

class CIntegrateWindow : public CGraphWindow
{
	Q_OBJECT

public:
	CIntegrateWindow(CMainWindow* wnd, CPostDocument* postDoc);

	void Update(bool breset = true, bool bfit = false) override;

private:
	void UpdateSourceOptions();

	void UpdateIntegral();
	void IntegrateSelection(CPlotData& data);
	void IntegratePlaneCut(Post::CGLPlaneCutPlot* pp, CPlotData& data);

private:
	std::vector<Post::CGLPlaneCutPlot*>	m_src;
	int		m_nsrc;
	bool	m_updating;
};
