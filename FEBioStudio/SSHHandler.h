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

#pragma once

#include "FEBioJob.h"
#include "LaunchConfig.h"
#include "MainWindow.h"
#include <QtCore/QObject>

class CSSHThread;
class CLaunchConfig;

enum nextFunc{ENDSSHSESSION = -1, STARTSSHSESSION, VERIFYSERVER, ADDTRUSETEDSERVER, AUTHENTICATE, TARGET,
		STARTREMOTEJOB, GETJOBFILES, GETQUEUESTATUS, CREATEREMOTEDIR};

enum msgCode{FAILED=-1, OK, NEEDSPSWD, YESNODIALOG, DONE};

class CSSHHandler : public QObject
{
	Q_OBJECT

	class SSHData;

public:
	CSSHHandler (CFEBioJob* job, CLaunchConfig* lc);
	~CSSHHandler();

	void StartRemoteSession();
	void RequestRemoteFiles();
	void RequestQueueStatus();

private:
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

	bool HandleSSHMessage();

public slots:
	void NextSSHFunction();

signals:
	void AddLogEntry(const QString&);
	void AddOutputEntry(const QString&);
	void ShowProgress(bool, QString message = "");
	void UpdateProgress(int);
	void sessionFinished();

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

	friend class CSSHThread;
};
