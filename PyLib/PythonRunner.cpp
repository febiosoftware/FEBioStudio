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
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "PythonRunner.h"
#include "PyFBS.h"
#include <algorithm>
#include <QCoreApplication>
#include <QDir>
namespace py = pybind11;

CPythonRunner* CPythonRunner::m_This = nullptr;


CPythonRunner::CPythonRunner(QObject* parent) : QObject(parent), m_pythonHome(L"")
{
	m_This = this;

    // Sets the python home directory for an installed version of FEBio Studio
    #ifdef LINUX
        QString path = QCoreApplication::applicationDirPath() + "/../lib/python";

        if(QDir(path).exists())
        {
            m_pythonHome = path.toStdWString();
        }
    #endif

}

CPythonRunner* CPythonRunner::GetInstance()
{
	assert(m_This);
	return m_This;
}

CPythonRunner::~CPythonRunner()
{
#ifdef HAS_PYTHON
	if (m_pythonInitialized)
	{
		finish_fbs_python();
		m_pythonInitialized = false;
	}
#endif
	m_This = nullptr;
}

void CPythonRunner::SetWorkingDirectory(QString cwd)
{
	m_cwd = cwd.toStdString();
}

bool CPythonRunner::isBusy() const
{
	return m_busy;
}

void CPythonRunner::interrupt()
{
#ifdef HAS_PYTHON
	// Acquire GIL and inject interrupt
	PyGILState_STATE gstate = PyGILState_Ensure();
	PyErr_SetInterrupt();  // Simulate Ctrl+C
	PyGILState_Release(gstate);
#endif
}

class set_busy
{
public:
	set_busy(bool& b) : m_b(b) { m_b = true; }
	~set_busy() { m_b = false; }

private:
	bool& m_b;
};

void CPythonRunner::runScript(QString script)
{
#ifdef HAS_PYTHON
	set_busy b(m_busy);

	if (!m_pythonInitialized)
	{
		init_fbs_python(m_pythonHome);
		m_pythonInitialized = true;
	}

	if (!m_cwd.empty())
	{
		// make sure all backslashes are replaced with forward slashes.
		std::string path(m_cwd);
		std::replace(path.begin(), path.end(), '\\', '/');
		setPythonCWD(path);
	}

	std::string s = script.toStdString();
	int result = PyRun_SimpleString(s.c_str());

	emit runScriptFinished(result == 0);
#else
	emit runScriptFinished(false);
#endif
}

void CPythonRunner::setPythonCWD(const std::string& cwd)
{
#ifdef HAS_PYTHON
	py::module_::import("os").attr("chdir")(cwd);
#endif
}
