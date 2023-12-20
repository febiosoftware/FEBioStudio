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
	else if (p->GetParamType() == Param_Type::Param_ARRAY_DOUBLE)
	{
		setCurrentIndex(0);
		std::vector<double> v = p->GetArrayDoubleValue();
		assert(v.size() == 3);
		setEditText(QString("%1,%2,%3").arg(v[0]).arg(v[1]).arg(v[2]));
	}
	else if (p->GetParamType() == Param_Type::Param_VEC3D)
	{
		setCurrentIndex(0);
		vec3d v = p->GetVec3dValue();
		setEditText(QString("%1,%2,%3").arg(v.x).arg(v.y).arg(v.z));
	}
	else if (p->GetParamType() == Param_Type::Param_MAT3DS)
	{
		setCurrentIndex(0);
		mat3ds v = p->GetMat3dsValue();
		setEditText(QString("%1,%2,%3,%4,%5,%6").arg(v.xx()).arg(v.yy()).arg(v.zz()).arg(v.xy()).arg(v.yz()).arg(v.xz()));
	}
	else if (p->GetParamType() == Param_Type::Param_MAT3D)
	{
		setCurrentIndex(0);
		mat3d v = p->GetMat3dValue();
		setEditText(QString("%1,%2,%3,%4,%5,%6,%7,%8,%9").arg(v[0][0]).arg(v[0][1]).arg(v[0][2]).arg(v[1][0]).arg(v[1][1]).arg(v[1][2]).arg(v[2][0]).arg(v[2][1]).arg(v[2][2]));
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
