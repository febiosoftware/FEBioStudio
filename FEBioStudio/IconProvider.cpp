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

#include "IconProvider.h"
#include <QString>
#include <QFile>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QApplication>
#include <QStyleHints>

CIconProvider* CIconProvider::m_instance = nullptr;
double CIconProvider::m_dpr = 0;

void CIconProvider::Instantiate(double dpr)
{
	if(!m_instance)
	{
		m_instance = new CIconProvider(dpr);
	}
}

QIcon CIconProvider::GetIcon(const QString& iconName)
{
	QString url = themedIconURL(iconName);

	QPixmap pixmap(url);
	pixmap.setDevicePixelRatio(m_dpr);
	QIcon icon;
	icon.addPixmap(pixmap);
	return icon;
}

QIcon CIconProvider::GetIcon(const QString& baseIconName, Emblem emblem)
{
	QString baseUrl = themedIconURL(baseIconName);
	QString emblemUrl = emblemIconURL(emblem);

    QPixmap basePixmap(baseUrl); 
    basePixmap.setDevicePixelRatio(m_dpr);

	QPixmap emblemPixmap = QPixmap(emblemUrl).scaled(2*basePixmap.size()/3);
    emblemPixmap.setDevicePixelRatio(m_dpr);

	QPainter painter(&basePixmap);
	painter.drawPixmap(basePixmap.width() - emblemPixmap.width(), basePixmap.height() - emblemPixmap.height(), emblemPixmap);

	return QIcon(basePixmap);
}

QIcon CIconProvider::CreateIcon(const QIcon& baseIcon, Emblem emblem)
{
	QString emblemUrl = emblemIconURL(emblem);

	QPixmap basePixmap = baseIcon.pixmap(baseIcon.actualSize(QSize(32, 32)));
	basePixmap.setDevicePixelRatio(m_dpr);

	QPixmap emblemPixmap = QPixmap(emblemUrl).scaled(2 * basePixmap.size() / 3);
	emblemPixmap.setDevicePixelRatio(m_dpr);

	QPainter painter(&basePixmap);
	painter.drawPixmap(basePixmap.width() - emblemPixmap.width(), basePixmap.height() - emblemPixmap.height(), emblemPixmap);

	return QIcon(basePixmap);
}

QIcon CIconProvider::GetIcon(const QString& baseIconName, const QString& emblemIconName)
{
	QString baseUrl = themedIconURL(baseIconName);
	QString emblemUrl = themedIconURL(emblemIconName);

	QPixmap basePixmap(baseUrl); 
    basePixmap.setDevicePixelRatio(m_dpr);

	QPixmap emblemPixmap = QPixmap(emblemUrl).scaled(basePixmap.size()/2);
    emblemPixmap.setDevicePixelRatio(m_dpr);

	QPainter painter(&basePixmap);
	painter.drawPixmap(basePixmap.width() - emblemPixmap.width(), basePixmap.height() - emblemPixmap.height(), emblemPixmap);

	return QIcon(basePixmap);
}

QIcon CIconProvider::GetIcon(const QString& baseIconName, QColor c, Shape shape)
{
    QString baseUrl = themedIconURL(baseIconName);

    QPixmap basePixmap(baseUrl); 
    basePixmap.setDevicePixelRatio(m_dpr);

    QPixmap emblemPixmap = BuildPixMap(c, shape, basePixmap.width()/2);
    emblemPixmap.setDevicePixelRatio(m_dpr);

    QPainter painter(&basePixmap);
	painter.drawPixmap(basePixmap.width() - emblemPixmap.width(), basePixmap.height() - emblemPixmap.height(), emblemPixmap);

	return QIcon(basePixmap);
}

QPixmap CIconProvider::BuildPixMap(const QColor& c, Shape shape, int size)
{
	if (size < 8) size = 8;

	QColor c2 = c;
	QColor c1 = c2.lighter();
	QColor c3 = c2.darker();

	QRadialGradient g(QPointF(size/3, size/3), size/2);
	g.setColorAt(0.0, c1);
	g.setColorAt(0.2, c2);
	g.setColorAt(1.0, c3);

	QPixmap pix(size, size);
//	pix.setDevicePixelRatio(m_list->devicePixelRatio());
	pix.fill(Qt::transparent);
	QPainter p(&pix);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::PenStyle::NoPen);
	p.setBrush(QBrush(g));
	if (shape == Shape::Circle)
		p.drawEllipse(2, 2, size - 4, size - 4);
	else
		p.drawRect(2, 2, size - 4, size - 4);

	p.end();

	return pix;
}

QString CIconProvider::themedIconURL(const QString& iconName)
{
    bool dark = qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark;

	QString rs(iconName);
	if (dark)
	{
		rs += "_neg";
	}
	QString url = ":/icons/" + rs + ".png";

	// make sure the icon exists
	if (dark)
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



