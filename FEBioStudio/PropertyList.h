#pragma once
#include <QtCore/QVariant>
#include <PostLib/convert.h>
#include <MathLib/math3d.h>

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
		Group,
		MathString,
		ColorMap,
		Vec3,

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
	CProperty() { param = nullptr; brange = false; }
	CProperty(const CProperty& p) { *this = p; }
	CProperty& operator = (const CProperty& p);

	CProperty& setIntRange(int Min, int Max) { brange = true;  imin = Min; imax = Max; return *this; }
	CProperty& setFloatRange(double Min, double Max) { brange = true;  fmin = Min; fmax = Max; return *this; }
	CProperty& setFloatStep(double fStep) { fstep = fStep; return *this; }
	CProperty& setEnumValues(QStringList& val) { values = val; return *this; }
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
	CProperty* addVec3Property     (vec3d* pd, const QString& name);

	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);
};

QString Vec3dToString(const vec3d& r);
vec3d StringToVec3d(const QString& s);
