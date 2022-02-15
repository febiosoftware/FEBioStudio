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

#pragma once

#include <QAbstractItemModel>
#include <QString>
#include <QStringList>
#include <vector>

using std::vector;
using std::string;

class CXMLDocument;

enum columnOrder
{
    TAG = 0, ID, TYPE, NAME, VALUE, COMMENT, NUM_COLUMNS
};

class XMLTreeItem
{
public:
    enum ItemType
    {
        ELEMENT, ATTRIBUTE
    };

public:
    XMLTreeItem(int depth);
    ~XMLTreeItem();

    bool setData(int index, const char* val);

    void SetTag(const char* val);
    void SetID(const char* val);
    void SetName(const char* val);
    void SetType(const char* val);
    void SetValue(const char* val);
    void SetComment(const char* val);

    void AddAttribtue(const char* tag, const char* val);

    int FirstElement() { return m_firstElement; }

    int Depth() { return m_depth; }

    bool Expanded() { return m_expanded; }
    void SetExpanded(bool expanded) { m_expanded = expanded; }

    void SetItemType(ItemType itemType) { m_itemType = itemType; }
    ItemType GetItemType() { return m_itemType; }
    
    void appendChild(XMLTreeItem *child);
    void insertChild(int index, XMLTreeItem *child);
    bool removeChild(int index);

    XMLTreeItem* child(int row);
    XMLTreeItem* findChlid(int column, QString value);
    vector<XMLTreeItem*> findAllChlidren(int column, QString value);
    int childCount() const;
    int columnCount() const;
    QString data(int column) const;
    int row() const;
    XMLTreeItem *parentItem();
    XMLTreeItem *ancestorItem(int depth);

private:
    void setParent(XMLTreeItem* parent);

private:
    ItemType m_itemType;
    vector<XMLTreeItem*> m_children;
    QString m_tag;
    QString m_id;
    QString m_name;
    QString m_type;
    QString m_value;
    QString m_comment;
    XMLTreeItem *m_parent;
    int m_depth;

    // int m_firstAttribute;
    int m_firstElement;

    bool m_expanded;
};

class XMLTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    XMLTreeModel(XMLTreeItem* root, CXMLDocument* doc, QObject *parent = nullptr);
    ~XMLTreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole) override;

    XMLTreeItem* GetRoot();

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool addRow(const QModelIndex& parent, XMLTreeItem::ItemType itemType);
    bool insertRow(const QModelIndex& parent, int row, XMLTreeItem* item);

    QModelIndex root() const;

    CXMLDocument* GetDocument();
public slots:
    void ItemExpanded(const QModelIndex &index);
    void ItemCollapsed(const QModelIndex &index);

private:
    XMLTreeItem *rootItem;

    CXMLDocument* m_doc;
};