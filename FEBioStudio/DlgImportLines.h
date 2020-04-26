#pragma once
#include <QDialog>

//-----------------------------------------------------------------------------
class CMainWindow;

class CDlgImportLinesUI;
class CDlgImportPointsUI;

//-----------------------------------------------------------------------------
class CDlgImportLines : public QDialog
{
	Q_OBJECT

public:
	// constructor
	CDlgImportLines(CMainWindow* wnd);

private slots:
	void OnApply();
	void OnBrowse();

private:
	bool ReadOldFormat(const char* szfile);
	int ReadAng2Format(const char* szfile);

private:
	CDlgImportLinesUI*		ui;
};

//-----------------------------------------------------------------------------
class CDlgImportPoints : public QDialog
{
	Q_OBJECT

public:
	// constructor
	CDlgImportPoints(CMainWindow* wnd);

private slots:
	void OnApply();
	void OnBrowse();

private:
	CDlgImportPointsUI*	ui;
};
