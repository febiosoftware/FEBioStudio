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
#ifdef HAS_PYTHON
// Note that we need to include pybind11 before we include anything that depends on Qt. 
// This is because pybind11 defines a "slots" variable, but Qt defines a slots macro and thus, 
// including Qt before pybind11 will break the pybind11 code. 
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "PyState.h"
#endif

#include "PythonTool.h"
#include "PythonThread.h"
#include "PyFBS.h"
#include <QMutex>
#include <QMutexLocker>

#ifdef HAS_PYTHON
namespace py = pybind11;
#endif

QMutex mutex;
CPyThread::CPyThread() : m_tool(nullptr)
{
	m_restart = false;
	m_stop = false;
}

CPyThread::~CPyThread()
{

}

void CPyThread::initPython()
{
	init_fbs_python();
}

void CPyThread::finalizePython()
{
#ifdef HAS_PYTHON
	CPyState::clear();
	finish_fbs_python();
#endif
}

void CPyThread::run()
{
	initPython();

	while(true)
	{
		mutex.lock();
		if (m_stop)
		{
			mutex.unlock();
			break;
		}
		else if (m_restart) runRestart();
		else if (m_tool) runTool();
		else if (!m_filename.isEmpty()) runScript();
		mutex.unlock();
		msleep(100);
	}

	finalizePython();
}

void CPyThread::SetTool(CPythonToolProps* tool)
{
	QMutexLocker lock(&mutex);
	m_tool = tool;
}

void CPyThread::SetFilename(QString& filename)
{
	QMutexLocker lock(&mutex);
	m_filename = filename;
}

void CPyThread::Restart()
{
	QMutexLocker lock(&mutex);
	m_restart = true;
}

void CPyThread::Stop()
{
	QMutexLocker lock(&mutex);
	m_stop = true;
}

void CPyThread::runRestart()
{
	finalizePython();
	initPython();
	m_restart = false;
	emit Restarted();
}

void CPyThread::runTool()
{
	bool returnValue = true;
#ifdef HAS_PYTHON
	try
	{
		py::dict kwargs;

		for (int prop = 0; prop < m_tool->Properties(); prop++)
		{
			CProperty& current = m_tool->Property(prop);
			std::string name = current.name.toStdString();
			void* d = current.data(); assert(d);

			switch (current.type)
			{
			case CProperty::Bool:
				kwargs[name.c_str()] = *((bool*)d);
				break;
			case CProperty::Int:
				kwargs[name.c_str()] = *((int*)d);
				break;
			case CProperty::Float:
				kwargs[name.c_str()] = *((double*)d);
				break;
			case CProperty::Vec3:
				kwargs[name.c_str()] = *((vec3d*)d);
				break;
			case CProperty::String:
				// TODO: Is this safe? This passes a pointer to a temporary object
				kwargs[name.c_str()] = ((QString*)d)->toStdString().c_str();
				break;
			case CProperty::Enum:
			{
				int n = *((int*)d);
				// TODO: Is this safe? This passes a pointer to a temporary object
				kwargs[name.c_str()] = py::make_tuple(n, current.values[n].toStdString().c_str());
			}
			break;
			case CProperty::Resource:
				// TODO: Is this safe? This passes a pointer to a temporary object
				kwargs[name.c_str()] = ((QString*)d)->toStdString().c_str();
				break;
			default:
				assert(false);
				returnValue = false;
			};
		}

		if (returnValue)
		{
			python_handle f = m_tool->GetFunction();
			auto func = CPyState::get_pyfunction(f);
			(*func)(**kwargs);
		}
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
	m_tool = nullptr;
	emit ToolFinished(returnValue);
}

void CPyThread::runScript()
{
#ifdef HAS_PYTHON
	PyObject* obj = Py_BuildValue("s", m_filename.toStdString().c_str());
	FILE* file = _Py_fopen_obj(obj, "r+");
	PyRun_SimpleFile(file, m_filename.toStdString().c_str());
	m_filename.clear();
#endif
	emit ExecDone();
}
