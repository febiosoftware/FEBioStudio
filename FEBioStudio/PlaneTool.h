#pragma once
#include "Tool.h"

class CDocument;
class CPlaneToolUI;
class CPlaneDecoration;

class CPlaneTool : public CAbstractTool
{
	Q_OBJECT

public:
	CPlaneTool(CMainWindow* wnd);

	void Update() override;

	QWidget* createUi() override;

private:
	void UpdateNormal();
	void addPoint(int n);

private slots:
	void on_change_node1();
	void on_change_node2();
	void on_change_node3();
	void onAlignView();

private:
	CPlaneToolUI*		ui;
};
