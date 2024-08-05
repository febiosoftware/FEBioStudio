/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/


#include "stdafx.h"

#ifdef HAS_SSH
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#endif

#include <string.h>
#include <string>
#include <fstream>
#include <fcntl.h>
#include "SSHHandler.h"
#include "Logger.h"
#include "Encrypter.h"
#include <FSCore/FSDir.h>
#include <QMessageBox>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QInputDialog>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QStandardPaths>

using std::ifstream;
using std::ios;

#define MAX_XFER_BUF_SIZE 16384

#ifdef WIN32
#define S_IRWXU (0400|0200|0100)
#endif

#ifdef HAS_SSH


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
	std::string newDir;

	bool isBusy;
	bool orphan;
};



CSSHHandler::CSSHHandler (CFEBioJob* job) : m_data(new CSSHHandler::SSHData) // @suppress("Class members should be properly initialized")
{
	m_data->job = job;
	m_data->passwdLength = -1;
	m_data->isBusy = false;
	m_data->orphan = false;

	// Get local .feb file name
	std::string localFile = FSDir::expandMacros(job->GetFEBFileName());

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
	std::string localFile = FSDir::expandMacros(m_data->job->GetFEBFileName());

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
	std::string localFile = FSDir::expandMacros(m_data->job->GetFEBFileName());

	// Get remote .feb file name
	std::string remoteFile = m_data->remoteFileBase + ".feb";

	// Send .feb file
	int error = SendFile(localFile, remoteFile);
	if(error == SSH_ERROR)
	{
		m_data->msg = QString(".feb file error: %1").arg(m_data->msg);
		return;
	}
	else if(error == YESNODIALOG)
	{
		return;
	}

	if(m_data->job->GetLaunchConfig()->type == REMOTE)
	{
		// Construct remote command
		std::vector<std::string> commands;
		commands.push_back(m_data->job->GetLaunchConfig()->path + " " + remoteFile);

		// Run the commands
//		if(RunCommand(command) != SSH_OK)
		if(RunCommandList(commands) != SSH_OK)
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
	else if(m_data->job->GetLaunchConfig()->type == CUSTOM)
	{
		std::vector<std::string> commands;
		if(ParseCustomFile(commands) != SSH_OK) return;

		if(commands.empty())
		{
			m_data->code = FAILED;
			m_data->msg = QString("File: %1\n contained no commands.");
			return;
		}

		if(RunCommandList(commands) != SSH_OK)
		{
			m_data->code = FAILED;
			m_data->msg = "Failed to run remote commands.";
			return;
		}
	}
}

void CSSHHandler::GetJobFiles()
{
	m_data->nextFunction = -1;
	m_data->code = DONE;

	// Get local .xplt file name
	std::string localFile = FSDir::expandMacros(m_data->job->GetPlotFileName());

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

//	if(RunCommand(command) != SSH_OK)
//	{
//		m_data->code = FAILED;
//		m_data->msg = "Failed to run remote command.";
//	}
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

void CSSHHandler::Orphan()
{
	m_data->orphan = true;
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

	QString sshDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.ssh";
	ssh_options_set(m_data->session, SSH_OPTIONS_SSH_DIR, sshDir.toStdString().c_str());

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
	char *hexa;
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

void CSSHHandler::CreateRemoteDir()
{
	if(sftp_mkdir(m_data->sftp, m_data->newDir.c_str(), S_IRWXU) != SSH_OK)
	{
		m_data->msg = QString("Cannot create remote directory:\n\n%1\n\nSFTP Error: %2")
				.arg(m_data->newDir.c_str()).arg(GetSFTPErrorText(sftp_get_error(m_data->sftp)).c_str());
		m_data->code = FAILED;
		return;
	}

	m_data->code = OK;
	m_data->nextFunction = TARGET;
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

int CSSHHandler::CheckRemoteDir()
{
	QString remoteDir = QString(m_data->job->GetLaunchConfig()->remoteDir.c_str());
	QStringList dirList = remoteDir.split("/");

	QString currentDir = "/";

	for(QString dir : dirList)
	{
		if(dir.isEmpty()) continue;

		currentDir += dir + "/";
		m_data->newDir = currentDir.toStdString();

		sftp_dir sftpDir = sftp_opendir(m_data->sftp, currentDir.toStdString().c_str());
		if(sftpDir == NULL)
		{
			if(sftp_get_error(m_data->sftp) == SSH_FX_NO_SUCH_FILE)
			{
				m_data->msg = QString("The directory:\n\n%1\n\n does not exist on the remote server.\n "
						"Would you like to create it?").arg(currentDir);
				m_data->code = YESNODIALOG;
				m_data->nextFunction = CREATEREMOTEDIR;
				return YESNODIALOG;
			}

			m_data->msg = QString("Unknown error: remote directory cannot be read.");
			m_data->code = FAILED;
			return SSH_ERROR;
		}
	}

	return SSH_OK;
}

int CSSHHandler::SendFile(std::string local, std::string remote)
{
	int rc, transferred = 0, size;
	int access_type = O_WRONLY | O_CREAT | O_TRUNC;
	sftp_file file;
	char buffer[MAX_XFER_BUF_SIZE];

	emit ShowProgress(true);
	emit AddLogEntry(QString("Sending remote file %1 ... ").arg(remote.c_str()));

	if(StartSFTPSession() != SSH_OK)
	{
		return SSH_ERROR;
	}

	// Check to see if remote directory exists.
	rc = CheckRemoteDir();
	if(rc != SSH_OK) return rc;

	file = sftp_open(m_data->sftp, remote.c_str(),
			access_type, S_IRWXU);
	if (file == NULL)
	{
		m_data->msg = QString("Can't open file for writing: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;
		return SSH_ERROR;
	}

	ifstream fin(local.c_str(), ios::binary);

	size = fin.tellg();

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
			if(size > 0) emit UpdateProgress(transferred/(size/100));
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
		emit UpdateProgress(transferred/(attributes->size/100));
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

//int CSSHHandler::RunCommandListOld(std::vector<std::string> commands)
//{
//	ssh_channel channel, channel2;
//	int rc;
//	char buffer[256];
//	char buffer2[256];
//	int nbytes;
//
//	channel = ssh_channel_new(m_data->session);
//	if (channel == NULL)
//		return SSH_ERROR;
//
//	rc = ssh_channel_open_session(channel);
//	if (rc != SSH_OK)
//	{
//		ssh_channel_free(channel);
//		return rc;
//	}
//
//	channel2 = ssh_channel_new(m_data->session);
//	if (channel2 == NULL)
//		return SSH_ERROR;
//
//	rc = ssh_channel_open_session(channel2);
//	if (rc != SSH_OK)
//	{
//		ssh_channel_free(channel2);
//		return rc;
//	}
//
//	rc = ssh_channel_request_shell(channel);
//	if (rc != SSH_OK) return rc;
//
//	rc = ssh_channel_request_shell(channel2);
//	if (rc != SSH_OK) return rc;
//
//	rc = ssh_channel_write(channel, "echo $$\n", sizeof("echo $$\n"));
//
//	nbytes = ssh_channel_read(channel, buffer2, sizeof(buffer2), 0);
//
//	string pidString(buffer2);
//
//	int pid = stoi(pidString);
//
//	for(string command : commands)
//	{
//		command.push_back('\n');
//
//		rc = ssh_channel_write(channel, command.c_str(), sizeof(char)*command.length());
//
//		bool cont = true;
//		while(cont || nbytes > 0)
//		{
//#ifndef WIN32
//			usleep(5000);
//#endif
//
//			string test("pgrep -P ");
//
//			test.append(to_string(pid));
//			test.append(" | wc -l\n");
//
//			rc = ssh_channel_write(channel2, test.c_str(), sizeof(char)*test.length());
//
//			ssh_channel_read(channel2, buffer2, sizeof(buffer2), 0);
//
//			string linesString(buffer2);
//
//			int lines = stoi(linesString);
//
//			if(lines < 1) cont = false;
//
//			nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer) - 1, 0);
//			buffer[nbytes] = '\0';
//			emit AddOutputEntry(buffer);
//
//			nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer) - 1, 1);
//			buffer[nbytes] = '\0';
//			emit AddOutputEntry(buffer);
//		}
//	}
//
//	ssh_channel_send_eof(channel);
//	ssh_channel_close(channel);
//	ssh_channel_free(channel);
//
//	ssh_channel_send_eof(channel2);
//	ssh_channel_close(channel2);
//	ssh_channel_free(channel2);
//	return SSH_OK;
//}

int CSSHHandler::RunCommandList(std::vector<std::string> commands)
{
	ssh_channel channel;
	int rc;
	char buffer[256], buffer2[256];
	int nbytes, nwritten, pos;

	channel = ssh_channel_new(m_data->session);
	if (channel == NULL)
		return SSH_ERROR;

	rc = ssh_channel_open_session(channel);
	if (rc != SSH_OK)
	{
		ssh_channel_free(channel);
		return rc;
	}

	rc = ssh_channel_request_pty(channel);
	if (rc != SSH_OK) return rc;

	rc = ssh_channel_change_pty_size(channel, 80, 24);
	if (rc != SSH_OK) return rc;

	rc = ssh_channel_request_shell(channel);
	if (rc != SSH_OK) return rc;

	std::string fullCommand = commands.at(0);
	for(int i = 1; i < commands.size(); i++)
	{
		fullCommand += " && ";
		fullCommand += commands.at(i);
	}
	fullCommand += "; echo \"ENDSSHNOW\"";
	fullCommand += "\n";

	std::string start = "\"ENDSSHNOW\"";
	std:: string stop = "ENDSSHNOW";

	nwritten = ssh_channel_write(channel, fullCommand.c_str(), sizeof(char)*fullCommand.length());

	std::string temp = "";
	int found = 0;
	while(found < 2)
	{
		nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0);
		buffer[nbytes] = '\0';

		temp += buffer;
		pos = temp.find(start);
		if(pos != -1)
		{
			temp = temp.substr(pos + 11);
			found++;
		}

	}

	// Trim off leading whitespace
	pos = temp.find_first_not_of(" \n\r\t\f\v");
	if(pos < temp.length())	temp = temp.substr(pos);

	emit AddOutputEntry(temp.c_str());
	temp.clear();

	// Because our ENDSSHNOW flag could be split among multiple buffers, we need to keep our
	// previous buffer. These pointers will allow us to swap between them each loop without
	// the need to copy data.
	char* pbuf1 = &buffer[0];
	char* pbuf2 = &buffer2[0];
	char* ptemp = pbuf2;

	// Null-out both buffers.
	*pbuf1 = '\0';
	*pbuf2 = '\0';

//	bool secondCommand = false;
	while(true)
	{
		// Swap buffers
		ptemp = pbuf2;
		pbuf2 = pbuf1;
		pbuf1 = ptemp;

		nbytes = ssh_channel_read(channel, pbuf2, sizeof(buffer) - 1, 0);
		*(pbuf2 + nbytes) = '\0';

		// Concatenate the buffers
		temp = pbuf1;
		temp += pbuf2;

//		// Depending on the response time of the server, the original command could be
//		// echoed twice. This looks for a second echo, and cleans up the output a little
//		if(!secondCommand)
//		{
//			pos = temp.find(start);
//			if(pos != -1)
//			{
//				temp = temp.substr(pos + 11);
//				secondCommand = true;
//
//				// It's possible that this same string also contains the ending code so
//				// we test for that and end if we find it
//				pos = temp.find(stop);
//				if(pos != -1)
//				{
//					// Only print until the start of ENDSSHNOW
//					temp = temp.substr(0, pos);
//					emit AddOutputEntry(temp.c_str());
//					break;
//				}
//
//				// Otherwise, we just print what comes after the start signal
//				emit AddOutputEntry(temp.c_str());
//
//				// Null-out both buffers.
//				*pbuf1 = '\0';
//				*pbuf2 = '\0';
//				continue;
//
//			}
//		}

		// Search for the string
		pos = temp.find(stop);
		if(pos != -1)
		{
			// Only print until the start of ENDSSHNOW
			temp = temp.substr(0, pos);
			emit AddOutputEntry(temp.c_str());
			break;
		}

		// Only print the previous buffer
		emit AddOutputEntry(pbuf1);

		if(m_data->orphan)
		{
			char ctrlZ = 0x1a;
			nwritten = ssh_channel_write(channel, &ctrlZ, sizeof(char));
			nwritten = ssh_channel_write(channel, "bg\n", sizeof("bg\n"));
			nwritten = ssh_channel_write(channel, "disown\n", sizeof("disown\n"));

			// Wait to ensure that the process has been broken before you stop
			QObject().thread()->usleep(5000000L);

			m_data->orphan = false;

			break;
		}

		// Sleep to save cycles
		QObject().thread()->usleep(5000L);
	}

	if (nbytes < 0)
	{
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return SSH_ERROR;
	}

	ssh_channel_send_eof(channel);
	ssh_channel_close(channel);
	ssh_channel_free(channel);
	return SSH_OK;
}


int CSSHHandler::CreateBashFile()
{
//	std::string bashString("#!/bin/bash\n\n");

//	bashString += bashText.toStdString();

//	if(config->type == PBS)
//	{
//		bashString += "#PBS -l nodes=1:ppn=" + std::to_string(config->procNum) + "\n\n";
//		bashString += "#PBS -l mem=" + std::to_string(config->ram) + "\n\n";
//		bashString += "#PBS -l walltime=" + config->walltime + "\n\n";
//		if(!config->jobName.empty())
//		{
//			bashString += "#PBS -N " + config->jobName + "\n\n";
//			bashString += "#PBS -o " + config->remoteDir + "/" + config->jobName + "_stdout.log\n\n";
//			bashString += "#PBS -e " + config->remoteDir + "/" + config->jobName + "_stderr.log\n\n";
//		}
//		bashString += "export OMP_NUM_THREADS=" + std::to_string(config->procNum) + "\n";
//		bashString += config->path + " " + m_data->remoteFileBase + ".feb";
//	}
//	else if(config->type == SLURM)
//	{
//		bashString += "#SBATCH -N 1\n\n";
//		bashString += "#SBATCH -n " + std::to_string(config->procNum) + "\n\n";
//		bashString += "#SBATCH --mem " + std::to_string(config->ram) + "\n\n";
//		bashString += "#SBATCH -t " + config->walltime + "\n\n";
//		if(!config->jobName.empty())
//		{
//			bashString += "#SBATCH -J " + config->jobName + "\n\n";
//			bashString += "#SBATCH -o " + config->remoteDir + "/" + config->jobName + "_stdout.log\n\n";
//			bashString += "#SBATCH -e " + config->remoteDir + "/" + config->jobName + "_stderr.log\n\n";
//		}
//		bashString += "export OMP_NUM_THREADS=" + std::to_string(config->procNum) + "\n";
//		bashString += config->path + " " + m_data->remoteFileBase + ".feb";
//	}

	QString bashText = m_data->job->GetLaunchConfig()->getText().c_str();

	ReplaceMacros(bashText);

	std::string remote = m_data->remoteFileBase + ".bash";

	return SendFile(bashText.toStdString().c_str(), sizeof(char)*bashText.length(), remote);

}

int CSSHHandler::ParseCustomFile(std::vector<std::string>& commands)
{
//	QString customFile = m_data->job->GetLaunchConfig()->customFile.c_str();
//	QFile file(customFile);
//	if(!file.open(QIODevice::ReadOnly))
//	{
//		m_data->msg = QString("Failed to open local custom script file:\n%1").arg(customFile);
//		m_data->code = FAILED;
//		return SSH_ERROR;
//	}
//
//	QTextStream in(&file);
//	while(!in.atEnd())
//	{
//		QString line = in.readLine();
//		if(line.startsWith("#")) continue;
//
//		commands.push_back(line.toStdString());
//	}
//	file.close();

	QString customScript = m_data->job->GetLaunchConfig()->getText().c_str();
	ReplaceMacros(customScript);
	QStringList commandList = customScript.split("\n", Qt::SkipEmptyParts); // QString::SkipEmptyParts is deprecated

	for(QString command : commandList)
	{
		if(command.startsWith("#")) continue;

		commands.push_back(command.toStdString());
	}

	return SSH_OK;
}

void CSSHHandler::ReplaceMacros(QString& string)
{
	string.replace("${FEBIO_PATH}", m_data->job->GetLaunchConfig()->path.c_str());
	string.replace("${JOB_NAME}", m_data->job->GetName().c_str());
	string.replace("${REMOTE_DIR}", m_data->job->GetLaunchConfig()->remoteDir.c_str());
}


bool CSSHHandler::IsBusy()
{
	return m_data->isBusy;
}

std::string CSSHHandler::GetSFTPErrorText(int sftpErr)
{
	std::string returnVal;

	switch(sftpErr)
	{
	case SSH_FX_OK:
		returnVal = "SSH_FX_OK";
		break;
	case SSH_FX_EOF:
		returnVal = "SSH_FX_EOF";
		break;
	case SSH_FX_NO_SUCH_FILE:
		returnVal = "SSH_FX_NO_SUCH_FILE";
		break;
	case SSH_FX_PERMISSION_DENIED:
		returnVal = "SSH_FX_PERMISSION_DENIED";
		break;
	case SSH_FX_FAILURE:
		returnVal = "SSH_FX_FAILURE";
		break;
	case SSH_FX_BAD_MESSAGE:
		returnVal = "SSH_FX_BAD_MESSAGE";
		break;
	case SSH_FX_NO_CONNECTION:
		returnVal = "SSH_FX_NO_CONNECTION";
		break;
	case SSH_FX_CONNECTION_LOST:
		returnVal = "SSH_FX_CONNECTION_LOST";
		break;
	case SSH_FX_OP_UNSUPPORTED:
		returnVal = "SSH_FX_OP_UNSUPPORTED";
		break;
	case SSH_FX_INVALID_HANDLE:
		returnVal = "SSH_FX_INVALID_HANDLE";
		break;
	case SSH_FX_NO_SUCH_PATH:
		returnVal = "SSH_FX_NO_SUCH_PATH";
		break;
	case SSH_FX_FILE_ALREADY_EXISTS:
		returnVal = "SSH_FX_FILE_ALREADY_EXISTS";
		break;
	case SSH_FX_WRITE_PROTECT:
		returnVal = "SSH_FX_WRITE_PROTECT";
		break;
	case SSH_FX_NO_MEDIA:
		returnVal = "SSH_FX_NO_MEDIA";
		break;
	}

	return returnVal;
}

#else
class CSSHHandler::SSHData
{
public:
	std::vector<unsigned char> passwdEnc;
};

CSSHHandler::CSSHHandler (CFEBioJob* job) : m_data(new CSSHHandler::SSHData) {}
CSSHHandler::~CSSHHandler() {delete m_data;}
void CSSHHandler::Update(CLaunchConfig& oldConfig) {}
void CSSHHandler::StartSSHSession() {}
void CSSHHandler::VerifyKnownHost() {}
void CSSHHandler::AddTrustedServer() {}
void CSSHHandler::Authenticate() {}
void CSSHHandler::CreateRemoteDir() {}
void CSSHHandler::EndSSHSession() {}
void CSSHHandler::StartRemoteJob() {}
void CSSHHandler::GetJobFiles() {}
void CSSHHandler::GetQueueStatus() {}
void CSSHHandler::SetPasswordLength(int l) {}
size_t CSSHHandler::GetPasswordLength() {return 0;}
void CSSHHandler::SetPasswdEnc(std::vector<unsigned char> passwdEnc) {}
std::vector<unsigned char>& CSSHHandler::GetPasswdEnc() {return m_data->passwdEnc;}
void CSSHHandler::SetMsgcode(int code) {}
int CSSHHandler::GetMsgCode() {return 0;}
int CSSHHandler::GetNextFunction() {return 0;}
void CSSHHandler::SetTargetFunction(int func) {}
int CSSHHandler::GetTargetFunction() {return 0;}
QString CSSHHandler::GetMessage(){return "";}
void CSSHHandler::Orphan() {}
bool CSSHHandler::IsBusy() {return 0;}
int CSSHHandler::RunCommand(std::string command) {return 0;}
int CSSHHandler::RunInteractiveNoRead(std::string command) {return 0;}
int CSSHHandler::RunCommandList(std::vector<std::string> commands) {return 0;}
int CSSHHandler::authenticatePubkey() {return 0;}
bool CSSHHandler::authenticatePassword() {return 0;}
int CSSHHandler::StartSFTPSession() {return 0;}
int CSSHHandler::EndSFTPSession() {return 0;}
int CSSHHandler::CheckRemoteDir() {return 0;}
int CSSHHandler::SendFile(std::string local, std::string remote) {return 0;}
int CSSHHandler::SendFile(const char * buf, int bufSize, std::string remote) {return 0;}
int CSSHHandler::GetFile(std::string local, std::string remote) {return 0;}
std::string CSSHHandler::GetSFTPErrorText(int sftpErr) {return "";}
int CSSHHandler::CreateBashFile() {return 0;}
int CSSHHandler::ParseCustomFile(std::vector<std::string>& commands) {return 0;}
void CSSHHandler::ReplaceMacros(QString& string) {}
#endif
