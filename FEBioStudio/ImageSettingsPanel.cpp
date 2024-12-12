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

#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLabel>
#include "ImageSettingsPanel.h"
#include "MainWindow.h"
#include "PropertyListForm.h"
#include "ObjectProps.h"
#include <ImageLib/ImageModel.h>
#include <ImageLib/3DImage.h>
#include <ImageLib/ImageAnalysis.h>
#include "InputWidgets.h"
#include "RangeSlider.h"
#include <vector>

class Ui::CImageParam
{
public:
    CDoubleSlider* slider;

    Param* m_param;
public:

    ~CImageParam()
    {
        delete slider;
    }
    
    void setup(::CImageParam* parent)
    {
        m_param = nullptr;

        QHBoxLayout* layout = new QHBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        layout->addWidget(slider = new CDoubleSlider);

        QObject::connect(slider, &CDoubleSlider::valueChanged, parent, &::CImageParam::updateParam);

        parent->setLayout(layout);
    }
};

CImageParam::CImageParam() : ui(new Ui::CImageParam)
{
    ui->setup(this);
}

void CImageParam::setParam(Param* param)
{
	ui->m_param = param;
	if (param == nullptr) return;

	ui->slider->blockSignals(true);
	if (param->GetFloatMax() != 0)
	{
		ui->slider->setRange(param->GetFloatMin(), param->GetFloatMax());
	}
	ui->slider->setValue(param->GetFloatValue());
	ui->slider->blockSignals(false);
}

CImageParam::~CImageParam()
{
    delete ui;
}

void CImageParam::updateParam()
{
	if (ui->m_param == nullptr) return;
    ui->m_param->SetFloatValue(ui->slider->getValue());
    emit paramChanged();
}
//=======================================================================================

class Ui::CImageParam2
{
public:
	CRangeSlider* slider;
	QDoubleSpinBox* spinLeft;
	QDoubleSpinBox* spinRight;

	Param* m_param1;
	Param* m_param2;

public:

	~CImageParam2()
	{
		delete slider;
	}

	void setup(::CImageParam2* parent)
	{
		m_param1 = nullptr;
		m_param2 = nullptr;

		QHBoxLayout* layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);

		layout->addWidget(spinLeft  = new QDoubleSpinBox);
		layout->addWidget(slider    = new CRangeSlider);
		layout->addWidget(spinRight = new QDoubleSpinBox);

		QObject::connect(slider, &CRangeSlider::positionChanged, parent, &::CImageParam2::updateSlider);
		QObject::connect(spinLeft , &QDoubleSpinBox::valueChanged, parent, &::CImageParam2::updateSpinBox);
		QObject::connect(spinRight, &QDoubleSpinBox::valueChanged, parent, &::CImageParam2::updateSpinBox);

		parent->setLayout(layout);

		slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	}
};

CImageParam2::CImageParam2() : ui(new Ui::CImageParam2)
{
	ui->setup(this);
}

void CImageParam2::setParams(Param* param1, Param* param2)
{
	ui->m_param1 = param1;
	ui->m_param2 = param2;
	if ((param1 == nullptr) || (param2 == nullptr)) return;

	double vmin = param1->GetFloatMin();
	double vmax = param1->GetFloatMax();
	double val1 = param1->GetFloatValue();
	double val2 = param2->GetFloatValue();

	ui->spinLeft->blockSignals(true);
	ui->spinLeft->setRange(vmin, vmax);
	ui->spinLeft->setSingleStep((vmax - vmin) / 100);
	ui->spinLeft->setValue(val1);
	ui->spinLeft->blockSignals(false);

	ui->spinRight->blockSignals(true);
	ui->spinRight->setRange(vmin, vmax);
	ui->spinRight->setSingleStep((vmax - vmin) / 100);
	ui->spinRight->setValue(val2);
	ui->spinRight->blockSignals(false);

	ui->slider->blockSignals(true);
	ui->slider->setRange(vmin, vmax);
	ui->slider->setPositions(val1, val2);
	ui->slider->blockSignals(false);
}

CImageParam2::~CImageParam2()
{
	delete ui;
}

void CImageParam2::setColor(QColor c)
{
	ui->slider->setSelectionColor(c);
}

void CImageParam2::updateSlider()
{
	if ((ui->m_param1 == nullptr) || (ui->m_param2 == nullptr)) return;

	double val1 = ui->slider->leftPosition();
	double val2 = ui->slider->rightPosition();
	ui->m_param1->SetFloatValue(val1);
	ui->m_param2->SetFloatValue(val2);

	// update the spin boxes
	ui->spinLeft->blockSignals(true); ui->spinLeft->setValue(val1); ui->spinLeft->blockSignals(false);
	ui->spinRight->blockSignals(true); ui->spinRight->setValue(val2); ui->spinRight->blockSignals(false);

	emit paramChanged();
}

void CImageParam2::updateSpinBox()
{
	double val1 = ui->spinLeft->value();
	double val2 = ui->spinRight->value();
	ui->m_param1->SetFloatValue(val1);
	ui->m_param2->SetFloatValue(val2);

	// update the slider
	ui->slider->blockSignals(true); 
	ui->slider->setPositions(val1, val2);
	ui->slider->blockSignals(false);

	emit paramChanged();
}

//=======================================================================================
class Ui::CImageSettingsWidget
{
public:
	QLabel*		name;
	QWidget*	w[3];
	QFormLayout* panel[3];

	::CImageParam* scale;
	::CImageParam* gamma;
	::CImageParam* hue;
	::CImageParam* sat;
	::CImageParam* lum;

	::CImageParam2* intensity;
	::CImageParam2* alphaRng;
	::CImageParam2* clipx;
	::CImageParam2* clipy;
	::CImageParam2* clipz;

	::CImageParam* chue1;
	::CImageParam* chue2;
	::CImageParam* chue3;

public:
    void setup(::CImageSettingsWidget* parent)
    {
        m_parent = parent;

		name = new QLabel;
		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(new QLabel("<b>Image volume:</b> "));
		h->addWidget(name);
		h->addStretch();

        panel[0] = new QFormLayout;
        panel[1] = new QFormLayout;
        panel[2] = new QFormLayout;

		w[0] = new QWidget;
		w[1] = new QWidget;
		w[2] = new QWidget;
		w[0]->setLayout(panel[0]);
		w[1]->setLayout(panel[1]);
		w[2]->setLayout(panel[2]);

		QHBoxLayout* layout = new QHBoxLayout;
		layout->addWidget(w[0]);
		layout->addWidget(w[1]);
		layout->addWidget(w[2]);

		scale = new ::CImageParam();
		gamma = new ::CImageParam();
		hue   = new ::CImageParam();
		sat   = new ::CImageParam();
		lum   = new ::CImageParam();

		intensity = new ::CImageParam2();
		alphaRng = new ::CImageParam2();
		clipx = new ::CImageParam2();
		clipy = new ::CImageParam2();
		clipz = new ::CImageParam2();

		clipx->setColor(QColor::fromRgb(255, 0, 0));
		clipy->setColor(QColor::fromRgb(0, 255, 0));
		clipz->setColor(QColor::fromRgb(0, 0, 255));

		chue1 = new ::CImageParam();
		chue2 = new ::CImageParam();
		chue3 = new ::CImageParam();

		addWidget(scale, "Alpha scale", 0);
		addWidget(gamma, "Gamma correction", 0);
		addWidget(hue, "Hue", 0);
		addWidget(sat, "Saturation", 0);
		addWidget(lum, "Luminance", 0);

		addWidget(intensity, "Intensity", 1);
		addWidget(alphaRng, "Alpha range", 1);
		addWidget(clipx, "Clip X", 1);
		addWidget(clipy, "Clip Y", 1);
		addWidget(clipz, "Clip Z", 1);

		addWidget(chue1, "Channel1 Hue", 2);
		addWidget(chue2, "Channel2 Hue", 2);
		addWidget(chue3, "Channel3 Hue", 2);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(h);
		mainLayout->addLayout(layout);
		parent->setLayout(mainLayout);
    }

    void setImageModel(CImageModel* img)
    {
        if(img)
        {
			name->setText(QString::fromStdString(img->GetName()));

            CImageViewSettings* settings = img->GetViewSettings();

			scale->setParam(&settings->GetParam(CImageViewSettings::ALPHA_SCALE));
			gamma->setParam(&settings->GetParam(CImageViewSettings::GAMMA));
			hue  ->setParam(&settings->GetParam(CImageViewSettings::HUE));
			sat  ->setParam(&settings->GetParam(CImageViewSettings::SAT));
			lum  ->setParam(&settings->GetParam(CImageViewSettings::LUM));

			intensity->setParams(&settings->GetParam(CImageViewSettings::MIN_INTENSITY), &settings->GetParam(CImageViewSettings::MAX_INTENSITY));
			alphaRng ->setParams(&settings->GetParam(CImageViewSettings::MIN_ALPHA), &settings->GetParam(CImageViewSettings::MAX_ALPHA));
			clipx    ->setParams(&settings->GetParam(CImageViewSettings::CLIPX_MIN), &settings->GetParam(CImageViewSettings::CLIPX_MAX));
			clipy    ->setParams(&settings->GetParam(CImageViewSettings::CLIPY_MIN), &settings->GetParam(CImageViewSettings::CLIPY_MAX));
			clipz    ->setParams(&settings->GetParam(CImageViewSettings::CLIPZ_MIN), &settings->GetParam(CImageViewSettings::CLIPZ_MAX));

			C3DImage* im = img->Get3DImage();
			if (im && ((im->PixelType() == CImage::INT_RGB8) || (im->PixelType() == CImage::UINT_RGB8) || (im->PixelType() == CImage::INT_RGB16)))
			{
				chue1->setParam(&settings->GetParam(CImageViewSettings::CHANNEL1_HUE));
				chue2->setParam(&settings->GetParam(CImageViewSettings::CHANNEL2_HUE));
				chue3->setParam(&settings->GetParam(CImageViewSettings::CHANNEL3_HUE));
				w[2]->show();
			}
			else
			{
				chue1->setParam(nullptr);
				chue2->setParam(nullptr);
				chue3->setParam(nullptr);
				w[2]->hide();
			}
		}
		else
		{
			name->setText("(none selected)");

			scale->setParam(nullptr);
			gamma->setParam(nullptr);
			hue->setParam(nullptr);
			sat->setParam(nullptr);
			lum->setParam(nullptr);
			intensity->setParams(nullptr, nullptr);
			alphaRng->setParams(nullptr, nullptr);
			clipx->setParams(nullptr, nullptr);
			clipy->setParams(nullptr, nullptr);
			clipz->setParams(nullptr, nullptr);
			chue1->setParam(nullptr);
			chue2->setParam(nullptr);
			chue3->setParam(nullptr);
		}
    }

	void addWidget(::CImageParam* w, const QString& name, int panelIndex)
	{
		panel[panelIndex]->addRow(name, w);
		QObject::connect(w, &::CImageParam::paramChanged, m_parent, &::CImageSettingsWidget::ParamChanged);
	}

	void addWidget(::CImageParam2* w, const QString& name, int panelIndex)
	{
		panel[panelIndex]->addRow(name, w);
		QObject::connect(w, &::CImageParam2::paramChanged, m_parent, &::CImageSettingsWidget::ParamChanged);
	}

private:
	::CImageSettingsWidget* m_parent;
};

CImageSettingsWidget::CImageSettingsWidget(QWidget* parent) 
    : QWidget(parent), ui(new Ui::CImageSettingsWidget)
{
    ui->setup(this);
}

 void CImageSettingsWidget::ImageModelChanged(CImageModel* model)
 {
    ui->setImageModel(model);
 }


//=======================================================================================
class Ui::CImageSettingsPanel
{
public:
	::CImageSettingsWidget* m_widget;

public:
    void setup(::CImageSettingsPanel* parent)
    {
        m_parent = parent;

        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        layout->addWidget(m_widget = new ::CImageSettingsWidget);

		parent->setLayout(layout);

        QObject::connect(m_widget, &::CImageSettingsWidget::ParamChanged, m_parent, &::CImageSettingsPanel::on_ParamChanged);
    }

private:
	::CImageSettingsPanel* m_parent;
};

CImageSettingsPanel::CImageSettingsPanel(CMainWindow* wnd, QWidget* parent)
    : CWindowPanel(wnd, parent), ui(new Ui::CImageSettingsPanel)
{
    ui->setup(this);
}

void CImageSettingsPanel::ModelTreeSelectionChanged(FSObject* obj)
{
    CImageModel* model = dynamic_cast<CImageModel*>(obj);
	if (model == nullptr)
	{
		CImageAnalysis* ima = dynamic_cast<CImageAnalysis*>(obj);
		if (ima) model = ima->GetImageModel();
	}

    ui->m_widget->ImageModelChanged(model);

	if (model)
	{
		parentWidget()->show();
		parentWidget()->raise();
		ui->m_widget->show();
	}
	else ui->m_widget->hide();

}

void CImageSettingsPanel::on_ParamChanged()
{
    GetMainWindow()->UpdateUiView();
}