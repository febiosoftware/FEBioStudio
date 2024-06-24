/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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

#include "stdafx.h"
#include "ObjectProps.h"
#include <FSCore/FSObject.h>
#include <FSCore/FSCore.h>

//=======================================================================================
CObjectProps::CObjectProps(FSBase* po)
{
	m_po = 0;
	if (po) BuildParamList(po);
}

void CObjectProps::AddParameter(Param& p)
{
	CProperty* prop = nullptr;

	const char* szname = p.GetLongName();
	string sname = FSCore::beautify_string(szname);
	QString paramName = QString::fromStdString(sname);

	switch (p.GetParamType())
	{
	case Param_FLOAT: 
	{
		prop = addProperty(paramName, CProperty::Float);
		if (p.UseRange())
		{
			prop->setFloatRange(p.GetFloatMin(), p.GetFloatMax());
			prop->setFloatStep(p.GetFloatStep());
		}
	}
	break;
	case Param_VEC3D : prop = addProperty(paramName, CProperty::Vec3); break;
	case Param_MAT3D : prop = addProperty(paramName, CProperty::Mat3); break;
	case Param_MAT3DS: prop = addProperty(paramName, CProperty::Mat3s); break;
	case Param_STRING: 
	{
		prop = addProperty(paramName, CProperty::String); 
		if (p.GetEnumNames())
		{
			QStringList ops = GetEnumValues(p.GetEnumNames());
			prop->setEnumValues(ops);
		}
	}
	break;
	case Param_MATH  : prop = addProperty(paramName, CProperty::MathString); break;
	case Param_COLOR : prop = addProperty(paramName, CProperty::Color); break;
	case Param_VEC2I : prop = addProperty(paramName, CProperty::Vec2i); break;
	case Param_BOOL:
	{
		const char* ch = p.GetEnumNames();
		if (ch)
		{
			QStringList ops;
			ops << QString(ch);
			ch = ch + strlen(ch) + 1;
			ops << QString(ch);
			prop = addProperty(paramName, CProperty::Enum);
			prop->setEnumValues(ops);
			break;
		}
		else prop = addProperty(paramName, CProperty::Bool);
	}
	break;
	case Param_CHOICE:
	case Param_INT:
	{
		const char* ch = p.GetEnumNames();
		if (ch)
		{
			if (ch[0] == '@')
			{
				if (strcmp(ch, "@data_scalar") == 0)
				{
					prop = addProperty(paramName, CProperty::DataScalar);
				}
				else if (strcmp(ch, "@data_vec3") == 0)
				{
					prop = addProperty(paramName, CProperty::DataVec3);
				}
				else if (strcmp(ch, "@data_mat3") == 0)
				{
					prop = addProperty(paramName, CProperty::DataMat3);
				}
				else if (strcmp(ch, "@color_map") == 0)
				{
					prop = addProperty(paramName, CProperty::ColorMap);
				}
			}
			else
			{
				QStringList ops = GetEnumValues(ch);
				prop = addProperty(paramName, CProperty::Enum);
				prop->setEnumValues(ops);
			}
			break;
		}
		else
		{
			prop = addProperty(paramName, CProperty::Int);
			if (p.UseRange())
			{
				prop->setIntRange(p.GetIntMin(), p.GetIntMax());
			}
		}
	}
	break;
	case Param_STD_VECTOR_INT:
	{
		prop = addProperty(paramName, CProperty::Std_Vector_Int);
		const char* szenum = p.GetEnumNames();
		if (szenum)
		{
			QStringList ops = GetEnumValues(szenum);
			prop->setEnumValues(ops);
		}
	}
	break;
	case Param_STD_VECTOR_DOUBLE:
	{
		prop = addProperty(paramName, CProperty::Std_Vector_Double);
	}
	break;
	case Param_ARRAY_INT:
	{
		prop = addProperty(paramName, CProperty::Std_Vector_Int);
	}
	break;
	case Param_ARRAY_DOUBLE:
	{
		prop = addProperty(paramName, CProperty::Std_Vector_Double);
	}
	break;
	default:
		prop = addProperty(paramName, CProperty::String);
		prop->setFlags(CProperty::Visible);
		break;
	}
	if (prop && (p.IsVisible() && (p.IsEditable() == false))) prop->flags = CProperty::Visible;
	if (prop && p.IsVariable() && (p.IsEditable())) prop->flags |= CProperty::Variable;
	if (prop) prop->param = &p;

	m_params.push_back(&p);
}

QStringList CObjectProps::GetEnumValues(const char* ch)
{
	QStringList ops;
	while (ch && (*ch))
	{
		ops << QString(ch);
		ch = ch + strlen(ch) + 1;
	}
	return ops;
}

void CObjectProps::BuildParamList(FSBase* po, bool showNonPersistent)
{
	m_po = po;
	m_params.clear();
	int NP = po->Parameters();
	int ng = -1;
	ParamBlock& PB = po->GetParamBlock();
	for (int i = 0; i<NP; ++i)
	{
		Param& p = po->GetParam(i);
		if ((showNonPersistent || p.IsPersistent()) && (p.IsEditable() && p.IsVisible()))
		{
			if (p.GetParameterGroup() != ng)
			{
				ng = p.GetParameterGroup();
				const char* sz = PB.GetParameterGroupName(ng);
				if (sz)
				{
					addProperty(sz, CProperty::Group);
					m_params.push_back(nullptr);
				}
			}
			AddParameter(p);
		}
	}
}

void CObjectProps::AddParameterList(FSBase* po)
{
	int NP = po->Parameters();
	for (int i = 0; i < NP; ++i)
	{
		Param& p = po->GetParam(i);
		if (p.IsEditable() || p.IsVisible())
		{
			AddParameter(p);
		}
	}
}

QVariant CObjectProps::GetPropertyValue(Param& p)
{
	switch (p.GetParamType())
	{
	case Param_CHOICE:
	case Param_INT: return p.GetIntValue(); break;
	case Param_FLOAT: return p.GetFloatValue(); break;
	case Param_STRING: return QString::fromStdString(p.GetStringValue()); break;
	case Param_MATH  : return QString::fromStdString(p.GetMathString()); break;
	case Param_BOOL:
	{
		if (p.GetEnumNames()) return (p.GetBoolValue() ? 1 : 0);
		else return p.GetBoolValue();
	}
	break;
	case Param_VEC3D:
	{
		vec3d r = p.GetVec3dValue();
		QString t = Vec3dToString(r);
		return t;
	}
	break;
	case Param_VEC2I:
	{
		vec2i r = p.GetVec2iValue();
		QString t = Vec2iToString(r);
		return t;
	}
	break;
	case Param_MAT3D:
	{
		mat3d m = p.GetMat3dValue();
		QString t = Mat3dToString(m);
		return t;
	}
	break;
	case Param_MAT3DS:
	{
		mat3ds m = p.GetMat3dsValue();
		QString t = Mat3dsToString(m);
		return t;
	}
	break;
	case Param_COLOR:
	{
		GLColor c = p.GetColorValue();
		QColor qcol = toQColor(c);
		return qcol;
	}
	break;
	case Param_STD_VECTOR_INT:
	{
		std::vector<int> v = p.GetVectorIntValue();
		QString t = VectorIntToString(v);
		return t;
	}
	break;
	case Param_STD_VECTOR_DOUBLE:
	{
		std::vector<double> v = p.GetVectorDoubleValue();
		QString t = VectorDoubleToString(v);
		return t;
	}
	break;
	case Param_ARRAY_INT:
	{
		std::vector<int> v = p.GetArrayIntValue();
		QString t = VectorIntToString(v);
		return t;
	}
	break;
	case Param_ARRAY_DOUBLE:
	{
		std::vector<double> v = p.GetArrayDoubleValue();
		QString t = VectorDoubleToString(v);
		return t;
	}
	break;
	default:
		return "(not supported)";
	}

	return QVariant();
}

QVariant CObjectProps::GetPropertyValue(int i)
{
	Param* p = m_params[i];
	return (p ? GetPropertyValue(*p) : QVariant());
}

void CObjectProps::SetPropertyValue(Param& p, const QVariant& v)
{
	switch (p.GetParamType())
	{
	case Param_CHOICE:
	case Param_INT: p.SetIntValue(v.toInt()); break;
	case Param_FLOAT: p.SetFloatValue(v.toDouble()); break;
	case Param_STRING: p.SetStringValue(v.toString().toStdString()); break;
	case Param_MATH  : p.SetMathString(v.toString().toStdString()); break;
	case Param_BOOL:
	{
		if (p.GetEnumNames()) p.SetBoolValue(v.toInt() != 0);
		else p.SetBoolValue(v.toBool());
	}
	break;
	case Param_VEC3D:
	{
		QString t = v.toString();
		vec3d r = StringToVec3d(t);
		p.SetVec3dValue(r);
	}
	break;
	case Param_VEC2I:
	{
		QString t = v.toString();
		vec2i r = StringToVec2i(t);
		p.SetVec2iValue(r);
	}
	break;
	case Param_MAT3D:
	{
		QString t = v.toString();
		mat3d m = StringToMat3d(t);
		p.SetMat3dValue(m);
	}
	break;
	case Param_MAT3DS:
	{
		QString t = v.toString();
		mat3ds m = StringToMat3ds(t);
		p.SetMat3dsValue(m);
	}
	break;
	case Param_COLOR:
	{
		QColor qcol = v.value<QColor>();
		GLColor c = toGLColor(qcol);
		p.SetColorValue(c);
	}
	break;
	case Param_STD_VECTOR_INT:
	{
		QString s = v.toString();
		std::vector<int> val = StringToVectorInt(s);
		p.SetVectorIntValue(val);
	}
	break;
	case Param_STD_VECTOR_DOUBLE:
	{
		QString s = v.toString();
		std::vector<double> val = StringToVectorDouble(s);
		if (val.empty() == false)
		{
			// Make sure we don't change the vector's size
			int n = p.GetVectorDoubleValue().size();
			if ((n != 0) && (n != val.size()))
			{
				std::vector<double> tmp = p.GetVectorDoubleValue();
				if (val.size() < n) n = val.size();
				for (int i = 0; i < n; ++i) tmp[i] = val[i];
				val = tmp;
			}
			p.SetVectorDoubleValue(val);
		}
	}
	break;
	case Param_ARRAY_INT:
	{
		QString s = v.toString();
		std::vector<int> val = StringToVectorInt(s);
		p.SetArrayIntValue(val);
	}
	break;
	case Param_ARRAY_DOUBLE:
	{
		QString s = v.toString();
		std::vector<double> val = StringToVectorDouble(s);
		p.SetArrayDoubleValue(val);
	}
	break;
	default:
		assert(false);
	}
}

void CObjectProps::SetPropertyValue(int i, const QVariant& v)
{
	Param& p = *m_params[i];
	SetPropertyValue(p, v);
	if (m_po && m_po->UpdateData())
	{
		Clear();
		BuildParamList(m_po);
		SetModified(true);
	}
}