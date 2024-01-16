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
#include "ImageMapTool.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include "IconProvider.h"
#include "PlotWidget.h"
#include <ImageLib/ImageModel.h>
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
    QCheckBox* normalize;
    QCheckBox* useFilter;
    CCurveEditWidget* curveEdit;
    QPushButton* create;

    QWidget* projectWidget;
    QCheckBox* projectSurface;
    QDoubleSpinBox* thresholdBox;
    QSpinBox* maxDepth;

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
        
        formLayout->addRow("Project Surface Nodes Inward:", projectSurface = new QCheckBox);
        projectSurface->setChecked(false);
        projectVisible = true;

        projectWidget = new QWidget;
        QFormLayout* projectLayout = new QFormLayout;
        projectLayout->setContentsMargins(15,0,0,0);

        projectLayout->addRow("Surface Threshold:", thresholdBox = new QDoubleSpinBox);
        thresholdBox->setValue(0);
        thresholdBox->setMaximum(std::numeric_limits<double>::max());
        thresholdBox->setMinimum(std::numeric_limits<double>::min());

        projectLayout->addRow("Max Search Depth (voxels):", maxDepth = new QSpinBox);
        maxDepth->setMinimum(1);
        maxDepth->setValue(5);

        projectWidget->setLayout(projectLayout);
        projectWidget->setVisible(false);

        formLayout->addRow(projectWidget);

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
        connect(projectSurface, &QCheckBox::stateChanged, tool, &CImageMapTool::on_projectSurface_stateChanged);
    }

public:
    LoadCurve loadCurve;
    bool projectVisible;
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

    GObject* po;
    FESelection* sel = pdoc->GetCurrentSelection();

    bool wholeObject = true;
    if(dynamic_cast<GObjectSelection*>(sel))
    {
        GObjectSelection* gSel = dynamic_cast<GObjectSelection*>(sel);
        po = gSel->Object(0);
    }
    else if(dynamic_cast<GPartSelection*>(sel))
    {
        GPartSelection* gSel = dynamic_cast<GPartSelection*>(sel);
        GPartSelection::Iterator it(gSel);

        po = dynamic_cast<GObject*>(it->Object());

        for(int i = 0; i < gSel->Count(); i++, ++it)
        {
            if(po != it->Object())
            {
                QMessageBox::critical(GetMainWindow(), "Tool", "All selected parts must be from the same object.");
		        return;
            }
        }

        wholeObject = false;
    }
	
    if (po == 0)
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "You must first select an object or part.");
		return;
	}

	// make sure there is a mesh
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0)
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "The object needs to be meshed before you can apply this tool.");
		return;
	}

    CImageModel* imageModel;
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

    // if an object is selected, we work with the whole object. 
    // otherwise we only do the selected parts.
    std::vector<bool> partIncluded(parts, true);
    if(wholeObject)
    {
        for (int i = 0; i < parts; ++i) partSet->add(i);
    }
    else
    {
        for (int i = 0; i < parts; ++i)
        {
            if(po->Part(i)->IsSelected()) partSet->add(i);
            else partIncluded[i] = false;
        }
    }

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
    mat3d orientation = imageModel->Get3DImage()->GetOrientation();

    switch(ui->methodBox->currentIndex())
    {
    case SAMPLE_NODES:
    {
        bool projectSurface = ui->projectSurface->isChecked();

        double threshold = ui->thresholdBox->value();

        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::min();

        // find all nodal normals
        unordered_map<int, vec3f> normals;
        if(projectSurface)
        {
            for(int i = 0; i < mesh->Faces(); i++)
            {
                FSFace& currentFace = mesh->Face(i);

                // If we're doing the whole object, don't include the inside surfaces.
                if(wholeObject && !currentFace.IsExterior()) continue;

                // Find which part(s) the face belongs to, and check if they were
                // selected by the user
                int elID1 = currentFace.m_elem[0].eid;
                int elID2 = currentFace.m_elem[1].eid;

                int partID1 = -1;
                int partID2 = -1;

                if(elID1 != -1)
                {
                    partID1 = mesh->Element(elID1).m_gid;
                }

                if(elID2 != -1)
                {
                    partID2 = mesh->Element(elID2).m_gid;
                }

                bool inPart1 = false;
                bool inPart2 = false;

                if(partID1 != -1)
                {
                    inPart1 = partIncluded[partID1];
                }

                if(partID2 != -1)
                {
                    inPart2 = partIncluded[partID2];
                }

                // If it belongs to 2 parts, and both parts have been selected, 
                // treat it as an interior surface.
                if(inPart1 && inPart2) continue;

                // The normal will point toward the first of the two parts.
                // We want the normal to point inward.
                int negate = inPart1 ? -1 : 1;

                for(int j = 0; j < currentFace.Nodes(); j++)
                {
                    int nodeID = currentFace.n[j];

                    try
                    {
                        normals.at(nodeID) += currentFace.m_nn[j]*negate;
                    }
                    catch(...)
                    {
                        normals[nodeID] = currentFace.m_nn[j]*negate;
                    }
                }
            }

            for(auto& normal : normals)
            {
                normal.second = normal.second.Normalize();
            }
        }

        int numNodes = mesh->Nodes();

        std::vector<double> vals(numNodes);

        // #pragma omp parallel for
        for(int i = 0; i < numNodes; i++)
        {
            vec3d pos = mesh->LocalToGlobal(mesh->Node(i).pos());
            double val = imageModel->Get3DImage()->ValueAtGlobalPos(pos);

            // see if the node belongs to one of the external faces
            if(projectSurface && normals.count(i) > 0)
            {
                vec3d locPos = orientation.transpose()*(pos - origin);

                int voxelIndexX = locPos.x/spacing.x;
                int voxelIndexY = locPos.y/spacing.y;
                int voxelIndexZ = locPos.z/spacing.z;

                double discreteVal = imageModel->Get3DImage()->Value(voxelIndexX, voxelIndexY, voxelIndexZ);
                if(discreteVal >= threshold)
                {
                    val = discreteVal;
                }
                else
                {
                    vec3d normal = orientation.transpose()*to_vec3d(normals[i]);

                    int xSign = normal.x > 0 ? 1 : -1;
                    int ySign = normal.y > 0 ? 1 : -1;
                    int zSign = normal.z > 0 ? 1 : -1;

                    int iter = 0;
                    int maxDepth = ui->maxDepth->value();
                    vec3d currentPos = locPos;
                    while(iter < maxDepth)
                    {
                        vec3d voxelCenter(voxelIndexX*spacing.x + spacing.x/2,
                        voxelIndexY*spacing.y + spacing.y/2,
                        voxelIndexZ*spacing.z + spacing.z/2);

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
                            voxelIndexZ >= imageModel->Get3DImage()->Depth() ||
                            voxelIndexX < 0 || voxelIndexY < 0 || voxelIndexZ < 0)
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

            #pragma omp critical
            {
                if(val < min)
                {
                    min = val;
                }
                else if(val > max)
                {
                    max = val;
                }
            }

            vals[i] = val;
        }

        for (int i = 0; i < NE; ++i)
        {
            FEElement_* el = elems[i];
            int ne = el->Nodes();
            for (int j = 0; j < ne; ++j)
            {
                pdata->SetValue(i, j, vals[el->m_node[j]]);
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

            double val = imageModel->Get3DImage()->ValueAtGlobalPos(pos);

            #pragma omp critical
            {
                if(val < min)
                {
                    min = val;
                }
                else if(val > max)
                {
                    max = val;
                }
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
        auto img = imageModel->Get3DImage();

        int imgWidth = img->Width();
        int imgHeight = img->Height();
        int imgDepth = img->Depth();

        double xScale = imgWidth/(box.x1 - box.x0);
        double yScale = imgHeight/(box.y1 - box.y0);
        double zScale = imgDepth/(box.z1 - box.z0);

        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::min();

        #pragma omp parallel for
        for (int elID = 0; elID < NE; ++elID)
        {
            FEElement_* el = elems[elID];

            // find bounding box of element
            double minX, maxX, minY, maxY, minZ, maxZ;
            vec3d firstPos = orientation*(mesh->LocalToGlobal(mesh->Node(el->m_node[0]).pos()) - origin);
            minX = maxX = firstPos.x;
            minY = maxY = firstPos.y;
            minZ = maxZ = firstPos.z;

            int ne = el->Nodes();
            for(int nodeID = 1; nodeID < ne; nodeID++)
            {
                vec3d pos = orientation*(mesh->LocalToGlobal(mesh->Node(el->m_node[nodeID]).pos()) - origin);

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
            int minXPixel = minX*xScale - 1;
            int maxXPixel = maxX*xScale + 1;
            int minYPixel = minY*yScale - 1;
            int maxYPixel = maxY*yScale + 1;
            int minZPixel = minZ*zScale - 1;
            int maxZPixel = maxZ*zScale + 1;

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
                vec3d pixelPos(0,0,k/zScale);

                for(int j = minYPixel; j < maxYPixel; j++)
                {
                    pixelPos.y = j/yScale;
                    for(int i = minXPixel; i < maxXPixel; i++)
                    {
                        pixelPos.x = i/xScale;

                        vec3f localPixelPos = to_vec3f(mesh->GlobalToLocal(pixelPos));

                        if(ProjectInsideElement(*mesh, *el, localPixelPos, r))
                        {
                            val += img->Value(i, j, k);
                            numPixels++;
                        }
                    }   
                }
            }

            if(numPixels > 0) val /= numPixels;

            #pragma omp critical
            {
                if(val < min)
                {
                    min = val;
                }
                else if(val > max)
                {
                    max = val;
                }
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
        if(!ui->projectVisible)
        {
            ui->formLayout->insertRow(3, "Project Surface Nodes Inward:", ui->projectSurface = new QCheckBox);
            ui->projectSurface->setChecked(false);
            connect(ui->projectSurface, &QCheckBox::stateChanged, this, &CImageMapTool::on_projectSurface_stateChanged);
            ui->projectVisible = true;
        }
    }
    else
    {
        if(ui->projectVisible)
        {
            ui->formLayout->removeRow(3);
            ui->projectWidget->setHidden(true);
            ui->projectVisible = false;
        }
        
    }
}

void CImageMapTool::on_projectSurface_stateChanged(int state)
{
    ui->projectWidget->setHidden(state == 0);
}