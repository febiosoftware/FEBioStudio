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

typedef size_t python_handle;

// The CPythonToolProps hold the properties of the tool. It also holds the python
// function that should be called to run the tool.
class CPythonToolProps : public CCachedPropertyList
{
	class Imp;

public:
	CPythonToolProps(const char* name, python_handle f);
	~CPythonToolProps();

	void SetID(int id);
	int GetID() const;

	const std::string& GetName();

	python_handle GetFunction();

	const std::string& GetInfo();

	void setInfo(const std::string& info);

public:
	void addBoolProperty(const std::string& name, bool v);
	void addIntProperty(const std::string& name, int v);
	void addDoubleProperty(const std::string& name, double v);
	void addVec3Property(const std::string& name, vec3d v);
	void addEnumProperty(const std::string& name, const std::string& labels, int v);
	void addStringProperty(const std::string& name, const char* v);
	void addResourceProperty(const std::string& name, const char* v);

private:
	Imp* m;
};

// The CPythonTool class creates the UI component that will be added 
// to the Python panel. It manages a CPythonToolProps that contains the properties of the tool.
class CPythonTool : public CAbstractTool
{
public:
	CPythonTool(CMainWindow* wnd, std::string name);
	~CPythonTool();

public:
	void SetProperties(CPythonToolProps* props);

	CPythonToolProps* GetProperties();

	// A form will be created based on the property list
	QWidget* createUi();

private:
	CPythonToolProps* m_props;
};
