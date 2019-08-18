#include "stdafx.h"
#include "DlgAddMeshData.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <FSCore/Object.h>
#include <QCheckBox>

class CDlgAddMeshDataUI
{
public:
	QComboBox*	m_param;
	QComboBox*	m_type;
	QCheckBox*	m_custom;

	struct Item
	{
		const char*	szname;
		const char* szparam;
		Param_Type	type;
	};

	std::vector<Item>	m_item;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* layout = new QVBoxLayout;
		m_param = new QComboBox;

		m_custom = new QCheckBox("custom");

		m_type = new QComboBox;
		m_type->addItem("scalar");
		m_type->addItem("vec3");
		m_type->setDisabled(true);

		QFormLayout* form = new QFormLayout;
		form->addRow("parameter", m_param);
		form->addRow("data type", m_type);

		layout->addLayout(form);
		layout->addWidget(m_custom);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(bb);

		dlg->setLayout(layout);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(m_custom, SIGNAL(clicked(bool)), dlg, SLOT(onCustom()));
		QObject::connect(m_param, SIGNAL(currentIndexChanged(int)), dlg, SLOT(setItem(int)));
	}
};

CDlgAddMeshData::CDlgAddMeshData(CObject* po, QWidget* pw) : QDialog(pw), ui(new CDlgAddMeshDataUI)
{
	setMinimumWidth(300);

	ui->setup(this);

	int n = po->Parameters();
	for (int i=0; i<n; ++i)
	{
		Param& p = po->GetParam(i);
		Param_Type type = p.GetParamType();
		if      ((type == Param_FLOAT) || (type == Param_VEC3D))
		{
			CDlgAddMeshDataUI::Item item = { p.GetLongName(), p.GetShortName(), type };
			ui->m_item.push_back(item);
			ui->m_param->addItem(p.GetLongName());
		}
	}

	setItem(0);
}

void CDlgAddMeshData::setItem(int n)
{
	if ((n<0) || (n >= (int) ui->m_item.size())) return;

	CDlgAddMeshDataUI::Item item = ui->m_item[n];

	ui->m_param->setEditText(item.szname);
	switch (item.type)
	{
	case Param_FLOAT: ui->m_type->setCurrentIndex(0); break;
	case Param_VEC3D: ui->m_type->setCurrentIndex(1); break;
	}
}

void CDlgAddMeshData::onCustom()
{
	bool b = ui->m_custom->isChecked();
	if (b)
	{
		ui->m_param->setEditable(true);
		ui->m_type->setEnabled(true);
	}	
	else
	{
		ui->m_param->setEditable(false);
		ui->m_type->setEnabled(false);
	}
}

std::string CDlgAddMeshData::GetMapName()
{
	QString txt = ui->m_param->currentText();
	return txt.toStdString();
}

std::string CDlgAddMeshData::GetParamName()
{
	bool b = ui->m_custom->isChecked();
	if (b) return GetMapName();
	else
	{
		int n = ui->m_param->currentIndex();
		return ui->m_item[n].szparam;
	}
}

Param_Type CDlgAddMeshData::GetParamType()
{
	int n = ui->m_type->currentIndex();

	if (n == 0) return Param_FLOAT;
	if (n == 1) return Param_VEC3D;

	return Param_UNDEF;
}
