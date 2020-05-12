#include "IconProvider.h"
#include <QString>
#include <QFile>
#include <QIcon>
#include <QPixmap>
#include <QPainter>

CIconProvider* CIconProvider::m_instance = nullptr;
int CIconProvider::m_theme = 0;
int CIconProvider::m_dpr = 0;

void CIconProvider::Instantiate(int theme, int dpr)
{
	if(!m_instance)
	{
		m_instance = new CIconProvider(theme, dpr);
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
	if ((m_theme == 1) || (m_theme == 3))
	{
		rs += "_neg";
	}
	QString url = ":/icons/" + rs + ".png";

	// make sure the icon exists
	if ((m_theme == 1) || (m_theme == 3))
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



