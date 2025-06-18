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
namespace py = pybind11;

CPythonRunner* CPythonRunner::m_This = nullptr;


CPythonRunner::CPythonRunner(QObject* parent) : QObject(parent)
{
	m_This = this;
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

void CPythonRunner::runFile(QString fileName)
{
#ifdef HAS_PYTHON
	set_busy b(m_busy);

	if (!m_pythonInitialized)
	{
		init_fbs_python();
		m_pythonInitialized = true;
	}

	std::string sfile = fileName.toStdString();
	std::replace(sfile.begin(), sfile.end(), '\\', '/');

	size_t n = sfile.rfind('/');
	if (n != std::string::npos)
	{
		std::string cwd = sfile.substr(0, n);
		setPythonCWD(cwd);
	}

	const char* szfile = sfile.c_str();

	PyObject* obj = Py_BuildValue("s", szfile);
	FILE* file = _Py_fopen_obj(obj, "r+");
	if (file == nullptr) emit runFileFinished(false);

	PyRun_SimpleFile(file, szfile);

#endif
	emit runFileFinished(true);
}

#ifdef HAS_PYTHON
bool BuildPythonDict(py::dict kwargs, CCachedPropertyList* tool)
{
	for (int prop = 0; prop < tool->Properties(); prop++)
	{
		CProperty& p = tool->Property(prop);
		if (p.isVisible())
		{
			std::string name = p.name.toStdString();
			const char* szname = name.c_str();
			void* d = p.data(); assert(d);

			switch (p.type)
			{
			case CProperty::Bool:
				kwargs[szname] = *((bool*)d);
				break;
			case CProperty::Int:
				kwargs[szname] = *((int*)d);
				break;
			case CProperty::Float:
				kwargs[szname] = *((double*)d);
				break;
			case CProperty::Vec3:
				kwargs[szname] = *((vec3d*)d);
				break;
			case CProperty::Color:
				{
					QColor qc = *((QColor*)d);
					GLColor c = toGLColor(qc);
					kwargs[szname] = c;
				}
				break;
			case CProperty::String:
				// TODO: Is this safe? This passes a pointer to a temporary object
				kwargs[szname] = ((QString*)d)->toStdString().c_str();
				break;
			case CProperty::Enum:
			{
				int n = *((int*)d);
				// TODO: Is this safe? This passes a pointer to a temporary object
				kwargs[szname] = py::make_tuple(n, p.values[n].toStdString().c_str());
			}
			break;
			case CProperty::Resource:
				// TODO: Is this safe? This passes a pointer to a temporary object
				kwargs[szname] = ((QString*)d)->toStdString().c_str();
				break;
			default:
				assert(false);
				return false;
			};
		}
	}
	return true;
}
#endif

void CPythonRunner::runTool(CCachedPropertyList* tool)
{
	set_busy b(m_busy);

	bool returnValue = true;
	assert(m_pythonInitialized);

#ifdef HAS_PYTHON

	try {
		QString fncName = tool->GetPropertyValue(QString("_function_")).toString();
		QString modName = tool->GetPropertyValue(QString("_module_")).toString();
		if (!fncName.isEmpty())
		{
			std::string sfnc = fncName.toStdString();
			std::string smod = modName.toStdString();
			if (smod.empty()) smod = "__main__";

			// Get the function by name
			auto func = py::module_::import(smod.c_str()).attr(sfnc.c_str());

			// call the function
			py::dict kwargs;
			if (BuildPythonDict(kwargs, tool))
			{
				func(**kwargs);
			}
			else
				returnValue = false;
		}
		else returnValue = false;
	}
	catch (py::error_already_set& e)
	{
		// Print the error message
		py::print(e.what());

		// Return execution to Python to allow the thread to exit
		e.restore();

		// Clear the error to allow further Python execution. 
		PyErr_Clear();

		returnValue = false;
	}

#endif
	emit runToolFinished(returnValue);
}

void CPythonRunner::runScript(QString script)
{
#ifdef HAS_PYTHON
	set_busy b(m_busy);

	if (!m_pythonInitialized)
	{
		init_fbs_python();
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
	PyRun_SimpleString(s.c_str());

#endif
	emit runScriptFinished(true);
}

void CPythonRunner::setPythonCWD(const std::string& cwd)
{
	std::string changeCWD = "import os\nos.chdir('" + cwd + "')\n";
	py::exec(changeCWD.c_str());
}
