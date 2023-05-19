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
#include <MeshLib/MeshTools.h>
#include <ImageLib/3DImage.h>
#include <FEMLib/FSModel.h>
#include <FEMLib/FELoadController.h>
#include <FSCore/LoadCurve.h>



class UIImageMapTool : public QWidget
{
public:
    QLineEdit* name;
    QComboBox* imageBox;
    QComboBox* methodBox;
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
        formLayout->addRow("Method:", methodBox = new QComboBox);
        methodBox->addItems(QStringList() << "Sample Image at Nodes" << "Average Intensity Over Elements");
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

    bool calcNodalValues = ui->methodBox->currentIndex() == 0;

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
    
    if(calcNodalValues)
    {
        pdata->Create(partSet, FEMeshData::DATA_SCALAR, FEMeshData::DATA_MULT);
    }
    else
    {
        pdata->Create(partSet, FEMeshData::DATA_SCALAR, FEMeshData::DATA_ITEM);
    }
    pm->AddMeshDataField(pdata);

    bool normalize = ui->normalize->isChecked();
    bool useFilter = ui->useFilter->isChecked();
    std::vector<double> mathArguments = { 0 };

    FEElemList* elemList = pdata->BuildElemList();
    int NE = elemList->Size();
    auto it = elemList->First();

    std::vector<FEElement_*> elems;
    for(int i = 0; i < NE; ++i, ++it)
    {
        elems.push_back(it->m_pi);
    }

    if(calcNodalValues)
    {
        #pragma omp parallel for
        for (int i = 0; i < NE; ++i)
        {
            FEElement_* el = elems[i];
            int ne = el->Nodes();
            for (int j = 0; j < ne; ++j)
            {
                vec3d pos = mesh->LocalToGlobal(mesh->Node(el->m_node[j]).pos());
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
    }
    else
    {
        BOX box = imageModel->GetBoundingBox();
        vec3d origin(box.x0, box.y0, box.z0);

        int imgWidth = imageModel->Get3DImage()->Width();
        int imgHeight = imageModel->Get3DImage()->Height();
        int imgDepth = imageModel->Get3DImage()->Depth();

        double xScale = imgWidth/(box.x1 - box.x0);
        double yScale = imgHeight/(box.y1 - box.y0);
        double zScale = imgDepth/(box.z1 - box.z0);

        #pragma omp parallel for
        for (int elID = 0; elID < NE; ++elID)
        {
            FEElement_* el = elems[elID];


            // find bounding box of element
            double minX, maxX, minY, maxY, minZ, maxZ;
            vec3d firstPos = mesh->LocalToGlobal(mesh->Node(el->m_node[0]).pos());
            minX = maxX = firstPos.x;
            minY = maxY = firstPos.y;
            minZ = maxZ = firstPos.z;

            int ne = el->Nodes();
            for(int nodeID = 1; nodeID < ne; nodeID++)
            {
                vec3d pos = mesh->LocalToGlobal(mesh->Node(el->m_node[nodeID]).pos());

                if(pos.x < minX)
                {
                    minX = pos.x;
                }
                else if(pos.x > maxX)
                {
                    maxX = pos.x;
                }

                if(pos.y < minY)
                {
                    minY = pos.y;
                }
                else if(pos.y > maxY)
                {
                    maxY = pos.y;
                }

                if(pos.z < minZ)
                {
                    minZ = pos.z;
                }
                else if(pos.z > maxZ)
                {
                    maxZ = pos.z;
                }
            }

            // find section of image that corresponds to bounding box
            int minXPixel = (minX - origin.x)*xScale - 1;
            int maxXPixel = (maxX - origin.x)*xScale + 1;
            int minYPixel = (minY - origin.y)*yScale - 1;
            int maxYPixel = (maxY - origin.y)*yScale + 1;
            int minZPixel = (minZ - origin.z)*zScale - 1;
            int maxZPixel = (maxZ - origin.z)*zScale + 1;

            if(minXPixel < 0) minXPixel = 0;
            if(maxXPixel > imgWidth) maxXPixel = imgWidth;
            if(minYPixel < 0) minYPixel = 0;
            if(maxYPixel > imgHeight) maxYPixel = imgHeight;
            if(minZPixel < 0) minZPixel = 0;
            if(maxZPixel > imgDepth) maxZPixel = imgDepth;

            // Sum pixel contribution
            double r[3];
            double val = 0;
            int numPixels = 0;
            for(int k = minZPixel; k < maxZPixel; k++)
            {
                vec3d pixelPos(0,0,k/zScale+origin.z);

                for(int j = minYPixel; j < maxYPixel; j++)
                {
                    pixelPos.y = j/yScale+origin.y;
                    for(int i = minXPixel; i < maxXPixel; i++)
                    {
                        pixelPos.x = i/xScale+origin.x;

                        vec3d localPixelPos = mesh->GlobalToLocal(pixelPos);

                        if(ProjectInsideElement(*mesh, *el, to_vec3f(localPixelPos), r))
                        {
                            val += data[k*imgWidth*imgHeight + j*imgWidth + i];
                            numPixels++;
                        }
                    }   
                }
            }

            if(numPixels > 0) val /= numPixels;
            
            if(normalize)
            {
                val = (val - min)/(max-min);
            }

            if(useFilter)
            {
                val = ui->loadCurve.value(val);
            }

            pdata->SetValue(elID, 0, val);
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