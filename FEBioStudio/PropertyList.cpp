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

#include "PropertyList.h"
#include <FSCore/math3d.h>
#include <FSCore/util.h>

#include <string>
#include <QDebug>
#include <iostream>
using namespace std;

CProperty& CProperty::operator = (const CProperty& p)
{
	type = p.type;
	flags = p.flags;
	name = p.name;
	info = p.info;
	values = p.values;
	imin = p.imin;
	imax = p.imax;
	fmin = p.fmin;
	fmax = p.fmax;
	fstep = p.fstep;
	bauto = p.bauto;
	pdata = p.pdata;
	param = p.param;
	brange = p.brange;
	return *this;
}

CProperty::CProperty(const QString& sname, CProperty::Type itype) : name(sname), type(itype), info(sname)
{
	flags = Visible | Editable;
	imin = -123456789;
	imax = 123457689;
	fmin = -1e99;
	fmax =  1e99;
	fstep = 0.01;
	bauto = false;
	brange = false;
	param = nullptr;
}

CProperty::CProperty(const QString& sname, CProperty::Type itype, const QString& sinfo) : name(sname), type(itype), info(sinfo)
{
	flags = Visible | Editable;
	imin = -123456789;
	imax = 123456789;
	fmin = -1e99;
	fmax =  1e99;
	fstep = 0.01;
	bauto = false;
	brange = false;
	param = nullptr;
}

int CPropertyList::FindPropertyIndex(const QString& propName) const
{
	for (int i = 0; i < Properties(); ++i)
	{
		const CProperty& p = Property(i);
		if (p.name == propName) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
CDataPropertyList::CDataPropertyList()
{
}

CProperty* CDataPropertyList::addBoolProperty(bool* pd, const QString& name)
{
	return addProperty(name, CProperty::Bool)->setData(pd);
}

CProperty* CDataPropertyList::addIntProperty(int* pd, const QString& name)
{
	return addProperty(name, CProperty::Int)->setData(pd);
}

CProperty* CDataPropertyList::addEnumProperty(int* pd, const QString& name)
{
	return addProperty(name, CProperty::Enum)->setData(pd);
}

CProperty* CDataPropertyList::addDoubleProperty(double* pd, const QString& name)
{
	return addProperty(name, CProperty::Float)->setData(pd);
}

CProperty* CDataPropertyList::addVec3Property(vec3d* pd, const QString& name)
{
	return addProperty(name, CProperty::Vec3)->setData(pd);
}

CProperty* CDataPropertyList::addVec2iProperty(vec2i* pd, const QString& name)
{
	return addProperty(name, CProperty::Vec2i)->setData(pd);
}

CProperty* CDataPropertyList::addMat3Property(mat3d* pd, const QString& name)
{
	return addProperty(name, CProperty::Mat3)->setData(pd);
}

CProperty* CDataPropertyList::addColorProperty(QColor* pd, const QString& name)
{
	return addProperty(name, CProperty::Color)->setData(pd);
}

CProperty* CDataPropertyList::addStringProperty(QString* pd, const QString& name)
{
	return addProperty(name, CProperty::String)->setData(pd);
}

CProperty* CDataPropertyList::addResourceProperty(QString* pd, const QString& name)
{
	return addProperty(name, CProperty::Resource)->setData(pd);
}

CProperty* CDataPropertyList::addInternalLinkProperty(QStringList* pd, const QString& name)
{
	return addProperty(name, CProperty::InternalLink)->setData(pd);
}

CProperty* CDataPropertyList::addExternalLinkProperty(QStringList* pd, const QString& name)
{
	return addProperty(name, CProperty::ExternalLink)->setData(pd);
}

CProperty* CDataPropertyList::addCurveProperty(QString* pd, const QString& name)
{
	return addProperty(name, CProperty::Curve)->setData(pd);
}

CProperty* CDataPropertyList::addCurveListProperty(QStringList* pd, const QString& name)
{
	return addProperty(name, CProperty::CurveList)->setData(pd);
}

QVariant CDataPropertyList::GetPropertyValue(int i)
{
	CProperty& p = Property(i);
	switch (p.type)
	{
	case CProperty::Bool: { bool v = *((bool*)(p.pdata)); return v; } break;
	case CProperty::Int : { int v = *((int*)(p.pdata)); return v; } break;
	case CProperty::Enum: { int v = *((int*)(p.pdata)); return v; } break;
	case CProperty::Float: { double v = *((double*)(p.pdata)); return v; } break;
	case CProperty::Color: { QColor v = *((QColor*)(p.pdata)); return v; } break;
	case CProperty::String: { QString v = *((QString*)(p.pdata)); return v; } break;
	case CProperty::Curve : { QString v = *((QString*)(p.pdata)); return v; } break;
	case CProperty::CurveList: { QStringList v = *((QStringList*)(p.pdata)); return v; } break;
	case CProperty::Resource: { QString v = *((QString*)(p.pdata)); return v; } break;
	case CProperty::Vec3: { vec3d v = *((vec3d*)(p.pdata)); return QString::fromStdString(Vec3dToString(v)); } break;
	case CProperty::Vec2i: { vec2i v = *((vec2i*)(p.pdata)); return QString::fromStdString(Vec2iToString(v)); } break;
	case CProperty::Mat3: { mat3d v = *((mat3d*)(p.pdata)); return QString::fromStdString(Mat3dToString(v)); } break;
	case CProperty::Std_Vector_Int: { std::vector<int> v = *((std::vector<int>*)(p.pdata)); return QString::fromStdString(VectorIntToString(v)); } break;
	case CProperty::Std_Vector_Double: { std::vector<double> v = *((std::vector<double>*)(p.pdata)); return QString::fromStdString(VectorDoubleToString(v)); } break;
	}

	return QVariant();
}

void CDataPropertyList::SetPropertyValue(int i, const QVariant& v)
{
	CProperty& p = Property(i);
	switch (p.type)
	{
	case CProperty::Bool : { bool& d = *((bool*)p.pdata); d = v.toBool(); } break;
	case CProperty::Int  : { int& d = *((int*)p.pdata); d = v.toInt(); } break;
	case CProperty::Enum : { int& d = *((int*)p.pdata); d = v.toInt(); } break;
	case CProperty::Float: { double& d = *((double*)p.pdata); d = v.toDouble(); } break;
	case CProperty::Color: { QColor& d = *((QColor*)p.pdata); d = v.value<QColor>(); } break;
	case CProperty::String: { QString& d = *((QString*)p.pdata); d = v.value<QString>(); } break;
	case CProperty::Curve: { QString& d = *((QString*)p.pdata); d = v.value<QString>(); } break;
	case CProperty::CurveList: { QStringList& d = *((QStringList*)p.pdata); d = v.value<QStringList>(); } break;
	case CProperty::Resource: { QString& d = *((QString*)p.pdata); d = v.value<QString>(); } break;
	case CProperty::Vec3: { vec3d& d = *((vec3d*)p.pdata); d = StringToVec3d(v.value<QString>().toStdString()); } break;
	case CProperty::Vec2i: { vec2i& d = *((vec2i*)p.pdata); d = StringToVec2i(v.value<QString>().toStdString()); } break;
	case CProperty::Mat3: { mat3d& d = *((mat3d*)p.pdata); d = StringToMat3d(v.value<QString>().toStdString()); } break;
	case CProperty::Std_Vector_Int: { std::vector<int>& d = *((std::vector<int>*)p.pdata); d = StringToVectorInt(v.value<QString>().toStdString()); } break;
	case CProperty::Std_Vector_Double: { std::vector<double>& d = *((std::vector<double>*)p.pdata); d = StringToVectorDouble(v.value<QString>().toStdString()); } break;
	}
}


CCachedPropertyList::CCachedPropertyList()
{

}

CCachedPropertyList::~CCachedPropertyList()
{
	for (int i = 0; i < Properties(); ++i)
	{
		CProperty& p = Property(i);
		void* d = p.data();
		switch (p.type)
		{
		case CProperty::Bool        : delete (bool*)d; break;
		case CProperty::Int         : delete (int*)d; break;
		case CProperty::Enum        : delete (int*)d; break;
		case CProperty::Float       : delete (double*)d; break;
		case CProperty::String      : delete (QString*)d; break;
		case CProperty::Color       : delete (QColor*)d; break;
		case CProperty::Resource    : delete (QString*)d; break;
		case CProperty::InternalLink: delete (QStringList*)d; break;
		case CProperty::ExternalLink: delete (QStringList*)d; break;
		case CProperty::Vec3        : delete (vec3d*)d; break;
		case CProperty::Vec2i       : delete (vec2i*)d; break;
		case CProperty::Mat3        : delete (mat3d*)d; break;
		default:
			assert(false);
		}
	}
}

CProperty* CCachedPropertyList::addBoolProperty(bool b, const QString& name)
{
	bool* v = new bool(b);
	return addProperty(name, CProperty::Bool)->setData(v);
}

CProperty* CCachedPropertyList::addIntProperty(int n, const QString& name)
{
	int* v = new int(n);
	return addProperty(name, CProperty::Int)->setData(v);
}

CProperty* CCachedPropertyList::addEnumProperty(int n, const QString& name)
{
	int* v = new int(n);
	return addProperty(name, CProperty::Enum)->setData(v);
}

CProperty* CCachedPropertyList::addDoubleProperty(double g, const QString& name)
{
	double* v = new double(g);
	return addProperty(name, CProperty::Float)->setData(v);
}

CProperty* CCachedPropertyList::addColorProperty(QColor c, const QString& name)
{
	QColor* v = new QColor(c);
	return addProperty(name, CProperty::Color)->setData(v);
}

CProperty* CCachedPropertyList::addStringProperty(QString s, const QString& name)
{
	QString* v = new QString(s);
	return addProperty(name, CProperty::String)->setData(v);
}

CProperty* CCachedPropertyList::addCurveProperty(QString pd, const QString& name)
{
	assert(false);
	return nullptr;
}

CProperty* CCachedPropertyList::addCurveListProperty(QStringList pd, const QString& name)
{
	assert(false);
	return nullptr;
}

CProperty* CCachedPropertyList::addResourceProperty(QString s, const QString& name)
{
	QString* v = new QString(s);
	return addProperty(name, CProperty::Resource)->setData(v);
}

CProperty* CCachedPropertyList::addInternalLinkProperty(QStringList s, const QString& name)
{
	QStringList* v = new QStringList(s);
	return addProperty(name, CProperty::InternalLink)->setData(v);
}

CProperty* CCachedPropertyList::addExternalLinkProperty(QStringList s, const QString& name)
{
	QStringList* v = new QStringList(s);
	return addProperty(name, CProperty::ExternalLink)->setData(v);
}

CProperty* CCachedPropertyList::addVec3Property(vec3d p, const QString& name)
{
	vec3d* v = new vec3d(p);
	return addProperty(name, CProperty::Vec3)->setData(v);
}

CProperty* CCachedPropertyList::addVec2iProperty(vec2i p, const QString& name)
{
	vec2i* v = new vec2i(p);
	return addProperty(name, CProperty::Vec2i)->setData(v);
}

CProperty* CCachedPropertyList::addMat3Property(mat3d m, const QString& name)
{
	mat3d* v = new mat3d(m);
	return addProperty(name, CProperty::Mat3)->setData(v);
}

QVariant CCachedPropertyList::GetPropertyValue(int i)
{
	CProperty& p = Property(i);
	switch (p.type)
	{
		case CProperty::Bool        : return value<bool>(i); break;
		case CProperty::Int         : return value<int>(i); break;
		case CProperty::Enum        : return value<int>(i); break;
		case CProperty::Float       : return value<double>(i); break;
		case CProperty::String      : return value<QString>(i); break;
		case CProperty::Color       : return value<QColor>(i); break;
		case CProperty::Resource    : return value<QString>(i); break;
		case CProperty::InternalLink: return value<QStringList>(i); break;
		case CProperty::ExternalLink: return value<QStringList>(i); break;
		case CProperty::Vec3        : return QString::fromStdString(Vec3dToString(value<vec3d>(i))); break;
		case CProperty::Vec2i       : return QString::fromStdString(Vec2iToString(value<vec2i>(i))); break;
		case CProperty::Mat3        : return QString::fromStdString(Mat3dToString(value<mat3d>(i))); break;
		default:
			assert(false);
	}
	return QVariant();
}

QVariant CCachedPropertyList::GetPropertyValue(const QString& propName)
{
	int n = FindPropertyIndex(propName);
	if (n >= 0) return GetPropertyValue(n);
	return QVariant();
}

void CCachedPropertyList::SetPropertyValue(int i, const QVariant& v)
{
	CProperty& p = Property(i);
	switch (p.type)
	{
	case CProperty::Bool        : value<bool>(i) = v.toBool(); break;
	case CProperty::Int         : value<int>(i) = v.toInt(); break;
	case CProperty::Enum        : value<int>(i) = v.toInt(); break;
	case CProperty::Float       : value<double>(i) = v.toDouble(); break;
	case CProperty::String      : value<QString>(i) = v.toString(); break;
	case CProperty::Color       : value<QColor>(i) = v.value<QColor>(); break;
	case CProperty::Resource    : value<QString>(i) = v.toString(); break;
	case CProperty::InternalLink: value<QStringList>(i) = v.toStringList(); break;
	case CProperty::ExternalLink: value<QStringList>(i) = v.toStringList(); break;
	case CProperty::Vec3        : value<vec3d>(i) = StringToVec3d(v.toString().toStdString()); break;
	case CProperty::Vec2i       : value<vec2i>(i) = StringToVec2i(v.toString().toStdString()); break;
	case CProperty::Mat3        : value<mat3d>(i) = StringToMat3d(v.toString().toStdString()); break;
	default:
		assert(false);
	}
}
