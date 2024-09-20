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
#include "PythonToolsPanel.h"
#include "PyInputWidgets.h"

CPythonInputHandler::CPythonInputHandler(CPythonToolsPanel* panel)
    : currentType(0), panel(panel)
{

}

std::string CPythonInputHandler::getString()
{
    return inputString;
}

int CPythonInputHandler::getInt()
{
    return inputInt;
}

int CPythonInputHandler::getSelection()
{
    return 0;
}

void CPythonInputHandler::getInput(int type, const QString& txt)
{
/*
    currentType = type;

    PyInputWidget* wgt;

	switch (type)
	{
	case STRING:
		wgt = new PyInputStringWidget(txt);
        break;
	case INT:
        wgt = new PyInputIntWidget(txt);
        break;
    case SELECTION:
        wgt = new PyInputSelectionWidget(txt);
        break;
	default:
        break;
	}

    QObject::connect(wgt, &PyInputWidget::done, this, &CPythonInputHandler::finishInput);
	panel->addInputPage(wgt);
*/
}

void CPythonInputHandler::finishInput()
{
	/*
    switch (currentType)
    {
    case STRING:
        inputString = dynamic_cast<PyInputStringWidget*>(panel->getInputWgt())->getVal();
        break;
    case INT:
        inputInt = dynamic_cast<PyInputIntWidget*>(panel->getInputWgt())->getVal();
        break;
    default:
        break;
    }

	panel->removeInputPage();
	*/
	emit inputReady();
}

#else
CPythonInputHandler::CPythonInputHandler(CPythonToolsPanel* panel) {}
std::string CPythonInputHandler::getString() {return "";}
int CPythonInputHandler::getInt() {return 0;}
void CPythonInputHandler::getInput(int type, const QString& txt) {}
int CPythonInputHandler::getSelection() {return 0;}
void CPythonInputHandler::finishInput() {}
#endif