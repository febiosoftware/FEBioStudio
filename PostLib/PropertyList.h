#pragma once
#include <QtCore/QVariant>
#include "convert.h"

namespace Post {

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
		Vec3,
		DataScalar,
		DataVec3,
		DataMat3
	};

	enum Flags
	{
		Editable = 1,		// property can be edited
		Visible = 2,		// property is visible
		Modified = 4		// property is modified (but not applied)
	};

public:
	CProperty::Type	type;		// type of property
	unsigned int	flags;		// editable flag
	int		imin, imax;			// range for Int (and Enum unless values is not empty)
	double	fmin, fmax;			// range for floats
	double	fstep;				// step for floats
	double	bauto;
	bool	brange;				// true if range was set

	QString			name;		// the name of the property
	QString			info;		// description of the property
	QStringList		values;		// set possible values for Enum properties (if not set, Enum is treated as Int)

public:
	CProperty(){}
	CProperty(const CProperty& p) { *this = p; }
	CProperty& operator = (const CProperty& p);

	CProperty& setIntRange(int Min, int Max) { imin = Min; imax = Max; brange = true; return *this; }
	CProperty& setFloatRange(double Min, double Max) { fmin = Min; fmax = Max; brange = true; return *this; }
	CProperty& setFloatStep(double fStep) { fstep = fStep; brange = true; return *this; }
	CProperty& setEnumValues(QStringList& val) { values = val; return *this; }
	CProperty& setAutoValue(bool b) { bauto = b; return *this; }

public:
	void setFlags(unsigned int flag) { flags = flag; }

	bool isEditable() const { return (flags & Editable); }
	bool isVisible() const { return (flags & Visible); }
	bool isModified() const { return (flags & Modified); }

	void setModified(bool b)
	{
		unsigned int m = (unsigned int) Modified;
		if (b) flags |= m;
		else flags &= ~m;
	}

public:
	CProperty(const QString& sname, CProperty::Type itype);
	CProperty(const QString& sname, CProperty::Type itype, const QString& sinfo);
};

//-----------------------------------------------------------------------------
class CPropertyList
{
public:
	CPropertyList(){}
	virtual ~CPropertyList(){}

	int Properties() const { return (int) m_list.size(); }

	CProperty* addProperty(const QString& sname, CProperty::Type itype) { addProperty(CProperty(sname, itype)); return &m_list[m_list.size()-1]; }
	CProperty* addProperty(const QString& sname, CProperty::Type itype, const QString& sinfo) { addProperty(CProperty(sname, itype, sinfo)); return &m_list[m_list.size()-1]; }

	CProperty& Property(int i) { return m_list[i]; }
	const CProperty& Property(int i) const { return m_list[i]; }

	virtual QVariant GetPropertyValue(int i) = 0;
	virtual void SetPropertyValue(int i, const QVariant& v) = 0;

private:
	CProperty* addProperty(const CProperty& p) { m_list.push_back(p); return &m_list[m_list.size()-1]; }

private:
	std::vector<CProperty>	m_list;
};
}
