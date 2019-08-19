#pragma once
#include <QTabBar>
#include <string>

class CMainWindow;

class CMainTabBar : public QTabBar
{
	Q_OBJECT

public:
	CMainTabBar(CMainWindow* wnd, QWidget* parent = nullptr);

	int views();
	int getActiveView();
	void setActiveView(int n);
	void addView(const std::string& name, bool makeActive = true);
};
