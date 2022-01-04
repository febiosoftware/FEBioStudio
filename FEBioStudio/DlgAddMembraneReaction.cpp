/*This file is part of the FEBio Studio source code and is licensed under the MIT license
 listed below.
 
 See Copyright-FEBio-Studio.txt for details.
 
 Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
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

#include "stdafx.h"
#include "DlgAddMembraneReaction.h"
#include "DlgAddChemicalReaction.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QPushButton>
#include <QBoxLayout>
#include <sstream>
#include <QMessageBox>
#include <FEMLib/FEMultiMaterial.h>

using std::stringstream;

//=================================================================================================

CMembraneReactionList::CMembraneReactionList(QWidget* parent) : QWidget(parent)
{
    m_list = new QListWidget;
    
    QPushButton* add = new QPushButton("Add");
    QPushButton* del = new QPushButton("Remove");
    
    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(add);
    l->addWidget(del);
    l->addStretch();
    l->setContentsMargins(0,0,0,0);
    
    QHBoxLayout* h = new QHBoxLayout;
    h->addWidget(m_list);
    h->addLayout(l);
    h->setContentsMargins(0,0,0,0);
    
    setLayout(h);
    
    QObject::connect(m_list, SIGNAL(currentRowChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    QObject::connect(add, SIGNAL(clicked(bool)), this, SLOT(OnAdd()));
    QObject::connect(del, SIGNAL(clicked(bool)), this, SLOT(OnRemove()));
}

void CMembraneReactionList::Clear()
{
    m_list->clear();
}

void CMembraneReactionList::AddItem(const QString& name, const QVariant& data, bool sel)
{
    QListWidgetItem* item = new QListWidgetItem(m_list);
    item->setText(name);
    item->setData(Qt::UserRole, data);
    
    if (sel) m_list->setCurrentItem(item);
}

void CMembraneReactionList::SetCurrentText(const QString& t)
{
    QListWidgetItem* current = m_list->currentItem();
    if (current) current->setText(t);
}

void CMembraneReactionList::onCurrentIndexChanged(int n)
{
    emit currentIndexChanged(n);
}

void CMembraneReactionList::OnAdd()
{
    emit onAdd();
}

void CMembraneReactionList::OnRemove()
{
    emit onRemove();
}

//=================================================================================================
class Ui::CDlgAddMembraneReaction
{
public:
    QComboBox*              mat;
    CMembraneReactionList*  reactions;
    QWidget*                dummy;
    QComboBox*              type;
    QComboBox*              fwdRate;
    QComboBox*              revRate;
    QLineEdit*              name;
    QCheckBox*              ovrVB;
    
    QSelectBox*           selectReactants;
    QSelectBox*           selectInternalReactants;
    QSelectBox*           selectExternalReactants;
    QSelectBox*           selectProducts;
    QSelectBox*           selectInternalProducts;
    QSelectBox*           selectExternalProducts;

    QDialogButtonBox* bb;
    
public:
    void setupUi(QWidget* parent)
    {
        QFormLayout* form = new QFormLayout;
        form->addRow("Material:", mat = new QComboBox);
        form->addRow("Membrane Reactions:", reactions = new CMembraneReactionList);
        
        QFormLayout* form2 = new QFormLayout;
        form2->addRow("Name:", name = new QLineEdit);
        form2->addRow("Type:", type = new QComboBox);
        form2->addRow("Forward Rate:", fwdRate = new QComboBox);
        form2->addRow("Reverse Rate:", revRate = new QComboBox); revRate->setEnabled(false); // deactive this since we'll start with a forward reaction
        form2->addRow("", ovrVB = new QCheckBox("Override Calculated VBar")); ovrVB->setCheckState(Qt::Checked);
        
        QGroupBox* pgInternalReactants = new QGroupBox("Internal Reactants:");
        pgInternalReactants->setFlat(true);
        QVBoxLayout* intreactantsLayout = new QVBoxLayout;
        intreactantsLayout->setContentsMargins(0,0,0,0);
        intreactantsLayout->addWidget(selectInternalReactants = new QSelectBox);
        pgInternalReactants->setLayout(intreactantsLayout);
        
        QGroupBox* pgInternalProducts = new QGroupBox("Internal Products:");
        pgInternalProducts->setFlat(true);
        QVBoxLayout* intproductsLayout = new QVBoxLayout;
        intproductsLayout->setContentsMargins(0,0,0,0);
        intproductsLayout->addWidget(selectInternalProducts = new QSelectBox);
        pgInternalProducts->setLayout(intproductsLayout);
        
        QGroupBox* pgExternalReactants = new QGroupBox("External Reactants:");
        pgExternalReactants->setFlat(true);
        QVBoxLayout* extreactantsLayout = new QVBoxLayout;
        extreactantsLayout->setContentsMargins(0,0,0,0);
        extreactantsLayout->addWidget(selectExternalReactants = new QSelectBox);
        pgExternalReactants->setLayout(extreactantsLayout);
        
        QGroupBox* pgExternalProducts = new QGroupBox("External Products:");
        pgExternalProducts->setFlat(true);
        QVBoxLayout* extproductsLayout = new QVBoxLayout;
        extproductsLayout->setContentsMargins(0,0,0,0);
        extproductsLayout->addWidget(selectExternalProducts = new QSelectBox);
        pgExternalProducts->setLayout(extproductsLayout);
        
        QGroupBox* pgReactants = new QGroupBox("Membrane Reactants:");
        pgReactants->setFlat(true);
        QVBoxLayout* reactantsLayout = new QVBoxLayout;
        reactantsLayout->setContentsMargins(0,0,0,0);
        reactantsLayout->addWidget(selectReactants = new QSelectBox);
        pgReactants->setLayout(reactantsLayout);
        
        QGroupBox* pgProducts = new QGroupBox("Membrane Products:");
        pgProducts->setFlat(true);
        QVBoxLayout* productsLayout = new QVBoxLayout;
        productsLayout->setContentsMargins(0,0,0,0);
        productsLayout->addWidget(selectProducts = new QSelectBox);
        pgProducts->setLayout(productsLayout);
        
        dummy = new QWidget;
        QVBoxLayout* dummyLayout = new QVBoxLayout;
        dummyLayout->setContentsMargins(0,0,0,0);
        dummyLayout->addLayout(form2);
        dummyLayout->addWidget(pgInternalReactants);
        dummyLayout->addWidget(pgInternalProducts);
        dummyLayout->addWidget(pgExternalReactants);
        dummyLayout->addWidget(pgExternalProducts);
        dummyLayout->addWidget(pgReactants);
        dummyLayout->addWidget(pgProducts);
        dummy->setLayout(dummyLayout);
        
        // disable dummy by default
        dummy->setEnabled(false);
        
        bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
        
        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->addLayout(form);
        mainLayout->addWidget(dummy);
        mainLayout->addWidget(bb);
        
        parent->setLayout(mainLayout);
        
        QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
        QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
        QObject::connect(bb, SIGNAL(clicked(QAbstractButton*)), parent, SLOT(onClicked(QAbstractButton*)));
        
        QObject::connect(type, SIGNAL(currentIndexChanged(int)), parent, SLOT(onReactionType(int)));
        QObject::connect(mat, SIGNAL(currentIndexChanged(int)), parent, SLOT(onMaterialChanged(int)));
        QObject::connect(reactions, SIGNAL(currentIndexChanged(int)), parent, SLOT(onReactionChanged(int)));
        QObject::connect(reactions, SIGNAL(onAdd()), parent, SLOT(onAddReaction()));
        QObject::connect(reactions, SIGNAL(onRemove()), parent, SLOT(onRemoveReaction()));
        QObject::connect(name, SIGNAL(textEdited(const QString&)), parent, SLOT(onNameChanged(const QString&)));
    }
    
    void setReactionName(const std::string& s)
    {
        name->setText(QString::fromStdString(s));
    }
    
    void setReactionType(int ntype)
    {
        type->blockSignals(true);
        int n = type->count();
        for (int i=0; i<n; ++i)
        {
            int m = type->itemData(i).toInt();
            if (ntype == m)
            {
                type->setCurrentIndex(i);
                break;
            }
        }
        type->blockSignals(false);
    }
    
    void setForwardRateType(int ntype)
    {
        if (ntype == -1) fwdRate->setEnabled(false);
        else fwdRate->setEnabled(true);
        
        fwdRate->blockSignals(true);
        int n = fwdRate->count();
        for (int i = 0; i<n; ++i)
        {
            int m = fwdRate->itemData(i).toInt();
            if (ntype == m)
            {
                fwdRate->setCurrentIndex(i);
                break;
            }
        }
        fwdRate->blockSignals(false);
    }
    
    void setReverseRateType(int ntype)
    {
        if (ntype == -1) revRate->setEnabled(false);
        else revRate->setEnabled(true);
        
        revRate->blockSignals(true);
        int n = revRate->count();
        for (int i = 0; i<n; ++i)
        {
            int m = revRate->itemData(i).toInt();
            if (ntype == m)
            {
                revRate->setCurrentIndex(i);
                break;
            }
        }
        revRate->blockSignals(false);
    }
    
};

CDlgAddMembraneReaction::CDlgAddMembraneReaction(CMainWindow* wnd) : m_wnd(wnd), QDialog(wnd), ui(new Ui::CDlgAddMembraneReaction)
{
    setWindowTitle("Membrane Reaction Editor");
    ui->setupUi(this);
    
    m_pmp = 0;
    m_reaction = 0;
    
    InitDialog();
}

void CDlgAddMembraneReaction::InitDialog()
{
    CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
    FSModel& fem = *doc->GetFSModel();
    
    // fill in the reactions
    list<FEMatDescriptor*> mats = FEMaterialFactory::Enumerate(FE_MAT_MREACTION);
    if (mats.empty() == false)
    {
        for (FEMatDescriptor* it : mats)
        {
            ui->type->addItem(it->GetTypeString(), it->GetTypeID());
        }
    }
    
    // fill in reaction rate options
    mats = FEMaterialFactory::Enumerate(FE_MAT_MREACTION_RATE);
    if (mats.empty() == false)
    {
        for (FEMatDescriptor* it : mats)
        {
            ui->fwdRate->addItem(it->GetTypeString(), it->GetTypeID());
            ui->revRate->addItem(it->GetTypeString(), it->GetTypeID());
        }
    }
    
    // build the species list
    m_species.clear();
    for (int i = 0; i<fem.Solutes(); ++i)
    {
        SoluteData& sd = fem.GetSoluteData(i);
        m_species.push_back(pair<string,int>(sd.GetName(), i));
    }
    for (int i = 0; i<fem.SBMs(); ++i)
    {
        SoluteData& sd = fem.GetSBMData(i);
        m_species.push_back(pair<string, int>(sd.GetName(), i + 0x100));
    }
    
    // fill all materials that support reactions
    for (int i = 0; i<fem.Materials(); ++i)
    {
        GMaterial& mat = *fem.GetMaterial(i);
        FSMaterial& props = *mat.GetMaterialProperties();
        if (props.FindProperty(FE_MAT_MREACTION))
        {
            ui->mat->addItem(QString::fromStdString(mat.GetName()), i);
        }
    }
}

void CDlgAddMembraneReaction::onClicked(QAbstractButton* button)
{
    if (ui->bb->buttonRole(button) == QDialogButtonBox::ApplyRole)
    {
        hasChanged();
        apply();
    }
}

void CDlgAddMembraneReaction::onMaterialChanged(int n)
{
    CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
    FSModel& fem = *doc->GetFSModel();
    
    int nmat = ui->mat->currentData().toInt();
    GMaterial* gmat = fem.GetMaterial(nmat);
    SetMaterial(gmat, fem);
}

void CDlgAddMembraneReaction::onReactionChanged(int n)
{
    if (m_pmp == 0) { ui->dummy->setEnabled(false); return; }
    
    if (n == -1) ui->dummy->setEnabled(false);
    else
    {
        ui->dummy->setEnabled(true);
        
        FSMaterial& props = *m_pmp->GetMaterialProperties();
        FSProperty* react = props.FindProperty(FE_MAT_MREACTION); assert(react);
        if (react)
        {
            FSMembraneReactionMaterial* r = dynamic_cast<FSMembraneReactionMaterial*>(react->GetComponent(n));
            SetReaction(r);
        }
    }
}

void CDlgAddMembraneReaction::onReactionType(int n)
{
    // get the reaction material
    int mat = ui->type->itemData(n).toInt();
    
    // activate the reverse rate if needed
    if (mat == FE_MMASS_ACTION_REVERSIBLE) ui->revRate->setEnabled(true);
    else ui->revRate->setEnabled(false);
}

void CDlgAddMembraneReaction::SetMaterial(GMaterial* mat, FSModel& fem)
{
    m_pmp = mat;
    
    FSMaterial& props = *mat->GetMaterialProperties();
    FSProperty* react = props.FindProperty(FE_MAT_MREACTION); assert(react);
    
    ui->reactions->Clear();
    if (react)
    {
        int N = react->Size();
        for (int i=0; i<N; ++i)
        {
            FSMembraneReactionMaterial* r = dynamic_cast<FSMembraneReactionMaterial*>(react->GetComponent(i)); assert(r);
            if (r)
            {
                string name = r->GetName();
                if (name.empty())
                {
                    // build a name based on the equation
                    name = buildMembraneReactionEquation(r, fem);
                }
                ui->reactions->AddItem(QString::fromStdString(name), i);
            }
        }
    }
}

void CDlgAddMembraneReaction::onAddReaction()
{
    if (m_pmp == 0) return;
    
    FSMaterial& props = *m_pmp->GetMaterialProperties();
    FSProperty* react = props.FindProperty(FE_MAT_MREACTION); assert(react);
    
    // create a default material
    FSMembraneReactionMaterial* r = dynamic_cast<FSMembraneReactionMaterial*>(FEMaterialFactory::Create(FE_MMASS_ACTION_FORWARD)); assert(r);
    react->AddComponent(r);
    
    stringstream ss;
    ss << "Reaction" << react->Size();
    r->SetName(ss.str());
    
    // add a constant forward reaction rate
    r->SetForwardRate(FEMaterialFactory::Create(FE_MREACTION_RATE_CONST));
    
    // add it to the list
    ui->reactions->AddItem(QString::fromStdString(r->GetName()), react->Size() - 1, true);
}

void CDlgAddMembraneReaction::onNameChanged(const QString& t)
{
    if (m_reaction)
    {
        string s = t.toStdString();
        m_reaction->SetName(s);
        ui->reactions->SetCurrentText(t);
    }
}

void CDlgAddMembraneReaction::onRemoveReaction()
{
    if ((m_pmp == 0) || (m_reaction == 0)) return;
    
    FSMaterial& props = *m_pmp->GetMaterialProperties();
    FSProperty* react = props.FindProperty(FE_MAT_MREACTION); assert(react);
    
    // remove the reaction
    react->RemoveComponent(m_reaction);
    
    CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
    FSModel& fem = *doc->GetFSModel();
    
    // update the list
    m_reaction = 0;
    SetMaterial(m_pmp, fem);
}

void CDlgAddMembraneReaction::SetReaction(FSMembraneReactionMaterial* mat)
{
    // see if we have a reaction present
    if (m_reaction)
    {
        // update reaction data and see if we need to change
        if (hasChanged())
        {
            if (QMessageBox::question(this, "Apply changes", "Do you want to apply the changes to the current material?") == QMessageBox::Yes)
            {
                apply();
            }
        }
    }
    
    // store the new active reaction
    m_reaction = mat;
    
    // update the UI
    int ntype = mat->Type();
    ui->setReactionType(ntype);
    
    // activate the reverse rate if needed
    if (ntype == FE_MMASS_ACTION_REVERSIBLE) ui->revRate->setEnabled(true);
    else ui->revRate->setEnabled(false);
    
    FSMaterial* fwd = mat->GetForwardRate();
    ui->setForwardRateType(fwd ? fwd->Type() : -1);
    
    FSMaterial* rev = mat->GetReverseRate();
    ui->setReverseRateType(rev ? rev->Type() : -1);
    
    ui->name->setText(QString::fromStdString(mat->GetName()));
    
    m_bovrd = mat->GetOvrd();
    ui->ovrVB->setChecked(m_bovrd);
    
    // set species
    ui->selectReactants->clear();
    ui->selectInternalReactants->clear();
    ui->selectExternalReactants->clear();
    ui->selectProducts->clear();
    ui->selectInternalProducts->clear();
    ui->selectExternalProducts->clear();

    int nr = mat->Reactants();
    for (int i=0; i<(int)m_species.size(); ++i)
    {
        pair<string,int>& spec = m_species[i];
        
        // see if the parent material has this reactant
        bool bvalid = true;
        FSMultiphasicMaterial* mp = dynamic_cast<FSMultiphasicMaterial*>(m_pmp->GetMaterialProperties());
        if (mp)
        {
            if (spec.second < 0x100)
            {
                if (mp->HasSolute(spec.second) == false) bvalid = false;
            }
            else
            {
                if (mp->HasSBM(spec.second - 0x100) == false) bvalid = false;
            }
        }
        
        bool bfound = false;
        for (int j=0; j<nr; ++j)
        {
            FSReactantMaterial* rm = mat->Reactant(j);
            
            int index = spec.second;
            if (rm->GetReactantType() == FSReactionSpecies::SBM_SPECIES) index -= 0x100;
            
            if (index == rm->GetIndex())
            {
                ui->selectReactants->addTargetItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
                bfound = true;
                break;
            }
        }
        
        if (bfound == false)
        {
            ui->selectReactants->addSourceItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
        }
    }
    
    int np = mat->Products();
    for (int i = 0; i<(int)m_species.size(); ++i)
    {
        pair<string, int>& spec = m_species[i];
        
        // see if the parent material has this product
        bool bvalid = true;
        FSMultiphasicMaterial* mp = dynamic_cast<FSMultiphasicMaterial*>(m_pmp->GetMaterialProperties());
        if (mp)
        {
            if (spec.second < 0x100)
            {
                if (mp->HasSolute(spec.second) == false) bvalid = false;
            }
            else
            {
                if (mp->HasSBM(spec.second - 0x100) == false) bvalid = false;
            }
        }
        
        bool bfound = false;
        for (int j = 0; j<np; ++j)
        {
            FSProductMaterial* rm = mat->Product(j);
            
            int index = spec.second;
            if (rm->GetProductType() == FSReactionSpecies::SBM_SPECIES) index -= 0x100;
            
            if (index == rm->GetIndex())
            {
                ui->selectProducts->addTargetItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
                bfound = true;
                break;
            }
        }
        
        if (bfound == false)
        {
            ui->selectProducts->addSourceItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
        }
    }

    int nri = mat->InternalReactants();
    for (int i=0; i<(int)m_species.size(); ++i)
    {
        pair<string,int>& spec = m_species[i];
        
        // see if the parent material has this reactant
        bool bvalid = true;
        FSMultiphasicMaterial* mp = dynamic_cast<FSMultiphasicMaterial*>(m_pmp->GetMaterialProperties());
        if (mp)
        {
            if (spec.second < 0x100)
            {
                if (mp->HasSolute(spec.second) == false) bvalid = false;
            }
            else
            {
                if (mp->HasSBM(spec.second - 0x100) == false) bvalid = false;
            }
        }
        
        bool bfound = false;
        for (int j=0; j<nri; ++j)
        {
            FSInternalReactantMaterial* rm = mat->InternalReactant(j);
            
            int index = spec.second;
            if (rm->GetReactantType() == FSReactionSpecies::SBM_SPECIES) index -= 0x100;
            
            if (index == rm->GetIndex())
            {
                ui->selectInternalReactants->addTargetItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
                bfound = true;
                break;
            }
        }
        
        if (bfound == false)
        {
            ui->selectInternalReactants->addSourceItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
        }
    }
    
    int npi = mat->InternalProducts();
    for (int i = 0; i<(int)m_species.size(); ++i)
    {
        pair<string, int>& spec = m_species[i];
        
        // see if the parent material has this product
        bool bvalid = true;
        FSMultiphasicMaterial* mp = dynamic_cast<FSMultiphasicMaterial*>(m_pmp->GetMaterialProperties());
        if (mp)
        {
            if (spec.second < 0x100)
            {
                if (mp->HasSolute(spec.second) == false) bvalid = false;
            }
            else
            {
                if (mp->HasSBM(spec.second - 0x100) == false) bvalid = false;
            }
        }
        
        bool bfound = false;
        for (int j = 0; j<npi; ++j)
        {
            FSInternalProductMaterial* rm = mat->InternalProduct(j);
            
            int index = spec.second;
            if (rm->GetProductType() == FSReactionSpecies::SBM_SPECIES) index -= 0x100;
            
            if (index == rm->GetIndex())
            {
                ui->selectInternalProducts->addTargetItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
                bfound = true;
                break;
            }
        }
        
        if (bfound == false)
        {
            ui->selectInternalProducts->addSourceItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
        }
    }

    int nre = mat->ExternalReactants();
    for (int i=0; i<(int)m_species.size(); ++i)
    {
        pair<string,int>& spec = m_species[i];
        
        // see if the parent material has this reactant
        bool bvalid = true;
        FSMultiphasicMaterial* mp = dynamic_cast<FSMultiphasicMaterial*>(m_pmp->GetMaterialProperties());
        if (mp)
        {
            if (spec.second < 0x100)
            {
                if (mp->HasSolute(spec.second) == false) bvalid = false;
            }
            else
            {
                if (mp->HasSBM(spec.second - 0x100) == false) bvalid = false;
            }
        }
        
        bool bfound = false;
        for (int j=0; j<nre; ++j)
        {
            FSExternalReactantMaterial* rm = mat->ExternalReactant(j);
            
            int index = spec.second;
            if (rm->GetReactantType() == FSReactionSpecies::SBM_SPECIES) index -= 0x100;
            
            if (index == rm->GetIndex())
            {
                ui->selectExternalReactants->addTargetItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
                bfound = true;
                break;
            }
        }
        
        if (bfound == false)
        {
            ui->selectExternalReactants->addSourceItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
        }
    }
    
    int npe = mat->ExternalProducts();
    for (int i = 0; i<(int)m_species.size(); ++i)
    {
        pair<string, int>& spec = m_species[i];
        
        // see if the parent material has this product
        bool bvalid = true;
        FSMultiphasicMaterial* mp = dynamic_cast<FSMultiphasicMaterial*>(m_pmp->GetMaterialProperties());
        if (mp)
        {
            if (spec.second < 0x100)
            {
                if (mp->HasSolute(spec.second) == false) bvalid = false;
            }
            else
            {
                if (mp->HasSBM(spec.second - 0x100) == false) bvalid = false;
            }
        }
        
        bool bfound = false;
        for (int j = 0; j<npe; ++j)
        {
            FSExternalProductMaterial* rm = mat->ExternalProduct(j);
            
            int index = spec.second;
            if (rm->GetProductType() == FSReactionSpecies::SBM_SPECIES) index -= 0x100;
            
            if (index == rm->GetIndex())
            {
                ui->selectExternalProducts->addTargetItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
                bfound = true;
                break;
            }
        }
        
        if (bfound == false)
        {
            ui->selectExternalProducts->addSourceItem(QString::fromStdString(spec.first), spec.second, (bvalid ? 0 : 1));
        }
    }
}

bool CDlgAddMembraneReaction::hasChanged()
{
    if (m_reaction == 0) return false;
    
    bool changed = false;
    
    // get the name
    QString name = ui->name->text();
    m_name = name.toStdString();
    if (m_name != m_reaction->GetName()) changed = true;
    
    // get the reaction material type
    m_reactionMat = ui->type->currentData().toInt();
    if (m_reaction->Type() != m_reactionMat) changed = true;
    
    // get the forward rate material
    m_fwdMat = ui->fwdRate->currentData().toInt();
    FSMaterial* fwd = m_reaction->GetForwardRate();
    if ((fwd == 0) || (fwd->Type() != m_fwdMat)) changed = true;
    
    // see if we have a reverse rate specified
    m_brr = ui->revRate->isEnabled();
    m_revMat = -1;
    if (m_brr) m_revMat = ui->revRate->currentData().toInt();
    
    // get the reverse rate material
    FSMaterial* rev = m_reaction->GetReverseRate();
    if (rev && (m_revMat != rev->Type())) changed = true;
    else if ((rev == 0) && m_brr) changed = true;

    vector<int> sbm;
    // get the reactants
    m_solR.clear();
    m_sbmR.clear();
    m_solRi.clear();
    m_solRe.clear();
    ui->selectReactants->targetData(m_solR, m_sbmR);
    ui->selectInternalReactants->targetData(m_solRi, sbm);
    ui->selectExternalReactants->targetData(m_solRe, sbm);

    // add the reactants
    vector<int> solR, solRi, solRe, sbmR;
    m_reaction->GetSoluteReactants(solR);
    m_reaction->GetInternalSoluteReactants(solRi);
    m_reaction->GetExternalSoluteReactants(solRe);
    m_reaction->GetSBMReactants(sbmR);
    if ((solR != m_solR) || (solRi != m_solRi) || (solRe != m_solRe) || (sbmR != m_sbmR)) changed = true;
    
    // get the products
    m_solP.clear();
    m_sbmP.clear();
    m_solPi.clear();
    m_solPe.clear();
    ui->selectProducts->targetData(m_solP, m_sbmP);
    ui->selectInternalProducts->targetData(m_solPi, sbm);
    ui->selectExternalProducts->targetData(m_solPe, sbm);

    vector<int> solP, solPi, solPe, sbmP;
    m_reaction->GetSoluteProducts(solP);
    m_reaction->GetInternalSoluteProducts(solPi);
    m_reaction->GetExternalSoluteProducts(solPe);
    m_reaction->GetSBMProducts(sbmP);
    if ((solP != m_solP) || (solPi != m_solPi) || (solPe != m_solPe) || (sbmP != m_sbmP)) changed = true;
    
    m_bovrd = ui->ovrVB->isChecked();
    if (m_bovrd != m_reaction->GetOvrd()) changed = true;
    
    return changed;
}

void CDlgAddMembraneReaction::apply()
{
    CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
    
    // make sure we have something to do
    if ((m_pmp == 0) || (m_reaction == 0)) return;
    
    FSMaterial* mat = m_pmp->GetMaterialProperties();
    FSProperty* reactProp = mat->FindProperty(FE_MAT_MREACTION);
    if (reactProp == 0) return;
    
    // create the reaction material and set its type
    if (m_reactionMat != m_reaction->Type())
    {
        // we need to create a new material
        FSMembraneReactionMaterial* pmat = dynamic_cast<FSMembraneReactionMaterial*>(FEMaterialFactory::Create(m_reactionMat)); assert(pmat);
        if (pmat == 0) return;
        
        // find the property
        int index = reactProp->GetComponentIndex(m_reaction); assert(index >= 0);
        if (index < 0) return;
        
        // replace the property
        reactProp->SetComponent(pmat, index);
        
        // continue with this material
        m_reaction = pmat;
    }
    
    // set the name
    m_reaction->SetName(m_name);
    
    // check if we override Vbar
    m_reaction->SetOvrd(m_bovrd);
    
    // set the forward rate
    FSMaterial* fwd = m_reaction->GetForwardRate();
    if ((fwd == 0) || (m_fwdMat != fwd->Type()))
    {
        m_reaction->SetForwardRate(FEMaterialFactory::Create(m_fwdMat));
    }
    
    // set the reverse rate
    FSMaterial* rev = m_reaction->GetReverseRate();
    if (m_brr)
    {
        if ((rev == 0) || (rev->Type() != m_revMat))
        {
            m_reaction->SetReverseRate(FEMaterialFactory::Create(m_revMat));
        }
    }
    else m_reaction->SetReverseRate(0);
    
    // add the reactants
    vector<int> solR, solRi, solRe, sbmR;
    m_reaction->GetSoluteReactants(solR);
    m_reaction->GetInternalSoluteReactants(solRi);
    m_reaction->GetExternalSoluteReactants(solRe);
    m_reaction->GetSBMReactants(sbmR);
    if ((solR != m_solR) || (solRi != m_solRi) || (solRe != m_solRe) || (sbmR != m_sbmR))
    {
        m_reaction->ClearReactants();
        
        // add the solute reactants
        for (int i = 0; i<(int)m_solR.size(); ++i)
        {
            FSReactantMaterial* ps = new FSReactantMaterial;
            ps->SetIndex(m_solR[i]);
            ps->SetReactantType(FSReactionSpecies::SOLUTE_SPECIES);
            m_reaction->AddReactantMaterial(ps);
        }
        
        // add the internal solute reactants
        for (int i = 0; i<(int)m_solRi.size(); ++i)
        {
            FSInternalReactantMaterial* ps = new FSInternalReactantMaterial;
            ps->SetIndex(m_solRi[i]);
            ps->SetReactantType(FSReactionSpecies::SOLUTE_SPECIES);
            m_reaction->AddInternalReactantMaterial(ps);
        }
        
        // add the external solute reactants
        for (int i = 0; i<(int)m_solRe.size(); ++i)
        {
            FSExternalReactantMaterial* ps = new FSExternalReactantMaterial;
            ps->SetIndex(m_solRe[i]);
            ps->SetReactantType(FSReactionSpecies::SOLUTE_SPECIES);
            m_reaction->AddExternalReactantMaterial(ps);
        }
        
        // add the sbm reactants
        for (int i = 0; i<(int)m_sbmR.size(); ++i)
        {
            FSReactantMaterial* ps = new FSReactantMaterial;
            ps->SetIndex(m_sbmR[i]);
            ps->SetReactantType(FSReactionSpecies::SBM_SPECIES);
            m_reaction->AddReactantMaterial(ps);
        }
    }
    
    // add the products
    vector<int> solP, solPi, solPe, sbmP;
    m_reaction->GetSoluteProducts(solP);
    m_reaction->GetInternalSoluteProducts(solPi);
    m_reaction->GetExternalSoluteProducts(solPe);
    m_reaction->GetSBMProducts(sbmP);
    if ((solP != m_solP) || (solPi != m_solPi) || (solPe != m_solPe) || (sbmP != m_sbmP))
    {
        m_reaction->ClearProducts();
        
        // add the solute products
        for (int i = 0; i<(int)m_solP.size(); ++i)
        {
            FSProductMaterial* ps = new FSProductMaterial;
            ps->SetIndex(m_solP[i]);
            ps->SetProductType(FSReactionSpecies::SOLUTE_SPECIES);
            m_reaction->AddProductMaterial(ps);
        }
        
        // add the internal solute products
        for (int i = 0; i<(int)m_solPi.size(); ++i)
        {
            FSInternalProductMaterial* ps = new FSInternalProductMaterial;
            ps->SetIndex(m_solPi[i]);
            ps->SetProductType(FSReactionSpecies::SOLUTE_SPECIES);
            m_reaction->AddInternalProductMaterial(ps);
        }
        
        // add the external solute products
        for (int i = 0; i<(int)m_solPe.size(); ++i)
        {
            FSExternalProductMaterial* ps = new FSExternalProductMaterial;
            ps->SetIndex(m_solPe[i]);
            ps->SetProductType(FSReactionSpecies::SOLUTE_SPECIES);
            m_reaction->AddExternalProductMaterial(ps);
        }
        
        // add the sbm products
        for (int i = 0; i<(int)m_sbmP.size(); ++i)
        {
            FSProductMaterial* ps = new FSProductMaterial;
            ps->SetIndex(m_sbmP[i]);
            ps->SetProductType(FSReactionSpecies::SBM_SPECIES);
            m_reaction->AddProductMaterial(ps);
        }
    }
}

void CDlgAddMembraneReaction::accept()
{
    // apply any changes
    hasChanged();
    apply();
    
    // call base class to close dialog
    QDialog::accept();
}
