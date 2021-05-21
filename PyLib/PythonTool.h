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
#include <pybind11/pybind11.h>
#include <FEBioStudio/Tool.h>
#include <unordered_map>

class CPythonTool : public CBasicTool
{

public:
    CPythonTool(CMainWindow* wnd, std::string name, pybind11::function func);
    ~CPythonTool();

    void addBoolProperty(const std::string& name, bool value = true);
	void addIntProperty(const std::string& name, int value = 0);
	void addEnumProperty(const std::string& name, const std::string& labels, int value = 0);
	void addDoubleProperty(const std::string& name, double value = 0);
    void addVec3Property(const std::string& name, vec3d value = vec3d());
    void addStringProperty(const std::string& name, std::string = "");
    void addResourceProperty(const std::string& name, std::string = "");

    bool OnApply();

    bool runFunc();

private:
    pybind11::function func;
    std::unordered_map<std::string, bool*> boolProps;
    std::unordered_map<std::string, int*> intProps;
    std::unordered_map<std::string, int*> enumProps;
    std::unordered_map<std::string, double*> dblProps;
    std::unordered_map<std::string, vec3d*> vec3Props;
    std::unordered_map<std::string, QString*> strProps;
    std::unordered_map<std::string, QString*> rscProps;
};


class CPythonDummyTool
{
public:
    CPythonDummyTool(const char* name, pybind11::function func);

    void addBoolProperty(const std::string& name, bool value = true);
	void addIntProperty(const std::string& name, int value = 0);
	void addEnumProperty(const std::string& name, const std::string& labels, int value = 0);
	void addDoubleProperty(const std::string& name, double value = 0);
    void addVec3Property(const std::string& name, vec3d value = vec3d());
    void addStringProperty(const std::string& name, char* value = "");
    void addResourceProperty(const std::string& name, char* value = "");


    std::string name;
    pybind11::function func;
    std::unordered_map<std::string, bool> boolProps;
    std::unordered_map<std::string, int> intProps;
    std::unordered_map<std::string, int> enumProps;
    std::unordered_map<std::string, std::string> enumLabels;
    std::unordered_map<std::string, double> dblProps;
    std::unordered_map<std::string, vec3d> vec3Props;
    std::unordered_map<std::string, std::string> strProps;
    std::unordered_map<std::string, std::string> rscProps;
};