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
#include "InputWidgets.h"

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
