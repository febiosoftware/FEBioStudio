#include "stdafx.h"
#include "MainTabBar.h"
#include "MainWindow.h"
#include "PostDoc.h"

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

void CMainTabBar::addView(const std::string& name, CPostDoc* postDoc, bool makeActive)
{
	addTab(QString::fromStdString(name));
	m_docs.push_back(postDoc);
	assert(m_docs.size() == count());

	if (makeActive)
	{
		setActiveView(views() - 1);
	}
}

int CMainTabBar::findView(CPostDoc* doc)
{
	for (int i = 0; i < (int)m_docs.size(); ++i)
	{
		if (m_docs[i] == doc) return i;
	}
	return -1;
}

CPostDoc* CMainTabBar::getActiveDoc()
{
	int n = currentIndex();
	return m_docs[n];
}

void CMainTabBar::closeView(int n)
{
	assert(n != 0);
	assert(count() > 1);
	m_docs.erase(m_docs.begin() + n);
	removeTab(n);
}

CPostDoc* CMainTabBar::getPostDoc(int i)
{
	return m_docs[i];
}
