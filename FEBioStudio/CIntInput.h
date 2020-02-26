#pragma once
#include <QLineEdit>
#include <QValidator>
#include <MathLib/math3d.h>

//-----------------------------------------------------------------------------
class CIntInput : public QLineEdit
{
public:
	CIntInput(QWidget* parent = 0) : QLineEdit(parent)
	{
		setValidator(new QIntValidator);
	}

	void setValue(int m) { setText(QString("%1").arg(m)); }
	int value() const { return text().toInt(); }
};

//-----------------------------------------------------------------------------
class CFloatInput : public QLineEdit
{
public:
	CFloatInput(QWidget* parent = 0) : QLineEdit(parent)
	{
		QDoubleValidator* pv = new QDoubleValidator;
		pv->setRange(-1e99, 1e99, 3);
		setValidator(pv);
	}

	void setValue(double v) { setText(QString("%1").arg(v)); }
	double value() const { return text().toDouble(); }
};

//-----------------------------------------------------------------------------
class CVec3Input : public QLineEdit
{
public:
	CVec3Input(QWidget* parent = 0);

	void setValue(const vec3f& v);

	vec3f value() const;
};

QString vecToString(const vec3f& f);

vec3f stringToVec(const QString& s);
