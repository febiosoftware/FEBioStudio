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

#include "WzdDIC.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QPixmap>
#include <QPainter>
#include <QPalette>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "MainWindow.h"
#include "ModelDocument.h"
#include <ImageLib/Image.h>
#include <ImageLib/3DImage.h>
#include <ImageLib/ImageSITK.h>
#include <PostLib/ImageModel.h>
#include "DlgDIC.h"

#include <sitkImage.h>
#include <sitkMaskImageFilter.h>

#include <iostream>

namespace sitk = itk::simple;

class CDICImageView : public QGraphicsView
{
public:
    CDICImageView()
    {
        setScene(&scene);
    }

    void SetImage(C3DImage* image)
    {
        if(image)
        {
            CImage imgSlice;
            image->GetSliceZ(imgSlice, 0);

            m_image = QPixmap::fromImage(QImage(imgSlice.GetBytes(), imgSlice.Width(), imgSlice.Height(), imgSlice.Width(), QImage::Format::Format_Grayscale8)).transformed(QTransform().scale(1,-1));

            scene.clear();
            scene.addPixmap(m_image);

            fitInView(scene.sceneRect(), Qt::KeepAspectRatio);
        }
    }

    void UpdatetRects(int xSize, int xSpacing, int ySize, int ySpacing)
    {
        scene.clear();

        QGraphicsPixmapItem* item = scene.addPixmap(m_image);
        fitInView(scene.sceneRect(), Qt::KeepAspectRatio);

        Qt::GlobalColor color;
        color = Qt::yellow;

        QPen pen;
        pen.setColor(color);

        int currentX = 0;
        int currentY = 0;
        while(true)
        {

            scene.addRect(QRect((xSize + xSpacing)*currentX, (ySize + ySpacing)*currentY, xSize, ySize), pen);

            currentX++;
            if(m_image.width() < (xSize + xSpacing)*(currentX) + xSize)
            {
                currentX = 0;
                currentY++;
            }

            if(m_image.height() < (ySize + ySpacing)*(currentY) + ySize)
            {
                break;
            }

        }

        update();


    }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QWidget::resizeEvent(event);

        fitInView(scene.sceneRect(), Qt::KeepAspectRatio);
    }

private:
    QGraphicsScene scene;
    QPixmap m_image;

};

//=================================================================================

enum pageIDS {IMAGE_PAGE = 0, MASK_PAGE, SUBDIV_PAGE};

class Ui::CWzdDIC
{
public:
    QWizardPage* imagePage;
    QComboBox* referenceSelection;
    CDICImageView* reference;
    QComboBox* deformedSelection;
    CDICImageView* deformed;

    QWizardPage* maskPage;
    QPushButton* clearMask;
    QPushButton* loadMask;
    QPushButton* drawMask;
    CDICImageView* maskImage;
    CDICImageView* maskedImage;

    QWizardPage* subdivisionPage;
    QSpinBox* xSize;
    QSpinBox* xSpacing;
    QSpinBox* ySize;
    QSpinBox* ySpacing;
    QPushButton* updateSubs;
    CDICImageView* subdivisionImage;
    
public:
    CWzdDIC() : m_mask(nullptr), m_masked(nullptr), m_referenceImage(nullptr), m_deformedImage(nullptr)
    {

    }

    ~CWzdDIC()
    {
        if(m_mask) delete m_mask;
        if(m_masked) delete m_masked;
    }

    void setupUi(::CWzdDIC* parent, ::CMainWindow* wnd)
    {
        m_parent = parent;
        m_wnd = wnd;

        // Image Selection page
        imagePage = new QWizardPage;

        QHBoxLayout* imagePageLayout = new QHBoxLayout;

        QVBoxLayout* leftImageLayout = new QVBoxLayout;

        leftImageLayout->addWidget(new QLabel("Reference Image:"));
        leftImageLayout->addWidget(referenceSelection = new QComboBox);
        referenceSelection->addItem("<select image>");
        leftImageLayout->addWidget(reference = new CDICImageView);

        imagePageLayout->addLayout(leftImageLayout);
        
        QVBoxLayout* rightImageLayout = new QVBoxLayout;

        rightImageLayout->addWidget(new QLabel("Deformed Image:"));
        rightImageLayout->addWidget(deformedSelection = new QComboBox);
        deformedSelection->addItem("<select image>");
        rightImageLayout->addWidget(deformed = new CDICImageView);

        auto doc = wnd->GetModelDocument();
        if(doc)
        {
            for(int i = 0; i < doc->ImageModels(); i++)   
            {
                referenceSelection->addItem(doc->GetImageModel(i)->GetName().c_str());
                deformedSelection->addItem(doc->GetImageModel(i)->GetName().c_str());
            }
        }

        imagePageLayout->addLayout(rightImageLayout);

        imagePage->setLayout(imagePageLayout);

        parent->addPage(imagePage);

        // Mask Page
        maskPage = new QWizardPage;

        QVBoxLayout* maskPageLayout = new QVBoxLayout;

        QHBoxLayout* topMaskLayout = new QHBoxLayout;
        topMaskLayout->addWidget(clearMask = new QPushButton("Clear Image Mask"));
        topMaskLayout->addWidget(loadMask = new QPushButton("Load Image Mask"));
        topMaskLayout->addWidget(drawMask = new QPushButton("Draw Image Mask"));
        maskPageLayout->addLayout(topMaskLayout);

        QHBoxLayout* bottomMaskLayout = new QHBoxLayout;

        QVBoxLayout* leftMaskLayout = new QVBoxLayout;
        leftMaskLayout->addWidget(new QLabel("Reference Image Mask:"));
        leftMaskLayout->addWidget(maskImage = new CDICImageView);
        bottomMaskLayout->addLayout(leftMaskLayout);

        QVBoxLayout* rightMaskLayout = new QVBoxLayout;
        rightMaskLayout->addWidget(new QLabel("Masked Reference Image:"));
        rightMaskLayout->addWidget(maskedImage = new CDICImageView);
        bottomMaskLayout->addLayout(rightMaskLayout);

        maskPageLayout->addLayout(bottomMaskLayout);

        maskPage->setLayout(maskPageLayout);

        parent->addPage(maskPage);

        // Subdivision Page

        subdivisionPage = new QWizardPage;

        QHBoxLayout* subdivisionLayout = new QHBoxLayout;

        QVBoxLayout* leftSDLayout = new QVBoxLayout;
        leftSDLayout->addWidget(new QLabel("X Subdivision Size (pixels):"));
        leftSDLayout->addWidget(xSize = new QSpinBox);
        xSize->setRange(1,99999);
        leftSDLayout->addWidget(new QLabel("X Spacing (pixels):"));
        leftSDLayout->addWidget(xSpacing = new QSpinBox);
        xSpacing->setRange(-99999,99999);
        leftSDLayout->addWidget(new QLabel("Y Subdivision Size (pixels):"));
        leftSDLayout->addWidget(ySize = new QSpinBox);
        ySize->setRange(1,99999);
        leftSDLayout->addWidget(new QLabel("Y Spacing (pixels):"));
        leftSDLayout->addWidget(ySpacing = new QSpinBox);
        ySpacing->setRange(-99999,99999);

        leftSDLayout->addWidget(updateSubs = new QPushButton("Update Subdivisions"));

        subdivisionLayout->addLayout(leftSDLayout);

        subdivisionLayout->addWidget(subdivisionImage = new CDICImageView);


        subdivisionPage->setLayout(subdivisionLayout);

        parent->addPage(subdivisionPage);

    }

    void setMask(C3DImage* mask)
    {
        if(m_mask) delete m_mask;
        if(m_masked) delete m_masked;

        m_mask = mask;

        sitk::Image ref = dynamic_cast<CImageSITK*>(m_referenceImage)->GetSItkImage();
        sitk::Image maskSITK = dynamic_cast<CImageSITK*>(m_mask)->GetSItkImage();

        m_masked = new CImageSITK();

        dynamic_cast<CImageSITK*>(m_masked)->SetItkImage(sitk::Mask(ref, maskSITK));

        maskImage->SetImage(m_mask);
        maskedImage->SetImage(m_masked);
    }

public:
    ::CWzdDIC* m_parent;
    ::CMainWindow* m_wnd;

    C3DImage* m_referenceImage;
    C3DImage* m_deformedImage;

    C3DImage* m_mask;
    C3DImage* m_masked;


};


CWzdDIC::CWzdDIC(CMainWindow* wnd) : ui(new Ui::CWzdDIC)
{
    resize(600, 500);

    ui->setupUi(this, wnd);

    connect(ui->referenceSelection, &QComboBox::currentIndexChanged, this, &CWzdDIC::on_referenceSelection_changed);
    connect(ui->deformedSelection, &QComboBox::currentIndexChanged, this, &CWzdDIC::on_deformedSelection_changed);
    connect(ui->updateSubs, &QPushButton::clicked, this, &CWzdDIC::on_divisions_changed);
    connect(ui->clearMask, &QPushButton::clicked, this, &CWzdDIC::on_clearMask_clicked);
    connect(ui->loadMask, &QPushButton::clicked, this, &CWzdDIC::on_loadMask_clicked);
    connect(ui->drawMask, &QPushButton::clicked, this, &CWzdDIC::on_drawMask_clicked);
}

CWzdDIC::~CWzdDIC()
{
    delete ui;
}

bool CWzdDIC::validateCurrentPage()
{
    int id = currentId();

    if(id == IMAGE_PAGE)
    {
        auto doc = ui->m_wnd->GetModelDocument();

        int index = ui->referenceSelection->currentIndex();

        if(index > 0 && doc->ImageModels() > 0)
        {
            ui->m_referenceImage = doc->GetImageModel(index-1)->Get3DImage();
        }

        index = ui->deformedSelection->currentIndex();

        if(index > 0 && doc->ImageModels() > 0)
        {
            ui->m_deformedImage = doc->GetImageModel(index-1)->Get3DImage();
        }
    }

    return true;
}

void CWzdDIC::initializePage(int id)
{
    if(id == MASK_PAGE)
    {
        if(!ui->m_mask)
        {
            on_clearMask_clicked();
        }
    }
}

void CWzdDIC::on_referenceSelection_changed(int i)
{
    auto doc = ui->m_wnd->GetModelDocument();
    if(!doc) return;

    if(i > 0 && doc->ImageModels() > 0)
    {
        ui->reference->SetImage(doc->GetImageModel(i-1)->Get3DImage());

        ui->subdivisionImage->SetImage(doc->GetImageModel(i-1)->Get3DImage());

    }
    else
    {
        ui->reference->SetImage(nullptr);
    }
}

void CWzdDIC::on_deformedSelection_changed(int i)
{
    auto doc = ui->m_wnd->GetModelDocument();
    if(!doc) return;

    if(i > 0 && doc->ImageModels() > 0)
    {
        ui->deformed->SetImage(doc->GetImageModel(i-1)->Get3DImage());
    }
    else
    {
        ui->deformed->SetImage(nullptr);
    }
}

void CWzdDIC::on_divisions_changed()
{
    ui->subdivisionImage->UpdatetRects(ui->xSize->value(), ui->xSpacing->value(), ui->ySize->value(), ui->ySpacing->value());
}

void CWzdDIC::on_clearMask_clicked()
{
    auto ref = ui->m_referenceImage;

    auto mask = new CImageSITK(ref->Width(), ref->Height(), ref->Depth());
    Byte* data = mask->GetBytes();
    for(int i = 0; i < ref->Width()*ref->Height()*ref->Depth(); i++)
    {
        data[i] = 255;
    }

    mask->GetSItkImage().SetOrigin(dynamic_cast<CImageSITK*>(ref)->GetSItkImage().GetOrigin());
    mask->GetSItkImage().SetSpacing(dynamic_cast<CImageSITK*>(ref)->GetSItkImage().GetSpacing());

    ui->setMask(mask);
}

void CWzdDIC::on_loadMask_clicked()
{

}

void CWzdDIC::on_drawMask_clicked()
{
    auto doc = ui->m_wnd->GetModelDocument();
    if(!doc) return;

    if(ui->referenceSelection->currentIndex() > doc->ImageModels()) return;

    CDlgDIC dlg(doc->GetImageModel(ui->referenceSelection->currentIndex() - 1));

    if(dlg.exec())
    {
        std::cout << "size: " << dlg.GetPixmap()->width() << ", " << dlg.GetPixmap()->height() <<std::endl;


        QImage dlgMask = dlg.GetPixmap()->toImage().transformed(QTransform().scale(1,-1));

        auto ref = ui->m_referenceImage;
        auto mask = new CImageSITK(ref->Width(), ref->Height(), ref->Depth());
        Byte* data = mask->GetBytes();
        for(int j = 0; j < ref->Height(); j++)
        {
            for(int i = 0; i < ref->Width(); i++)
            {
                data[j*ref->Width() + i] = (dlgMask.pixelColor(i,j) == Qt::white ? 255 : 0);
            }
        }

        mask->GetSItkImage().SetOrigin(dynamic_cast<CImageSITK*>(ref)->GetSItkImage().GetOrigin());
        mask->GetSItkImage().SetSpacing(dynamic_cast<CImageSITK*>(ref)->GetSItkImage().GetSpacing());

        ui->setMask(mask);

    }
}