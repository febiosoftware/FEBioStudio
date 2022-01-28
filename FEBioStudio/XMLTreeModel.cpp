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

#include "XMLTreeModel.h"

XMLTreeItem::XMLTreeItem(const QStringList &data)
    : m_itemData(data), m_parent(nullptr)
{

}

XMLTreeItem::~XMLTreeItem()
{
    for(auto child : m_children)
    {
        delete child;
    }
}

void XMLTreeItem::appendChild(XMLTreeItem *child)
{
    m_children.push_back(child);
    child->setParent(this);
}

XMLTreeItem* XMLTreeItem::child(int row)
{
    if(row < 0 || row >= m_children.size())
    {
        return nullptr;
    }

    return m_children[row];
}

int XMLTreeItem::childCount() const
{
    return m_children.size();
}

int XMLTreeItem::columnCount() const
{
    return m_itemData.size();
}

QString XMLTreeItem::data(int column) const
{
    if(column < 0 || column >= m_itemData.size())
    {
        return "";
    }

    return m_itemData[column];
    
}

int XMLTreeItem::row() const
{
    if(m_parent)
    {
        int index = 0;
        for(XMLTreeItem* sibling : m_parent->m_children)
        {
            index++;
            if(sibling == this)
            {
                return index;
            }
        }
    }

    return 0;
}

XMLTreeItem* XMLTreeItem::parentItem()
{
    return m_parent;
}

void XMLTreeItem::setParent(XMLTreeItem* parent)
{
    m_parent = parent;
}


////////////////////////////////////////////////

XMLTreeModel::XMLTreeModel(XMLTreeItem* root, QObject *parent)
    : QAbstractItemModel(parent), rootItem(root)
{
   
}

XMLTreeModel::~XMLTreeModel()
{
    delete rootItem;
}

QModelIndex XMLTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    XMLTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<XMLTreeItem*>(parent.internalPointer());

    XMLTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex XMLTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    XMLTreeItem *childItem = static_cast<XMLTreeItem*>(index.internalPointer());
    XMLTreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int XMLTreeModel::rowCount(const QModelIndex &parent) const
{
    XMLTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<XMLTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int XMLTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<XMLTreeItem*>(parent.internalPointer())->columnCount();
    return rootItem->columnCount();
}

QVariant XMLTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    XMLTreeItem *item = static_cast<XMLTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags XMLTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant XMLTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}