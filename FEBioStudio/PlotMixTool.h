#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CDocument;
class CPlotMixToolUI;
class CKinematToolUI;

class CPlotMixTool : public CAbstractTool
{
	Q_OBJECT

public:
	// constructor
	CPlotMixTool(CMainWindow* wnd);

	// get the property list
	QWidget* createUi();

private slots:
	void OnBrowse();
	void OnRemove();
	void OnMoveUp();
	void OnMoveDown();
	void OnApply();

private:
	CPlotMixToolUI*		ui;
	friend class Props;
};

class CKinematTool : public CAbstractTool
{
	Q_OBJECT

public:
	// constructor
	CKinematTool(CMainWindow* wnd);

	// get the property list
	QWidget* createUi();

private slots:
	void OnBrowse1();
	void OnBrowse2();
	void OnApply();

private:
	CKinematToolUI*		ui;
	friend class Props;
};
