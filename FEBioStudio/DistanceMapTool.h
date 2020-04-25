#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CDocument;

class CDistanceMapToolUI;
class CCurvatureMapToolUI;

//-----------------------------------------------------------------------------
class CDistanceMapTool : public CAbstractTool
{
	Q_OBJECT

public:
	// constructor
	CDistanceMapTool(CMainWindow* wnd);

	// get the property list
	QWidget* createUi();

private slots:
	void OnAssign1();
	void OnAssign2();
	void OnApply();

private:
	CDistanceMapToolUI*		ui;
	friend class Props;
};

//-----------------------------------------------------------------------------
class CCurvatureMapTool : public CAbstractTool
{
	Q_OBJECT

public:
	// constructor
	CCurvatureMapTool(CMainWindow* wnd);

	// get the property list
	QWidget* createUi();

private slots:
	void OnAssign1();
	void OnAssign2();
	void OnApply();

private:
	CCurvatureMapToolUI*	ui;
	friend class Props;
};
