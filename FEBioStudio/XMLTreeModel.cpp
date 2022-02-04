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

#include <QColor>
#include <QPixmap>
#include "XMLTreeModel.h"
#include "IconProvider.h"

XMLTreeItem::XMLTreeItem(int depth)
    : m_parent(nullptr), m_depth(depth), m_expanded(false), m_itemType(ELEMENT),
        m_firstAttribute(0), m_firstElement(0)
{

}

XMLTreeItem::~XMLTreeItem()
{
    for(auto child : m_children)
    {
        delete child;
    }
}

bool XMLTreeItem::setData(int index, const char* val)
{
    switch (index)
    {
    case TAG:
        SetTag(val);
        break;
    case ID:
        SetID(val);
        break;
    case TYPE:
        SetType(val);
        break;
    case NAME:
        SetName(val);
        break;
    case VALUE:
        SetValue(val);
        break;
    default:
        return false;
    }

    return true;
}

void XMLTreeItem::SetTag(const char* val)
{
    m_tag = val;
}

void XMLTreeItem::SetID(const char* val)
{
    m_id = val;
}

void XMLTreeItem::SetName(const char* val)
{
    m_name = val;
}

void XMLTreeItem::SetType(const char* val)
{
    m_type = val;
}

void XMLTreeItem::SetValue(const char* val)
{
    m_value = val;
}

void XMLTreeItem::AddComment(const char* comment)
{
    XMLTreeItem* child = new XMLTreeItem(m_depth + 1);
    child->m_tag = "comment";
    child->m_value = comment;
    child->SetItemType(COMMENT);

    appendChild(child);
    m_firstAttribute++;
    m_firstElement++;
}

void XMLTreeItem::AddAttribtue(const char* tag, const char* val)
{
    XMLTreeItem* attr = new XMLTreeItem(m_depth + 1);
    attr->m_tag = tag;
    attr->m_value = val;
    attr->SetItemType(ATTRIBUTE);

    appendChild(attr);
    m_firstElement++;
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
    return 5;
}

QString XMLTreeItem::data(int column) const
{
    switch (column)
    {
    case TAG:
        return m_tag;
    case ID:
        return m_id;
    case TYPE:
        return m_type;
    case NAME:
        return m_name;
    case VALUE:
        return m_value;
    default:
        return "";
    }    
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

XMLTreeItem* XMLTreeItem::ancestorItem(int depth)
{
    if(m_depth < depth)
        return this;
    
    XMLTreeItem* item = this;

    while(item->Depth() != depth) 
        item = item->parentItem();

    return item;
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

    XMLTreeItem *item = static_cast<XMLTreeItem*>(index.internalPointer());

    if(index.column() == 0 && role == Qt::DecorationRole)
    {
        Shape shape = Shape::Circle;
        QColor color = Qt::blue;

        if(item->childCount() > 0)
        {
            shape = Shape::Square;
            color = Qt::green;
        }

        if(item->GetItemType() == XMLTreeItem::ATTRIBUTE)
        {
            color = Qt::red;
        }

        return CIconProvider::BuildPixMap(color, shape);
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    return item->data(index.column());
}

bool XMLTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) return false;

    const char* strValue = value.toString().toStdString().c_str();

    XMLTreeItem *item = static_cast<XMLTreeItem*>(index.internalPointer());

    bool changed = item->setData(index.column(), strValue);

    emit dataChanged(index, index);

    return changed;
}

Qt::ItemFlags XMLTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

QVariant XMLTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex XMLTreeModel::root() const
{  
    XMLTreeItem* root = rootItem->child(0);

    return createIndex(0, 0, root);
}

void XMLTreeModel::ItemExpanded(const QModelIndex &index)
{
    if(!index.isValid()) return;

    XMLTreeItem *item = static_cast<XMLTreeItem*>(index.internalPointer());

    item->SetExpanded(true);
}

void XMLTreeModel::ItemCollapsed(const QModelIndex &index)
{
    if(!index.isValid()) return;

    XMLTreeItem *item = static_cast<XMLTreeItem*>(index.internalPointer());

    item->SetExpanded(false);
}