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

#include "XMLCommands.h"
#include "XMLTreeView.h"
#include "XMLTreeModel.h"


//////////////////////////////////////////////////////////////////////
// CXMLCommand
//////////////////////////////////////////////////////////////////////

CXMLCommand::CXMLCommand(XMLTreeModel* model, std::string name) : CCommand(name), m_model(model)
{

}

//////////////////////////////////////////////////////////////////////
// CCmdEditCell
//////////////////////////////////////////////////////////////////////

CCmdEditCell::CCmdEditCell(QPersistentModelIndex index,  QString& newText, QString& oldText, XMLTreeModel* model) 
    : CXMLCommand(model, "Edit Cell"), m_index(index), m_newText(newText), m_oldText(oldText)
{

}

void CCmdEditCell::Execute()
{
    m_model->setData(m_index, m_newText, Qt::DisplayRole);
}

void CCmdEditCell::UnExecute()
{
    m_model->setData(m_index, m_oldText, Qt::DisplayRole);
}

//////////////////////////////////////////////////////////////////////
// CCmdRemoveRow
//////////////////////////////////////////////////////////////////////

CCmdRemoveRow::CCmdRemoveRow(QPersistentModelIndex parent, int row, XMLTreeItem* item, XMLTreeModel* model)
    : CXMLCommand(model, "Remove Row"), m_parent(parent), m_row(row), m_item(item), m_ownsItem(false)
{

}

CCmdRemoveRow::~CCmdRemoveRow()
{
    if(m_ownsItem)
    {
        delete m_item;
    }
}

void CCmdRemoveRow::Execute()
{
    m_model->removeRow(m_row, m_parent);

    m_ownsItem = true;
}

void CCmdRemoveRow::UnExecute()
{
    m_model->insertRow(m_parent, m_row, m_item);

    m_ownsItem = false;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddAttribute
//////////////////////////////////////////////////////////////////////

CCmdAddAttribute::CCmdAddAttribute(QPersistentModelIndex parent, XMLTreeModel* model)
    : CXMLCommand(model, "Add Attribute"), m_parent(parent)
{
    
}

void CCmdAddAttribute::Execute()
{
    m_model->addRow(m_parent, XMLTreeItem::ATTRIBUTE);
}

void CCmdAddAttribute::UnExecute()
{
    XMLTreeItem* parentItem = static_cast<XMLTreeItem*>(m_parent.internalPointer());

    m_model->removeRow(parentItem->FirstElement() - 1, m_parent);
}

//////////////////////////////////////////////////////////////////////
// CCmdAddElement
//////////////////////////////////////////////////////////////////////

CCmdAddElement::CCmdAddElement(QPersistentModelIndex parent, XMLTreeModel* model)
    : CXMLCommand(model, "Add Attribute"), m_parent(parent)
{
    
}

void CCmdAddElement::Execute()
{
    m_model->addRow(m_parent, XMLTreeItem::ELEMENT);
}

void CCmdAddElement::UnExecute()
{
    XMLTreeItem* parentItem = static_cast<XMLTreeItem*>(m_parent.internalPointer());

    m_model->removeRow(parentItem->childCount() - 1, m_parent);
}