#include "stdafx.h"
#include "PostPanel.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include "PostDoc.h"
#include <QSlider>
#include "DataFieldSelector.h"

class Ui::CPostPanel
{
public:
	CPostDoc*	m_pd;

public:
	CDataFieldSelector*	m_data;

public:
	void setup(QWidget* w)
	{
		m_pd = nullptr;

		QFormLayout* form = new QFormLayout;
		form->addRow("Data:", m_data = new CDataFieldSelector);
		form->setLabelAlignment(Qt::AlignRight);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		w->setLayout(l);

		QObject::connect(m_data, SIGNAL(currentValueChanged(int)), w, SLOT(on_data_changed(int)));
	}
};

CPostPanel::CPostPanel(QWidget* parent) : QWidget(parent), ui(new Ui::CPostPanel)
{
	ui->setup(this);
}

void CPostPanel::SetPostDoc(CPostDoc* pd)
{
	if (pd != ui->m_pd)
	{
		ui->m_pd = pd;
		if (pd)
		{
			ui->m_data->BuildMenu(pd->GetFEModel(), Post::Data_Tensor_Type::DATA_SCALAR);
		}
	}
}

void CPostPanel::on_data_changed(int n)
{
	if (ui->m_pd)
	{
		ui->m_pd->SetDataField(n);
		emit dataChanged();
	}
}
