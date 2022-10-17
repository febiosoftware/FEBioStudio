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
#include <QLabel>
#include "ImageSettingsPanel.h"
#include "MainWindow.h"
#include "PropertyListForm.h"
#include "ObjectProps.h"
#include <PostLib/ImageModel.h>
#include "InputWidgets.h"
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

        layout->addWidget(new QLabel(QString(param->GetLongName()) + ":"));
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

class Ui::CImageSettingsPanel
{
public:
    QGridLayout* layout = new QGridLayout;

    std::vector<::CImageParam*> params;

    QLabel* noImage;

public:
    void setup(::CImageSettingsPanel* panel)
    {
        m_panel = panel;

        layout = new QGridLayout;

        noImage = new QLabel("(No Image Selected)");
        layout->addWidget(noImage);

        panel->setLayout(layout);
    }

    void setImageModel(Post::CImageModel* img)
    {
        
        for(auto param : params)
        {
            delete param;
        }

        params.clear();

        if(img)
        {
            noImage->hide();

            CImageViewSettings* settings = img->GetViewSettings();

            for(int param = 0; param < settings->Parameters(); param++)
            {
                ::CImageParam* imgParam = new ::CImageParam(&settings->GetParam(param));

                QObject::connect(imgParam, &::CImageParam::paramChanged, m_panel, &::CImageSettingsPanel::ParamChanged);

                params.push_back(imgParam);

                layout->addWidget(imgParam, param/2, param % 2);
            }
        }
        else
        {
            noImage->show();
        }
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