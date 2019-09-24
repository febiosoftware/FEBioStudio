#ifdef HAS_SSH
#include "stdafx.h"
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <string.h>
#include <string>
#include <fstream>
#include <fcntl.h>
#include "SSHHandler.h"
#include "Logger.h"
#include "Encrypter.h"
#include <FSCore/FSDir.h>
#include <iostream>
#include <QMessageBox>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QInputDialog>

#define MAX_XFER_BUF_SIZE 16384

#ifdef WIN32
#define S_IRWXU (0400|0200|0100)
#endif

class CSSHHandler::SSHData
{
public:
	CFEBioJob* job;
	ssh_session session;
	sftp_session sftp;
	std::string remoteFileBase;
	int passwdLength;
	std::vector<unsigned char> passwdEnc;

	int code;
	QString msg;

	int nextFunction;
	int targetFunc;

	bool isBusy;
};

CSSHHandler::CSSHHandler (CFEBioJob* job) : m_data(new CSSHHandler::SSHData) // @suppress("Class members should be properly initialized")
{
	m_data->job = job;
	m_data->passwdLength = -1;
	m_data->isBusy = false;

	// Get local .feb file name
	std::string localFile = FSDir::toAbsolutePath(job->GetFileName());

	QFileInfo info(localFile.c_str());
	std::string baseName = info.baseName().toStdString();

	// Get remote base file name
	m_data->remoteFileBase = job->GetLaunchConfig()->remoteDir + "/" + baseName;
}

CSSHHandler::~CSSHHandler() { delete m_data; }

void CSSHHandler::Update(CLaunchConfig& oldConfig)
{
	if(!m_data->job->GetLaunchConfig()->SameServer(oldConfig))
	{
		m_data->passwdEnc.clear();
		m_data->passwdLength = -1;
	}

	// Get local .feb file name
	std::string localFile = FSDir::toAbsolutePath(m_data->job->GetFileName());

	QFileInfo info(localFile.c_str());
	std::string baseName = info.baseName().toStdString();

	// Get remote base file name
	m_data->remoteFileBase = m_data->job->GetLaunchConfig()->remoteDir + "/" + baseName;
}

void CSSHHandler::StartRemoteJob()
{
	m_data->nextFunction = -1;
	m_data->code = DONE;

	// Get local .feb file name
	std::string localFile = FSDir::toAbsolutePath(m_data->job->GetFileName());

	// Get remote .feb file name
	std::string remoteFile = m_data->remoteFileBase + ".feb";

	// Send .feb file
	if(SendFile(localFile, remoteFile) != SSH_OK)
	{
		m_data->msg = QString(".feb file error: %1").arg(m_data->msg);
		return;
	}

	if(m_data->job->GetLaunchConfig()->type == REMOTE)
	{
		// Construct remote command
		std::string command = m_data->job->GetLaunchConfig()->path + " " + remoteFile;

		// Run the commands
		if(RunCommand(command) != SSH_OK)
		{
			m_data->code = FAILED;
			m_data->msg = "Failed to run remote command.";
			return;
		}
	}
	else if(m_data->job->GetLaunchConfig()->type == PBS)
	{
		// Create bash file for PBS queueing system
		if(CreateBashFile() != SSH_OK)
		{
			m_data->msg = QString("Bash file error: %1").arg(m_data->msg);
			return;
		}

		// Construct remote command
		std::string command = "qsub " + m_data->remoteFileBase + ".bash";

		if(RunInteractiveNoRead(command) != SSH_OK)
		{
			m_data->code = FAILED;
			m_data->msg = "Failed to run remote command.";
			return;
		}
	}
	else if(m_data->job->GetLaunchConfig()->type == SLURM)
	{
		// Create bash file for PBS queueing system
		if(CreateBashFile() != SSH_OK)
		{
			m_data->msg = QString("Bash file error: %1").arg(m_data->msg);
			return;
		}

		// Construct remote command
		std::string command = "sbatch " + m_data->remoteFileBase + ".bash";

		if(RunInteractiveNoRead(command) != SSH_OK)
		{
			m_data->code = FAILED;
			m_data->msg = "Failed to run remote command.";
			return;
		}
	}
}

void CSSHHandler::GetJobFiles()
{
	m_data->nextFunction = -1;
	m_data->code = DONE;

	// Get local .xplt file name
	std::string localFile = FSDir::toAbsolutePath(m_data->job->GetPlotFileName());

	// Get remote .xplt file name
	std::string remoteFile = m_data->remoteFileBase + ".xplt";

	// Get .xplt file
	if(GetFile(localFile, remoteFile) != SSH_OK)
	{
		m_data->msg = QString("xplt file error: %1").arg(m_data->msg);
		return;
	}

	// Get local .log file name
	localFile.replace(localFile.end()-4, localFile.end(), "log");

	// Get remote .log file name
	remoteFile = m_data->remoteFileBase + ".log";

	// Get .log file
	if(GetFile(localFile, remoteFile) != SSH_OK)
	{
		m_data->msg = QString("xplt file error: %1").arg(m_data->msg);
	}
}

void CSSHHandler::GetQueueStatus()
{
	m_data->nextFunction = -1;
	m_data->code = DONE;

	std::string command;

	if(m_data->job->GetLaunchConfig()->type == PBS)
	{
		command = "qstat";
	}
	else
	{
		command = "squeue";
	}

	std::vector<std::string> list;
	list.push_back(command);

	if(RunCommandList(list) != SSH_OK)
	{
		m_data->code = FAILED;
		m_data->msg = "Failed to run remote command.";
	}
}

void CSSHHandler::SetPasswordLength(int l)
{
	m_data->passwdLength = l;
}
size_t CSSHHandler::GetPasswordLength()
{
	return m_data->passwdLength;
}

void CSSHHandler::SetPasswdEnc(std::vector<unsigned char> passwdEnc)
{
	m_data->passwdEnc = passwdEnc;
}

std::vector<unsigned char>& CSSHHandler::GetPasswdEnc()
{
	return m_data->passwdEnc;
}

void CSSHHandler::SetMsgcode(int code)
{
	m_data->code = code;
}

int CSSHHandler::GetMsgCode()
{
	return m_data->code;
}

int CSSHHandler::GetNextFunction()
{
	return m_data->nextFunction;
}

QString CSSHHandler::GetMessage()
{
	return m_data->msg;
}

void CSSHHandler::SetTargetFunction(int func)
{
	m_data->targetFunc = func;
}

int CSSHHandler::GetTargetFunction()
{
	return m_data->targetFunc;
}

void CSSHHandler::StartSSHSession()
{
	m_data->isBusy = true;
	m_data->nextFunction = -1;

	int error;

	m_data->session = ssh_new();
	if(m_data->session == NULL)
	{
		m_data->code = FAILED;
		m_data->msg = "Could not initialize SSH session.";
		return;
	}

	ssh_options_set(m_data->session, SSH_OPTIONS_HOST, m_data->job->GetLaunchConfig()->server.c_str());
	ssh_options_set(m_data->session, SSH_OPTIONS_PORT, &(m_data->job->GetLaunchConfig()->port));
	ssh_options_set(m_data->session, SSH_OPTIONS_USER, m_data->job->GetLaunchConfig()->userName.c_str());

	// Connect to server
	error = ssh_connect(m_data->session);
	if (error != SSH_OK)
	{
		m_data->code = FAILED;
		m_data->msg = QString("Error connecting to server: %1").arg(ssh_get_error(m_data->session));
		return;
	}

	m_data->code = OK;
	m_data->nextFunction = VERIFYSERVER;
}

void CSSHHandler::EndSSHSession()
{
	ssh_disconnect(m_data->session);
	ssh_free(m_data->session);
	m_data->isBusy = false;
}

void CSSHHandler::VerifyKnownHost()
{
	enum ssh_known_hosts_e state;
	unsigned char *hash = NULL;
	ssh_key srv_pubkey = NULL;
	size_t hlen;
	char buf[10];
	char *hexa;
	char *p;
	int cmp;
	int rc;

	m_data->nextFunction = -1;

	rc = ssh_get_server_publickey(m_data->session, &srv_pubkey);
	if (rc < 0)
	{
		m_data->code = FAILED;
		m_data->msg = "Could not get server's public key.";
		return;
	}

	rc = ssh_get_publickey_hash(srv_pubkey,
			SSH_PUBLICKEY_HASH_SHA1,
			&hash,
			&hlen);
	ssh_key_free(srv_pubkey);
	if (rc < 0)
	{
		m_data->code = FAILED;
		m_data->msg = "Could not get server's public key hash.";
		return;
	}

	state = ssh_session_is_known_server(m_data->session);
	switch (state) {
	case SSH_KNOWN_HOSTS_OK:
		/* OK */
		break;
	case SSH_KNOWN_HOSTS_CHANGED:
		hexa = ssh_get_hexa(hash, hlen);

		m_data->code = FAILED;
		m_data->msg = QString("Host key for server changed: it is now: %1\n"
				"For security reasons, connection will be stopped").arg(hexa);

		ssh_clean_pubkey_hash(&hash);
		return;
	case SSH_KNOWN_HOSTS_OTHER:

		m_data->code = FAILED;
		m_data->msg = QString("The host key for this server was not found but an other"
				"type of key exists.\nAn attacker might change the default server key to confuse your client "
				"into thinking the key does not exist.");

		ssh_clean_pubkey_hash(&hash);
		return;
	case SSH_KNOWN_HOSTS_NOT_FOUND:
		emit AddOutputEntry("Could not find known host file.\n "
				"If you accept the host key here, the file will be automatically created.\n");


		/* FALL THROUGH to SSH_SERVER_NOT_KNOWN behavior */

	case SSH_KNOWN_HOSTS_UNKNOWN:
		hexa = ssh_get_hexa(hash, hlen);

		m_data->code = YESNODIALOG;
		m_data->msg = QString("The server is unknown. Do you trust the host key?\nPublic key hash: %1").arg(hexa);
		m_data->nextFunction = ADDTRUSETEDSERVER;

		ssh_string_free_char(hexa);
		ssh_clean_pubkey_hash(&hash);

		break;
	case SSH_KNOWN_HOSTS_ERROR:
		m_data->code = FAILED;
		m_data->msg = QString("Error : %1").arg(strerror(errno));
		ssh_clean_pubkey_hash(&hash);
		return;
	}

	ssh_clean_pubkey_hash(&hash);

	m_data->code = OK;
	m_data->nextFunction = AUTHENTICATE;
	return;
}

void CSSHHandler::AddTrustedServer()
{
	m_data->nextFunction = -1;

	int rc = ssh_session_update_known_hosts(m_data->session);
	if (rc < 0)
	{
		m_data->code = FAILED;
		m_data->msg = QString("Error : %1").arg(strerror(errno));
		return;
	}

	m_data->code = OK;
	m_data->nextFunction = AUTHENTICATE;
}

void CSSHHandler::Authenticate()
{
	if(m_data->passwdLength == -1)
	{
		m_data->code = authenticatePubkey();
		if(m_data->code == OK)
		{
			m_data->nextFunction = TARGET;
			return;
		}
	}

	std::string userAndServer = m_data->job->GetLaunchConfig()->userName + "@" + m_data->job->GetLaunchConfig()->server;
	m_data->msg = QString("Please enter a password for %1").arg(userAndServer.c_str());

	if (m_data->passwdLength != -1)
	{
		if(authenticatePassword())
		{
			m_data->code = OK;
			m_data->nextFunction = TARGET;
			return;
		}

		m_data->msg = QString("Could not authenticate with password.\nPlease enter a password for %1").arg(userAndServer.c_str());
	}

	m_data->code = NEEDSPSWD;
	m_data->nextFunction = AUTHENTICATE;
}

int CSSHHandler::authenticatePubkey()
{
	int rc;

	// Attempt Public Key authentication
	rc = ssh_userauth_publickey_auto(m_data->session, NULL, NULL);
	if (rc == SSH_AUTH_SUCCESS)
	{
		return OK;
	}

	return FAILED;
}

bool CSSHHandler::authenticatePassword()
{
	std::string password = CEncrypter::Instance()->Decrypt(m_data->passwdEnc, m_data->passwdLength);

	int rc = ssh_userauth_password(m_data->session, NULL, password.c_str());
	if (rc != SSH_AUTH_SUCCESS) return false;

	return true;
}

int CSSHHandler::StartSFTPSession()
{
	int rc;

	m_data->sftp = sftp_new(m_data->session);
	if (m_data->sftp == NULL)
	{
		m_data->msg = QString("Error allocating SFTP session: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;
		return SSH_ERROR;
	}

	rc = sftp_init(m_data->sftp);
	if (rc != SSH_OK)
	{
		m_data->msg = QString("Error initializing SFTP session: %1\n").arg(sftp_get_error(m_data->sftp));
		m_data->code = FAILED;
		sftp_free(m_data->sftp);
		return rc;
	}

	return rc;
}

int CSSHHandler::EndSFTPSession()
{
	sftp_free(m_data->sftp);
	return SSH_OK;
}

int CSSHHandler::SendFile(std::string local, std::string remote)
{
	int rc, nwritten, transferred = 0;
	int access_type = O_WRONLY | O_CREAT | O_TRUNC;
	sftp_file file;
	char buffer[MAX_XFER_BUF_SIZE];

	emit ShowProgress(true);
	emit AddLogEntry(QString("Sending remote file %1 ... ").arg(remote.c_str()));

	if(StartSFTPSession() != SSH_OK)
	{
		return SSH_ERROR;
	}

	file = sftp_open(m_data->sftp, remote.c_str(),
			access_type, S_IRWXU);
	if (file == NULL)
	{
		m_data->msg = QString("Can't open file for writing: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;
		return SSH_ERROR;
	}

	ifstream fin(local.c_str(), ios::binary);

	while(fin)
	{
		fin.read(buffer, sizeof(buffer));

		if(fin.gcount() > 0)
		{
			ssize_t nwritten = sftp_write(file, buffer, fin.gcount());
			if (nwritten != fin.gcount())
			{
				m_data->msg = QString("Can't write data to file: %1\n").arg(ssh_get_error(m_data->session));
				m_data->code = FAILED;
				sftp_close(file);
				return SSH_ERROR;
			}

			transferred += nwritten;
			emit UpdateProgress(transferred*100/fin.tellg());
		}
	}

	rc = sftp_close(file);
	if (rc != SSH_OK)
	{
		m_data->msg = QString("Can't close the written file: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;
		return rc;
	}

	emit ShowProgress(false);
	emit AddLogEntry(QString("success!\n"));

	return EndSFTPSession();
}

int CSSHHandler::SendFile(const char * buf, int bufSize, std::string remote)
{
	int rc, nwritten;
	int access_type = O_WRONLY | O_CREAT | O_TRUNC;
	sftp_file file;
	QString error;

	emit AddLogEntry(QString("Sending remote file %1 ... ").arg(remote.c_str()));

	if(StartSFTPSession() != SSH_OK)
	{
		return SSH_ERROR;
	}

	file = sftp_open(m_data->sftp, remote.c_str(),
			access_type, S_IRWXU);
	if (file == NULL)
	{
		m_data->msg = QString("Can't open file for writing: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;
		return SSH_ERROR;
	}

	nwritten = sftp_write(file, buf, bufSize);
	if (nwritten != bufSize)
	{
		m_data->msg = QString("Can't write data to file: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;
		sftp_close(file);
		return SSH_ERROR;
	}

	rc = sftp_close(file);
	if (rc != SSH_OK)
	{
		m_data->msg = QString("Can't close the written file: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;
		return rc;
	}

	emit AddLogEntry(QString("success!\n"));

	return EndSFTPSession();
}

int CSSHHandler::GetFile(std::string local, std::string remote)
{
	int rc, nbytes, transferred = 0;
	int access_type = O_RDWR;
	sftp_file file;
	sftp_attributes attributes;
	char buffer[MAX_XFER_BUF_SIZE];

	emit ShowProgress(true);
	emit AddLogEntry(QString("Fetching remote file %1 ... ").arg(remote.c_str()));

#ifdef WIN32
	HANDLE fileHandle;
	DWORD nwritten;
#else
	int fd, nwritten;
#endif

	StartSFTPSession();

	file = sftp_open(m_data->sftp, remote.c_str(), access_type, S_IRWXU);
	if (file == NULL)
	{
		m_data->msg = QString("Can't open file for writing: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;
		return SSH_ERROR;
	}

	attributes = sftp_fstat(file);

#ifdef WIN32
	fileHandle = CreateFileA(local.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
	fd = open(local.c_str(), O_RDWR | O_CREAT, S_IRWXU);
	if (fd < 0) {
		m_data->msg = QString("Can't open file for writing: %1\n").arg(strerror(errno));
		m_data->code = FAILED;
		return SSH_ERROR;
	}
#endif


	while(true)
	{
		nbytes = sftp_read(file, buffer, sizeof(buffer));

		if (nbytes == 0)
		{
			break; // EOF
		}
		else if (nbytes < 0)
		{
			m_data->msg = QString("Error while reading file: %1\n").arg(ssh_get_error(m_data->session));
			m_data->code = FAILED;
			sftp_close(file);

#ifdef WIN32
			CloseHandle(fileHandle);
#endif
			return SSH_ERROR;
		}

#ifdef WIN32
		WriteFile(fileHandle, buffer, nbytes, &nwritten, NULL);
#else
		nwritten = write(fd, buffer, nbytes);

#endif
		if (nwritten != nbytes)
		{
			m_data->msg = QString("Error writing: %1\n").arg(strerror(errno));
			m_data->code = FAILED;
			sftp_close(file);

#ifdef WIN32
			CloseHandle(fileHandle);
#endif
			return SSH_ERROR;
		}

		transferred += nwritten;
		emit UpdateProgress(transferred*100/attributes->size);
	}

	rc = sftp_close(file);
	if (rc != SSH_OK) {
		m_data->msg = QString("Can't close the read file: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;

#ifdef WIN32
		CloseHandle(fileHandle);
#endif

		return rc;
	}

#ifdef WIN32
	CloseHandle(fileHandle);
#endif

	emit ShowProgress(false);
	emit AddLogEntry(QString("success!\n"));

	return EndSFTPSession();
}

int CSSHHandler::RunCommand(std::string command)
{
	ssh_channel channel;
	int rc;
	char buffer[256];
	int nbytes;

	channel = ssh_channel_new(m_data->session);
	if (channel == NULL)
		return SSH_ERROR;

	rc = ssh_channel_open_session(channel);
	if (rc != SSH_OK)
	{
		ssh_channel_free(channel);
		return rc;
	}

	command.push_back('\n');

	rc = ssh_channel_request_exec(channel, command.c_str());
	if (rc != SSH_OK)
	{
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return rc;
	}

	nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0);
	while (nbytes > 0)
	{
		buffer[nbytes] = '\0';
		emit AddOutputEntry(buffer);

		nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0);
	}

//	if (nbytes < 0)
//	{
//		ssh_channel_close(channel);
//		ssh_channel_free(channel);
//		return SSH_ERROR;
//	}

	ssh_channel_send_eof(channel);
	ssh_channel_close(channel);
	ssh_channel_free(channel);
	return SSH_OK;
}

int CSSHHandler::RunInteractiveNoRead(std::string command)
{
	ssh_channel channel;
	int rc;
	char buffer[256];
	int nbytes;

	channel = ssh_channel_new(m_data->session);
	if (channel == NULL)
		return SSH_ERROR;

	rc = ssh_channel_open_session(channel);
	if (rc != SSH_OK)
	{
		ssh_channel_free(channel);
		return rc;
	}

	rc = ssh_channel_request_shell(channel);
	if (rc != SSH_OK) return rc;

	command.push_back('\n');

	rc = ssh_channel_write(channel, command.c_str(), sizeof(char)*command.length());

#ifndef WIN32
	sleep(1);
#endif

	ssh_channel_send_eof(channel);
	ssh_channel_close(channel);
	ssh_channel_free(channel);
	return SSH_OK;
}

int CSSHHandler::RunCommandList(std::vector<std::string> commands)
{
	ssh_channel channel, channel2;
	int rc;
	char buffer[256];
	char buffer2[256];
	int nbytes;

	channel = ssh_channel_new(m_data->session);
	if (channel == NULL)
		return SSH_ERROR;

	rc = ssh_channel_open_session(channel);
	if (rc != SSH_OK)
	{
		ssh_channel_free(channel);
		return rc;
	}

	channel2 = ssh_channel_new(m_data->session);
	if (channel2 == NULL)
		return SSH_ERROR;

	rc = ssh_channel_open_session(channel2);
	if (rc != SSH_OK)
	{
		ssh_channel_free(channel2);
		return rc;
	}

	rc = ssh_channel_request_shell(channel);
	if (rc != SSH_OK) return rc;

	rc = ssh_channel_request_shell(channel2);
	if (rc != SSH_OK) return rc;

	rc = ssh_channel_write(channel, "echo $$\n", sizeof("echo $$\n"));

	nbytes = ssh_channel_read(channel, buffer2, sizeof(buffer2), 0);

	string pidString(buffer2);

	int pid = stoi(pidString);

	for(string command : commands)
	{
		command.push_back('\n');

		rc = ssh_channel_write(channel, command.c_str(), sizeof(char)*command.length());

		bool cont = true;
		while(cont || nbytes > 0)
		{
#ifndef WIN32
			usleep(5000);
#endif

			string test("pgrep -P ");

			test.append(to_string(pid));
			test.append(" | wc -l\n");

			rc = ssh_channel_write(channel2, test.c_str(), sizeof(char)*test.length());

			ssh_channel_read(channel2, buffer2, sizeof(buffer2), 0);

			string linesString(buffer2);

			int lines = stoi(linesString);

			if(lines < 1) cont = false;

			nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer) - 1, 0);
			buffer[nbytes] = '\0';
			emit AddOutputEntry(buffer);
		}
	}

	ssh_channel_send_eof(channel);
	ssh_channel_close(channel);
	ssh_channel_free(channel);

	ssh_channel_send_eof(channel2);
	ssh_channel_close(channel2);
	ssh_channel_free(channel2);
	return SSH_OK;
}


int CSSHHandler::CreateBashFile()
{
	std::string bashString("#!/bin/bash\n\n");

	CLaunchConfig* config = m_data->job->GetLaunchConfig();

	if(config->type == PBS)
	{
		bashString += "#PBS -l nodes=1:ppn=" + std::to_string(config->procNum) + "\n\n";
		bashString += "#PBS -l mem=" + std::to_string(config->ram) + "\n\n";
		bashString += "#PBS -l walltime=" + config->walltime + "\n\n";
		if(!config->jobName.empty())
		{
			bashString += "#PBS -N " + config->jobName + "\n\n";
			bashString += "#PBS -o " + config->remoteDir + "/" + config->jobName + "_stdout.log\n\n";
			bashString += "#PBS -e " + config->remoteDir + "/" + config->jobName + "_stderr.log\n\n";
		}
		bashString += "export OMP_NUM_THREADS=" + std::to_string(config->procNum) + "\n";
		bashString += config->path + " " + m_data->remoteFileBase + ".feb";
	}
	else if(config->type == SLURM)
	{
		bashString += "#SBATCH -N 1\n\n";
		bashString += "#SBATCH -n " + std::to_string(config->procNum) + "\n\n";
		bashString += "#SBATCH --mem " + std::to_string(config->ram) + "\n\n";
		bashString += "#SBATCH -t " + config->walltime + "\n\n";
		if(!config->jobName.empty())
		{
			bashString += "#SBATCH -J " + config->jobName + "\n\n";
			bashString += "#SBATCH -o " + config->remoteDir + "/" + config->jobName + "_stdout.log\n\n";
			bashString += "#SBATCH -e " + config->remoteDir + "/" + config->jobName + "_stderr.log\n\n";
		}
		bashString += "export OMP_NUM_THREADS=" + std::to_string(config->procNum) + "\n";
		bashString += config->path + " " + m_data->remoteFileBase + ".feb";
	}

	std::string remote = m_data->remoteFileBase + ".bash";

	return SendFile(bashString.c_str(), sizeof(char)*bashString.length(), remote);

}

bool CSSHHandler::IsBusy()
{
	return m_data->isBusy;
}


#endif
