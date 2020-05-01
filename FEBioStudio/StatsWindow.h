#pragma once
#include <QMainWindow>
#include "Document.h"
#include "GraphWindow.h"

class CMainWindow;

class CStatsWindow : public CGraphWindow
{
	Q_OBJECT

public:
	CStatsWindow(CMainWindow* wnd, CPostDocument* postDoc);

	void Update(bool breset = true, bool bfit = false) override;

private:
	void UpdateSelection(bool breset);
};
