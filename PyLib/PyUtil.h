/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2026 University of Utah, The Trustees of Columbia University in
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

//NOTE: this file should only be included in files that are compiled with python support (HAS_PYTHON defined)
#include <functional>
#include <string>
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <FSCore/math3d.h>
#include <FSCore/ParamBlock.h>
#include <FEMLib/FELoadController.h>
#include <FEMLib/FEModelComponent.h>
#include <FEMLib/FSModel.h>

namespace py = pybind11;

std::vector<double> DoubleSequenceFromPython(py::handle value, size_t size, const std::string& name);
py::tuple Vec2iToPython(const vec2i& v);
py::tuple Vec2dToPython(const vec2d& v);
py::list Vec2dVectorToPython(const std::vector<vec2d>& v);
std::vector<vec2d> Vec2dVectorFromPython(py::handle value);
vec3d Vec3dFromPython(py::handle value, const std::string& typeName = "vec3d");

py::object ParamToPython(Param& p);
void SetParamFromPython(Param& p, py::handle value);

class PyVec2dList
{
public:
	PyVec2dList(Param* param) : m_param(param) {}

	int size() const
	{
		return (int)m_param->GetVectorVec2dValue().size();
	}

	py::tuple get(int i) const
	{
		std::vector<vec2d> v = m_param->GetVectorVec2dValue();
		int n = (int)v.size();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
		{
			throw py::index_error("point index out of range");
		}

		return Vec2dToPython(v[i]);
	}

	void add(double x, double y)
	{
		std::vector<vec2d> v = m_param->GetVectorVec2dValue();
		v.push_back(vec2d(x, y));
		m_param->SetVectorVec2dValue(v);
		m_param->SetModified(true);
	}

	void add(py::handle value)
	{
		std::vector<double> p = DoubleSequenceFromPython(value, 2, "point");
		add(p[0], p[1]);
	}

	void clear()
	{
		m_param->SetVectorVec2dValue(std::vector<vec2d>());
		m_param->SetModified(true);
	}

	py::iterator iter() const
	{
		return py::iter(Vec2dVectorToPython(m_param->GetVectorVec2dValue()));
	}

private:
	Param* m_param = nullptr;
};

class PyParameter
{
public:
	PyParameter(Param* param, FSModel* mdl = nullptr) : m_param(param), m_model(mdl) {}

	py::object value() const
	{
		return ParamToPython(*m_param);
	}

	void setValue(py::object value)
	{
		SetParamFromPython(*m_param, value);
	}

	py::object lcID() const
	{
		int id = m_param->GetLoadCurveID();
		if (id < 0) return py::none();
		return py::int_(id);
	}

	void setLC(FSLoadController* plc)
	{
		if (plc == nullptr)
			m_param->SetLoadCurveID(-1);
		else
			m_param->SetLoadCurveID(plc->GetID());
	}

	FSLoadController* getLC() const
	{
		int id = m_param->GetLoadCurveID();
		if (id < 0) return nullptr;
		if (m_model == nullptr) return nullptr;
		return m_model->GetLoadControllerFromID(id);
	}

	void setLCID(py::object value)
	{
		if (value.is_none())
			m_param->SetLoadCurveID(-1);
		else
			m_param->SetLoadCurveID(value.cast<int>());

		m_param->SetModified(true);
	}

	std::string name() const
	{
		const char* sz = m_param->GetShortName();
		return (sz ? sz : "");
	}

	std::string longName() const
	{
		const char* sz = m_param->GetLongName();
		return (sz ? sz : "");
	}

	std::string unit() const
	{
		const char* sz = m_param->GetUnit();
		return (sz ? sz : "");
	}

private:
	Param* m_param = nullptr;
	FSModel* m_model = nullptr;
};

class PyParameterList
{
public:
	PyParameterList(ParamContainer* params, FSModel* mdl = nullptr) : m_params(params), m_model(mdl) {}

	int size() const
	{
		return m_params->Parameters();
	}

	PyParameter get(int i) const
	{
		int n = m_params->Parameters();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
		{
			throw py::index_error("parameter index out of range");
		}

		return PyParameter(&m_params->GetParam(i), m_model);
	}

	PyParameter get(const std::string& name) const
	{
		Param* p = m_params->GetParam(name.c_str());
		if (p == nullptr)
		{
			throw py::key_error("parameter not found: " + name);
		}

		return PyParameter(p, m_model);
	}

	py::list iter() const
	{
		py::list items;
		for (int i = 0; i < size(); ++i)
			items.append(get(i));
		return items;
	}

	bool contains(const std::string& name) const
	{
		return m_params->GetParam(name.c_str()) != nullptr;
	}

private:
	ParamContainer* m_params = nullptr;
	FSModel* m_model = nullptr;
};


class PyPropertySlot
{
public:
	PyPropertySlot(FSCoreBase* pmc, const std::string& name) : m_pmc(pmc), m_name(name) {}

	void create(const std::string& type);

	void clear()
	{
		getProperty()->SetComponent(nullptr);
	}

	bool isSet() const
	{
		return getProperty()->GetComponent() != nullptr;
	}

	py::object typeStr() const
	{
		FSCoreBase* component = getProperty()->GetComponent();
		if (component == nullptr) return py::none();

		const char* type = component->GetTypeString();
		if (type) return py::str(type);
		return py::none();
	}

private:
	FSProperty* getProperty() const
	{
		FSProperty* prop = m_pmc->FindProperty(m_name);
		if (prop == nullptr)
		{
			throw py::attribute_error("unknown property: " + m_name);
		}
		return prop;
	}

private:
	FSCoreBase* m_pmc = nullptr;
	std::string m_name;
};

py::object GetDynamicAttribute(FSObject& self, const std::string& name);
void SetDynamicAttribute(FSObject& self, const std::string& name, py::object value);
void SetDynamicAttributes(FSObject& self, py::kwargs kwargs);

template <class Owner, class Item>
class PyIndexedCollection
{
public:
	using CountFn = std::function<int(Owner*)>;
	using GetFn = std::function<Item* (Owner*, int)>;

	PyIndexedCollection(Owner* owner, CountFn count, GetFn get, std::string itemName = "item")
		: m_owner(owner), m_count(count), m_get(get), m_itemName(std::move(itemName)) {}

	int size() const { return m_count(m_owner); }

	Item* get(int i) const
	{
		int n = size();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
			throw py::index_error(m_itemName + " index out of range");

		return m_get(m_owner, i);
	}

	py::iterator iter() const
	{
		py::list items;
		for (int i = 0; i < size(); ++i)
			items.append(py::cast(get(i), py::return_value_policy::reference));
		return py::iter(items);
	}

private:
	Owner* m_owner;
	CountFn m_count;
	GetFn m_get;
	std::string m_itemName;
};

template <class Owner, class Item>
class PyNamedCollection
{
public:
	using CountFn = std::function<int(Owner*)>;
	using GetFn = std::function<Item* (Owner*, int)>;
	using FindFn = std::function<Item* (Owner*, const std::string&)>;

	PyNamedCollection(Owner* owner, CountFn count, GetFn get, FindFn find, std::string itemName = "item")
		: m_owner(owner), m_count(count), m_get(get), m_find(find), m_itemName(std::move(itemName)) {}

	int size() const
	{
		return m_count(m_owner);
	}

	Item* get(int i) const
	{
		int n = size();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
		{
			throw py::index_error(m_itemName + " index out of range");
		}

		return m_get(m_owner, i);
	}

	Item* get(const std::string& name) const
	{
		Item* item = m_find(m_owner, name);
		if (item == nullptr)
		{
			throw py::key_error(m_itemName + " not found: " + name);
		}

		return item;
	}

	py::iterator iter() const
	{
		py::list items;
		for (int i = 0; i < size(); ++i)
		{
			items.append(py::cast(get(i), py::return_value_policy::reference));
		}
		return py::iter(items);
	}

private:
	Owner* m_owner;
	CountFn m_count;
	GetFn m_get;
	FindFn m_find;
	std::string m_itemName;
};
