#pragma once

#ifdef HAS_SSH
#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include "FEBioJob.h"
#include "LaunchConfig.h"
#include "MainWindow.h"

enum status{FAILED=-1, OK=0, NEEDSPSWD=1};

class CSSHHandler : public QObject
{
	Q_OBJECT

public:
	CSSHHandler (CFEBioJob* job);
	~CSSHHandler (){}
	void Update(CLaunchConfig& oldConfig);

	void StartRemoteJob();
	void GetJobFiles();
	void GetQueueStatus();

	int StartSSHSession();
	void EndSSHSession();

	bool authenticatePassword();

	void SetPasswordLength(int l);
	size_t GetPasswordLength();

	void SetPasswdEnc(std::vector<unsigned char> passwdEnc);
	std::vector<unsigned char>& GetPasswdEnc();

signals:
	void AddOutput(const QString&);

private:
	int RunCommand(std::string command);
	int RunInteractiveNoRead(std::string command);
	int RunCommandList(std::vector<std::string> commands);

	int authenticatePubkey();

	int StartSFTPSession();
	int EndSFTPSession();
	int SendFile(std::string local, std::string remote);
	int SendFile(const char * buf, int bufSize, std::string remote);
	int GetFile(std::string local, std::string remote);
	int verify_knownhost();
	int CreateBashFile();

private:
	CFEBioJob* job;
	ssh_session session;
	sftp_session sftp;
	std::string remoteFileBase;
	int passwdLength;
	std::vector<unsigned char> passwdEnc;








};
















#endif
