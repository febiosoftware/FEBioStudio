#pragma once
#include <QDialog>

class CDlgPlotMixUI;
class CMainWindow;

class CDlgPlotMix : public QDialog
{
	Q_OBJECT

public:
	CDlgPlotMix(CMainWindow* parent);

private slots:
	void OnBrowse();
	void OnRemove();
	void OnMoveUp();
	void OnMoveDown();
	void OnApply();

private:
	CDlgPlotMixUI*	ui;
};
