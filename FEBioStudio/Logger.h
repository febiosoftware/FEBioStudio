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
#include <QtCore/QObject>
#include <QtCore/QString>
#include <FSCore/FSLogger.h>

class CMainWindow;

enum destination{LOG, OUTPUT};

class CLogger : public QObject, public FSLogOutput
{
	Q_OBJECT

public:
	static void Instantiate(CMainWindow* mainWindow);

	void Write(const std::string& s) override;

public slots:
	static void AddLogEntry(const QString& txt);
	static void AddOutputEntry(const QString& txt);
	static void AddBuildLogEntry(const QString& txt);
	static void AddPythonLogEntry(const QString& txt);

signals:
	void SendLogEntry(const QString& txt);
	void SendOutputEntry(const QString& txt);
	void SendBuildLogEntry(const QString& txt);
	void SendPythonLogEntry(const QString& txt);

private:
	CLogger() {}
	CLogger(CMainWindow* mainWindow) {m_mainWindow = mainWindow;}
	CLogger(CLogger const&) {}
	CLogger& operator=(CLogger const&) { return *this; }
	virtual ~CLogger(){}

	static CLogger* m_instance;
	static CMainWindow* m_mainWindow;
};
