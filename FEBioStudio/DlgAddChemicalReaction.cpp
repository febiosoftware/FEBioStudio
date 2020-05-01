#include "stdafx.h"
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

//=================================================================================================
QSelectBox::QSelectBox(QWidget* parent) : QWidget(parent)
{
	source = new QListWidget;
	target = new QListWidget;

	QPushButton* toTarget = new QPushButton(">"); toTarget->setFixedWidth(30); toTarget->setObjectName("toTarget");
	QPushButton* toSource = new QPushButton("<"); toSource->setFixedWidth(30); toSource->setObjectName("toSource");

	QVBoxLayout* buttonLayout = new QVBoxLayout;
	buttonLayout->setMargin(0);
	buttonLayout->addWidget(toTarget);
	buttonLayout->addWidget(toSource);
	buttonLayout->addStretch();

	QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->setMargin(0);
	mainLayout->addWidget(source);
	mainLayout->addLayout(buttonLayout);
	mainLayout->addWidget(target);

	setLayout(mainLayout);

	QMetaObject::connectSlotsByName(this);
}

void QSelectBox::clear()
{
	source->clear();
	target->clear();
}

void QSelectBox::addSourceItem(const QString& item, int data, int flag)
{
	QListWidgetItem* li = new QListWidgetItem(source);
	li->setText(item);
	li->setData(Qt::UserRole, data);
	if (flag == 1)
	{
		li->setForeground(Qt::red);
	}
}

void QSelectBox::addTargetItem(const QString& item, int data, int flag)
{
	QListWidgetItem* li = new QListWidgetItem(target);
	li->setText(item);
	li->setData(Qt::UserRole, data);
	if (flag == 1)
	{
		li->setForeground(Qt::red);
	}
}

void QSelectBox::on_toTarget_clicked()
{
	QListWidgetItem* item = source->currentItem();
	if (item)
	{
		QListWidgetItem* newItem = item->clone();
		target->addItem(newItem);
		target->setCurrentItem(newItem);
		delete item;
	}
}

void QSelectBox::on_toSource_clicked()
{
	QListWidgetItem* item = target->currentItem();
	if (item)
	{
		QListWidgetItem* newItem = item->clone();
		source->addItem(newItem);
		source->setCurrentItem(newItem);
		delete item;
	}
}

list<int> QSelectBox::targetData()
{
	list<int> data;
	int N = target->count();
	for (int i=0; i<N; ++i)
	{
		int m = target->item(i)->data(Qt::UserRole).toInt();
		data.push_back(m);
	}
	return data;
}

void QSelectBox::targetData(vector<int>& sols, vector<int>& sbms)
{
	list<int> reactants = targetData();
	for (int r : reactants)
	{
		int m = r;
		if (m < 0x100)
		{
			sols.push_back(m);
		}
		else
		{
			sbms.push_back(m - 0x100);
		}
	}
}

//=================================================================================================

CReactionList::CReactionList(QWidget* parent) : QWidget(parent)
{
	m_list = new QListWidget;

	QPushButton* add = new QPushButton("Add");
	QPushButton* del = new QPushButton("Remove");

	QVBoxLayout* l = new QVBoxLayout;
	l->addWidget(add);
	l->addWidget(del);
	l->addStretch();
	l->setMargin(0);

	QHBoxLayout* h = new QHBoxLayout;
	h->addWidget(m_list);
	h->addLayout(l);
	h->setMargin(0);

	setLayout(h);

	QObject::connect(m_list, SIGNAL(currentRowChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
	QObject::connect(add, SIGNAL(clicked(bool)), this, SLOT(OnAdd()));
	QObject::connect(del, SIGNAL(clicked(bool)), this, SLOT(OnRemove()));
}

void CReactionList::Clear()
{
	m_list->clear();
}

void CReactionList::AddItem(const QString& name, const QVariant& data, bool sel)
{
	QListWidgetItem* item = new QListWidgetItem(m_list);
	item->setText(name);
	item->setData(Qt::UserRole, data);

	if (sel) m_list->setCurrentItem(item);
}

void CReactionList::SetCurrentText(const QString& t)
{
	QListWidgetItem* current = m_list->currentItem();
	if (current) current->setText(t);
}

void CReactionList::onCurrentIndexChanged(int n)
{
	emit currentIndexChanged(n);
}

void CReactionList::OnAdd()
{
	emit onAdd();
}

void CReactionList::OnRemove()
{
	emit onRemove();
}

//=================================================================================================
class Ui::CDlgAddChemicalReaction
{
public:
	QComboBox*		mat;
	CReactionList*	reactions;
	QWidget*		dummy;
	QComboBox*		type;
	QComboBox*		fwdRate;
	QComboBox*		revRate;
	QLineEdit*		name;
	QCheckBox*		ovrVB;

	QSelectBox*	selectReactants;
	QSelectBox* selectProducts;

	QDialogButtonBox* bb;

public:
	void setupUi(QWidget* parent)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow("Material:", mat = new QComboBox);
		form->addRow("Reactions:", reactions = new CReactionList);

		QFormLayout* form2 = new QFormLayout;
		form2->addRow("Name:", name = new QLineEdit);
		form2->addRow("Type:", type = new QComboBox);
		form2->addRow("Forward Rate:", fwdRate = new QComboBox);
		form2->addRow("Reverse Rate:", revRate = new QComboBox); revRate->setEnabled(false); // deactive this since we'll start with a forward reaction
		form2->addRow("", ovrVB = new QCheckBox("Override Calculated VBar"));

		QGroupBox* pgReactants = new QGroupBox("Reactants:");
		pgReactants->setFlat(true);
		QVBoxLayout* reactantsLayout = new QVBoxLayout;
		reactantsLayout->setMargin(0);
		reactantsLayout->addWidget(selectReactants = new QSelectBox);
		pgReactants->setLayout(reactantsLayout);

		QGroupBox* pgProducts = new QGroupBox("Products:");
		pgProducts->setFlat(true);
		QVBoxLayout* productsLayout = new QVBoxLayout;
		productsLayout->setMargin(0);
		productsLayout->addWidget(selectProducts = new QSelectBox);
		pgProducts->setLayout(productsLayout);

		dummy = new QWidget;
		QVBoxLayout* dummyLayout = new QVBoxLayout;
		dummyLayout->setMargin(0);
		dummyLayout->addLayout(form2);
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

CDlgAddChemicalReaction::CDlgAddChemicalReaction(CMainWindow* wnd) : m_wnd(wnd), QDialog(wnd), ui(new Ui::CDlgAddChemicalReaction)
{
	setWindowTitle("Chemical Reaction Editor");
	ui->setupUi(this);

	m_pmp = 0;
	m_reaction = 0;
	
	InitDialog();
}

void CDlgAddChemicalReaction::InitDialog()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	FEModel& fem = *doc->GetFEModel();

	// fill in the reactions
	list<FEMatDescriptor*> mats = FEMaterialFactory::Enumerate(FE_MAT_REACTION);
	if (mats.empty() == false)
	{
        for (FEMatDescriptor* it : mats)
		{
			ui->type->addItem(it->GetTypeString(), it->GetTypeID());
		}
	}

	// fill in reaction rate options
	mats = FEMaterialFactory::Enumerate(FE_MAT_REACTION_RATE);
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
		FESoluteData& sd = fem.GetSoluteData(i);
		m_species.push_back(pair<string,int>(sd.GetName(), i));
	}
	for (int i = 0; i<fem.SBMs(); ++i)
	{
		FESoluteData& sd = fem.GetSBMData(i);
		m_species.push_back(pair<string, int>(sd.GetName(), i + 0x100));
	}

	// fill all materials that support reactions
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial& mat = *fem.GetMaterial(i);
		FEMaterial& props = *mat.GetMaterialProperties();
		if (props.FindProperty(FE_MAT_REACTION))
		{
			ui->mat->addItem(QString::fromStdString(mat.GetName()), i);
		}
	}
}

void CDlgAddChemicalReaction::onClicked(QAbstractButton* button)
{
	if (ui->bb->buttonRole(button) == QDialogButtonBox::ApplyRole) 
	{
		hasChanged();
		apply();
	}
}

void CDlgAddChemicalReaction::onMaterialChanged(int n)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	FEModel& fem = *doc->GetFEModel();

	int nmat = ui->mat->currentData().toInt();
	GMaterial* gmat = fem.GetMaterial(nmat);
	SetMaterial(gmat, fem);
}

void CDlgAddChemicalReaction::onReactionChanged(int n)
{
	if (m_pmp == 0) { ui->dummy->setEnabled(false); return; }

	if (n == -1) ui->dummy->setEnabled(false);
	else
	{
		ui->dummy->setEnabled(true);

		FEMaterial& props = *m_pmp->GetMaterialProperties();
		FEMaterialProperty* react = props.FindProperty(FE_MAT_REACTION); assert(react);
		if (react)
		{
			FEReactionMaterial* r = dynamic_cast<FEReactionMaterial*>(react->GetMaterial(n));
			SetReaction(r);
		}
	}
}

void CDlgAddChemicalReaction::onReactionType(int n)
{
	// get the reaction material
	int mat = ui->type->itemData(n).toInt();

	// activate the reverse rate if needed
	if (mat == FE_MASS_ACTION_REVERSIBLE) ui->revRate->setEnabled(true);
	else ui->revRate->setEnabled(false);
}

void CDlgAddChemicalReaction::SetMaterial(GMaterial* mat, FEModel& fem)
{
	m_pmp = mat;

	FEMaterial& props = *mat->GetMaterialProperties();
	FEMaterialProperty* react = props.FindProperty(FE_MAT_REACTION); assert(react);

	ui->reactions->Clear();
	if (react)
	{
		int N = react->Size();
		for (int i=0; i<N; ++i)
		{
			FEReactionMaterial* r = dynamic_cast<FEReactionMaterial*>(react->GetMaterial(i)); assert(r);
			if (r)
			{
				string name = r->GetName();
				if (name.empty())
				{
					// build a name based on the equation
					name = buildReactionEquation(r, fem);
				}
				ui->reactions->AddItem(QString::fromStdString(name), i);
			}
		}
	}
}

void CDlgAddChemicalReaction::onAddReaction()
{
	if (m_pmp == 0) return;

	FEMaterial& props = *m_pmp->GetMaterialProperties();
	FEMaterialProperty* react = props.FindProperty(FE_MAT_REACTION); assert(react);

	// create a default material
	FEReactionMaterial* r = dynamic_cast<FEReactionMaterial*>(FEMaterialFactory::Create(FE_MASS_ACTION_FORWARD)); assert(r);
	react->AddMaterial(r);

	stringstream ss;
	ss << "Reaction" << react->Size();
	r->SetName(ss.str());

	// add a constant forward reaction rate
	r->SetForwardRate(FEMaterialFactory::Create(FE_REACTION_RATE_CONST));

	// add it to the list
	ui->reactions->AddItem(QString::fromStdString(r->GetName()), react->Size() - 1, true);
}

void CDlgAddChemicalReaction::onNameChanged(const QString& t)
{
	if (m_reaction)
	{
		string s = t.toStdString();
		m_reaction->SetName(s);
		ui->reactions->SetCurrentText(t);
	}
}

void CDlgAddChemicalReaction::onRemoveReaction()
{
	if ((m_pmp == 0) || (m_reaction == 0)) return;

	FEMaterial& props = *m_pmp->GetMaterialProperties();
	FEMaterialProperty* react = props.FindProperty(FE_MAT_REACTION); assert(react);

	// remove the reaction
	react->RemoveMaterial(m_reaction);

	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	FEModel& fem = *doc->GetFEModel();

	// update the list
	m_reaction = 0;
	SetMaterial(m_pmp, fem);
}

void CDlgAddChemicalReaction::SetReaction(FEReactionMaterial* mat)
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
	if (ntype == FE_MASS_ACTION_REVERSIBLE) ui->revRate->setEnabled(true);
	else ui->revRate->setEnabled(false);

	FEMaterial* fwd = mat->GetForwardRate(); 
	ui->setForwardRateType(fwd ? fwd->Type() : -1);

	FEMaterial* rev = mat->GetReverseRate();
	ui->setReverseRateType(rev ? rev->Type() : -1);

	ui->name->setText(QString::fromStdString(mat->GetName()));

	// set species
	ui->selectReactants->clear();
	ui->selectProducts->clear();

	int nr = mat->Reactants();
	for (int i=0; i<(int)m_species.size(); ++i)
	{
		pair<string,int>& spec = m_species[i];

		// see if the parent material has this reactant
		bool bvalid = true;
		FEMultiphasicMaterial* mp = dynamic_cast<FEMultiphasicMaterial*>(m_pmp->GetMaterialProperties());
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
			FEReactantMaterial* rm = mat->Reactant(j);

			int index = spec.second;
			if (rm->GetReactantType() == FEReactionMaterial::SBM_SPECIES) index -= 0x100;

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
		FEMultiphasicMaterial* mp = dynamic_cast<FEMultiphasicMaterial*>(m_pmp->GetMaterialProperties());
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
			FEProductMaterial* rm = mat->Product(j);

			int index = spec.second;
			if (rm->GetProductType() == FEReactionMaterial::SBM_SPECIES) index -= 0x100;

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
}

bool CDlgAddChemicalReaction::hasChanged()
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
	FEMaterial* fwd = m_reaction->GetForwardRate();
	if ((fwd == 0) || (fwd->Type() != m_fwdMat)) changed = true;

	// see if we have a reverse rate specified
	m_brr = ui->revRate->isEnabled();
	m_revMat = -1;
	if (m_brr) m_revMat = ui->revRate->currentData().toInt();

	// get the reverse rate material
	FEMaterial* rev = m_reaction->GetReverseRate();
	if (rev && (m_revMat != rev->Type())) changed = true;
	else if ((rev == 0) && m_brr) changed = true;

	// get the reactants
	m_solR.clear();
	m_sbmR.clear();
	ui->selectReactants->targetData(m_solR, m_sbmR);

	// add the reactants
	vector<int> solR, sbmR;
	m_reaction->GetSoluteReactants(solR);
	m_reaction->GetSBMReactants(sbmR);
	if ((solR != m_solR) || (sbmR != m_sbmR)) changed = true;

	// get the products
	m_solP.clear();
	m_sbmP.clear();
	ui->selectProducts->targetData(m_solP, m_sbmP);

	vector<int> solP, sbmP;
	m_reaction->GetSoluteProducts(solP);
	m_reaction->GetSBMProducts(sbmP);
	if ((solP != m_solP) || (sbmP != m_sbmP)) changed = true;

	m_bovrd = ui->ovrVB->isChecked();
	if (m_bovrd != m_reaction->GetOvrd()) changed = true;

	return changed;
}

void CDlgAddChemicalReaction::apply()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	FEModel& fem = *doc->GetFEModel();

	// make sure we have something to do
	if ((m_pmp == 0) || (m_reaction == 0)) return;

	FEMaterial* mat = m_pmp->GetMaterialProperties();
	FEMaterialProperty* reactProp = mat->FindProperty(FE_MAT_REACTION);
	if (reactProp == 0) return;

	// create the reaction material and set its type
	if (m_reactionMat != m_reaction->Type())
	{
		// we need to create a new material
		FEReactionMaterial* pmat = dynamic_cast<FEReactionMaterial*>(FEMaterialFactory::Create(m_reactionMat)); assert(pmat);
		if (pmat == 0) return;

		// find the property
		int index = reactProp->GetMaterialIndex(m_reaction); assert(index >= 0);
		if (index < 0) return;

		// replace the property
		reactProp->SetMaterial(pmat, index);

		// continue with this material
		m_reaction = pmat;
	}

	// set the name
	m_reaction->SetName(m_name);

	// check if we override Vbar
	m_reaction->SetOvrd(m_bovrd);

	// set the forward rate
	FEMaterial* fwd = m_reaction->GetForwardRate();
	if ((fwd == 0) || (m_fwdMat != fwd->Type()))
	{
		m_reaction->SetForwardRate(FEMaterialFactory::Create(m_fwdMat));
	}

	// set the reverse rate
	FEMaterial* rev = m_reaction->GetReverseRate();
	if (m_brr)
	{
		if ((rev == 0) || (rev->Type() != m_revMat))
		{
			m_reaction->SetReverseRate(FEMaterialFactory::Create(m_revMat));
		}
	}
	else m_reaction->SetReverseRate(0);

	// add the reactants
	vector<int> solR, sbmR;
	m_reaction->GetSoluteReactants(solR);
	m_reaction->GetSBMReactants(sbmR);
	if ((solR != m_solR) || (sbmR != m_sbmR))
	{
		m_reaction->ClearReactants();

		// add the solute reactants
		for (int i = 0; i<(int)m_solR.size(); ++i)
		{
			FEReactantMaterial* ps = new FEReactantMaterial;
			ps->SetIndex(m_solR[i]);
			ps->SetReactantType(FEReactionMaterial::SOLUTE_SPECIES);
			m_reaction->AddReactantMaterial(ps);
		}

		// add the sbm reactants
		for (int i = 0; i<(int)m_sbmR.size(); ++i)
		{
			FEReactantMaterial* ps = new FEReactantMaterial;
			ps->SetIndex(m_sbmR[i]);
			ps->SetReactantType(FEReactionMaterial::SBM_SPECIES);
			m_reaction->AddReactantMaterial(ps);
		}
	}

	// add the products
	vector<int> solP, sbmP;
	m_reaction->GetSoluteProducts(solP);
	m_reaction->GetSBMProducts(sbmP);
	if ((solP != m_solP) || (sbmP != m_sbmP))
	{
		m_reaction->ClearProducts();

		// add the solute products
		for (int i = 0; i<(int)m_solP.size(); ++i)
		{
			FEProductMaterial* ps = new FEProductMaterial;
			ps->SetIndex(m_solP[i]);
			ps->SetProductType(FEReactionMaterial::SOLUTE_SPECIES);
			m_reaction->AddProductMaterial(ps);
		}

		// add the sbm products
		for (int i = 0; i<(int)m_sbmP.size(); ++i)
		{
			FEProductMaterial* ps = new FEProductMaterial;
			ps->SetIndex(m_sbmP[i]);
			ps->SetProductType(FEReactionMaterial::SBM_SPECIES);
			m_reaction->AddProductMaterial(ps);
		}
	}
}

void CDlgAddChemicalReaction::accept()
{
	// apply any changes
	hasChanged();
	apply();

	// call base class to close dialog
	QDialog::accept();
}
