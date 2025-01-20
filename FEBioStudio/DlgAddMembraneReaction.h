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

#pragma once
#include <vector>
#include <list>
#include <QDialog>

class CMainWindow;
class QListWidget;
class QListWidgetItem;

class FSMaterial;
class FSMaterialProperty;
class FEBioMembraneReactionMaterial;
class FSModel;
class GMaterial;
class QAbstractButton;
class QSelectBox;

class CMembraneReactionList : public QWidget
{
    Q_OBJECT
    
public:
    CMembraneReactionList(QWidget* parent = 0);
    
    void Clear();
    
    void AddItem(const QString& name, const QVariant& data, bool sel = false);
    
    void SetCurrentText(const QString& t);
    
signals:
    void currentIndexChanged(int n);
    void onAdd();
    void onRemove();
    
private slots:
    void onCurrentIndexChanged(int n);
    void OnAdd();
    void OnRemove();
    
private:
    QListWidget*    m_list;
};

namespace Ui {
class CDlgAddMembraneReaction;
};

class CDlgAddMembraneReaction : public QDialog
{
    Q_OBJECT
    
public:
    CDlgAddMembraneReaction(CMainWindow* wnd);
    
private:
    void InitDialog();
    void apply();
    void accept();
    void SetMaterial(GMaterial* mat, FSModel& fem);
    void SetReaction(FSMaterialProperty* mat);
    bool hasChanged();
    
    protected slots:
    void onMaterialChanged(int n);
    void onReactionChanged(int n);
    void onReactionType(int n);
    void onAddReaction();
    void onRemoveReaction();
    void onNameChanged(const QString& t);
    void onClicked(QAbstractButton* button);
    
private:
    GMaterial*                      m_pmp;          // parent multiphasic material
    FEBioMembraneReactionMaterial*  m_reaction;	    // active reaction

	std::string         m_name;         // name of chemical reaction
    bool                m_bovrd;        // override calculated Vbar
    std::vector<int>    m_solR;         // solute reactants
    std::vector<int>    m_solRi;        // internal solute reactants
    std::vector<int>    m_solRe;        // external solute reactants
    std::vector<int>    m_sbmR;         // solid-bound molecule reactants
    std::vector<int>    m_solP;         // solute products
    std::vector<int>    m_solPi;        // internal solute products
    std::vector<int>    m_solPe;        // external solute products
    std::vector<int>    m_sbmP;         // solid-bound molecule products
    
    int             m_reactionMat;      // reaction material
    int             m_fwdMat;           // forward rate material
    int             m_revMat;           // backward rate material
    bool            m_brr;              // flag for specification of reverse rate
    
private:
    CMainWindow*    m_wnd;
    Ui::CDlgAddMembraneReaction*    ui;
	std::vector<std::pair<std::string, int> >      m_species;
};
