/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <FEBioStudio/PropertyList.h>

// This singleton class is used to interact with Python.
// This class is stored in a separate thread so the best way to interact with it is through signal/slot
// The only exception is calling the interrupt member, which is supposed to interrupt a long python call.
class CPythonRunner : public QObject
{
	Q_OBJECT

public:
	CPythonRunner(QObject* parent = nullptr);
	~CPythonRunner();

	bool isBusy() const;

	void interrupt();

	void SetWorkingDirectory(QString cwd);

	// Use this method to get a pointer to the runner. 
	static CPythonRunner* GetInstance();

public slots:
	void runFile(QString fileName);
	void runTool(CCachedPropertyList* tool);
	void runScript(QString script);

signals:
	void runFileFinished(bool);
	void runToolFinished(bool);
	void runScriptFinished(bool);

private:
	void setPythonCWD(const std::string& cwd);

private:
	bool m_busy = false;
	bool m_pythonInitialized = false;

	std::string	m_cwd;
    std::wstring m_pythonHome;

	static CPythonRunner* m_This;
};
