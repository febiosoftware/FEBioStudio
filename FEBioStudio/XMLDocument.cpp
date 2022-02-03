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

#include "XMLDocument.h"
#include <XML/XMLReader.h>
#include <XML/XMLWriter.h>

CXMLDocument::CXMLDocument(CMainWindow* wnd) : CDocument(wnd), m_treeModel(nullptr)
{
	SetIcon(":/icons/febio.png");
}

CXMLDocument::~CXMLDocument()
{
    if(m_treeModel) delete m_treeModel;
}

XMLTreeModel* CXMLDocument::GetModel()
{
    return m_treeModel;
}

bool CXMLDocument::ReadFromFile(const QString& fileName)
{
    XMLReader reader;
    if(!reader.Open(fileName.toStdString().c_str())) return false;

    // Find the root element
	XMLTag tag;
	try
	{
		if (reader.FindTag("febio_spec", tag) == false) return false;
	}
	catch (...)
	{
		return false;
	}
    
    // Dummy root item holds the header names
    XMLTreeItem* root = new XMLTreeItem(-1);
    root->SetTag("Item");
    root->SetID("ID");
    root->SetName("Name");
    root->SetType("Type");
    root->SetValue("Value");

    root->appendChild(getChild(tag, -1));
    root->child(0)->SetExpanded(true);
    
    m_treeModel = new XMLTreeModel(root);

    return true;
}

XMLTreeItem* CXMLDocument::getChild(XMLTag& tag, int depth)
{
    depth++; 

    XMLTreeItem* child = new XMLTreeItem(depth);
    char szval[256];

    tag.value(szval);
    child->SetTag(tag.Name());
    child->SetValue(szval);

    for(int index = 0; index < tag.m_natt; index++)
    {
        XMLAtt attr =tag.m_att[index];

        if(strcmp(attr.name(), "id") == 0)
        {
            child->SetID(attr.cvalue());
        }
        else if (strcmp(attr.name(), "name") == 0)
        {
            child->SetName(attr.cvalue());
        }
        else if (strcmp(attr.name(), "type") == 0)
        {
            child->SetType(attr.cvalue());
        }
        else
        {
            XMLTreeItem* attrItem = new XMLTreeItem(depth + 1);
            attrItem->SetTag(attr.name());
            attrItem->SetValue(attr.cvalue());
            attrItem->SetIsAttribute(true);

            child->appendChild(attrItem);
        }
    }
    
    if(tag.isleaf())
    {   
        return child;
    }

    int children = tag.children();
    for(int index = 0; index < children; index++)
    {
        ++tag;
        while(tag.isend())
        {
            ++tag;
        }

        child->appendChild(getChild(tag, depth));
    }

    return child;
}

bool CXMLDocument::SaveDocument()
{
    XMLWriter writer;
    if(writer.open(GetDocFilePath().c_str()) == false) return false;

    writeChild(static_cast<XMLTreeItem*>(m_treeModel->root().internalPointer()), writer);
}

void CXMLDocument::writeChild(XMLTreeItem* item, XMLWriter& writer)
{
    XMLElement current(item->data(TAG).toStdString().c_str());

    QString data = item->data(ID);
    if(!data.isEmpty())
    {
        current.add_attribute("id", data.toStdString());
    }

    data = item->data(NAME);
    if(!data.isEmpty())
    {
        current.add_attribute("name", data.toStdString());
    }

    data = item->data(TYPE);
    if(!data.isEmpty())
    {
        current.add_attribute("type", data.toStdString());
    }

    int nattr = 0;
    for(int child = 0; child < item->childCount(); child++)
    {
        XMLTreeItem* childItem = item->child(child);
        if(childItem->IsAttribute())
        {
            current.add_attribute(childItem->data(NAME).toStdString().c_str(), childItem->data(VALUE).toStdString());
            nattr++;
        }
    }

    if((item->childCount() - nattr) > 0)
    {
        writer.add_branch(current);

        for(int child = 0; child < item->childCount(); child++)
        {
            XMLTreeItem* childItem = item->child(child);
            if(!childItem->IsAttribute())
            {
                writeChild(childItem, writer);
            }
        }

        writer.close_branch();
    }
    else
    {
        if(item->data(VALUE).isEmpty())
        {
            writer.add_empty(current);
        }
        else
        {
            current.value(item->data(VALUE).toStdString().c_str());
            writer.add_leaf(current);
        }
    }
}