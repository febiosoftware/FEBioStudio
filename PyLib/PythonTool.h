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

#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#else
// Since objects of this type are member variables of these classes, they require
// "full" class definitions, rather than just forward declarations
namespace pybind11
{
    class function {};
    class dict {};
}
#endif

#include <FEBioStudio/Tool.h>
#include <queue>
#include <unordered_map>

class CPythonTool : public CBasicTool
{

public:
    CPythonTool(CMainWindow* wnd, std::string name, int id);
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
    int m_id;
    CMainWindow* m_wnd;
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
    CPythonDummyTool(const char* name, int id);

    void addBoolProperty(const std::string& name, bool value = true);
	void addIntProperty(const std::string& name, int value = 0);
	void addEnumProperty(const std::string& name, const std::string& labels, int value = 0);
	void addDoubleProperty(const std::string& name, double value = 0);
    void addVec3Property(const std::string& name, vec3d value = vec3d());
    void addStringProperty(const std::string& name, const char* value = "");
    void addResourceProperty(const std::string& name, const char* value = "");

    int m_id;
    std::string name;
    std::vector<int> propOrder;
    std::queue<std::pair<std::string, bool>> boolProps;
    std::queue<std::pair<std::string, int>> intProps;
    std::queue<std::pair<std::string, int>> enumProps;
    std::queue<std::string> enumLabels;
    std::queue<std::pair<std::string, double>> dblProps;
    std::queue<std::pair<std::string, vec3d>> vec3Props;
    std::queue<std::pair<std::string, std::string>> strProps;
    std::queue<std::pair<std::string, std::string>> rscProps;
};