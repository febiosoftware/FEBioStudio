#pragma once

class QIcon;
class QString;

enum class Emblem{Plus, Minus, Check, Warning, Caution, Missing};

class CIconProvider
{
public:
	static void Instantiate(int theme, int dpr);

	static QIcon GetIcon(const QString& iconName);
	static QIcon GetIcon(const QString& baseIconName, Emblem emblem);
	static QIcon GetIcon(const QString& baseIconName, const QString& emblemIconName);

private:
	CIconProvider() {}
	CIconProvider(int theme, int dpr) {m_theme = theme; m_dpr = dpr;}
	CIconProvider(CIconProvider const&) {}
	CIconProvider& operator=(CIconProvider const&) { return *this; }
	virtual ~CIconProvider(){}

	static QString themedIconURL(const QString& iconName);
	static QString emblemIconURL(Emblem emblem);

	static CIconProvider* m_instance;
	static int m_theme;
	static int m_dpr;
};
