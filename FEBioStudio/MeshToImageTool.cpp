/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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

#include <QFormLayout>
#include <QBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolButton>
#include <QFileDialog>
#include <QFileInfo>
#include "MainWindow.h"
#include "MeshToImageTool.h"
#include "ModelDocument.h"
#include "ImageLib/3DImage.h"
#include "IconProvider.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSFindElement.h>

class UIMeshToImageTool : public QWidget
{
public:
    QLineEdit* m_filename;
    QSpinBox* m_xPixels;
    QSpinBox* m_yPixels;
    QSpinBox* m_zPixels;
    QToolButton* m_browse;
    QPushButton* m_create;

public:
    UIMeshToImageTool(CMeshToImageTool* tool)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        
        QFormLayout* form = new QFormLayout;
        form->setContentsMargins(0, 0, 0, 0);

        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->setContentsMargins(0, 0, 0, 0);
        
        hbox->addWidget(m_filename = new QLineEdit);
        hbox->addWidget(m_browse = new QToolButton);
        m_browse->setIcon(CIconProvider::GetIcon("open"));

        form->addRow("Filename", hbox);

        form->addRow("Pixels in X", m_xPixels = new QSpinBox);
        m_xPixels->setRange(1, 10000);
        m_xPixels->setValue(100);

        form->addRow("Pixels in Y", m_yPixels = new QSpinBox);
        m_yPixels->setRange(1, 10000);
        m_yPixels->setValue(100);
        
        form->addRow("Pixels in Z", m_zPixels = new QSpinBox);
        m_zPixels->setRange(1, 10000);
        m_zPixels->setValue(100);

        layout->addLayout(form);

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->setContentsMargins(0,0,0,0);
        buttonLayout->addStretch();
        buttonLayout->addWidget(m_create = new QPushButton("Create"));
        m_create->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        layout->addLayout(buttonLayout);
        layout->addStretch();

        setLayout(layout);

        connect(m_browse, &QToolButton::clicked, tool, &CMeshToImageTool::OnBrowse);
        connect(m_create, &QPushButton::clicked, tool, &CMeshToImageTool::OnCreate);
    }
};

CMeshToImageTool::CMeshToImageTool(CMainWindow* wnd) 
    : CAbstractTool(wnd, "Mesh to Image"), ui(nullptr)
{
    
}

QWidget* CMeshToImageTool::createUi()
{
    if(ui == nullptr) ui = new UIMeshToImageTool(this);
    return ui;
}

void CMeshToImageTool::OnBrowse()
{
    QString filename = QFileDialog::getSaveFileName(GetMainWindow(), "Select output file", 
        QString(), "NRRD Image (*.nrrd);;TIFF Image (*.tif *.tiff);;Raw Image (*.raw)");
    if (filename.isEmpty() == false)
    {
        ui->m_filename->setText(filename);
    }
}

void CMeshToImageTool::OnCreate()
{
    // make sure the filename is valid
    QString filename = ui->m_filename->text();
    if (filename.isEmpty())
    {
        QMessageBox::critical(GetMainWindow(), "Tool", "You must specify a filename.");
        return;
    }
    QFileInfo info(filename);
    if (info.suffix().isEmpty())
    {
        QMessageBox::critical(GetMainWindow(), "Tool", "You must specify a filename with an extension.");
        return;
    }
    if (info.suffix() != "tif" && info.suffix() != "tiff" && info.suffix() != "nrrd" && info.suffix() != "raw")
    {
        QMessageBox::critical(GetMainWindow(), "Tool", "The file extension must be either .tif, .tiff, .nrrd, or .raw");
        return;
    }

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

    C3DImage img;
    img.Create(ui->m_xPixels->value(), ui->m_yPixels->value(), ui->m_zPixels->value());
    BOX box = pm->GetBoundingBox();
    img.SetBoundingBox(box);

    FSFindElement finder(*pm);
    finder.Init();

    size_t nx = img.Width();
    size_t ny = img.Height();
    size_t nz = img.Depth();

    double dx = box.Width() / nx;
    double dy = box.Height() / ny;
    double dz = box.Depth() / nz;
    vec3f r0 = to_vec3f(box.r0());
    uint8_t* data = img.GetBytes();

    #pragma omp parallel for
    for(int k = 0; k < img.Depth(); ++k)
    {
        for(int j = 0; j < img.Height(); ++j)
        {
            for(int i = 0; i < img.Width(); ++i)
            {
                vec3f r = r0 + vec3f(i*dx + dx/2, j*dy + dy/2, k*dz + dz/2);
                int nelem;
                double r0[3];
                if(finder.FindElement(r, nelem, r0))
                {
                    data[nx*(k*ny + j) + i] = 255;
                }
                else
                {
                    data[nx*(k*ny + j) + i] = 0;
                }
            }
        }
    }

    // save the image
    if(info.suffix() == "raw")
    {
        img.ExportRAW(filename.toStdString());
    }
    else
    {
        img.ExportSITK(filename.toStdString());
    }

    QMessageBox::information(GetMainWindow(), "Tool", "Image successfully created.");
}

