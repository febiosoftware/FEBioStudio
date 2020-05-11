#pragma once
#include <QString>
#include <QTextBrowser>

class CMainWindow;

class CWelcomePage : public QTextBrowser
{
public:
	CWelcomePage(CMainWindow* wnd);

	void Refresh();

	void Close();

private:
	CMainWindow*	m_wnd;
};
