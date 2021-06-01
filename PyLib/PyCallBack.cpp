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



#include "PythonInputHandler.h"
#ifdef HAS_PYTHON
#include <QEventLoop>
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include "PythonToolsPanel.h"
#include "PyCallBack.h"

void PySetProgress(int prog)
{
    auto wnd = PRV::getMainWindow();
    auto panel = wnd->GetPythonToolsPanel();

    QMetaObject::invokeMethod(panel, "setProgress", Q_ARG(int, prog));
}

CPythonInputHandler* PyGetInput(int type, const char* txt)
{
    auto wnd = PRV::getMainWindow();
    auto inputHandler = wnd->GetPythonToolsPanel()->getInputHandler();

    QMetaObject::invokeMethod(inputHandler, "getInput", Q_ARG(int, type), Q_ARG(const QString&, txt));

    QEventLoop loop;
    QObject::connect(inputHandler, &CPythonInputHandler::inputReady, &loop, &QEventLoop::quit);
    loop.exec();

    return inputHandler;
}

std::string PyGetString(const char* txt)
{
    return PyGetInput(CPythonInputHandler::STRING, txt)->getString();
}

int PyGetInt(const char* txt)
{
    return PyGetInput(CPythonInputHandler::INT, txt)->getInt();
}

#else
CPythonInputHandler* PyGetInput(int type) {return nullptr;}
std::string PyGetString(const char* txt) {return "";}
int PyGetInt(const char* txt){return 0;}
#endif