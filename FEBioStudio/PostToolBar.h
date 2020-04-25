#pragma once
#include <QToolBar>

class CMainWindow;

class UIPostToolBar;

class CPostToolBar : public QToolBar
{
public:
	CPostToolBar(CMainWindow* wnd);

	void Update();

	void CheckPlayButton(bool b);

	void CheckColorMap(bool b);
	bool IsColorMapActive();
	void ToggleColorMap();

	void SetDataField(int n);
	int GetDataField();

	void SetSpinValue(int n, bool blockSignals = false);

private:
	UIPostToolBar*	ui;
};

