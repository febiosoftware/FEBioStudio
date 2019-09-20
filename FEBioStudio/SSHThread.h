#pragma once

#ifdef HAS_SSH
#include <QtCore/QThread>

class CSSHHandler;

enum funcName{STARTREMOTEJOB, GETJOBFILES, GETQUEUESTATUS};

class CSSHThread : public QThread
{
	Q_OBJECT

	void run() Q_DECL_OVERRIDE;

public:
	CSSHThread(CSSHHandler* sshHandler, funcName func);

public slots:
	void GetOutput(const QString&);
signals:
	void AddOutput(const QString&);

private:
	CSSHHandler*	sshHandler;
	funcName		func;
};

#endif
