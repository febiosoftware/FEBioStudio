/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "MainTabBar.h"
#include "MainWindow.h"
#include "Document.h"

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

void CMainTabBar::addView(const std::string& name, CDocument* doc, bool makeActive, const std::string& iconName)
{
	m_docs.push_back(doc);
	if (iconName.empty())
		addTab(QString::fromStdString(name));
	else
		addTab(QIcon(QString::fromStdString(iconName)), QString::fromStdString(name));

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

int CMainTabBar::findView(const std::string& title)
{
	for (int i = 0; i < (int)m_docs.size(); ++i)
	{
		CDocument* doc = m_docs[i];
		if (doc->GetDocTitle() == title) return i;
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
