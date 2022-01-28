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

#include <XML/XMLReader.h>
#include "XMLDocument.h"

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
    XMLTreeItem* root = new XMLTreeItem({"Item", "Value"});
    
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
    
    root->appendChild(getChild(tag));
    
    reader.Close();

    m_treeModel = new XMLTreeModel(root);

    return true;
}

XMLTreeItem* CXMLDocument::getChild(XMLTag& tag)
{
    XMLTreeItem* child;
    char szval[256];

    tag.value(szval);
    child = new XMLTreeItem({tag.Name(), szval});

    for(int index = 0; index < tag.m_natt; index++)
    {
        XMLAtt attr =tag.m_att[index];
        XMLTreeItem* attrItem = new XMLTreeItem({attr.name(), attr.cvalue()});

        child->appendChild(attrItem);
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

        child->appendChild(getChild(tag));
    }

    return child;
}