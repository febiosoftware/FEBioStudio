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

class QIcon;
class QColor;
class QPixmap;
class QString;

enum class Emblem {Plus, Minus, Check, Warning, Caution, Missing};
enum class Shape {Circle, Square};

class CIconProvider
{
public:
	static void Instantiate(double dpr);

	static QIcon GetIcon(const QString& iconName);
	static QIcon GetIcon(const QString& baseIconName, Emblem emblem);
	static QIcon GetIcon(const QString& baseIconName, const QString& emblemIconName);
    static QIcon GetIcon(const QString& baseIconName, QColor c, Shape shape);

	static QIcon CreateIcon(const QIcon& baseIcon, Emblem emblem);

    static QPixmap BuildPixMap(const QColor& c, Shape shape = Shape::Circle, int size = 12);

private:
	CIconProvider() {}
	CIconProvider(double dpr) {m_dpr = dpr;}
	CIconProvider(CIconProvider const&) {}
	CIconProvider& operator=(CIconProvider const&) { return *this; }
	virtual ~CIconProvider(){}

	static QString themedIconURL(const QString& iconName);
	static QString emblemIconURL(Emblem emblem);

	static CIconProvider* m_instance;
	static double m_dpr;
};
