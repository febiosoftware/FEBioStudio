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

#include <string>
#include <QDebug>
#include <iostream>

using namespace std;

vec2d StringToVec2d(const QString& s)
{
	string st = s.toStdString();
	const char* sz = st.c_str();
	double x = 0.0, y = 0.0;
	if (strcmp(sz, "x") == 0) return vec2d(1, 0);
	if (strcmp(sz, "y") == 0) return vec2d(0, 1);
	if (sz[0] == '{')
		sscanf(sz, "{%lg,%lg}", &x, &y);
	else
		sscanf(sz, "%lg,%lg", &x, &y);
	return vec2d(x, y);
}

vec3d StringToVec3d(const QString& s)
{
	string st = s.toStdString();
	const char* sz = st.c_str();
	vec3d r(0,0,0);
	if (strcmp(sz, "x") == 0) return vec3d(1, 0, 0);
	if (strcmp(sz, "y") == 0) return vec3d(0, 1, 0);
	if (strcmp(sz, "z") == 0) return vec3d(0, 0, 1);
	if (sz[0] == '{')
		sscanf(sz, "{%lg,%lg,%lg}", &r.x, &r.y, &r.z);
	else
		sscanf(sz, "%lg,%lg,%lg", &r.x, &r.y, &r.z);
	return r;
}

vec3f StringToVec3f(const QString& s)
{
	string st = s.toStdString();
	const char* sz = st.c_str();
	vec3f r(0, 0, 0);
	if (strcmp(sz, "x") == 0) return vec3f(1, 0, 0);
	if (strcmp(sz, "y") == 0) return vec3f(0, 1, 0);
	if (strcmp(sz, "z") == 0) return vec3f(0, 0, 1);
	if (sz[0] == '{')
		sscanf(sz, "{%g,%g,%g}", &r.x, &r.y, &r.z);
	else
		sscanf(sz, "%g,%g,%g", &r.x, &r.y, &r.z);
	return r;
}

mat3d StringToMat3d(const QString& s)
{
	string st = s.toStdString();
	const char* sz = st.c_str();
	double a[9] = { 0 };
	int n = 0;
	if (sz[0] == '{')
	{
		n = sscanf(sz, "{{%lg,%lg,%lg},{%lg,%lg,%lg},{%lg,%lg,%lg}}", a, a + 1, a + 2, a + 3, a + 4, a + 5, a + 6, a + 7, a + 8);
	}
	else
	{
		n = sscanf(sz, "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg", a, a + 1, a + 2, a + 3, a + 4, a + 5, a + 6, a + 7, a + 8);
	}

	if (n == 1) return mat3d(mat3dd(a[0]));
	else if (n == 3) return mat3d(mat3dd(a[0], a[1], a[2]));
	else return mat3d(a);
}

mat3ds StringToMat3ds(const QString& s)
{
	string st = s.toStdString();
	const char* sz = st.c_str();
	double a[6] = { 0 };
	if (sz[0] == '{')
		sscanf(sz, "{%lg,%lg,%lg,%lg,%lg,%lg}", a, a + 1, a + 2, a + 3, a + 4, a + 5);
	else
		sscanf(sz, "%lg,%lg,%lg,%lg,%lg,%lg", a, a + 1, a + 2, a + 3, a + 4, a + 5);
	return mat3ds(a[0], a[1], a[2], a[3], a[4], a[5]);
}

vec2i StringToVec2i(const QString& s)
{
	std::string str = s.toStdString();
	const char* sz = str.c_str();
	vec2i r;
	sscanf(sz, "%d,%d", &r.x, &r.y);
	return r;
}

QString VectorIntToString(const std::vector<int>& v)
{
	QString s;
	for (int i = 0; i < v.size(); ++i)
	{
		s += QString::number(v[i]);
		if (i != v.size() - 1) s += ",";
	}
	return s;
}

std::vector<int> StringToVectorInt(const QString& s)
{
	vector<int> v;
	if (s.isEmpty()) return v;
	std::string str = s.toStdString();
	const char* sz = str.c_str();
	while (sz && *sz) {
		const char* ch = strchr(sz, ',');
		int n = atoi(sz);
		v.push_back(n);
		if (ch) sz = ch + 1; else sz = nullptr;
	};
	return v;
}

QString VectorDoubleToString(const std::vector<double>& v)
{
	QString s;
	for (int i = 0; i < v.size(); ++i)
	{
		s += QString::number(v[i]);
		if (i != v.size() - 1) s += ",";
	}
	return s;
}

std::vector<double> StringToVectorDouble(const QString& s)
{
	vector<double> v;
	if (s.isEmpty()) return v;
	std::string str = s.toStdString();
	const char* sz = str.c_str();
	while (sz && *sz) {
		const char* ch = strchr(sz, ',');
		double f = atof(sz);
		v.push_back(f);
		if (ch) sz = ch + 1; else sz = nullptr;
	};
	return v;
}

QString Vec2dToString(const vec2d& r)
{
	return QString("{%1,%2}").arg(r.x()).arg(r.y());
}

QString Vec3dToString(const vec3d& r)
{
	return QString("{%1,%2,%3}").arg(r.x).arg(r.y).arg(r.z);
}

QString Vec3fToString(const vec3f& r)
{
	return QString("{%1,%2,%3}").arg(r.x).arg(r.y).arg(r.z);
}

QString Vec2iToString(const vec2i& r)
{
	return QString("%1,%2").arg(r.x).arg(r.y);
}

QString Mat3dToString(const mat3d& a)
{
	QString s;
	s += "{";
	for (int i = 0; i < 3; ++i)
	{
		s += "{";
		for (int j = 0; j < 3; ++j)
		{
			s += QString("%1").arg(a(i, j));
			if (j != 2) s += ",";
		}
		s += "}";
		if (i != 2) s += ",";
	}
	s += "}";
	return s;
}

QString Mat3dsToString(const mat3ds& a)
{
	QString s;
	s = QString("{%1").arg(a.xx());
	s += QString(",%1").arg(a.yy());
	s += QString(",%1").arg(a.zz());
	s += QString(",%1").arg(a.xy());
	s += QString(",%1").arg(a.yz());
	s += QString(",%1}").arg(a.xz());
	return s;
}

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
	case CProperty::Vec3: { vec3d v = *((vec3d*)(p.pdata)); return Vec3dToString(v); } break;
	case CProperty::Vec2i: { vec2i v = *((vec2i*)(p.pdata)); return Vec2iToString(v); } break;
	case CProperty::Mat3: { mat3d v = *((mat3d*)(p.pdata)); return Mat3dToString(v); } break;
	case CProperty::Std_Vector_Int: { std::vector<int> v = *((std::vector<int>*)(p.pdata)); return VectorIntToString(v); } break;
	case CProperty::Std_Vector_Double: { std::vector<double> v = *((std::vector<double>*)(p.pdata)); return VectorDoubleToString(v); } break;
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
	case CProperty::Vec3: { vec3d& d = *((vec3d*)p.pdata); d = StringToVec3d(v.value<QString>()); } break;
	case CProperty::Vec2i: { vec2i& d = *((vec2i*)p.pdata); d = StringToVec2i(v.value<QString>()); } break;
	case CProperty::Mat3: { mat3d& d = *((mat3d*)p.pdata); d = StringToMat3d(v.value<QString>()); } break;
	case CProperty::Std_Vector_Int: { std::vector<int>& d = *((std::vector<int>*)p.pdata); d = StringToVectorInt(v.value<QString>()); } break;
	case CProperty::Std_Vector_Double: { std::vector<double>& d = *((std::vector<double>*)p.pdata); d = StringToVectorDouble(v.value<QString>()); } break;
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
		case CProperty::Vec3        : return Vec3dToString(value<vec3d>(i)); break;
		case CProperty::Vec2i       : return Vec2iToString(value<vec2i>(i)); break;
		case CProperty::Mat3        : return Mat3dToString(value<mat3d>(i)); break;
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
	case CProperty::Vec3        : value<vec3d>(i) = StringToVec3d(v.toString()); break;
	case CProperty::Vec2i       : value<vec2i>(i) = StringToVec2i(v.toString()); break;
	case CProperty::Mat3        : value<mat3d>(i) = StringToMat3d(v.toString()); break;
	default:
		assert(false);
	}
}
