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
	int StartSSHSession();
	void EndSSHSession();

	int RunCommand(std::string command);
	int RunInteractiveNoWait(std::string command);
	int RunCommandList(std::vector<std::string> commands);

	int StartSFTPSession();
	int EndSFTPSession();
	int SendFile(std::string local, std::string remote);
	int SendFile(const char * buf, int bufSize, std::string remote);
	int GetFile(std::string local, std::string remote);
	int verify_knownhost();
	int authenticate();
	int CreateBashFile();

private:
	CFEBioJob* job;
	ssh_session session;
	sftp_session sftp;
	std::string remoteFileBase;
	std::string password;








};
















#endif
