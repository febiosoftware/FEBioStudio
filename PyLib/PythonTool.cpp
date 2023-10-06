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

#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include "PyThread.h"
#include "PyState.h"

#include <FEBioStudio/MainWindow.h>
#include "PythonToolsPanel.h"
#include <iostream>

CPythonTool::CPythonTool(CMainWindow* wnd, std::string name, int id)
    : CBasicTool(wnd, name.c_str(), HAS_APPLY_BUTTON), m_id(id), m_wnd(wnd)
{
    SetApplyButtonText("Run");
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

    for(auto el : vec3Props)
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

void CPythonTool::addEnumProperty(const std::string& name, const std::string& labels, int value)
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

void CPythonTool::addVec3Property(const std::string& name, vec3d value)
{
    vec3d* val = new vec3d;
    *val = value;
    vec3Props[name] = val;

    CDataPropertyList::addVec3Property(val, QString::fromStdString(name));
}

void CPythonTool::addStringProperty(const std::string& name, std::string value)
{
    QString * val = new QString(value.c_str());
    strProps[name] = val;

    CDataPropertyList::addStringProperty(val, QString::fromStdString(name));
}

void CPythonTool::addResourceProperty(const std::string& name, std::string value)
{
    QString * val = new QString(value.c_str());
    rscProps[name] = val;

    CDataPropertyList::addResourceProperty(val, QString::fromStdString(name));
}

bool CPythonTool::OnApply()
{
    m_wnd->GetPythonToolsPanel()->GetThread()->SetTool(this);

    return true;
}

bool CPythonTool::runFunc()
{
    try
    {
        pybind11::dict kwargs;

        for(int prop = 0; prop < Properties(); prop++)
        {
            CProperty& current = Property(prop);
            std::string name = current.name.toStdString();

            switch(current.type)
            {
                case CProperty::Bool:
                    kwargs[name.c_str()] = *boolProps[name];
                    break;
                case CProperty::Int:
                    kwargs[name.c_str()] = *intProps[name];
                    break;
                case CProperty::Float:
                    kwargs[name.c_str()] = *dblProps[name];
                    break;
                case CProperty::Vec3:
                    kwargs[name.c_str()] = *vec3Props[name];
                    break;
                case CProperty::String:
                    kwargs[name.c_str()] = strProps[name]->toStdString().c_str();
                    break;
                case CProperty::Enum:
                    kwargs[name.c_str()] = pybind11::make_tuple(*enumProps[name], current.values[*enumProps[name]].toStdString().c_str());
                    break;
                case CProperty::Resource:
                    kwargs[name.c_str()] = rscProps[name]->toStdString().c_str();
                    break;

                default:
                    return false;
            };
        }

        auto func = m_wnd->GetPythonToolsPanel()->GetThread()->GetState()->GetFunc(m_id);

        (*func)(**kwargs);
    }
    catch(pybind11::error_already_set &e)
    {
        // Print the error message
        pybind11::print(e.what());

        // Return execution to Python to allow the thread to exit
        e.restore();

        // Clear the error to allow further Python execution. 
        PyErr_Clear();
    }

    return true;
}

// Dummy tool

CPythonDummyTool::CPythonDummyTool(const char* name, int id)
    : name(name), m_id(id)
{

}

void CPythonDummyTool::addBoolProperty(const std::string& name, bool value)
{
    propOrder.push_back(CProperty::Bool);
    boolProps.emplace(name, value);
}

void CPythonDummyTool::addIntProperty(const std::string& name, int value)
{
    propOrder.push_back(CProperty::Int);
    intProps.emplace(name, value);
}

void CPythonDummyTool::addEnumProperty(const std::string& name, const std::string& labels, int value)
{
    propOrder.push_back(CProperty::Enum);
    enumProps.emplace(name, value);
    enumLabels.emplace(labels);
}

void CPythonDummyTool::addDoubleProperty(const std::string& name, double value)
{
    propOrder.push_back(CProperty::Float);
    dblProps.emplace(name, value);
}

void CPythonDummyTool::addVec3Property(const std::string& name, vec3d value)
{
    propOrder.push_back(CProperty::Vec3);
    vec3Props.emplace(name, value);
}

void CPythonDummyTool::addStringProperty(const std::string& name, const char* value)
{
    propOrder.push_back(CProperty::String);
    strProps.emplace(name, value);
}

void CPythonDummyTool::addResourceProperty(const std::string& name, const char* value)
{
    propOrder.push_back(CProperty::Resource);
    rscProps.emplace(name, value);
}

#else
CPythonTool::CPythonTool(CMainWindow* wnd, std::string name, pybind11::function func) : CBasicTool(wnd, name.c_str(), HAS_APPLY_BUTTON) {}
CPythonTool::~CPythonTool() {}
void CPythonTool::addBoolProperty(const std::string& name, bool value) {}
void CPythonTool::addIntProperty(const std::string& name, int value) {}
void CPythonTool::addEnumProperty(const std::string& name, const std::string& labels, int value) {}
void CPythonTool::addDoubleProperty(const std::string& name, double value) {}
void CPythonTool::addVec3Property(const std::string& name, vec3d value) {}
void CPythonTool::addStringProperty(const std::string& name, std::string value) {}
void CPythonTool::addResourceProperty(const std::string& name, std::string value) {}
bool CPythonTool::OnApply() {return false;}
bool CPythonTool::runFunc() {return false;}
#endif