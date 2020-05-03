#pragma once
#include <QString>
#include <QTextBrowser>

class CMainWindow;

class CWelcomePage : public QTextBrowser
{
public:
	CWelcomePage(CMainWindow* wnd);

	void Refresh();

private:
	CMainWindow*	m_wnd;
};
