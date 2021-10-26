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

#include <QVBoxLayout>
#include <QTreeWidget>
#include "ImagePanel.h"
#include "ImageDocument.h"

#include <iostream>

enum ITEMTYPES {IMGMODEL = 1001};
enum DATATYPES {INDEX = 1001};

class Ui::CImagePanel
{
public:
    QVBoxLayout* layout;
    QTreeWidget* tree;


    QTreeWidgetItem* imagesItem;
public:
    void setup(::CImagePanel* panel)
    {
        m_panel = panel;

        layout = new QVBoxLayout;

        tree = new QTreeWidget;
        tree->setObjectName("tree");
        tree->setSelectionMode(QAbstractItemView::SingleSelection);
		tree->setContextMenuPolicy(Qt::CustomContextMenu);
        tree->setHeaderHidden(true);
        layout->addWidget(tree);

        m_panel->setLayout(layout);
    }

private:
    ::CImagePanel* m_panel;

};


CImagePanel::CImagePanel(CMainWindow* wnd, QWidget* parent)
    : CCommandPanel(wnd, parent), ui(new Ui::CImagePanel)
{
    ui->setup(this);
    QMetaObject::connectSlotsByName(this);

    Update();
}

void CImagePanel::Update()
{
    CImageDocument* doc = dynamic_cast<CImageDocument*>(GetDocument());

    if(!doc) return;

    ui->tree->clear();

    ui->tree->addTopLevelItem(ui->imagesItem = new QTreeWidgetItem(QStringList() << "Images"));

    for(int img = 0; img < doc->ImageModels(); img++)
    {
        QString name = doc->GetImageModel(img)->GetName().c_str();

        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << name, IMGMODEL);
        item->setData(0, INDEX, img);

        std::cout << img << ": " << item->data(0, INDEX).toInt() << std::endl;

        ui->imagesItem->addChild(item);
    }
}

void CImagePanel::on_tree_itemSelectionChanged()
{
    CImageDocument* doc = dynamic_cast<CImageDocument*>(GetDocument());

    if(!doc) return;

    if(ui->tree->selectedItems().length() > 0)
    {
        QTreeWidgetItem* current = ui->tree->selectedItems()[0];
    
        if(current->type() == IMGMODEL)
        {
            doc->SetActiveModel(current->data(0, INDEX).toInt());

            emit ImageModelChanged();
        }
    }
}