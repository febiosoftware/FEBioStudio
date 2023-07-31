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
#include <QDoubleSpinBox>
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
#include <limits>
#include <unordered_map>

using std::unordered_map;

enum {SAMPLE_NODES=0, SAMPLE_CENTROIDS, AVERAGE_ELEMS};

class UIImageMapTool : public QWidget
{
public:
    QFormLayout* formLayout;

    QLineEdit* name;
    QComboBox* imageBox;
    QComboBox* methodBox;
    QDoubleSpinBox*  thresholdBox;
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

        formLayout = new QFormLayout;
        formLayout->setContentsMargins(0,0,0,0);
        
        formLayout->addRow("Name:", name = new QLineEdit);

        formLayout->addRow("Image Model:", imageBox = new QComboBox);
        formLayout->addRow("Method:", methodBox = new QComboBox);
        methodBox->addItems(QStringList() << "Sample Image at Nodes" << "Sample at Element Centroids" << "Average Intensity Over Elements");
        formLayout->addRow("Surface Node Threshold:", thresholdBox = new QDoubleSpinBox);
        thresholdBox->setValue(0);
        thresholdBox->setMaximum(std::numeric_limits<double>::max());
        thresholdBox->setMinimum(std::numeric_limits<double>::min());
        thresholdVisible = true;
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
        connect(methodBox, &QComboBox::currentIndexChanged, tool, &CImageMapTool::on_methodBox_currentIndexChanged);
        connect(useFilter, &QCheckBox::stateChanged, tool, &CImageMapTool::on_useFilter_stateChanged);
    }

public:
    LoadCurve loadCurve;
    bool thresholdVisible;
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
    
    if(ui->methodBox->currentIndex() == SAMPLE_NODES)
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

    BOX box = imageModel->GetBoundingBox();
    vec3d origin(box.x0, box.y0, box.z0);
    vec3d spacing((box.x1-box.x0)/imageModel->Get3DImage()->Width(), 
        (box.y1-box.y0)/imageModel->Get3DImage()->Height(),
        (box.z1-box.z0)/imageModel->Get3DImage()->Depth());

    switch(ui->methodBox->currentIndex())
    {
    case SAMPLE_NODES:
    {
        double threshold = ui->thresholdBox->value();

        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::min();

        // find all nodal normals
        unordered_map<int, vec3f> normals;
        for(int i = 0; i < mesh->Faces(); i++)
        {
            FSFace& currentFace = mesh->Face(i);

            if(!currentFace.IsExterior()) continue;

            for(int j = 0; j < currentFace.Nodes(); j++)
            {
                int nodeID = currentFace.n[j];

                try
                {
                    normals.at(nodeID) += currentFace.m_nn[j];
                }
                catch(...)
                {
                    normals[nodeID] = currentFace.m_nn[j];
                }
            }
        }

        for(auto& normal : normals)
        {
            normal.second = normal.second.Normalize()*-1;
        }

        #pragma omp parallel for
        for (int i = 0; i < NE; ++i)
        {
            FEElement_* el = elems[i];
            int ne = el->Nodes();
            for (int j = 0; j < ne; ++j)
            {
                int nodeID = el->m_node[j];
                FSNode& current = mesh->Node(el->m_node[j]);

                vec3d pos = mesh->LocalToGlobal(current.pos());
                double val = imageModel->ValueAtGlobalPos(pos);

                if(current.IsExterior())
                {
                    int voxelIndexX = (pos.x - origin.x)/spacing.x;
                    int voxelIndexY = (pos.y - origin.y)/spacing.y;
                    int voxelIndexZ = (pos.z - origin.z)/spacing.z;

                    double discreteVal = imageModel->Get3DImage()->Value(voxelIndexX, voxelIndexY, voxelIndexZ);
                    if(discreteVal >= threshold)
                    {
                        val = discreteVal;
                    }
                    else
                    {
                        vec3f normal = normals[nodeID];
                        
                        int xSign = normal.x > 0 ? 1 : -1;
                        int ySign = normal.y > 0 ? 1 : -1;
                        int zSign = normal.z > 0 ? 1 : -1;

                        int iter = 0;
                        vec3d currentPos = pos;
                        while(iter < 5)
                        {
                            vec3d voxelCenter(origin.x + voxelIndexX*spacing.x + spacing.x/2,
                            origin.y + voxelIndexY*spacing.y + spacing.y/2,
                            origin.z + voxelIndexZ*spacing.z + spacing.z/2);

                            double vFarthestX = voxelCenter.x + spacing.x/2*xSign;
                            double vFarthestY = voxelCenter.y + spacing.y/2*ySign;
                            double vFarthestZ = voxelCenter.z + spacing.z/2*zSign;

                            double xSteps, ySteps, zSteps;
                            if(normal.x == 0)
                            {
                                xSteps = INFINITY;
                            }
                            else
                            {
                                xSteps = abs((vFarthestX - currentPos.x)/normal.x);
                            }
                            
                            if(normal.y == 0)
                            {
                                ySteps = INFINITY;
                            }
                            else
                            {
                                ySteps = abs((vFarthestY - currentPos.y)/normal.y);
                            }
                            
                            if(normal.z == 0)
                            {
                                zSteps = INFINITY;
                            }
                            else
                            {
                                zSteps = abs((vFarthestZ - currentPos.z)/normal.z);
                            }

                            double min = std::min({xSteps, ySteps, zSteps});

                            if(min == xSteps)
                            {
                                currentPos.y += normal.y/normal.x*(vFarthestX - currentPos.x);
                                currentPos.z += normal.z/normal.x*(vFarthestX - currentPos.x);

                                currentPos.x = vFarthestX;

                                voxelIndexX += xSign;
                            }
                            else if(min == ySteps)
                            {
                                currentPos.x += normal.x/normal.y*(vFarthestY - currentPos.y);
                                currentPos.z += normal.z/normal.y*(vFarthestY - currentPos.y);

                                currentPos.y = vFarthestY;

                                voxelIndexY += ySign;
                            }
                            else
                            {
                                currentPos.x += normal.x/normal.z*(vFarthestZ - currentPos.z);
                                currentPos.y += normal.y/normal.z*(vFarthestZ - currentPos.z);
                                
                                currentPos.z = vFarthestZ;

                                voxelIndexZ += zSign;
                            }

                            if(voxelIndexX >= imageModel->Get3DImage()->Width() || 
                                voxelIndexY >= imageModel->Get3DImage()->Height() ||
                                voxelIndexZ >= imageModel->Get3DImage()->Depth())
                            {
                                break;
                            }

                            double tempVal = imageModel->Get3DImage()->Value(voxelIndexX, voxelIndexY, voxelIndexZ);

                            if(tempVal >= threshold)
                            {
                                val = tempVal;
                                break;
                            }

                            iter++;
                        }
                    }
                }

                if(val < min)
                {
                    min = val;
                }
                else if(val > max)
                {
                    max = val;
                }

                pdata->SetValue(i, j, val);
            }
        }

        #pragma omp parallel for
        for (int i = 0; i < NE; ++i)
        {
            FEElement_* el = elems[i];
            int ne = el->Nodes();
            for (int j = 0; j < ne; ++j)
            {
                double val = pdata->GetValue(i,j);

                if(normalize)
                {
                    val = (val - min)/(max-min);
                }

                if(useFilter)
                {
                    val = ui->loadCurve.value(val);
                }

                pdata->SetValue(i, j, val);
            }

        }
        break;
    }
    case SAMPLE_CENTROIDS:
    {
        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::min();

        #pragma omp parallel for
        for (int i = 0; i < NE; ++i)
        {
            FEElement_* el = elems[i];
            int ne = el->Nodes();
            
            // Find element centroid
            vec3d pos(0);
            for (int j = 0; j < ne; ++j)
            {
                pos += mesh->LocalToGlobal(mesh->Node(el->m_node[j]).pos());
            }
            pos /= ne;

            double val = imageModel->ValueAtGlobalPos(pos);

            if(val < min)
            {
                min = val;
            }
            else if(val > max)
            {
                max = val;
            }

            pdata->SetValue(i, 0, val);
        }

        #pragma omp parallel for
        for (int i = 0; i < NE; ++i)
        {
            double val = pdata->GetValue(i, 0);

            if(normalize)
            {
                val = (val - min)/(max-min);
            }

            if(useFilter)
            {
                val = ui->loadCurve.value(val);
            }

            pdata->SetValue(i, 0, val);
        }
        break;
    }
    case AVERAGE_ELEMS:
    {
        BOX box = imageModel->GetBoundingBox();
        vec3d origin(box.x0, box.y0, box.z0);

        int imgWidth = imageModel->Get3DImage()->Width();
        int imgHeight = imageModel->Get3DImage()->Height();
        int imgDepth = imageModel->Get3DImage()->Depth();

        double xScale = imgWidth/(box.x1 - box.x0);
        double yScale = imgHeight/(box.y1 - box.y0);
        double zScale = imgDepth/(box.z1 - box.z0);

        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::min();

        Byte* data = imageModel->Get3DImage()->GetBytes();

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

                        vec3f localPixelPos = to_vec3f(mesh->GlobalToLocal(pixelPos));

                        if(ProjectInsideElement(*mesh, *el, localPixelPos, r))
                        {
                            val += data[k*imgWidth*imgHeight + j*imgWidth + i];
                            numPixels++;
                        }
                    }   
                }
            }

            if(numPixels > 0) val /= numPixels;

            if(val < min)
            {
                min = val;
            }
            else if(val > max)
            {
                max = val;
            }
            
            pdata->SetValue(elID, 0, val);
        }

        #pragma omp parallel for
        for (int i = 0; i < NE; ++i)
        {
            double val = pdata->GetValue(i, 0);

            if(normalize)
            {
                val = (val - min)/(max-min);
            }

            if(useFilter)
            {
                val = ui->loadCurve.value(val);
            }

            pdata->SetValue(i, 0, val);
        }
        break;
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

void CImageMapTool::on_methodBox_currentIndexChanged(int index)
{
    if(index == 0)
    {
        if(!ui->thresholdVisible)
        {
            ui->formLayout->insertRow(3, "Surface Node Threshold", ui->thresholdBox = new QDoubleSpinBox);
            ui->thresholdBox->setValue(0);
            ui->thresholdBox->setMaximum(std::numeric_limits<double>::max());
            ui->thresholdBox->setMinimum(std::numeric_limits<double>::min());
            ui->thresholdVisible = true;
        }
    }
    else
    {
        if(ui->thresholdVisible)
        {
            ui->formLayout->removeRow(3);
            ui->thresholdVisible = false;
        }
        
    }
}