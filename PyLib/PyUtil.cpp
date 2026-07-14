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

#ifdef HAS_PYTHON
#include "PyUtil.h"
#include <FEBioStudio/PropertyList.h> // for Mat3dToString, Mat3dsToString, StringToMat3d, StringToMat3ds (TODO: Move these functions to FSCore?)
#include <FEBioLink/FEBioClass.h>

std::vector<double> DoubleSequenceFromPython(py::handle value, size_t size, const std::string& name)
{
	if (!py::isinstance<py::sequence>(value) || py::isinstance<py::str>(value))
	{
		throw py::type_error(name + " expects a sequence");
	}

	py::sequence seq = py::reinterpret_borrow<py::sequence>(value);
	if (seq.size() != size)
	{
		throw py::value_error(name + " expects " + std::to_string(size) + " values");
	}

	std::vector<double> v(size);
	for (size_t i = 0; i < size; ++i) v[i] = seq[i].cast<double>();
	return v;
}

py::tuple Vec2iToPython(const vec2i& v)
{
	return py::make_tuple(v.x, v.y);
}

py::tuple Vec2dToPython(const vec2d& v)
{
	return py::make_tuple(v.x(), v.y());
}

py::list Vec2dVectorToPython(const std::vector<vec2d>& v)
{
	py::list out;
	for (const vec2d& pi : v) out.append(Vec2dToPython(pi));
	return out;
}

std::vector<vec2d> Vec2dVectorFromPython(py::handle value)
{
	if (!py::isinstance<py::sequence>(value) || py::isinstance<py::str>(value))
	{
		throw py::type_error("expected a sequence of 2-value sequences");
	}

	std::vector<vec2d> v;
	py::sequence seq = py::reinterpret_borrow<py::sequence>(value);
	v.reserve(seq.size());

	for (py::handle item : seq)
	{
		std::vector<double> p = DoubleSequenceFromPython(item, 2, "vec2d");
		v.push_back(vec2d(p[0], p[1]));
	}

	return v;
}

vec3d Vec3dFromPython(py::handle value, const std::string& typeName)
{
	try
	{
		return value.cast<vec3d>();
	}
	catch (const py::cast_error&)
	{
		std::vector<double> v = DoubleSequenceFromPython(value, 3, typeName);
		return vec3d(v[0], v[1], v[2]);
	}
}

py::object ParamToPython(Param& p)
{
	switch (p.GetParamType())
	{
	case Param_INT:
	case Param_CHOICE:
		return py::int_(p.GetIntValue());
	case Param_FLOAT:
		return py::float_(p.GetFloatValue());
	case Param_BOOL:
		return py::bool_(p.GetBoolValue());
	case Param_VEC3D:
		return py::cast(p.GetVec3dValue());
	case Param_STRING:
		return py::str(p.GetStringValue());
	case Param_MATH:
		return py::str(p.GetMathString());
	case Param_COLOR:
		return py::cast(p.GetColorValue());
	case Param_MAT3D:
		return py::str(Mat3dToString(p.GetMat3dValue()).toStdString());
	case Param_MAT3DS:
		return py::str(Mat3dsToString(p.GetMat3dsValue()).toStdString());
	case Param_VEC2I:
		return Vec2iToPython(p.GetVec2iValue());
	case Param_VEC2D:
		return Vec2dToPython(p.GetVec2dValue());
	case Param_STD_VECTOR_INT:
		return py::cast(p.GetVectorIntValue());
	case Param_STD_VECTOR_DOUBLE:
		return py::cast(p.GetVectorDoubleValue());
	case Param_STD_VECTOR_VEC2D:
		return py::cast(PyVec2dList(&p));
	case Param_ARRAY_INT:
		return py::cast(p.GetArrayIntValue());
	case Param_ARRAY_DOUBLE:
		return py::cast(p.GetArrayDoubleValue());
	case Param_URL:
		return py::str(p.GetURLValue());
	default:
		throw py::type_error("unsupported parameter type");
	}
}

void SetParamFromPython(Param& p, py::handle value)
{
	switch (p.GetParamType())
	{
	case Param_INT:
	case Param_CHOICE:
		p.SetIntValue(value.cast<int>());
		break;
	case Param_FLOAT:
		p.SetFloatValue(value.cast<double>());
		break;
	case Param_BOOL:
		p.SetBoolValue(value.cast<bool>());
		break;
	case Param_VEC3D:
	{
		try
		{
			p.SetVec3dValue(value.cast<vec3d>());
		}
		catch (const py::cast_error&)
		{
			std::vector<double> v = DoubleSequenceFromPython(value, 3, "vec3d");
			p.SetVec3dValue(vec3d(v[0], v[1], v[2]));
		}
	}
	break;
	case Param_STRING:
		p.SetStringValue(value.cast<std::string>());
		break;
	case Param_MATH:
		p.SetMathString(value.cast<std::string>());
		break;
	case Param_COLOR:
		p.SetColorValue(value.cast<GLColor>());
		break;
	case Param_MAT3D:
		p.SetMat3dValue(StringToMat3d(QString::fromStdString(value.cast<std::string>())));
		break;
	case Param_MAT3DS:
		p.SetMat3dsValue(StringToMat3ds(QString::fromStdString(value.cast<std::string>())));
		break;
	case Param_VEC2I:
	{
		std::vector<double> v = DoubleSequenceFromPython(value, 2, "vec2i");
		p.SetVec2iValue(vec2i((int)v[0], (int)v[1]));
	}
	break;
	case Param_VEC2D:
	{
		std::vector<double> v = DoubleSequenceFromPython(value, 2, "vec2d");
		p.SetVec2dValue(vec2d(v[0], v[1]));
	}
	break;
	case Param_STD_VECTOR_INT:
		p.SetVectorIntValue(value.cast<std::vector<int>>());
		break;
	case Param_STD_VECTOR_DOUBLE:
		p.SetVectorDoubleValue(value.cast<std::vector<double>>());
		break;
	case Param_STD_VECTOR_VEC2D:
		p.SetVectorVec2dValue(Vec2dVectorFromPython(value));
		break;
	case Param_ARRAY_INT:
		p.SetArrayIntValue(value.cast<std::vector<int>>());
		break;
	case Param_ARRAY_DOUBLE:
		p.SetArrayDoubleValue(value.cast<std::vector<double>>());
		break;
	case Param_URL:
		p.SetURLValue(value.cast<std::string>());
		break;
	default:
		throw py::type_error("unsupported parameter type");
	}

	p.SetModified(true);
}

void PyPropertySlot::create(const std::string& type)
{
	FSProperty* prop = getProperty();

	FSModelComponent* pmc = dynamic_cast<FSModelComponent*>(m_pmc);
	if (pmc == nullptr)
	{
		throw std::runtime_error("component is not a model component");
	}

	FSModel* model = pmc->GetFSModel();
	if (model == nullptr)
	{
		throw std::runtime_error("component is not attached to a model");
	}

	FSModelComponent* component = FEBio::CreateClass(prop->GetSuperClassID(), type, model, prop->GetFlags());
	if (component == nullptr)
	{
		throw py::value_error("failed to create property '" + m_name + "' with type '" + type + "'");
	}

	prop->SetComponent(component);
}

py::object GetDynamicAttribute(FSObject& self, const std::string& name)
{
	Param* p = self.GetParam(name.c_str());
	if (p) return ParamToPython(*p);

	FSCoreBase* pc = dynamic_cast<FSCoreBase*>(&self);
	if (pc)
	{
		FSProperty* prop = pc->FindProperty(name);
		if (prop) return py::cast(PyPropertySlot(pc, name));
	}
	throw py::attribute_error("unknown attribute: " + name);
}

void SetDynamicAttribute(FSObject& self, const std::string& name, py::object value)
{
	if (name == "name") {
		self.SetName(value.cast<std::string>());
		return;
	}

	Param* p = self.GetParam(name.c_str());
	if (p) {
		SetParamFromPython(*p, value);
		return;
	}

	FSCoreBase* pc = dynamic_cast<FSCoreBase*>(&self);
	if (pc)
	{
		FSProperty* prop = pc->FindProperty(name);
		if (prop) {
			if (value.is_none()) {
				prop->SetComponent(nullptr);
				return;
			}
			throw py::type_error("property '" + name + "' can only be assigned None");
		}
	}

	throw py::attribute_error("unknown attribute: " + name);
}

#endif // HAS_PYTHON
