#include "stdafx.h"
#include "CIntInput.h"

QString vecToString(const vec3f& v)
{
	return QString("%1,%2,%3").arg(v.x).arg(v.y).arg(v.z);
}

vec3f stringToVec(const QString& s)
{
	QStringList l = s.split(',');
	int N = l.size();
	vec3f v(0.f, 0.f, 0.f);
	v.x = (N > 0 ? l[0].toFloat() : 0.f);
	v.y = (N > 1 ? l[1].toFloat() : 0.f);
	v.z = (N > 2 ? l[2].toFloat() : 0.f);

	return v;
}

CVec3Input::CVec3Input(QWidget* parent) : QLineEdit(parent) {}

void CVec3Input::setValue(const vec3f& v)
{
	setText(vecToString(v));
}

vec3f CVec3Input::value() const
{
	return stringToVec(text());
}
