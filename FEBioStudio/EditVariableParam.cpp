#include "stdafx.h"
#include "EditVariableParam.h"
#include <FSCore/ParamBlock.h>
#include <QComboBox>
#include <QBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include "FEBioStudio.h"
#include "MainWindow.h"
#include <FEMLib/FSModel.h>

class UIEditVariableParam
{
public:
	FSModel* m_fem = nullptr;
	Param* m_param = nullptr;

	QComboBox* combo;
	QPushButton* edit;

	void setup(CEditVariableParam* parent)
	{
		combo = new QComboBox(parent);
		combo->addItem("<constant>");
		combo->addItem("<math>");
		combo->addItem("<map>");
		combo->addItem("<code>");

		combo->setEditable(true);
		combo->setInsertPolicy(QComboBox::NoInsert);
		combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

		edit = new QPushButton();
		edit->setIcon(QIcon(":/icons/edit.png"));
		edit->hide(); // we will show this button only for code types

		QHBoxLayout* l = new QHBoxLayout;
		l->setContentsMargins(0, 0, 0, 0);
		l->setSpacing(0);
		l->addWidget(combo);
		l->addWidget(edit);
		parent->setLayout(l);

		QObject::connect(combo, &QComboBox::currentIndexChanged, parent, &CEditVariableParam::onCurrentIndexChanged);
		QObject::connect(combo, &QComboBox::editTextChanged, parent, &CEditVariableParam::onTextChanged);
		QObject::connect(edit , &QPushButton::pressed, parent, &CEditVariableParam::onEditPressed);
	}
};

//-----------------------------------------------------------------------------
CEditVariableParam::CEditVariableParam(FSModel* fem, QWidget* parent) : QWidget(parent), ui(new UIEditVariableParam)
{
	ui->m_fem = fem;
	ui->setup(this);
}

QString CEditVariableParam::text() const
{
	return ui->combo->currentText();
}

void CEditVariableParam::setParam(Param* p)
{
	ui->m_param = p;
	if (p == nullptr) return;

	bool showEditButton = false;

	ui->combo->blockSignals(true);
	if (p->GetParamType() == Param_Type::Param_FLOAT)
	{
		ui->combo->setCurrentIndex(0);
		ui->combo->setEditText(QString("%1").arg(p->GetFloatValue()));
	}
	else if (p->GetParamType() == Param_Type::Param_ARRAY_DOUBLE)
	{
		ui->combo->setCurrentIndex(0);
		std::vector<double> v = p->GetArrayDoubleValue();
		assert(v.size() == 3);
		ui->combo->setEditText(QString("%1,%2,%3").arg(v[0]).arg(v[1]).arg(v[2]));
	}
	else if (p->GetParamType() == Param_Type::Param_VEC3D)
	{
		ui->combo->setCurrentIndex(0);
		vec3d v = p->GetVec3dValue();
		ui->combo->setEditText(QString("%1,%2,%3").arg(v.x).arg(v.y).arg(v.z));
	}
	else if (p->GetParamType() == Param_Type::Param_MAT3DS)
	{
		ui->combo->setCurrentIndex(0);
		mat3ds v = p->GetMat3dsValue();
		ui->combo->setEditText(QString("%1,%2,%3,%4,%5,%6").arg(v.xx()).arg(v.yy()).arg(v.zz()).arg(v.xy()).arg(v.yz()).arg(v.xz()));
	}
	else if (p->GetParamType() == Param_Type::Param_MAT3D)
	{
		ui->combo->setCurrentIndex(0);
		mat3d v = p->GetMat3dValue();
		ui->combo->setEditText(QString("%1,%2,%3,%4,%5,%6,%7,%8,%9").arg(v[0][0]).arg(v[0][1]).arg(v[0][2]).arg(v[1][0]).arg(v[1][1]).arg(v[1][2]).arg(v[2][0]).arg(v[2][1]).arg(v[2][2]));
	}
	else if (p->GetParamType() == Param_Type::Param_MATH)
	{
		ui->combo->setCurrentIndex(1);
		ui->combo->setEditText(QString::fromStdString(p->GetMathString()));
	}
	else if (p->GetParamType() == Param_Type::Param_STRING)
	{
		ui->combo->setCurrentIndex(2);
		ui->combo->setEditText(QString::fromStdString(p->GetStringValue()));
	}
	else if (p->GetParamType() == Param_Type::Param_CODE)
	{
		ui->combo->setCurrentIndex(3);

		int id = p->GetScriptID();
		if (ui->m_fem)
		{
			FEBCodeScript* ps = ui->m_fem->GetScriptFromID(id);
			if (ps)
			{
				ui->combo->setEditText(QString::fromStdString(ps->GetName()));
			}
			else ui->combo->setEditText("");
		}
		else
			ui->combo->setEditText("(invalid)");
		showEditButton = true;
	}
	else
	{
		assert(false);
	}
	ui->combo->blockSignals(false);

	if (showEditButton) ui->edit->show();
	else ui->edit->hide();
}

void CEditVariableParam::onCurrentIndexChanged(int index)
{
	if (ui->m_param == nullptr) return;

	if (index == 0) ui->m_param->SetParamType(ui->m_param->GetVariableType());
	if (index == 1) ui->m_param->SetParamType(Param_MATH);
	if (index == 2) ui->m_param->SetParamType(Param_STRING);
	if (index == 3) ui->m_param->SetParamType(Param_CODE);

	setParam(ui->m_param);

	emit typeChanged();
}

void CEditVariableParam::onTextChanged(const QString& txt)
{
	if (txt.isEmpty()) return;
	if (ui->m_param == nullptr) return;

	Param* p = ui->m_param;
	if ((txt[0] == '=') && (p->GetParamType() != Param_MATH))
	{
		p->SetParamType(Param_MATH);
		blockSignals(true);
		ui->combo->setCurrentIndex(1);
		ui->combo->setEditText(txt);
		blockSignals(false);
	}
	else if ((txt[0] == '\"') && (p->GetParamType() != Param_STRING))
	{
		p->SetParamType(Param_STRING);
		blockSignals(true);
		ui->combo->setCurrentIndex(2);
		ui->combo->setEditText(txt);
		blockSignals(false);
	}
	else if ((txt[0] == '{') && (p->GetParamType() != Param_CODE))
	{
		p->SetParamType(Param_CODE);
		blockSignals(true);
		ui->combo->setCurrentIndex(3);
		ui->combo->setEditText(txt);
		blockSignals(false);
	}
}

void CEditVariableParam::onEditPressed()
{
	if (ui->m_param == nullptr) return;
	if (ui->m_param->GetParamType() != Param_CODE) return;
	if (ui->m_fem == nullptr) return;

	int id = ui->m_param->GetScriptID();
	QString scriptName;
	FEBCodeScript* ps = ui->m_fem->GetScriptFromID(id);
	if (ps) scriptName = QString::fromStdString(ps->GetName());
	else
	{
		static int n = 1;
		scriptName = QString("Script%1").arg(n);
		scriptName = QInputDialog::getText(this, "New Script", "Enter script name:", QLineEdit::Normal, scriptName);
		if (scriptName.isEmpty()) return;
		n++;

		ps = ui->m_fem->AddScript(scriptName.toStdString(), "return 0.0;");
		ui->m_param->SetScriptID(ps->GetID());
	}

	ui->combo->setCurrentText(scriptName);
	emit requestClose();

	CMainWindow* wnd = FBS::getMainWindow();
	wnd->OpenCodeEditor(scriptName);
}
