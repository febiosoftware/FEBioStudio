#pragma once

#ifdef HAS_SSH
#include <QtCore/QThread>

class CSSHHandler;

class CSSHThread : public QThread
{
	Q_OBJECT

	void run() Q_DECL_OVERRIDE;

public:
	CSSHThread(CSSHHandler* sshHandler, int func = -1);

	void SetFuncName(int func);

public slots:
	void GetOutput(const QString&);
	void SendFinishedPart();
signals:
	void AddOutput(const QString&);
	void FinishedPart(CSSHHandler*);

private:
	CSSHHandler*	sshHandler;
	int				func;
};

#endif
