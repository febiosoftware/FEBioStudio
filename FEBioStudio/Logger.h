#pragma once
#include <QtCore/QObject>
#include <QtCore/QString>

class CMainWindow;

class CLogger : public QObject
{
	Q_OBJECT

public:
	static void Instantiate(CMainWindow* mainWindow);

public slots:
	static void AddLogEntry(const QString& txt);
	static void AddOutputEntry(const QString& txt);

private:
	CLogger() {}
	CLogger(CMainWindow* mainWindow) {m_mainWindow = mainWindow;}
	CLogger(CLogger const&) {}
    CLogger& operator=(CLogger const&) { return *this; }
	virtual ~CLogger(){}

	static CLogger* m_instance;
	static CMainWindow* m_mainWindow;
};
