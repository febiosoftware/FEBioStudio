#include "stdafx.h"
#include "EditVariableParam.h"
#include <FSCore/ParamBlock.h>

//-----------------------------------------------------------------------------
CEditVariableParam::CEditVariableParam(QWidget* parent) : QComboBox(parent)
{
	addItem("<constant>");
	addItem("<math>");
	addItem("<map>");

	setEditable(true);
	setInsertPolicy(QComboBox::NoInsert);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_param = nullptr;

	QObject::connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
	QObject::connect(this, SIGNAL(editTextChanged(const QString&)), this, SLOT(onEditTextChanged(const QString&)));
}

void CEditVariableParam::setParam(Param* p)
{
	m_param = p;
	if (p == nullptr) return;

	blockSignals(true);
	if (p->GetParamType() == Param_Type::Param_FLOAT)
	{
		setCurrentIndex(0);
		setEditText(QString("%1").arg(p->GetFloatValue()));
	}
	if (p->GetParamType() == Param_Type::Param_VEC3D)
	{
		setCurrentIndex(0);
		vec3d v = p->GetVec3dValue();
		setEditText(QString("%1,%2,%d").arg(v.x, v.y, v.z));
	}
	else if (p->GetParamType() == Param_Type::Param_MATH)
	{
		setCurrentIndex(1);
		setEditText(QString::fromStdString(p->GetMathString()));
	}
	else if (p->GetParamType() == Param_Type::Param_STRING)
	{
		setCurrentIndex(2);
		setEditText(QString::fromStdString(p->GetStringValue()));
	}
	else
	{
		assert(false);
	}
	blockSignals(false);
}

void CEditVariableParam::onCurrentIndexChanged(int index)
{
	if (m_param == nullptr) return;

	if (index == 0) m_param->SetParamType(m_param->GetVariableType());
	if (index == 1) m_param->SetParamType(Param_MATH);
	if (index == 2) m_param->SetParamType(Param_STRING);

	setParam(m_param);

	emit typeChanged();
}

void CEditVariableParam::onEditTextChanged(const QString& txt)
{
	if (txt.isEmpty()) return;
	if (m_param == nullptr) return;

	Param* p = m_param;
	if ((txt[0] == '=') && (p->GetParamType() != Param_MATH))
	{
		p->SetParamType(Param_MATH);
		blockSignals(true);
		setCurrentIndex(1);
		setEditText(txt);
		blockSignals(false);
	}
	else if ((txt[0] == '\"') && (p->GetParamType() != Param_STRING))
	{
		p->SetParamType(Param_STRING);
		blockSignals(true);
		setCurrentIndex(2);
		setEditText(txt);
		blockSignals(false);
	}
}
