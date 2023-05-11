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

#include <QWidget>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QCheckBox>
#include <QMessageBox>
#include "ImageMapTool.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include "IconProvider.h"
#include "PlotWidget.h"
#include <PostLib/ImageModel.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FEElementData.h>
#include <ImageLib/3DImage.h>
#include <FEMLib/FSModel.h>
#include <FEMLib/FELoadController.h>
#include <FSCore/LoadCurve.h>



class UIImageMapTool : public QWidget
{
public:
    QLineEdit* name;
    QComboBox* imageBox;
    QCheckBox* normalize;
    QCheckBox* useFilter;
    CCurveEditWidget* curveEdit;
    QPushButton* create;

public:
    UIImageMapTool(CImageMapTool* tool)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        QScrollArea* scrollArea = new QScrollArea;
        scrollArea->setWidgetResizable(true);
        scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

        QWidget* innerWidget = new QWidget;
        innerWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        QVBoxLayout* innerLayout = new QVBoxLayout;

        QFormLayout* formLayout = new QFormLayout;
        formLayout->setContentsMargins(0,0,0,0);
        
        formLayout->addRow("Name:", name = new QLineEdit);

        formLayout->addRow("Image Model:", imageBox = new QComboBox);
        formLayout->addRow("Normalize:", normalize = new QCheckBox);
        normalize->setChecked(true);
        formLayout->addRow("Filter:", useFilter = new QCheckBox);
        useFilter->setChecked(false);
        innerLayout->addLayout(formLayout);

        innerLayout->addWidget(curveEdit = new CCurveEditWidget);
        loadCurve.SetExtendMode(PointCurve::EXTRAPOLATE);
        curveEdit->SetLoadCurve(&loadCurve);
        curveEdit->hide();

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->setContentsMargins(0,0,0,0);
        buttonLayout->addStretch();
        buttonLayout->addWidget(create = new QPushButton("Create"));
        create->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        innerLayout->addLayout(buttonLayout);

        innerLayout->addStretch();

        innerWidget->setLayout(innerLayout);

        scrollArea->setWidget(innerWidget);

        layout->addWidget(scrollArea);

        setLayout(layout);

        connect(create, &QPushButton::clicked, tool, &CImageMapTool::OnCreate);
        connect(useFilter, &QCheckBox::stateChanged, tool, &CImageMapTool::on_useFilter_stateChanged);
    }

public:
    LoadCurve loadCurve;
};

CImageMapTool::CImageMapTool(CMainWindow* wnd)
    : CAbstractTool(wnd, "Image Map"), m_po(nullptr), ui(nullptr)
{
}

QWidget* CImageMapTool::createUi()
{
	if (ui == nullptr) ui = new UIImageMapTool(this);
	return ui;
}

void CImageMapTool::Activate()
{
    CModelDocument* doc = GetMainWindow()->GetModelDocument();
    
    ui->imageBox->clear();

    if(doc)
    {
        for(int model = 0; model < doc->ImageModels(); model++)
        {
            ui->imageBox->addItem(doc->GetImageModel(model)->GetName().c_str());
        }
    }

    Clear();
	CAbstractTool::Activate();
}

void CImageMapTool::Clear()
{
	m_po = nullptr;
	ui->name->clear();
}

void CImageMapTool::OnCreate()
{
    // get the document
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());

	// get the currently selected object
	GObject* po = pdoc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "You must first select an object.");
		return;
	}

	// make sure there is a mesh
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0)
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "The object needs to be meshed before you can apply this tool.");
		return;
	}

    Post::CImageModel* imageModel;
    if(pdoc->ImageModels() < ui->imageBox->currentIndex())
    {
        QMessageBox::critical(GetMainWindow(), "Tool", QString("The chosen image model, %1, does not exist.").arg(ui->imageBox->currentText()));
		return;
    }
    else
    {
        imageModel = pdoc->GetImageModel(ui->imageBox->currentIndex());
    }

	QString name = ui->name->text();
	if (name.isEmpty())
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "You must enter a valid name.");
		return;
	}

    // find the min and max intensities
    Byte min = 255;
    Byte max = 0;
    Byte* data = imageModel->Get3DImage()->GetBytes();
    int size = imageModel->Get3DImage()->Width()*imageModel->Get3DImage()->Height()*imageModel->Get3DImage()->Depth();

    for(int index = 0; index < size; index++)
    {
        Byte val = data[index];
        if(val > max) max = val;
        if(val < min) min = val;
    }

	//get the model and nodeset
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

    // create element data
    int parts = po->Parts();
	FSPartSet* partSet = new FSPartSet(po);
    for (int i = 0; i < parts; ++i) partSet->add(i);
	partSet->SetName(name.toStdString());
	po->AddFEPartSet(partSet);

    FSMesh* mesh = po->GetFEMesh();
    FEPartData* pdata = new FEPartData(mesh);
    pdata->SetName(name.toStdString());
    pdata->Create(partSet, FEMeshData::DATA_SCALAR, FEMeshData::DATA_MULT);
    pm->AddMeshDataField(pdata);

    bool normalize = ui->normalize->isChecked();
    bool useFilter = ui->useFilter->isChecked();
    std::vector<double> mathArguments = { 0 };

    FEElemList* elemList = pdata->BuildElemList();
    int NE = elemList->Size();
    auto it = elemList->First();
    for (int i = 0; i < NE; ++i, ++it)
    {
        FEElement_& el = *it->m_pi;
        int ne = el.Nodes();
        for (int j = 0; j < ne; ++j)
        {
            vec3d pos = mesh->LocalToGlobal(mesh->Node(el.m_node[j]).pos());
            double data = imageModel->ValueAtGlobalPos(pos);

            if(normalize)
            {
                data = (data - min)/(max-min);
            }

            if(useFilter)
            {
                data = ui->loadCurve.value(data);
            }

            pdata->SetValue(i, j, data);
        }
    }
    delete elemList;

    Clear();
	GetMainWindow()->UpdateModel();
	QMessageBox::information(GetMainWindow(), "Tool", "Datafield successfully added.");
}

void CImageMapTool::on_useFilter_stateChanged(int state)
{
    ui->curveEdit->setHidden(state == 0);
}