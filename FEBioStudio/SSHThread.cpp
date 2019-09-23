#include "SSHThread.h"
#include "Logger.h"
#include "SSHHandler.h"


CSSHThread::CSSHThread(CSSHHandler* sshHandler, int func) : sshHandler(sshHandler), func(func)
{
	QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
//	QObject::connect(this, &CSSHThread::finished, sshHandler, &CSSHHandler::ThreadFinished);
	QObject::connect(this, &CSSHThread::finished, this, &CSSHThread::SendFinishedPart);


	QObject::connect(sshHandler, &CSSHHandler::AddOutput, this, &CSSHThread::GetOutput);
	QObject::connect(this, &CSSHThread::AddOutput, &CLogger::AddOutputEntry);
}

void CSSHThread::SetFuncName(int func)
{
	this->func = func;
}

void CSSHThread::run()
{
	if(func == TARGET)
	{
		func = sshHandler->GetTargetFunction();
	}

	switch(func)
	{
	case ENDSSHSESSION:
		sshHandler->EndSSHSession();
		break;
	case STARTSSHSESSION:
		sshHandler->StartSSHSession();
		break;
	case VERIFYSERVER:
		sshHandler->VerifyKnownHost();
		break;
	case ADDTRUSETEDSERVER:
		sshHandler->AddTrustedServer();
		break;
	case AUTHENTICATE:
		sshHandler->Authenticate();
		break;
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

void CSSHThread::SendFinishedPart()
{
	emit FinishedPart(sshHandler);
}
