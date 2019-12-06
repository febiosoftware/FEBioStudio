#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CDocument;

class CImportLinesToolUI;
class CImportPointsToolUI;

//-----------------------------------------------------------------------------
class CImportLinesTool : public CAbstractTool
{
	Q_OBJECT

public:
	// constructor
	CImportLinesTool();

	// get the property list
	QWidget* createUi();

private slots:
	void OnApply();
	void OnBrowse();

private:
	bool ReadOldFormat(const char* szfile);
	int ReadAng2Format(const char* szfile);

private:
	CImportLinesToolUI*		ui;
	friend class Props;
};

//-----------------------------------------------------------------------------
class CImportPointsTool : public CAbstractTool
{
	Q_OBJECT

public:
	// constructor
	CImportPointsTool();

	// get the property list
	QWidget* createUi();

private slots:
	void OnApply();
	void OnBrowse();

private:
	CImportPointsToolUI*	ui;
	friend class Props;
};
