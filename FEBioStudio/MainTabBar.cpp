#include "stdafx.h"
#include "MainTabBar.h"
#include "MainWindow.h"

CMainTabBar::CMainTabBar(CMainWindow* wnd, QWidget* parent) : QTabBar(parent)
{
	setExpanding(false);
	setTabsClosable(true);
}

int CMainTabBar::views()
{
	return count();
}

int CMainTabBar::getActiveView()
{
	return currentIndex();
}

void CMainTabBar::setActiveView(int n)
{
	setCurrentIndex(n);
}

void CMainTabBar::addView(const std::string& name, bool makeActive)
{
	addTab(QString::fromStdString(name));
	if (makeActive)
	{
		setActiveView(views() - 1);
	}
}
