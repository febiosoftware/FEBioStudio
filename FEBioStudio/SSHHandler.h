#pragma once

#ifdef HAS_SSH
#include "FEBioJob.h"
#include "LaunchConfig.h"
#include "MainWindow.h"
#include <QtCore/QObject>

class QSemaphore;

enum nextFunc{ENDSSHSESSION = -1, STARTSSHSESSION, VERIFYSERVER, ADDTRUSETEDSERVER, AUTHENTICATE, TARGET,
		STARTREMOTEJOB, GETJOBFILES, GETQUEUESTATUS, CREATEREMOTEDIR};

enum msgCode{FAILED=-1, OK, NEEDSPSWD, YESNODIALOG, DONE};

class CSSHHandler : public QObject
{
	Q_OBJECT

	class SSHData;

public:
	CSSHHandler (CFEBioJob* job);
	~CSSHHandler();
	void Update(CLaunchConfig& oldConfig);

	void StartSSHSession();
	void VerifyKnownHost();
	void AddTrustedServer();
	void Authenticate();
	void CreateRemoteDir();
	void EndSSHSession();

	void StartRemoteJob();
	void GetJobFiles();
	void GetQueueStatus();

	void SetPasswordLength(int l);
	size_t GetPasswordLength();

	void SetPasswdEnc(std::vector<unsigned char> passwdEnc);
	std::vector<unsigned char>& GetPasswdEnc();

	void SetMsgcode(int code);
	int GetMsgCode();

	int GetNextFunction();

	void SetTargetFunction(int func);
	int GetTargetFunction();

	QString GetMessage();

	void Orphan();

	bool IsBusy();

signals:
	void AddLogEntry(const QString&);
	void AddOutputEntry(const QString&);
	void ShowProgress(bool, QString message = "");
	void UpdateProgress(int);

private:
	int RunCommand(std::string command);
	int RunInteractiveNoRead(std::string command);
//	int RunCommandListOld(std::vector<std::string> commands);
	int RunCommandList(std::vector<std::string> commands);

	int authenticatePubkey();
	bool authenticatePassword();

	int StartSFTPSession();
	int EndSFTPSession();
	int CheckRemoteDir();
	int SendFile(std::string local, std::string remote);
	int SendFile(const char * buf, int bufSize, std::string remote);
	int GetFile(std::string local, std::string remote);

	std::string GetSFTPErrorText(int sftpErr);

	int CreateBashFile();
	int ParseCustomFile(std::vector<std::string>& commands);

	void ReplaceMacros(QString& string);

private:
	SSHData*	m_data;
};
#endif
