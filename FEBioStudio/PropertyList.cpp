#include "PropertyList.h"
#include <MathLib/mat3d.h>

vec3d StringToVec3d(const QString& s)
{
	string st = s.toStdString();
	const char* sz = st.c_str();
	vec3d r;
	sscanf(sz, "%lg,%lg,%lg", &r.x, &r.y, &r.z);
	return r;
}

mat3d StringToMat3d(const QString& s)
{
	string st = s.toStdString();
	const char* sz = st.c_str();
	double a[9];
	sscanf(sz, "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg", a, a+1, a+2, a+3, a+4, a+5, a+6, a+7, a+8);
	return mat3d(a);
}

vec2i StringToVec2i(const QString& s)
{
	string st = s.toStdString();
	const char* sz = st.c_str();
	vec2i r;
	sscanf(sz, "%ld,%ld", &r.x, &r.y);
	return r;
}

QString Vec3dToString(const vec3d& r)
{
	return QString("%1,%2,%3").arg(r.x).arg(r.y).arg(r.z);
}

QString Vec2iToString(const vec2i& r)
{
	return QString("%1,%2").arg(r.x).arg(r.y);
}

QString Mat3dToString(const mat3d& a)
{
	QString s;
	for (int i = 0; i<3; ++i)
		for (int j=0; j<3; ++j)
		{ 
			s += QString("%1").arg(a(i, j));

			if ((i != 2) || (j != 2)) s += ",";
		}
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
	}
}
