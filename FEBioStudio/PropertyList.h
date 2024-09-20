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

#pragma once
#include <QtCore/QVariant>
#include <GLWLib/convert.h>
#include <FSCore/math3d.h>

class Param;

//-----------------------------------------------------------------------------
class CProperty
{
public:
	enum Type
	{
		Bool,
		Int,
		Float,
		String,
		Enum,
		Color,
		Resource,	// external resource (e.g. file)
		InternalLink,
		ExternalLink,
		Group,
		MathString,
		ColorMap,
		Vec3,
		Mat3,
		Mat3s,
		Vec2i,
		Vec2d,
		Std_Vector_Int,
		Std_Vector_Double,

		// from PostView
		DataScalar,
		DataVec3,
		DataMat3,

		// types for selecting mesh items
		Curve,
		CurveList,

		// Actions
		Action
	};

	enum Flags
	{
		Editable = 1,		// property can be edited
		Visible  = 2,		// property is visible (in a property editor)
		Variable = 4		// property's type can change
	};

public:
	CProperty::Type	type;		// type of property
	unsigned int	flags;		// editable flag
	int		imin, imax;			// range for Int (and Enum unless values is not empty)
	double	fmin, fmax;			// range for floats
	double	fstep;				// step for floats
	double	bauto;
	void*	pdata;				// data pointer
	bool	brange;

	QString			name;		// the name of the property
	QString			info;		// description of the property
	QStringList		values;		// set possible values for Enum properties (if not set, Enum is treated as Int)

	Param*		param;

public:
	CProperty() { param = nullptr; brange = false; pdata = nullptr; }
	CProperty(const CProperty& p) { *this = p; }
	CProperty& operator = (const CProperty& p);

	CProperty& setIntRange(int Min, int Max) { brange = true;  imin = Min; imax = Max; return *this; }
	CProperty& setFloatRange(double Min, double Max) { brange = true;  fmin = Min; fmax = Max; return *this; }
	CProperty& setFloatStep(double fStep) { fstep = fStep; return *this; }
	CProperty& setEnumValues(const QStringList& val) { values = val; return *this; }
	CProperty& setAutoValue(bool b) { bauto = b; return *this; }

	CProperty* setData(void* pd) { pdata = pd; return this; }
	void* data() { return pdata; }

public:
	void setFlags(unsigned int flag) { flags = flag; }

	bool isEditable() const { return (flags & Editable); }
	bool isVisible() const { return (flags & Visible); }

public:
	CProperty(const QString& sname, CProperty::Type itype);
	CProperty(const QString& sname, CProperty::Type itype, const QString& sinfo);
};

//-----------------------------------------------------------------------------
// Containter for properties
class CPropertyList
{
public:
	CPropertyList(){ m_bmodified = false; }
	virtual ~CPropertyList(){}

	int Properties() const { return (int) m_list.size(); }

	void Clear() { m_list.clear(); }

	CProperty* addProperty(const QString& sname, CProperty::Type itype) { return addProperty(CProperty(sname, itype)); }
	CProperty* addProperty(const QString& sname, CProperty::Type itype, const QString& sinfo) { return addProperty(CProperty(sname, itype, sinfo)); }

	const CProperty& Property(int i) const { return m_list[i]; }
	CProperty& Property(int i) { return m_list[i]; }

	// these two functions need to be overwritten by derived classes
	virtual QVariant GetPropertyValue(int i) = 0;
	virtual void SetPropertyValue(int i, const QVariant& v) = 0;

	virtual void Update() {}

public:
	bool IsModified() const { return m_bmodified; }
	void SetModified(bool b) { m_bmodified = b; }

private:
	CProperty* addProperty(const CProperty& p) { m_list.push_back(p); return &m_list[m_list.size()-1]; }

protected:
	std::vector<CProperty>	m_list;
	bool	m_bmodified;
};

//-----------------------------------------------------------------------------
// This class maps properties directly to data variables
class CDataPropertyList : public CPropertyList
{
public:
	CDataPropertyList();

	CProperty* addBoolProperty     (bool*   pd, const QString& name);
	CProperty* addIntProperty      (int*    pd, const QString& name);
	CProperty* addEnumProperty     (int*    pd, const QString& name);
	CProperty* addDoubleProperty   (double* pd, const QString& name);
	CProperty* addColorProperty    (QColor* pd, const QString& name);
	CProperty* addStringProperty   (QString* pd, const QString& name);
	CProperty* addCurveProperty    (QString* pd, const QString& name);
	CProperty* addCurveListProperty(QStringList* pd, const QString& name);
	CProperty* addResourceProperty (QString* pd, const QString& name);
	CProperty* addInternalLinkProperty (QStringList* pd, const QString& name);
	CProperty* addExternalLinkProperty (QStringList* pd, const QString& name);
	CProperty* addVec3Property     (vec3d* pd, const QString& name);
	CProperty* addVec2iProperty    (vec2i* pd, const QString& name);
	CProperty* addMat3Property     (mat3d* pd, const QString& name);

	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);
};

// Property list that manages its own data
class CCachedPropertyList : public CPropertyList
{
public:
	CCachedPropertyList();
	~CCachedPropertyList();

	CProperty* addBoolProperty(bool pd, const QString& name);
	CProperty* addIntProperty(int pd, const QString& name);
	CProperty* addEnumProperty(int pd, const QString& name);
	CProperty* addDoubleProperty(double pd, const QString& name);
	CProperty* addColorProperty(QColor pd, const QString& name);
	CProperty* addStringProperty(QString pd, const QString& name);
	CProperty* addCurveProperty(QString pd, const QString& name);
	CProperty* addCurveListProperty(QStringList pd, const QString& name);
	CProperty* addResourceProperty(QString pd, const QString& name);
	CProperty* addInternalLinkProperty(QStringList pd, const QString& name);
	CProperty* addExternalLinkProperty(QStringList pd, const QString& name);
	CProperty* addVec3Property(vec3d pd, const QString& name);
	CProperty* addVec2iProperty(vec2i pd, const QString& name);
	CProperty* addMat3Property(mat3d pd, const QString& name);

	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

protected:
	template <class T>
	T& value(int n) { return *((T*)Property(n).data()); }
};

QString Vec2dToString(const vec2d& r);
QString Vec3dToString(const vec3d& r);
vec2d StringToVec2d(const QString& s);
vec3d StringToVec3d(const QString& s);
mat3d StringToMat3d(const QString& s);
mat3ds StringToMat3ds(const QString& s);
QString Mat3dToString(const mat3d& a);
QString Mat3dsToString(const mat3ds& a);
QString Vec2iToString(const vec2i& r);
vec2i StringToVec2i(const QString& s);
std::vector<int> StringToVectorInt(const QString& s);
QString VectorIntToString(const std::vector<int>& v);
std::vector<double> StringToVectorDouble(const QString& s);
QString VectorDoubleToString(const std::vector<double>& v);
