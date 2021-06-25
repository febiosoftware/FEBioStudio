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

CPyThread::CPyThread(CPythonToolsPanel* panel, CPythonTool* tool) 
    : tool(tool), panel(panel)
{
    panel->startRunning(QString("Running %1...").arg(tool->name()));

    init();
}

CPyThread::CPyThread(CPythonToolsPanel* panel, QString& filename) 
    : tool(nullptr), panel(panel), filename(filename)
{
    panel->startRunning(QString("Running %1...").arg(filename));

    init();
}


void CPyThread::init()
{
    QObject::connect(this, &CPyThread::finished, panel, &CPythonToolsPanel::endThread);
    QObject::connect(this, &QThread::finished, this, &QThread::deleteLater);
}

void CPyThread::run()
{
    if(tool)
    {
        tool->runFunc();
    }
    else if(panel)
    {
        panel->runScript(filename);
    }
}