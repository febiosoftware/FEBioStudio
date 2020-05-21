#pragma once
#include <QMainWindow>
#include "GraphWindow.h"
#include "Document.h"

class CMainWindow;
class CPostDocument;

namespace Post {
	class FEPostModel;
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
	CSummaryWindow(CMainWindow* wnd, CPostDocument* postDoc);

	void Update(bool breset, bool bfit = false) override;

private:
	RANGE EvalNodeRange(Post::FEPostModel& fem, int ntime, bool bsel);
	RANGE EvalEdgeRange(Post::FEPostModel& fem, int ntime, bool bsel);
	RANGE EvalFaceRange(Post::FEPostModel& fem, int ntime, bool bsel, bool bvol);
	RANGE EvalElemRange(Post::FEPostModel& fem, int ntime, bool bsel, bool bvol);

private slots:
	void onSelectionOnlyChanged(int n);

private:
	int		m_ncurrentData;
	bool	m_bselectionOnly;
};
