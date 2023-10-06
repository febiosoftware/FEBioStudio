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

#include "PythonTool.h"
#include "PyThread.h"
#include "PythonToolsPanel.h"
#include "PyState.h"

#include "PyFBS.cpp"

#include <QMetaObject>

CPyThread::CPyThread(CPythonToolsPanel* panel) 
    : m_panel(panel), m_tool(nullptr), m_restart(false), m_state(new CPyState)
{

}

CPyThread::~CPyThread()
{
    delete m_state;
}

void CPyThread::initPython()
{
	pybind11::initialize_interpreter();
 
	// setup output
 	auto sysm = pybind11::module::import("sys");
	auto output = pybind11::module::import("fbs").attr("ui").attr("PyOutput");
	sysm.attr("stdout") = output();
	sysm.attr("stderr") = output();
}

void CPyThread::finalizePython()
{
	pybind11::finalize_interpreter();
}

void CPyThread::run()
{
    initPython();

    while(true)
    {
        if(m_restart)
        {
            m_state->ClearFuncs();

            finalizePython();
            initPython();

            emit Restarted();
        }
        else if(m_tool)
        {
            m_tool->runFunc();
            m_tool = nullptr;
        }
        else if(!m_filename.isEmpty())
        {
            m_panel->runScript(m_filename);
            m_filename.clear();

            emit ExecDone();
        }

        msleep(100);
    }

    finalizePython();
}

void CPyThread::SetTool(CPythonTool* tool)
{
    m_tool = tool;
}
    
void CPyThread::SetFilename(QString& filename)
{
    m_filename = filename;
}   

void CPyThread::Restart()
{
    m_restart = true;
}

CPyState* CPyThread::GetState()
{
    return m_state;
}
