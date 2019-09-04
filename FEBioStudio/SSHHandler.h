#pragma once

#ifdef HAS_SSH
#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include <FEBioJob.h>
#include <LaunchConfig.h>
#include <MainWindow.h>


class CSSHHandler
{
public:
	CSSHHandler (CFEBioJob* job);
	~CSSHHandler (){}

	void StartRemoteJob();

	void GetJobFiles();

private:
	void StartSSHSession();
	void EndSSHSession();

	int RunCommand(std::string command);
	int RunInteractiveNoWait(std::string command);
	int RunCommandList(std::vector<std::string> commands);

	int SendFile(std::string local, std::string remote);
	int SendFile(const char * buf, int bufSize, std::string remote);
	int GetFile(std::string local, std::string remote);
	int verify_knownhost();
	int authenticate_pubkey();
	int CreateBashFile();

private:
	CFEBioJob* job;
	ssh_session session;
	std::string remoteFileBase;








};
















#endif
