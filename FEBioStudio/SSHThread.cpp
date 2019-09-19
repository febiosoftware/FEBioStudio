#include "SSHThread.h"
#include "Logger.h"


CSSHThread::CSSHThread(CSSHHandler* sshHandler, funcName func) : sshHandler(sshHandler), func(func)
{
	QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}


void CSSHThread::run()
{
	switch(func)
	{
	case STARTREMOTEJOB:
		sshHandler->StartRemoteJob();
		break;
	case GETJOBFILES:
		sshHandler->GetJobFiles();
		break;
	case GETQUEUESTATUS:
		sshHandler->GetQueueStatus();
		break;
	}
}


void CSSHThread::GetOutput(const QString& str)
{
	emit AddOutput(str);
}
