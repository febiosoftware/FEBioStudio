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

void CMainTabBar::addView(const std::string& name, CDocument* doc, bool makeActive)
{
	m_docs.push_back(doc);
	addTab(QString::fromStdString(name));
	assert(m_docs.size() == count());

	if (makeActive)
	{
		setActiveView(views() - 1);
	}
}

int CMainTabBar::findView(CDocument* doc)
{
	for (int i = 0; i < (int)m_docs.size(); ++i)
	{
		if (m_docs[i] == doc) return i;
	}
	return -1;
}

CDocument* CMainTabBar::getActiveDoc()
{
	int n = currentIndex();
	return ((n >= 0) && (n < m_docs.size()) ? m_docs[n] : nullptr);
}

CDocument* CMainTabBar::getDocument(int n)
{
	return ((n >= 0) && (n < m_docs.size()) ? m_docs[n] : nullptr);
}

void CMainTabBar::closeView(int n)
{
	m_docs.erase(m_docs.begin() + n);
	removeTab(n);
}
