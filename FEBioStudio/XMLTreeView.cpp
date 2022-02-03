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

#include <QComboBox>
#include <QCompleter>
#include "XMLTreeView.h"
#include "XMLTreeModel.h"
#include <FECore/fecore_enum.h>
#include <FEBioLink/FEBioClass.h>

XMLItemDelegate::XMLItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent), m_superID(new int(FEINVALID_ID))
{

}

XMLItemDelegate::~XMLItemDelegate()
{
    delete m_superID;
}

QWidget* XMLItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    *m_superID = FEINVALID_ID;

    if(!index.isValid())
    {
        QWidget* pw = QStyledItemDelegate::createEditor(parent, option, index);
        return pw;
    }

    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());

    if(index.column() == TYPE)
    {
        XMLTreeItem* ancestor = item->ancestorItem(1);

        if(ancestor->data(TAG) == "Material")
        {
            *m_superID = FEMATERIAL_ID;
        }
        else if(ancestor->data(TAG) == "Boundary")
        {
            *m_superID = FEBC_ID;
        }
        else if(ancestor->data(TAG) == "Contact")
        {
            *m_superID = FESURFACEINTERFACE_ID;
        }
        else if(ancestor->data(TAG) == "Load")
        {
            *m_superID = FELOAD_ID;
        }
        else if(ancestor->data(TAG) == "Initial")
        {
            *m_superID = FEIC_ID;
        }
        else if(ancestor->data(TAG) == "Discrete")
        {
            *m_superID = FEDISCRETEMATERIAL_ID;
        }

        // if(item->data(TAG) == "material")
        // {
        //     m_superID = FEMATERIAL_ID;
        // }
        // else if(item->data(TAG) == "contact")
        // {
        //     m_superID = FESURFACEINTERFACE_ID;
        // }

        if(*m_superID == FEINVALID_ID)
        {
            QWidget* pw = QStyledItemDelegate::createEditor(parent, option, index);
            return pw;
        }

        QStringList classNames;

        for(auto item : FEBio::FindAllClasses(-1, *m_superID))
        {
            classNames.append(item.sztype);
        }

        classNames.sort(Qt::CaseInsensitive);

        QComboBox* pw = new QComboBox(parent);
        pw->setEditable(true);

        QCompleter* completer = new QCompleter(classNames);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);

        pw->setCompleter(completer);
        
        pw->addItems(classNames);
        connect(pw, &QComboBox::currentIndexChanged, this, &XMLItemDelegate::OnEditorSignal);

        pw->setSizePolicy(QSizePolicy::Expanding, pw->sizePolicy().verticalPolicy());
        return pw;
    }

    QWidget* pw = QStyledItemDelegate::createEditor(parent, option, index);
    return pw;
}

void XMLItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const 
{
    if(!index.isValid()) return;

    // XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());





    QStyledItemDelegate::setEditorData(editor, index);

}

void XMLItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (!index.isValid()) return;



    if(dynamic_cast<QComboBox*>(editor))
    {
        QComboBox* comboBox = dynamic_cast<QComboBox*>(editor);

        QString val = comboBox->currentText();

        XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());

        item->setData(index.column(), val.toStdString().c_str());

        return;
    }

    QStyledItemDelegate::setModelData(editor, model, index);
}

void XMLItemDelegate::OnEditorSignal()
{
	QWidget* sender = dynamic_cast<QWidget*>(QObject::sender());
	emit commitData(sender);
}

/////////////////////////////////////////////////////////////////////////////////////

XMLTreeView::XMLTreeView(QWidget* parent) : QTreeView(parent)
{
    setItemDelegate(new XMLItemDelegate);

    connect(itemDelegate(), &QStyledItemDelegate::commitData, this, &XMLTreeView::modelEdited);
}

void XMLTreeView::setModel(QAbstractItemModel* newModel)
{
    XMLTreeModel* current = dynamic_cast<XMLTreeModel*>(model());

    if(current)
    {
        disconnect(this, &XMLTreeView::expanded, current, &XMLTreeModel::ItemExpanded);
        disconnect(this, &XMLTreeView::collapsed, current, &XMLTreeModel::ItemCollapsed);
    }

    QTreeView::setModel(newModel);

    current = static_cast<XMLTreeModel*>(newModel);

    expandToMatch(current->root());

    connect(this, &XMLTreeView::expanded, current, &XMLTreeModel::ItemExpanded);
    connect(this, &XMLTreeView::collapsed, current, &XMLTreeModel::ItemCollapsed);
}

void XMLTreeView::expandToMatch(const QModelIndex& index)
{
    if(!index.isValid()) return;

    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());

    if(item->Expanded())
    {
        expand(index);
    }
    else
    {
        return;
    }

    if(item->childCount() == 0)
    {
        return;
    }

    XMLTreeModel* xmlModel = static_cast<XMLTreeModel*>(model());

    for(int child = 0; child < item->childCount(); child++)
    {
        expandToMatch(xmlModel->index(child, 0, index));
    }
}