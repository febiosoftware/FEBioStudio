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
	QSlider*			m_slider;
	CDataFieldSelector*	m_data;

public:
	void setup(QWidget* w)
	{
		m_pd = nullptr;

		QFormLayout* form = new QFormLayout;
		form->addRow("State:", m_slider = new QSlider);
		form->addRow("Data:", m_data = new CDataFieldSelector);
		form->setLabelAlignment(Qt::AlignRight);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		w->setLayout(l);

		m_slider->setOrientation(Qt::Horizontal);
		m_slider->setDisabled(true);

		QObject::connect(m_slider, SIGNAL(valueChanged(int)), w, SLOT(on_slider_changed(int)));
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
			ui->m_slider->setEnabled(true);

			int N = pd->GetStates();
			ui->m_slider->setRange(0, N - 1);
			ui->m_slider->setPageStep(1);
			ui->m_slider->setValue(0);

			ui->m_data->BuildMenu(pd->GetFEModel(), Post::Data_Tensor_Type::DATA_SCALAR);
		}
		else ui->m_slider->setDisabled(true);
	}
}

void CPostPanel::on_slider_changed(int n)
{
	if (ui->m_pd)
	{
		if ((n >= 0) && (n < ui->m_pd->GetStates()))
		{
			ui->m_pd->SetActiveState(n);
		}

		emit dataChanged();
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
