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
#include "PythonThread.h"
#endif


#include <FEBioStudio/Tool.h>

class CPythonToolProps : public CCachedPropertyList
{
public:
	CPythonToolProps(const char* name, int id, pybind11::function func) : m_name(name), m_id(id), m_func(func) {}

	int GetID() const { return m_id; }

	const std::string& GetName() { return m_name; }

	pybind11::function GetFunction() { return m_func; }

	const std::string& GetInfo() { return m_info; }

	void setInfo(const std::string& info) { m_info = info; }
	void addBoolProperty(const std::string& name, bool v);
	void addIntProperty(const std::string& name, int v);
	void addDoubleProperty(const std::string& name, double v);
	void addVec3Property(const std::string& name, vec3d v);
	void addEnumProperty(const std::string& name, const std::string& labels, int v);
	void addStringProperty(const std::string& name, const char* v);
	void addResourceProperty(const std::string& name, const char* v);

private:
	int m_id;
	std::string m_name;
	std::string m_info;
	pybind11::function m_func;
};

class CPythonTool : public CAbstractTool
{
public:
	CPythonTool(CMainWindow* wnd, std::string name, int id);
	~CPythonTool();

public:
	void OnApply();
	bool runFunc();

	void SetProperties(CPythonToolProps* props);

	// A form will be created based on the property list
	QWidget* createUi();

private:
	int m_id;
	CPythonToolProps* m_props;
};
