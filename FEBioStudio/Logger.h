#pragma once
#include <QtCore/QString>

class CMainWindow;

class CLogger
{
public:
	static void Instantiate(CMainWindow* mainWindow);
	static void AddLogEntry(const QString& txt);

private:
	CLogger() {}
	CLogger(CMainWindow* mainWindow) {m_mainWindow = mainWindow;}
	CLogger(CLogger const&) {}
    CLogger& operator=(CLogger const&) { return *this; }
	virtual ~CLogger(){}

	static CLogger* m_instance;
	static CMainWindow* m_mainWindow;
};
