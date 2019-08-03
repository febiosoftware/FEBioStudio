#pragma once
#include <QMainWindow>
#include "GraphWindow.h"
#include "Document.h"

class CMainWindow;
class CPostDoc;

namespace Post {
	class FEModel;
}

class CSummaryWindow : public CGraphWindow
{
	Q_OBJECT

public:
	struct RANGE
	{
		float	fmax, fmin, favg;
	};

public:
	CSummaryWindow(CMainWindow* wnd, CPostDoc* postDoc);

	void Update(bool breset, bool bfit = false) override;

private:
	RANGE EvalNodeRange(Post::FEModel& fem, int ntime, bool bsel);
	RANGE EvalEdgeRange(Post::FEModel& fem, int ntime, bool bsel);
	RANGE EvalFaceRange(Post::FEModel& fem, int ntime, bool bsel, bool bvol);
	RANGE EvalElemRange(Post::FEModel& fem, int ntime, bool bsel, bool bvol);

private:
	int		m_ncurrentData;
};
