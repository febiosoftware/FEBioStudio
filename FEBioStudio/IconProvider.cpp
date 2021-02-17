/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include "IconProvider.h"
#include <QString>
#include <QFile>
#include <QIcon>
#include <QPixmap>
#include <QPainter>

CIconProvider* CIconProvider::m_instance = nullptr;
bool CIconProvider::m_dark = false;
int CIconProvider::m_dpr = 0;

void CIconProvider::Instantiate(bool dark, int dpr)
{
	if(!m_instance)
	{
		m_instance = new CIconProvider(dark, dpr);
	}
}

QIcon CIconProvider::GetIcon(const QString& iconName)
{
	QString url = themedIconURL(iconName);

	QPixmap pixmap(url);
//	pixmap.setDevicePixelRatio(m_dpr);
	QIcon icon;
	icon.addPixmap(pixmap);
	return icon;
}

QIcon CIconProvider::GetIcon(const QString& baseIconName, Emblem emblem)
{
	QString baseUrl = themedIconURL(baseIconName);
	QString emblemUrl = emblemIconURL(emblem);

	QPixmap basePixmap(baseUrl);
	QPixmap emblemPixmap = QPixmap(emblemUrl).scaled(basePixmap.size()/2);

	QPainter painter(&basePixmap);
	painter.drawPixmap(basePixmap.width() - emblemPixmap.width(), basePixmap.height() - emblemPixmap.height(), emblemPixmap);

	return QIcon(basePixmap);
}

QIcon CIconProvider::GetIcon(const QString& baseIconName, const QString& emblemIconName)
{
	QString baseUrl = themedIconURL(baseIconName);
	QString emblemUrl = themedIconURL(emblemIconName);

	QPixmap basePixmap(baseUrl);
	QPixmap emblemPixmap = QPixmap(emblemUrl).scaled(basePixmap.size()/2);

	QPainter painter(&basePixmap);
	painter.drawPixmap(basePixmap.width() - emblemPixmap.width(), basePixmap.height() - emblemPixmap.height(), emblemPixmap);

	return QIcon(basePixmap);
}

QString CIconProvider::themedIconURL(const QString& iconName)
{
	QString rs(iconName);
	if (m_dark)
	{
		rs += "_neg";
	}
	QString url = ":/icons/" + rs + ".png";

	// make sure the icon exists
	if (m_dark)
	{
		QFile f(url);
		if (!f.exists())
		{
			// use the regular version instead
			url = ":/icons/" + iconName + ".png";
		}
	}

	return url;
}

QString CIconProvider::emblemIconURL(Emblem emblem)
{
	switch(emblem)
	{
	case Emblem::Plus:
		return ":/icons/emblems/plus.png";
	case Emblem::Minus:
		return ":/icons/emblems/minus.png";
	case Emblem::Check:
		return ":/icons/emblems/check.png";
	case Emblem::Warning:
		return ":/icons/emblems/warning.png";
	case Emblem::Caution:
		return ":/icons/emblems/caution.png";
	case Emblem::Missing:
		return ":/icons/emblems/missing.png";
	default:
		return "";
	}
}



