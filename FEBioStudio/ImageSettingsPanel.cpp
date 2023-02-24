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
#include "ImageSettingsPanel.h"
#include "MainWindow.h"
#include "PropertyListForm.h"
#include "ObjectProps.h"
#include <PostLib/ImageModel.h>
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
    
    void setup(::CImageParam* parent, Param* param)
    {
        m_param = param;

        QHBoxLayout* layout = new QHBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        layout->addWidget(slider = new CDoubleSlider);

        if(param->GetFloatMax() != 0)
        {
            slider->setRange(param->GetFloatMin(), param->GetFloatMax());
        }

        slider->setValue(m_param->GetFloatValue());

        QObject::connect(slider, &CDoubleSlider::valueChanged, parent, &::CImageParam::updateParam);

        parent->setLayout(layout);
    }
};

CImageParam::CImageParam(Param* param) : ui(new Ui::CImageParam)
{
    ui->setup(this, param);
}

CImageParam::~CImageParam()
{
    delete ui;
}

void CImageParam::updateParam()
{
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

	void setup(::CImageParam2* parent, Param* param1, Param* param2)
	{
		m_param1 = param1;
		m_param2 = param2;

		QHBoxLayout* layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);

		layout->addWidget(spinLeft  = new QDoubleSpinBox);
		layout->addWidget(slider    = new CRangeSlider);
		layout->addWidget(spinRight = new QDoubleSpinBox);

		double vmin = param1->GetFloatMin();
		double vmax = param1->GetFloatMax();
		double val1 = param1->GetFloatValue();
		double val2 = param2->GetFloatValue();

		spinLeft->setRange(vmin, vmax); 
		spinLeft->setSingleStep((vmax - vmin) / 100);
		spinLeft->setValue(val1);

		spinRight->setRange(vmin, vmax); 
		spinRight->setSingleStep((vmax - vmin) / 100);
		spinRight->setValue(val2);

		slider->setRange(vmin, vmax);
		slider->setPositions(val1, val2);

		QObject::connect(slider, &CRangeSlider::positionChanged, parent, &::CImageParam2::updateSlider);
		QObject::connect(spinLeft , &QDoubleSpinBox::valueChanged, parent, &::CImageParam2::updateSpinBox);
		QObject::connect(spinRight, &QDoubleSpinBox::valueChanged, parent, &::CImageParam2::updateSpinBox);

		parent->setLayout(layout);

		slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	}
};

CImageParam2::CImageParam2(Param* param1, Param* param2) : ui(new Ui::CImageParam2)
{
	ui->setup(this, param1, param2);
}

CImageParam2::~CImageParam2()
{
	delete ui;
}

void CImageParam2::updateSlider()
{
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
class Ui::CImageSettingsPanel
{
public:
    QFormLayout* leftPanel;
    QFormLayout* rightPanel;

	std::vector<QWidget*>	m_children;

public:
    void setup(::CImageSettingsPanel* panel)
    {
        m_panel = panel;

        leftPanel = new QFormLayout;
        rightPanel = new QFormLayout;

		QHBoxLayout* layout = new QHBoxLayout;
		layout->addLayout(leftPanel);
		layout->addLayout(rightPanel);

        panel->setLayout(layout);
    }

    void setImageModel(Post::CImageModel* img)
    {
        for(auto w : m_children)
        {
            delete w;
        }

        m_children.clear();

        if(img)
        {
            CImageViewSettings* settings = img->GetViewSettings();

			::CImageParam* scale = new ::CImageParam(&settings->GetParam(CImageViewSettings::ALPHA_SCALE));
			::CImageParam* gamma = new ::CImageParam(&settings->GetParam(CImageViewSettings::GAMMA));
			::CImageParam* hue   = new ::CImageParam(&settings->GetParam(CImageViewSettings::HUE));
			::CImageParam* sat   = new ::CImageParam(&settings->GetParam(CImageViewSettings::SAT));
			::CImageParam* lum   = new ::CImageParam(&settings->GetParam(CImageViewSettings::LUM));

			::CImageParam2* intensity = new ::CImageParam2(&settings->GetParam(CImageViewSettings::MIN_INTENSITY), &settings->GetParam(CImageViewSettings::MAX_INTENSITY));
			::CImageParam2* alphaRng  = new ::CImageParam2(&settings->GetParam(CImageViewSettings::MIN_ALPHA), &settings->GetParam(CImageViewSettings::MAX_ALPHA));

			addWidget(scale, "Alpha scale");
			addWidget(gamma, "Gamma correction");
			addWidget(hue  , "Hue");
			addWidget(sat  , "Saturation");
			addWidget(lum  , "Luminance");

			addWidget(intensity, "Intensity");
			addWidget(alphaRng , "Alpha range");
        }
    }

	void addWidget(::CImageParam* w, const QString& name)
	{
		leftPanel->addRow(name, w);
		m_children.push_back(w);
		QObject::connect(w, &::CImageParam::paramChanged, m_panel, &::CImageSettingsPanel::ParamChanged);
	}

	void addWidget(::CImageParam2* w, const QString& name)
	{
		rightPanel->addRow(name, w);
		m_children.push_back(w);
		QObject::connect(w, &::CImageParam2::paramChanged, m_panel, &::CImageSettingsPanel::ParamChanged);
	}

private:
	::CImageSettingsPanel* m_panel;
};

CImageSettingsPanel::CImageSettingsPanel(CMainWindow* wnd, QWidget* parent)
    : CCommandPanel(wnd, parent), ui(new Ui::CImageSettingsPanel)
{
    ui->setup(this);
}

void CImageSettingsPanel::ModelTreeSelectionChanged(FSObject* obj)
{
    Post::CImageModel* model = dynamic_cast<Post::CImageModel*>(obj);

    ui->setImageModel(model);

    if(model)
    {
        parentWidget()->show();
        parentWidget()->raise();
    }

}

void CImageSettingsPanel::ParamChanged()
{
    GetMainWindow()->UpdateUiView();
}