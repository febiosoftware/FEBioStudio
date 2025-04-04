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

#include <QBoxLayout>
#include <QAction>
#include <QToolButton>
#include <QPushButton>
#include "ToolBox.h"
#include "PropertyListView.h"
#include "ObjectProps.h"
#include "FEBioStudio.h"
#include "IconProvider.h"
#include "ImageFilterWidget.h"
#include "ImageThread.h"
#include <ImageLib/ImageModel.h>
#include <ImageLib/ImageFilter.h>
#include "DlgImageFilter.h"
#include <FSCore/ClassDescriptor.h>
#include "DlgStartThread.h"

CImageFilterWidget::CImageFilterWidget(CMainWindow* wnd)
    : m_imgModel(nullptr), m_wnd(wnd)
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    CToolBox* filterBox = new CToolBox;

    QHBoxLayout* listLayout = new QHBoxLayout;
    listLayout->setContentsMargins(0,0,0,0);

    QWidget* listWidget = new QWidget;
    
    m_list = new FilterListWidget;
    m_list->setObjectName("list");
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    m_list->setDragDropMode(QAbstractItemView::InternalMove);
    listLayout->addWidget(m_list);

    QVBoxLayout* buttonLayout = new QVBoxLayout;
    buttonLayout->setContentsMargins(0,0,0,0);

    QAction* addFilter = new QAction;
    addFilter->setIcon(CIconProvider::GetIcon("selectAdd"));
    QToolButton* addFilterBtn = new QToolButton;
    addFilterBtn->setObjectName("addFilterBtn");
    addFilterBtn->setDefaultAction(addFilter);
    buttonLayout->addWidget(addFilterBtn);

    QAction* delFilter = new QAction;
    delFilter->setIcon(CIconProvider::GetIcon("selectDel"));
    QToolButton* delFilterBtn = new QToolButton;
    delFilterBtn->setObjectName("delFilterBtn");
    delFilterBtn->setDefaultAction(delFilter);
    buttonLayout->addWidget(delFilterBtn);
    buttonLayout->addStretch();

    listLayout->addLayout(buttonLayout);
    listWidget->setLayout(listLayout);

    filterBox->addTool("Filters", listWidget);

    m_filterProps = new ::CPropertyListView;
    filterBox->addTool("Filter Properties", m_filterProps);

    layout->addWidget(filterBox);

    m_applyFilters = new QPushButton("Apply Filter Changes");
    m_applyFilters->setIcon(CIconProvider::GetIcon("emblems/caution"));
    m_applyFilters->setObjectName("applyFilters");
    m_applyFilters->hide();

    layout->addWidget(m_applyFilters);

    setLayout(layout);

    QMetaObject::connectSlotsByName(this);
    connect(m_filterProps, &CPropertyListView::dataChanged, this, &CImageFilterWidget::on_filterProps_changed);
}

CImageFilterWidget::~CImageFilterWidget()
{
    Clear();
}

void CImageFilterWidget::SetImageModel(CImageModel* img)
{
    m_imgModel = img;

    Update();
}

void CImageFilterWidget::Clear()
{
    m_list->clear();
    
    for(auto prop : m_props)
    {
        delete prop;
    }
    m_props.clear();
    m_filterProps->Update(nullptr);
}

void CImageFilterWidget::UpdateApplyButton()
{
    if(m_imgModel)
    {
        if(m_imgModel->AreFiltersUnapplied())
        {
            m_applyFilters->show();
            emit filterStatusChanged(true);
            return;
        }
    }

    m_applyFilters->hide();
    emit filterStatusChanged(false);
}

void CImageFilterWidget::Update()
{
    Clear();
    
    if(m_imgModel)
    {
        for(int filter = 0; filter < m_imgModel->ImageFilters(); filter++)
        {
            CImageFilter* current = m_imgModel->GetImageFilter(filter);
            QListWidgetItem* item = new QListWidgetItem(current->GetName().c_str());
            item->setData(1001, filter);
            m_list->addItem(item);

            m_props.push_back(new CObjectProps(current));
        }
    }

    UpdateApplyButton();
}

void CImageFilterWidget::on_list_itemSelectionChanged()
{
    if(m_list->selectedItems().length() == 0) return;

    if(m_imgModel)
    {
        int filterIndex = m_list->selectedItems()[0]->data(1001).toInt();

        if(filterIndex < m_imgModel->ImageFilters())
        {
            m_filterProps->Update(m_props[filterIndex]);
        }
    }
}

void CImageFilterWidget::on_list_internalMove(int fromIndex, int toIndex)
{
    m_imgModel->MoveFilter(fromIndex, toIndex);

    Update();

    m_list->setCurrentRow(toIndex);
}

void CImageFilterWidget::on_addFilterBtn_clicked()
{
    if(m_imgModel)
    {
        CDlgImageFilter dlg;

        if(dlg.exec())
        {
            m_imgModel->AddImageFilter(static_cast<CImageFilter*>(dlg.GetClassDescriptor()->Create()));
        }
    }

    Update();
}

void CImageFilterWidget::on_delFilterBtn_clicked()
{
    if(m_list->selectedItems().length() == 0) return;

    if(m_imgModel)
    {
        int filterIndex = m_list->selectedItems()[0]->data(1001).toInt();

        if(filterIndex < m_imgModel->ImageFilters())
        {
            m_imgModel->RemoveFilter(m_imgModel->GetImageFilter(filterIndex));
        }
    }

    Update();

}

void CImageFilterWidget::on_applyFilters_clicked()
{
    if(m_imgModel)
    {
        m_imgModel->ClearFilters();

        CDlgStartThread dlg(m_wnd, new CImageFilterThread(m_imgModel));

        if(!dlg.exec())
        {
            m_imgModel->ClearFilters();
        }

        m_imgModel->UpdateRenderers();
    }

    UpdateApplyButton();
}

void CImageFilterWidget::on_filterProps_changed()
{
    if(m_imgModel)
    {
        m_imgModel->FilterPropsChanged();
    }

    UpdateApplyButton();
}