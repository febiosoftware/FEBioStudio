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

#include "DlgAddMaterial.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include "XMLTreeView.h"
#include "XMLTreeModel.h"
#include <QWidget>
#include <QBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <FEMLib/FEMaterial.h>
#include <FEBioLink/FEBioClass.h>
#include <XML/XMLReader.h>
#include <FEBio/FEBioInputModel.h>
#include <FEBio/FEBioFormat4.h>
#include "PublicationWidgetView.h"

#include <iostream>


class Publication
{
public:
    Publication() {};

public:
    QString title;
	QString year;
	QString journal;
	QString volume;
	QString issue;
	QString pages;
	QString DOI;

	QStringList authorGiven;
	QStringList authorFamily;
};

class RepoMaterial
{
public:
    RepoMaterial(QString name, QString description, std::vector<Publication>& pubs, XMLTreeModel* model) :
        m_name(name), m_description(description), m_pubs(pubs), m_model(model)
    {

    }

	RepoMaterial(RepoMaterial&& rm)
	{
		m_name = rm.m_name;
		m_description = rm.m_description;
		m_pubs = rm.m_pubs;
		m_model = rm.m_model;
		rm.m_model = nullptr;
	}

    ~RepoMaterial()
    {
        if(m_model) delete m_model;
    }


public:
    QString m_name;
    QString m_description;
    std::vector<Publication> m_pubs;
    XMLTreeModel* m_model;

};

class Ui::CDlgAddMaterial
{
public:
    QWidget* tab;

    QListWidget* list;
    QLabel* desc;
    ::CPublicationWidgetView* pubs;

    ::XMLTreeView* tree;
public:

    void setupUI(::CDlgAddMaterial* dlg, ::CMainWindow* wnd)
    {
        tab = new QWidget;

        QHBoxLayout* layout = new QHBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        QVBoxLayout* leftLayout = new QVBoxLayout;

        leftLayout->addWidget(list = new QListWidget);
        leftLayout->addWidget(desc = new QLabel);
        leftLayout->addWidget(pubs = new ::CPublicationWidgetView);

        layout->addLayout(leftLayout);
        
        layout->addWidget(tree = new ::XMLTreeView(nullptr));

        tab->setLayout(layout);
    }

    void update()
    {
        list->blockSignals(true);

        list->clear();

        for(auto& mat : m_mats)
        {
            list->addItem(mat.m_name);
        }

        list->blockSignals(false);

        list->setCurrentRow(0);
    }

public:
    vector<RepoMaterial> m_mats;

};

CDlgAddMaterial::CDlgAddMaterial(QString windowName, int superID, int baseClassID, FSModel* fem, bool includeModuleDependencies, bool showStepList, CMainWindow* parent)
    : m_wnd(parent), CDlgAddPhysicsItem(windowName, superID, baseClassID, fem, includeModuleDependencies, showStepList, parent), ui(new Ui::CDlgAddMaterial)
{
    ui->setupUI(this, parent);

    connect(ui->list, &QListWidget::currentRowChanged, this, &CDlgAddMaterial::on_list_currentRowChanged);

    createTabs("FEBio");
    addTab(ui->tab, "Repository");

    ReadXML();

    std::cout << ui->m_mats[0].m_name.toStdString() << std::endl;
    std::cout << ui->m_mats[0].m_description.toStdString() << std::endl;

    auto& mat = ui->m_mats[0];

    // std::cout << mat.m_model->data(mat.m_model->index(0,0), 0).toString().toStdString() << std::endl;
    std::cout << mat.m_model->GetRoot()->child(0)->data(0).toStdString() << std::endl;

    printInfo(ui->m_mats[0].m_model->GetRoot()->child(0), 0);

    ui->update();

}

void CDlgAddMaterial::ReadXML()
{
    XMLReader reader;
    reader.Open("/home/mherron/Desktop/materialRepo.xml");

    XMLTag tag;
    
    bool found = reader.FindTag("Materials", tag);

    try
    {
        ++tag;
        do
        {
            if(tag == "item")
            {
                string name;
                string desc;
                std::vector<Publication> pubs;
                XMLTreeModel* treeModel;

                ++tag;
                do
                {
                    if(tag == "name")
                    {
                        tag.value(name);
                    }
                    else if (tag == "description")
                    {
                        tag.value(desc);
                    }
                    else if(tag == "Publications")
                    {
                        ++tag;
                        do
                        {
                            if(tag == "publication")
                            {
                                Publication pub;

                                ++tag;
                                do
                                {
                                    if(tag == "title")
                                    {
                                        string val;
                                        tag.value(val);

                                        pub.title = val.c_str();
                                    }
                                    else if(tag == "year")
                                    {
                                        string val;
                                        tag.value(val);

                                        pub.year = val.c_str();
                                    }
                                    else if(tag == "journal")
                                    {
                                        string val;
                                        tag.value(val);

                                        pub.journal = val.c_str();
                                    }
                                    else if(tag == "volume")
                                    {
                                        string val;
                                        tag.value(val);

                                        pub.volume = val.c_str();
                                    }
                                    else if(tag == "issue")
                                    {
                                        string val;
                                        tag.value(val);

                                        pub.issue = val.c_str();
                                    }
                                    else if(tag == "pages")
                                    {
                                        string val;
                                        tag.value(val);

                                        pub.pages = val.c_str();
                                    }
                                    else if(tag == "DOI")
                                    {
                                        string val;
                                        tag.value(val);

                                        pub.DOI = val.c_str();
                                    }
                                    else if(tag == "authors")
                                    {
                                        ++tag;
                                        do
                                        {
                                            if(tag == "author")
                                            {
                                                ++tag;
                                                do
                                                {
                                                    if(tag == "given")
                                                    {
                                                        string val;
                                                        tag.value(val);

                                                        pub.authorGiven.append(val.c_str());
                                                    }
                                                    if(tag == "family")
                                                    {
                                                        string val;
                                                        tag.value(val);
                                                        
                                                        pub.authorFamily.push_back(val.c_str());
                                                    }

                                                    ++tag;
                                                }
                                                while (!tag.isend());
                                            }

                                            ++tag;
                                        }
                                        while (!tag.isend());
                                    }

                                    ++tag;
                                } while (!tag.isend());

                                pubs.push_back(pub);
                            }

                            ++tag;
                        } while (!tag.isend());
                        
                    }
                    else if(tag == "material")
                    {
                        // Dummy root item holds the header names
                        XMLTreeItem* root = new XMLTreeItem(-1);
                        root->SetTag("Item");
                        root->SetID("ID");
                        root->SetName("Name");
                        root->SetType("Type");
                        root->SetValue("Value");
                        root->SetComment("Comment");

                        root->appendChild(getChild(tag, -1));
                        root->child(0)->SetExpanded(true);
                        
                        treeModel = new XMLTreeModel(root, nullptr);

                        ++tag;
                    }

                    ++tag;
                } while (!tag.isend());

                ui->m_mats.emplace_back(name.c_str(), desc.c_str(), pubs, treeModel);
                // std::cout << treeModel->GetRoot() << ", " << &treeModel->GetRoot()->m_children << ", " << treeModel->GetRoot()->m_children.size() << std::endl;
                // std::cout << ui->m_mats[0].m_model->GetRoot() << ", " << &ui->m_mats[0].m_model->GetRoot()->m_children <<  ", " << ui->m_mats[0].m_model->GetRoot()->m_children.size()  << std::endl << std::endl;
                // printInfo(treeModel->GetRoot()->child(0), 0);
                // printInfo(ui->m_mats[0].m_model->GetRoot()->child(0), 0);
            }

            ++tag;
        } while (!tag.isend());
        
    }
    catch(XMLReader::EndOfFile& e)
    {
        //ignore
    }
    




}

XMLTreeItem* CDlgAddMaterial::getChild(XMLTag& tag, int depth)
{
    depth++; 

    XMLTreeItem* child = new XMLTreeItem(depth);

    // if(!tag.comment().empty())
    // {
    //     child->SetComment(tag.comment().c_str());
    // }

    char szval[256];
    tag.value(szval);
    child->SetTag(tag.Name());
    child->SetValue(szval);

    for(XMLAtt& att : tag.m_att)
    {
        if(strcmp(att.name(), "id") == 0)
        {
            child->SetID(att.cvalue());
        }
        else if (strcmp(att.name(), "name") == 0)
        {
            child->SetName(att.cvalue());
        }
        else if (strcmp(att.name(), "type") == 0)
        {
            child->SetType(att.cvalue());
        }
        else
        {
            child->AddAttribtue(att.name(), att.cvalue());
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

void CDlgAddMaterial::printInfo(XMLTreeItem* item, int depth)
{
    for(int i = 0; i < depth; i++)
    {
        std::cout << "    ";
    }

    depth++;
    
    for(int col = 0; col < item->columnCount(); col++)
    {
        std::cout << item->data(col).toStdString();
        
        if(col != item->columnCount()) std::cout << ", ";
    }

    std::cout << std::endl;

    for(int child = 0; child < item->childCount(); child++)
    {
        printInfo(item->child(child), depth);
    }
}

void CDlgAddMaterial::on_list_currentRowChanged(int index)
{
    auto& mat = ui->m_mats[index];

    ui->desc->setText(mat.m_description);
    
    ui->pubs->clear();
    for(auto& pub : mat.m_pubs)
    {
        ui->pubs->addPublication(pub.title, pub.year, pub.journal, pub.volume, 
            pub.issue, pub.pages, pub.DOI, pub.authorGiven, pub.authorFamily);
    }

    printInfo(mat.m_model->GetRoot()->child(0), 0);

    ui->tree->setModel(mat.m_model);
}