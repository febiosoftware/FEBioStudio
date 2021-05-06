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

#pragma once
#include <FEBioStudio/Tool.h>
#include <unordered_map>


// Forward declaration of PyObject
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif

class CPythonTool : public CBasicTool
{

public:
	CPythonTool(CMainWindow* wnd, const char* name, PyObject* func);
    ~CPythonTool();

    CProperty* addBoolProperty(const std::string& name, bool value);
	CProperty* addIntProperty(const std::string& name, int value);
	CProperty* addEnumProperty(const std::string& name, int value, const std::string& labels);
	CProperty* addDoubleProperty(const std::string& name, double value);
    CProperty* addStringProperty(const std::string& name, char* value);
    CProperty* addResourceProperty(const std::string& name, char* value);

    bool OnApply();


private:
    PyObject* func;
    std::unordered_map<std::string, bool*> boolProps;
    std::unordered_map<std::string, int*> intProps;
    std::unordered_map<std::string, int*> enumProps;
    std::unordered_map<std::string, double*> dblProps;
    std::unordered_map<std::string, QString*> strProps;
    std::unordered_map<std::string, QString*> rscProps;
};
