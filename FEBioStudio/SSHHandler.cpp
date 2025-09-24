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
#include "Encrypter.h"
#include "FEBioStudio.h"
#include "MainWindow.h"
#include <QMessageBox>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QInputDialog>
#include <QStandardPaths>
#include <QRegularExpression>

#ifdef WIN32
#undef GetMessage
#endif

#include "SSHHandler.h"
#include "SSHThread.h"

using std::ifstream;
using std::ios;

// NOTE: Note sure why the value of 16383 was chosen, but bigger values
// seem to speed up transers. Although chosing values too large don't seem to work.
// Apparently we should be able to query the max read buffer size with sftp_limits() but that 
// doesn't seem to exist in the sshlib we link to. 
//#define MAX_XFER_BUF_SIZE 16383
#define MAX_XFER_BUF_SIZE 65535

#ifdef WIN32
#define S_IRWXU (0400|0200|0100)
#endif

class CSSHHandler::SSHData
{
public:
	// session info
#ifdef HAS_SSH
	ssh_session session = NULL;
	sftp_session sftp = NULL;
#endif

	// connection info
	int port = -1;
	std::string server;
	std::string username;
	int passwdLength = -1;
	std::vector<unsigned char> passwdEnc;

	// return info
	int code = 0;
	QString msg;

	// status info
	bool isBusy = false;
	bool orphan = false;
	int nextFunction = 0;
	int targetFunc = 0;
	std::string queueStatusCmd;

	// file info
	SchedulerType scheduler = NO_SCHEDULER;
	std::string runScript;
	std::string newDir;
	std::string localFile;
	std::string remoteDir;
	std::string remoteFileBase;
	std::string remoteFile;
};

CSSHHandler::CSSHHandler() : m_data(new CSSHHandler::SSHData) // @suppress("Class members should be properly initialized")
{
    CMainWindow* wnd = FBS::getMainWindow();
	QObject::connect(this, &CSSHHandler::AddLogEntry, wnd, &CMainWindow::AddLogEntry);
	QObject::connect(this, &CSSHHandler::AddOutputEntry, wnd, &CMainWindow::AddOutputEntry);
}

void CSSHHandler::setPort(int port) { m_data->port = port; }
void CSSHHandler::setServerName(const std::string& server) { m_data->server = server; }
void CSSHHandler::setUserName(const std::string& userName) { m_data->username = userName; }
void CSSHHandler::setRemoteDir(const std::string& remoteDir) { m_data->remoteDir = remoteDir; }

CSSHHandler::~CSSHHandler() { delete m_data; }

void CSSHHandler::SendLocalFile()
{
#ifdef HAS_SSH
	m_data->nextFunction = -1;
	m_data->code = DONE;

	int error = SendFile(m_data->localFile, m_data->remoteFile);
	if (error == SSH_ERROR)
	{
		m_data->msg = QString(".feb file error: %1").arg(m_data->msg);
		return;
	}
	else if (error == YESNODIALOG)
	{
		return;
	}
#endif
}

void CSSHHandler::StartRemoteJob()
{
#ifdef HAS_SSH
	m_data->nextFunction = -1;
	m_data->code = DONE;

	std::string remoteFile = m_data->remoteFileBase + ".feb";

	if(m_data->scheduler == NO_SCHEDULER)
	{
		// Construct remote command
		std::vector<std::string> commands;
		commands.push_back(m_data->runScript + " " + remoteFile);

		// Run the commands
//		if(RunCommand(command) != SSH_OK)
		if(RunCommandList(commands) != SSH_OK)
		{
			m_data->code = FAILED;
			m_data->msg = "Failed to run remote command.";
			return;
		}
	}
	else if (m_data->scheduler == PBS_SCHEDULER)
	{
		m_data->queueStatusCmd = "qstat";

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
	else if (m_data->scheduler == SLURM_SCHEDULER)
	{
		m_data->queueStatusCmd = "squeue";

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
	else if (m_data->scheduler == CUSTOM_SCHEDULER)
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
#else
	setError("SSH not available in this build.");
#endif
}

void CSSHHandler::GetJobFiles()
{
#ifdef HAS_SSH
	m_data->nextFunction = -1;
	m_data->code = DONE;

	// Get remote .xplt file name
	std::string remoteFile = m_data->remoteFileBase + ".xplt";

	// Get .xplt file
	if(GetFile(m_data->localFile, remoteFile) != SSH_OK)
	{
		m_data->msg = QString("xplt file error: %1").arg(m_data->msg);
		return;
	}

	// Get local .log file name
	std::string localFile = m_data->localFile;
	localFile.replace(localFile.end()-4, localFile.end(), "log");

	// Get remote .log file name
	remoteFile = m_data->remoteFileBase + ".log";

	// Get .log file
	if (GetFile(localFile, remoteFile) != SSH_OK)
	{
		m_data->msg = QString("log file error: %1").arg(m_data->msg);
	}
#else
	setError("SSH not available in this build.");
#endif
}

void CSSHHandler::GetRemoteFile()
{
#ifdef HAS_SSH
	m_data->nextFunction = -1;
	m_data->code = DONE;

	if (GetFile(m_data->localFile, m_data->remoteFile) != SSH_OK)
	{
		m_data->msg = QString("xplt file error: %1").arg(m_data->msg);
		return;
	}
#else
	setError("SSH not available in this build.");
#endif
}

void CSSHHandler::GetQueueStatus()
{
#ifdef HAS_SSH
	m_data->nextFunction = -1;
	m_data->code = DONE;

	if (m_data->queueStatusCmd.empty())
	{
		setError("Invalid queue status command");
		return;
	}

	std::vector<std::string> list;
	list.push_back(m_data->queueStatusCmd);

	if (RunCommandList(list) != SSH_OK)
	{
		m_data->code = FAILED;
		m_data->msg = "Failed to run remote command.";
	}
#else
	setError("SSH not available in this build.");
#endif
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

void CSSHHandler::setError(const QString& msg)
{
	m_data->code = FAILED;
	m_data->msg = msg;
}

void CSSHHandler::StartSSHSession()
{
#ifdef HAS_SSH
	// some sanity checks first
	if (m_data->port == -1) { setError("Invalid port number"); return; }
	if (m_data->server.empty()) { setError("server name not set"); return; }
	if (m_data->username.empty()) { setError("username not provided"); return; }

	m_data->isBusy = true;
	m_data->nextFunction = -1;

	m_data->session = ssh_new();
	if(m_data->session == NULL)
	{
		setError("Could not initialize SSH session.");
		return;
	}


	ssh_options_set(m_data->session, SSH_OPTIONS_HOST, m_data->server.c_str());
	ssh_options_set(m_data->session, SSH_OPTIONS_PORT, &m_data->port);
	ssh_options_set(m_data->session, SSH_OPTIONS_USER, m_data->username.c_str());

	QString sshDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.ssh";
	std::string sdir = sshDir.toStdString();
	ssh_options_set(m_data->session, SSH_OPTIONS_SSH_DIR, sdir.c_str());

	// Connect to server
	int error = ssh_connect(m_data->session);
	if (error != SSH_OK)
	{
		setError(QString("Error connecting to server: %1").arg(ssh_get_error(m_data->session)));
		return;
	}

	m_data->code = OK;
	m_data->nextFunction = VERIFYSERVER;
#endif
}

void CSSHHandler::EndSSHSession()
{
#ifdef HAS_SSH
	ssh_disconnect(m_data->session);
	ssh_free(m_data->session);
	m_data->isBusy = false;
	emit sessionFinished(m_data->targetFunc);
#endif
}

void CSSHHandler::VerifyKnownHost()
{
#ifdef HAS_SSH
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
#endif
}

void CSSHHandler::AddTrustedServer()
{
#ifdef HAS_SSH
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
#endif	
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

	std::string userAndServer = m_data->username + "@" + m_data->server;
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
#ifdef HAS_SSH
	// Attempt Public Key authentication
	int rc = ssh_userauth_publickey_auto(m_data->session, NULL, NULL);
	if (rc == SSH_AUTH_SUCCESS)
	{
		return OK;
	}
	return FAILED;
#else
	return 0;
#endif
}

bool CSSHHandler::authenticatePassword()
{
#ifdef HAS_SSH
	std::string password = CEncrypter::Instance()->Decrypt(m_data->passwdEnc, m_data->passwdLength);

	int rc = ssh_userauth_password(m_data->session, NULL, password.c_str());
	if (rc != SSH_AUTH_SUCCESS) return false;
	return true;
#else
	return false;
#endif
}

void CSSHHandler::CreateRemoteDir()
{
#ifdef HAS_SSH
	if(sftp_mkdir(m_data->sftp, m_data->newDir.c_str(), S_IRWXU) != SSH_OK)
	{
		m_data->msg = QString("Cannot create remote directory:\n\n%1\n\nSFTP Error: %2")
				.arg(m_data->newDir.c_str()).arg(GetSFTPErrorText(sftp_get_error(m_data->sftp)).c_str());
		m_data->code = FAILED;
		return;
	}

	m_data->code = OK;
	m_data->nextFunction = TARGET;
#endif
}

int CSSHHandler::StartSFTPSession()
{
#ifdef HAS_SSH
	m_data->sftp = sftp_new(m_data->session);
	if (m_data->sftp == NULL)
	{
		m_data->msg = QString("Error allocating SFTP session: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;
		return SSH_ERROR;
	}

	int rc = sftp_init(m_data->sftp);
	if (rc != SSH_OK)
	{
		m_data->msg = QString("Error initializing SFTP session: %1\n").arg(sftp_get_error(m_data->sftp));
		m_data->code = FAILED;
		sftp_free(m_data->sftp);
		return rc;
	}

	return rc;
#else
	return 0;
#endif
}

int CSSHHandler::EndSFTPSession()
{
#ifdef HAS_SSH
	sftp_free(m_data->sftp);
	return SSH_OK;
#else
	return 0;
#endif
}

int CSSHHandler::CheckRemoteDir()
{
#ifdef HAS_SSH
	QString remoteDir = QString(m_data->remoteDir.c_str());
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
#else
	return 0;
#endif
}

int CSSHHandler::SendFile(std::string local, std::string remote)
{
#ifdef HAS_SSH
	int access_type = O_WRONLY | O_CREAT | O_TRUNC;
	sftp_file file;

	emit AddLogEntry(QString("Sending remote file %1 ... ").arg(remote.c_str()));

	if(StartSFTPSession() != SSH_OK)
	{
		return SSH_ERROR;
	}

	// Check to see if remote directory exists.
	int rc = CheckRemoteDir();
	if(rc != SSH_OK) return rc;

	file = sftp_open(m_data->sftp, remote.c_str(),
			access_type, S_IRWXU);
	if (file == NULL)
	{
		m_data->msg = QString("Can't open file for writing: %1\n").arg(ssh_get_error(m_data->session));
		m_data->code = FAILED;
		return SSH_ERROR;
	}

	QFileInfo fi(local.c_str());
	size_t fileSize = fi.size();

	ifstream fin(local.c_str(), ios::binary);

	size_t transferred = 0;
	std::vector<char> buffer(MAX_XFER_BUF_SIZE + 1, 0);
	while(fin)
	{
		fin.read(buffer.data(), MAX_XFER_BUF_SIZE);

		if(fin.gcount() > 0)
		{
			ssize_t nwritten = sftp_write(file, buffer.data(), fin.gcount());
			if (nwritten != fin.gcount())
			{
				m_data->msg = QString("Can't write data to file: %1\n").arg(ssh_get_error(m_data->session));
				m_data->code = FAILED;
				sftp_close(file);
				return SSH_ERROR;
			}

			transferred += nwritten;
			if(fileSize > 0) emit UpdateProgress(transferred/(fileSize /100));
		}
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
#else
	return 0;
#endif
}

int CSSHHandler::SendFile(const char * buf, int bufSize, std::string remote)
{
#ifdef HAS_SSH
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
#else
	return 0;
#endif
}

int CSSHHandler::GetFile(std::string local, std::string remote)
{
#ifdef HAS_SSH
	int rc, nbytes, transferred = 0;
	int access_type = O_RDWR;
	sftp_file file;
	sftp_attributes attributes;
	std::vector<char> buffer(MAX_XFER_BUF_SIZE + 1);

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
		nbytes = sftp_read(file, buffer.data(), MAX_XFER_BUF_SIZE);

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
		WriteFile(fileHandle, buffer.data(), nbytes, &nwritten, NULL);
#else
		nwritten = write(fd, buffer.data(), nbytes);

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

	emit AddLogEntry(QString("success!\n"));

	return EndSFTPSession();
#else
	return 0;
#endif
}

int CSSHHandler::RunCommand(std::string command)
{
#ifdef HAS_SSH
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
#else
	return 0;
#endif
}

int CSSHHandler::RunInteractiveNoRead(std::string command)
{
#ifdef HAS_SSH
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
#else
	return 0;
#endif
}

int CSSHHandler::RunCommandList(std::vector<std::string> commands)
{
#ifdef HAS_SSH
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

		// see if we can extract the progress
		size_t c1 = temp.find('\033');
		if (c1 != -1)
		{
			size_t c2 = temp.find('\007');
			if (c2 != -1)
			{
				std::string t = temp.substr(c1 + 4, c2 - c1 - 4);
				QRegularExpression e("\\([^\\)]*\\)");
				QRegularExpressionMatch m = e.match(QString::fromStdString(t));
				if (m.isValid())
				{
					QString s = m.captured(0);
					std::string sz = s.toStdString();
					double f = atof(sz.c_str() + 1);
					UpdateProgress(f);
				}
			}
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
#else
	return 0;
#endif
}

int CSSHHandler::CreateBashFile()
{
	std::string bashFile = m_data->remoteFileBase + ".bash";
	return SendFile(m_data->runScript.c_str(), m_data->runScript.length(), bashFile);
}

int CSSHHandler::ParseCustomFile(std::vector<std::string>& commands)
{
#ifdef HAS_SSH
	QString customScript = QString::fromStdString(m_data->runScript);
	QStringList commandList = customScript.split("\n", Qt::SkipEmptyParts); // QString::SkipEmptyParts is deprecated

	for(QString command : commandList)
	{
		if(command.startsWith("#")) continue;

		commands.push_back(command.toStdString());
	}
	return SSH_OK;
#else
	return 0;
#endif
}

void CSSHHandler::RequestRemoteFiles(const std::string& localFile)
{
	if (!IsBusy())
	{
		m_data->localFile = localFile;

		QFileInfo info(QString::fromStdString(localFile));
		QString baseName = info.baseName();
		m_data->remoteFileBase = m_data->remoteDir + "/" + baseName.toStdString();

		SetTargetFunction(GETJOBFILES);
		CSSHThread* sshThread = new CSSHThread(this, STARTSSHSESSION);
		sshThread->start();
	}
}

void CSSHHandler::RequestRemoteFile(const std::string& localFile)
{
	if (!IsBusy())
	{
		m_data->localFile = localFile;

		QFileInfo info(QString::fromStdString(localFile));
		QString fileName = info.fileName();
		m_data->remoteFile = m_data->remoteDir + "/" + fileName.toStdString();

		SetTargetFunction(GETREMOTEFILE);
		CSSHThread* sshThread = new CSSHThread(this, STARTSSHSESSION);
		sshThread->start();
	}
}

void CSSHHandler::RequestQueueStatus()
{
	if (!IsBusy())
	{
		// Copy remote files to local dir
		SetTargetFunction(GETQUEUESTATUS);
		CSSHThread* sshThread = new CSSHThread(this, STARTSSHSESSION);
		sshThread->start();
	}
}

void CSSHHandler::SendFileToServer(const std::string& localFile)
{
	if (!IsBusy())
	{
		m_data->localFile = localFile;
		QFileInfo info(QString::fromStdString(localFile));
		QString baseName = info.baseName();
		QString fileName = info.fileName();
		m_data->remoteFileBase = m_data->remoteDir + "/" + baseName.toStdString();
		m_data->remoteFile = m_data->remoteDir + "/" + fileName.toStdString();

		SetTargetFunction(SENDFILE);
		CSSHThread* sshThread = new CSSHThread(this, STARTSSHSESSION);
		sshThread->start();
	}
}

void CSSHHandler::RunRemoteJob(CSSHHandler::SchedulerType scheduler, const std::string& runScript)
{
	if (!IsBusy())
	{
		m_data->scheduler = scheduler;
		m_data->runScript = runScript;
		SetTargetFunction(STARTREMOTEJOB);
		CSSHThread* sshThread = new CSSHThread(this, STARTSSHSESSION);
		sshThread->start();
	}
}

bool CSSHHandler::IsBusy()
{
	return m_data->isBusy;
}

std::string CSSHHandler::GetSFTPErrorText(int sftpErr)
{
#ifdef HAS_SSH
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
#else
	return std::string();
#endif
}

bool CSSHHandler::HandleSSHMessage()
{
#ifdef HAS_SSH
	QString QPasswd;
	QMessageBox::StandardButton reply;

	switch (GetMsgCode())
	{
	case FAILED:
		QMessageBox::critical(nullptr, "FEBio Studio", GetMessage());
		return false;
	case NEEDSPSWD:
		bool ok;
		QPasswd = QInputDialog::getText(nullptr, "Password", GetMessage(), QLineEdit::Password, "", &ok);

		if (ok)
		{
			std::string password = QPasswd.toStdString();
			SetPasswordLength(password.length());
			SetPasswdEnc(CEncrypter::Instance()->Encrypt(password));
		}
		else
		{
			return false;
		}
		break;
	case YESNODIALOG:
		reply = QMessageBox::question(nullptr, "FEBio Studio", GetMessage(),
			QMessageBox::Yes | QMessageBox::No);

		return reply == QMessageBox::Yes;
	case DONE:
		return false;
	}
	return true;
#else
	return false;
#endif
}

void CSSHHandler::NextSSHFunction()
{
	if (!HandleSSHMessage())
	{
		EndSSHSession();
		return;
	}

	CSSHThread* sshThread = new CSSHThread(this, GetNextFunction());
	sshThread->start();
}
