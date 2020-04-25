#pragma once
#include "Tool.h"

class CAreaCoverageToolUI;

class CAreaCoverageTool : public CAbstractTool
{
	Q_OBJECT

public:
	CAreaCoverageTool(CMainWindow* wnd);

	QWidget* createUi();

private slots:
	void OnAssign1();
	void OnAssign2();
	bool OnApply();

private:
	CAreaCoverageToolUI*	ui;
};
