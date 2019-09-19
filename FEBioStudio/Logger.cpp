#include "Logger.h"
#include "MainWindow.h"

CLogger* CLogger::m_instance = nullptr;
CMainWindow* CLogger::m_mainWindow = nullptr;

void CLogger::Instantiate(CMainWindow* mainWindow)
{
	if(!m_instance)
	{
		m_instance = new CLogger(mainWindow);
	}
}

void CLogger::AddLogEntry(const QString& txt)
{
	m_mainWindow->AddLogEntry(txt);
}

void CLogger::AddOutputEntry(const QString& txt)
{
	m_mainWindow->AddOutputEntry(txt);
}
