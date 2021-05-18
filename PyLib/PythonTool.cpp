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

#include "Python.h"
#include "PythonTool.h"

#include <iostream>

CPythonTool::CPythonTool(CMainWindow* wnd, const char* name, PyObject* func)
    : CBasicTool(wnd, name, HAS_APPLY_BUTTON)
{
    SetApplyButtonText("Run");

    // Make a copy of the function object so that it doesn't get invalidted by
    // importing another script with the same function name
    this->func = PyFunction_New(PyFunction_GetCode(func), PyFunction_GetGlobals(func));

    std::cout << PyObject_HasAttrString(func, "name") << std::endl;
    std::cout << PyObject_HasAttrString(func, "_name") << std::endl;
    std::cout << PyObject_HasAttrString(func, "_name_") << std::endl;
    std::cout << PyObject_HasAttrString(func, "__name") << std::endl;
    std::cout << PyObject_HasAttrString(func, "__name__") << std::endl;

    std::cout << PyUnicode_AsUTF8(PyObject_GetAttrString(func, "__name__")) << std::endl;
}

CPythonTool::~CPythonTool()
{
    for(auto el : boolProps)
    {
        delete el.second;
    }

    for(auto el : intProps)
    {
        delete el.second;
    }

    for(auto el : enumProps)
    {
        delete el.second;
    }

    for(auto el : dblProps)
    {
        delete el.second;
    }

    for(auto el : strProps)
    {
        delete el.second;
    }

    for(auto el : rscProps)
    {
        delete el.second;
    }
}

void CPythonTool::addBoolProperty(const std::string& name, bool value)
{
    bool* val = new bool;
    *val = value;
    boolProps[name] = val;

    CDataPropertyList::addBoolProperty(val, QString::fromStdString(name));
}

void CPythonTool::addIntProperty(const std::string& name, int value)
{
    int* val = new int;
    *val = value;
    intProps[name] = val;

    CDataPropertyList::addIntProperty(val, QString::fromStdString(name));
}

void CPythonTool::addEnumProperty(const std::string& name, int value, const std::string& labels)
{
    int* val = new int;
    *val = value;
    enumProps[name] = val;

    QStringList qlabels = QString::fromStdString(labels).split(";");

    CDataPropertyList::addEnumProperty(val, QString::fromStdString(name))->setEnumValues(qlabels);
}

void CPythonTool::addDoubleProperty(const std::string& name, double value)
{
    double* val = new double;
    *val = value;
    dblProps[name] = val;

    CDataPropertyList::addDoubleProperty(val, QString::fromStdString(name));
}

void CPythonTool::addStringProperty(const std::string& name, char* value)
{
    QString * val = new QString(value);
    strProps[name] = val;

    CDataPropertyList::addStringProperty(val, QString::fromStdString(name));
}

void CPythonTool::addResourceProperty(const std::string& name, char* value)
{
    QString * val = new QString(value);
    rscProps[name] = val;

    CDataPropertyList::addResourceProperty(val, QString::fromStdString(name));
}


bool CPythonTool::OnApply()
{
    std::cout << PyFunction_Check(func) << std::endl;

    PyObject* args = PyTuple_New(0);
    PyObject* kwargs = PyDict_New();

    std::vector<PyObject*> objs;

    for(int prop = 0; prop < Properties(); prop++)
    {
        CProperty current = Property(prop);
        std::string name = current.name.toStdString();

        PyObject* obj;

        switch(current.type)
        {
            case CProperty::Bool:
                obj = *boolProps[name] ? Py_True: Py_False;
                break;
            case CProperty::Int:
                obj = Py_BuildValue("i", *intProps[name]);
                objs.push_back(obj);
                break;
            case CProperty::Float:
                obj = Py_BuildValue("d", *dblProps[name]);
                objs.push_back(obj);
                break;
            case CProperty::String:
                obj = Py_BuildValue("s", strProps[name]->toStdString().c_str());
                objs.push_back(obj);
                break;
            case CProperty::Enum:
                obj = Py_BuildValue("is", *enumProps[name], current.values[*enumProps[name]].toStdString().c_str());
                objs.push_back(obj);
                break;
            case CProperty::Resource:
                obj = Py_BuildValue("s", rscProps[name]->toStdString().c_str());
                objs.push_back(obj);
                break;

            default:
                return false;
        };

        PyDict_SetItemString(kwargs, current.name.toStdString().c_str(), obj);

    }

    PyObject_Call(func, args, kwargs);

    Py_DECREF(args);
    Py_DECREF(kwargs);
    for(auto obj : objs)
    {
        Py_DECREF(obj);
    }

    return true;
}