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
#include <FEBioStudio/PropertyList.h>

namespace py = pybind11;
#endif

#include "PythonThread.h"
#include "PyFBS.h"
#include "PyRunContext.h"
#include <FEBioStudio/Document.h>

CPyThread::CPyThread(CDocument* doc, CCachedPropertyList* params) : m_doc(doc), m_params(params)
{
	QObject::connect(this, &QThread::finished, this, &QObject::deleteLater);
}

void CPyThread::runFile(const QString& filename)
{
	m_filename = filename;
	m_script.clear();
	start();
}

void CPyThread::runScript(const QString& script)
{
	m_filename.clear();
	m_script = script;
	start();
}

void CPyThread::run()
{
	// let's clear the undo stack, since we won't be able to undo the result
	// of the python script anyways
	CUndoDocument* undoDoc = dynamic_cast<CUndoDocument*>(m_doc);
	if (undoDoc) undoDoc->ClearCommandStack();

	// Store the document that was active when the thread started.
	// This will be the document that will be modified
	PyRunContext::SetDocument(m_doc);

	init_fbs_python();

	bool b = runScript();

	emit threadFinished(b);

	finish_fbs_python();

	if (undoDoc) undoDoc->Update();

	PyRunContext::SetDocument(nullptr);
}

bool CPyThread::runScript()
{
#ifdef HAS_PYTHON
	try
	{
		py::dict kwargs;

		if (m_params)
		{
			for (int prop = 0; prop < m_params->Properties(); prop++)
			{
				CProperty& current = m_params->Property(prop);
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
				case CProperty::Color:
				{
					GLColor c = toGLColor(*((QColor*)d));
					kwargs[name.c_str()] = c;
				}
				break;
				default:
					assert(false);
					return false;
				};
			}
		}

		auto m = py::module::import("fbs");
		m.attr("args") = kwargs;

		if (!m_filename.isEmpty())
		{
			PyObject* obj = Py_BuildValue("s", m_filename.toStdString().c_str());
			FILE* file = _Py_fopen_obj(obj, "r+");
			PyRun_SimpleFile(file, m_filename.toStdString().c_str());
		}
		else if (!m_script.isEmpty())
		{
			std::string s = m_script.toStdString();
			PyRun_SimpleString(s.c_str());
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

		return false;
	}
	catch (...)
	{
		py::print("unknown exception");

		return false;
	}
#endif

	return true;
}
