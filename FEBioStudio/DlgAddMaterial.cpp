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
#include "FEClassPropsView.h"
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
    RepoMaterial(QString name, QString description, std::vector<Publication>& pubs, FSMaterial* mat) :
        m_name(name), m_description(description), m_pubs(pubs), m_mat(mat)
    {

    }

    ~RepoMaterial()
    {
        if(m_mat) delete m_mat;
    }

public:
    QString m_name;
    QString m_description;
    std::vector<Publication> m_pubs;
    FSMaterial* m_mat;

};

class Ui::CDlgAddMaterial
{
public:
    QWidget* tab;

    QListWidget* list;
    QLabel* desc;
    ::CPublicationWidgetView* pubs;

    FEClassEdit* edit;
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
        
        layout->addWidget(edit = new FEClassEdit(wnd));

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

    ui->update();

}

void CDlgAddMaterial::ReadXML()
{
    XMLReader reader;
    reader.Open("/home/mherron/Desktop/materialRepo.xml");

    XMLTag tag;
    
    bool found = reader.FindTag("Materials", tag);

    auto fem = m_wnd->GetModelDocument()->GetFSModel();
    FEBioInputModel model(*fem);
    FEBioFormat4 format(nullptr, model); 

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
                FSMaterial* mat = nullptr;

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
                        
                        // do
                        // {
                        //     ++tag;

                            // get the material type
                            XMLAtt& mtype = tag.Attribute("type");
                            const char* sztype = mtype.cvalue();

                            // get the material name
                            XMLAtt* pan = tag.AttributePtr("name");
                            const char* szname = pan->cvalue();

                            // allocate a new material
                            mat = FEBio::CreateMaterial(sztype, fem);

                            // parse material
		                    format.ParseModelComponent(mat, tag);

                        //     ++tag;
                        // } while (!tag.isend());
                    }

                    ++tag;
                } while (!tag.isend());


                ui->m_mats.emplace_back(name.c_str(), desc.c_str(), pubs, mat);
                
            }

            ++tag;
        } while (!tag.isend());
        
    }
    catch(XMLReader::EndOfFile& e)
    {
        //ignore
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

    // ui->edit->SetFEClass(mat.m_mat, m_wnd->GetModelDocument()->GetFSModel());
}