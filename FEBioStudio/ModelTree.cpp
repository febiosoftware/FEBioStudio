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

#include "stdafx.h"
#include "ModelTree.h"
#include "ModelDocument.h"
#include <FEMLib/FSModel.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEAnalysisStep.h>
#include <FEMLib/FERigidLoad.h>
#include <QTreeWidgetItemIterator>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QHeaderView>
#include "ObjectProps.h"
#include "FEObjectProps.h"
#include "ModelViewer.h"
#include "PostDocument.h"
#include <GeomLib/GObject.h>
#include <PostGL/GLModel.h>
#include <ImageLib/ImageModel.h>
#include <PostLib/GLImageRenderer.h>
#include <QMessageBox>
#include <QtCore/QFileInfo>
#include <FEMLib/FELoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GGroup.h>
#include "MainWindow.h"
#include "SSHThread.h"
#include "SSHHandler.h"
#include "Logger.h"
#include "IconProvider.h"

// list of warnings generated
#define WARNING_NONE				0
#define WARNING_OBJECT_NOT_MESHED	1
#define WARNING_BC_NO_SELECTION		2
#define WARNING_BC_INVALID_REF		3
#define WARNING_NO_STEPS			4
#define WARNING_MAT_NOT_ASSIGNED	5	
#define WARNING_MAT_NO_PROPS		6
#define WARNING_CONTACT_INCOMPLETE	7
#define WARNING_JOB_NO_FEB			8
#define WARNING_LC_NOT_USED			9
#define WARNING_SEL_NOT_USED		10
#define WARNING_IMAGE_NO_LOAD		11
#define WARNING_DISCRETE_SET_EMPTY	12
#define WARNING_ZERO_SHELL_THICKNESS	13

// base class for object validators
// - define warning IDs (see list above)
// - override the IsValid and GetErrorString functions
// - override the GetWarningID function
class CObjectValidator
{
public:
	CObjectValidator() {}
	virtual ~CObjectValidator(){}

	virtual QString GetErrorString() const = 0;

	virtual bool Validate(FSObject* po) = 0;

	virtual unsigned int GetWarningID() const = 0;
};

template <class T> 
class CObjectValidator_T : public CObjectValidator
{
public:
	CObjectValidator_T() : m_po(nullptr) {}
	bool Validate(FSObject* po) override
	{
		m_po = dynamic_cast<T*>(po);
		if (m_po) return IsValid();
		else return false;
	}

protected:
	virtual bool IsValid() = 0;

protected:
	T* m_po;
};

class CGObjectValidator : public CObjectValidator_T<GObject>
{
public:
	CGObjectValidator() {}

	QString GetErrorString() const { 
		QString name = QString::fromStdString(m_po->GetName());
		return QString("\"%1\" is not meshed").arg(name);
	}

	bool IsValid()
	{
		if ((m_po == 0) || (m_po->GetFEMesh() == 0)) return false;
		return true;
	}

	unsigned int GetWarningID() const override { return WARNING_OBJECT_NOT_MESHED; };
};

class CPartValidator : public CObjectValidator_T<GPart>
{
public:
	CPartValidator() {}

	QString GetErrorString() const override {
		QString name = QString::fromStdString(m_po->GetName());
		return QString("Part \"%1\" has zero shell thickness").arg(name);
	}

	bool IsValid() override
	{
		if (m_po && m_po->IsShell())
		{
			GShellSection* section = dynamic_cast<GShellSection*>(m_po->GetSection());
			if (section == nullptr) return false;

			if (section->shellThickness() == 0) return false;
		}
		return true;
	}

	unsigned int GetWarningID() const override { return WARNING_ZERO_SHELL_THICKNESS; };
};

class CBCValidator : public CObjectValidator_T<FSDomainComponent>
{
public:
	CBCValidator() : m_err(0) {}

	QString GetErrorString() const 
	{ 
		QString name = QString::fromStdString(m_po->GetName());
		if      (m_err == 1) return QString("No selection assigned to bc \"%1\"").arg(name);
		else if (m_err == 2) return QString("Bc \"%1\" contains invalid references").arg(name);
		return "No problems";
	}

	bool IsValid()
	{
		m_err = 0;
		if (m_po == 0) { m_err = 1; return false; }
		FEItemListBuilder* item = m_po->GetItemList();
		if ((m_po->GetMeshItemType() != 0) &&
			((item==0) || (item->size() == 0))) { m_err = 1; return false; }
		else if (item && (item->IsValid() == false)) { m_err = 2; return false; }
		return true;
	}

	unsigned int GetWarningID() const override { 
		if (m_err == 1) return WARNING_BC_NO_SELECTION;
		if (m_err == 2) return WARNING_BC_INVALID_REF;
		return WARNING_NONE;
	};


private:
	int	m_err;
};

class CStepValidator : public CObjectValidator_T<FSModel>
{
public:
	CStepValidator() {}

	QString GetErrorString() const { return "No steps defined"; }

	bool IsValid()
	{
		if (m_po == 0) return false;
		int nsteps = m_po->Steps() - 1; // subtract one for initial step
		return (nsteps > 0);
	}

	unsigned int GetWarningID() const override { return WARNING_NO_STEPS; };
};

class CMaterialValidator : public CObjectValidator_T<GMaterial>
{
public:
	CMaterialValidator(FSModel* fem) : m_fem(fem), m_err(0) {}

	QString GetErrorString() const 
	{ 
		QString name = QString::fromStdString(m_po->GetName());
		if (m_err == 0) return "";
		if (m_err == 1) return QString("Material \"%1\" not assigned yet").arg(name); 
		if (m_err == 2) return QString("Material \"%1\" has no properties").arg(name);
		return "unknown error";
	}

	unsigned int GetWarningID() const override {
		if (m_err == 1) return WARNING_MAT_NOT_ASSIGNED;
		if (m_err == 2) return WARNING_MAT_NO_PROPS;
		return WARNING_NONE;
	};

	bool IsValid()
	{
		m_err = 0;
		if ((m_fem == 0) || (m_po == 0))
		{
			m_err = -1;
			return false;
		}

		if (m_po->GetMaterialProperties() == 0)
		{
			m_err = 2;
			return false;
		}

		GModel& mdl = m_fem->GetModel();
		for (int i=0; i<mdl.Objects(); ++i)
		{
			GObject* po = mdl.Object(i);
			for (int i=0; i<po->Parts(); ++i)
			{
				GPart& p = *po->Part(i);
				if (p.GetMaterialID() == m_po->GetID()) return true;
			}
		}

		m_err = 1;
		return false;
	}

private:
	FSModel*	m_fem;
	int			m_err;
};

class CContactValidator : public CObjectValidator_T<FSPairedInterface>
{
public:
	CContactValidator() {}

	QString GetErrorString() const { 
		QString name = QString::fromStdString(m_po->GetName());
		return QString("primary/secondary not specified for \"%1\"").arg(name);
	}

	bool IsValid()
	{
		if (m_po == 0) return true;
		FEItemListBuilder* surf1 = m_po->GetPrimarySurface();
		FEItemListBuilder* surf2 = m_po->GetSecondarySurface();
		if ((surf1 == 0) || (surf1->size() == 0)) return false;
		if ((surf2 == 0) || (surf2->size() == 0)) return false;
		return true;
	}

	unsigned int GetWarningID() const override { return WARNING_CONTACT_INCOMPLETE; };
};

class CJobValidator : public CObjectValidator_T<CFEBioJob>
{
public:
	CJobValidator() {}

	QString GetErrorString() const
	{
		std::string febFile = m_po->GetFEBFileName();
		if (febFile.empty()) return "";

		QFileInfo fi(QString::fromStdString(febFile));
		if (fi.exists() == false)
		{
			QString name = QString::fromStdString(m_po->GetName());
			return QString("Job \"%1\" : feb file does not exist.").arg(name);
		}
		else return "";
	}

	unsigned int GetWarningID() const override { return WARNING_JOB_NO_FEB; };

	bool IsValid()
	{
		std::string febFile = m_po->GetFEBFileName();
		if (febFile.empty()) return true;
		
		QFileInfo fi(QString::fromStdString(febFile));
		return fi.exists();
	}
};

class CLCValidator : public CObjectValidator_T<FSLoadController>
{
public:
	CLCValidator() {}

	QString GetErrorString() const override
	{
		if (m_po && m_po->GetReferenceCount() > 0) return "";
		QString name = QString::fromStdString(m_po->GetName());
		return QString("Load controller \"%1\" is not used.").arg(name);
	}

	unsigned int GetWarningID() const override { return WARNING_LC_NOT_USED; };

	bool IsValid() override
	{
		return (m_po && m_po->GetReferenceCount() > 0);
	}
};

class CGroupValidator : public CObjectValidator_T<FEItemListBuilder>
{
public:
	CGroupValidator() {}

	QString GetErrorString() const override
	{
		if (m_po && m_po->GetReferenceCount() > 0) return "";
		QString name = QString::fromStdString(m_po->GetName());
		return QString("Named selection \"%1\" is not used.").arg(name);
	}

	unsigned int GetWarningID() const override { return WARNING_SEL_NOT_USED; };

	bool IsValid() override
	{
		return (m_po && m_po->GetReferenceCount() > 0);
	}
};

class CImageModelValidator : public CObjectValidator_T<CImageModel>
{
public:
	CImageModelValidator() {}

	QString GetErrorString() const override
	{
		if ((m_po == nullptr) || (m_po->Get3DImage() == nullptr))
			return "Failed to load image data.";
		return "";
	}

	unsigned int GetWarningID() const override { return WARNING_IMAGE_NO_LOAD; };

	bool IsValid() override
	{
		return ((m_po != nullptr) && (m_po->Get3DImage() != nullptr));
	}
};

class CDiscreteSetValidator : public CObjectValidator_T<GDiscreteElementSet>
{
public:
	CDiscreteSetValidator() {}

	QString GetErrorString() const override
	{
		QString err;
		if (m_po ==nullptr) err = QString("nullptr!");
		else if (m_po->size() == 0)
			err = QString("Discrete element set \"%1\" is empty.").arg(QString::fromStdString(m_po->GetName()));
		return err;
	}

	unsigned int GetWarningID() const override { return WARNING_DISCRETE_SET_EMPTY; };

	bool IsValid() override
	{
		return (m_po && (m_po->size() > 0));
	}
};

//=============================================================================

CModelTree::CModelTree(CModelViewer* view, QWidget* parent) : QTreeWidget(parent), m_view(view), m_nfilter(0)
{
//	setAlternatingRowColors(true);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setColumnCount(1);
//	QHeaderView* head = header();
//	head->setStretchLastSection(true);
	setHeaderHidden(true);
}

void CModelTree::SetFilter(int n)
{
	m_nfilter = n;
}

CModelTreeItem* CModelTree::GetCurrentData()
{
	QTreeWidgetItem* item = currentItem();
	if (item == 0) return 0;

	int n = item->data(0, Qt::UserRole).toInt();
	if ((n>=0)&&(n<m_data.size()))
	{
		return &(m_data[n]);
	}
	else
	{
		assert(false);
		return 0;
	}
}

QTreeWidgetItem* CModelTree::FindItem(FSObject* o)
{
	QTreeWidgetItemIterator it(this);
	while (*it)
	{
		QVariant data = (*it)->data(0, Qt::UserRole);
		int index = data.toInt();
		CModelTreeItem& item = m_data[index];

		if (((item.flag & OBJECT_NOT_EDITABLE) == 0) && (item.obj == o))
		{
			return *it;
		}

		++it;
	}

	return 0;
}

bool CModelTree::GetSelection(std::vector<FSObject*>& objList)
{
	objList.clear();

	QList<QTreeWidgetItem*> sel = selectedItems();
	if (sel.empty()) return false;

	CModelTreeItem* data = GetCurrentData();
	if (data == 0)
	{
		QTreeWidgetItem* it = sel.at(0);
		int index = it->data(0, Qt::UserRole).toInt();
		data = &GetItem(index);
	}

	if (sel.size() == 1)
	{
		objList.push_back(data->obj);
		return true;
	}
	else
	{
		int ntype = data->type;
		if (ntype == 0) return false;

		// only show the context menu if all objects are the same type
		QList<QTreeWidgetItem*>::iterator it = sel.begin();
		while (it != sel.end())
		{
			int index = (*it)->data(0, Qt::UserRole).toInt();
			CModelTreeItem& di = GetItem(index);

			if (di.type != ntype) 
			{
				objList.clear();
				return false;
			}

			objList.push_back(di.obj);

			++it;
		}

		return true;
	}
}

void CModelTree::contextMenuEvent(QContextMenuEvent* ev)
{
	QPoint pt = ev->globalPos();

	// clear the selection
	m_view->ClearSelection();

	QList<QTreeWidgetItem*> sel = selectedItems();
	if (sel.empty()) return;

	CModelTreeItem* data = GetCurrentData();
	if (data == 0)
	{
		QTreeWidgetItem* it = sel.at(0);
		int index = it->data(0, Qt::UserRole).toInt();
		data = &GetItem(index);
	}

	if (sel.size() == 1)
	{
		m_view->SetSelection(data->obj);
		m_view->ShowContextMenu(data, pt);
	}
	else
	{
		int ntype = data->type;
		if (ntype == 0) return;

		// only show the context menu if all objects are the same type
		vector<FSObject*> objList;
		QList<QTreeWidgetItem*>::iterator it = sel.begin();
		while (it != sel.end())
		{
			int index = (*it)->data(0, Qt::UserRole).toInt();
			CModelTreeItem& di = GetItem(index);

			if (di.type != ntype) return;

			objList.push_back(di.obj);

			++it;
		}

		// okay, we should only get here if the type is the same for all types
		m_view->SetSelection(objList);
		m_view->ShowContextMenu(data, pt);
	}
}

QTreeWidgetItem* CModelTree::AddTreeItem(QTreeWidgetItem* parent, const QString& name, int ntype, int ncount, FSObject* po, int flags, const char* szicon)
{
	QTreeWidgetItem* t2 = (parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(this));

	QString txt;
	if (ncount == 0) txt = name;
	else txt = QString("%1 (%2)").arg(name).arg(ncount);
	t2->setText(0, txt);

	CObjectValidator* val = nullptr;
	if (m_props.find(ntype) != m_props.end()) val = m_props[ntype].val;

	if (val && (val->Validate(po) == false))
	{
		if (szicon) t2->setIcon(0, CIconProvider::GetIcon(szicon, Emblem::Caution));
		else t2->setIcon(0, QIcon(":/icons/warning.png"));

		t2->setToolTip(0, QString("<font color=\"black\">") + val->GetErrorString());
		if (parent) parent->setExpanded(true);
		if (m_view) m_view->IncWarningCount();
	}
	else
	{
		if (po && (po->GetInfo().empty() == false))
		{
			std::string s = po->GetInfo();
			if (s.size() > 40)
			{
				s.erase(s.begin() + 37, s.end());
				s.append("...");
			}
			if (szicon) t2->setIcon(0, CIconProvider::GetIcon(szicon, "info"));
			else t2->setIcon(0, QIcon(":/icons/info.png"));
			t2->setToolTip(0, QString::fromStdString(s));
		}
		else if (szicon)
		{
			t2->setIcon(0, CIconProvider::GetIcon(szicon));
		}
	}

	t2->setData(0, Qt::UserRole, (int)m_data.size());

	CModelTreeItem it = { po, flags, ntype, szicon };
	m_data.push_back(it);

	return t2;
}

void CModelTree::ClearData()
{
	m_data.clear();

	for (auto it : m_props)
	{
		delete it.second.props;
		delete it.second.val;
	}
	m_props.clear();
}

QStringList CModelTree::GetAllWarnings()
{
	QStringList errs;
	for (int i = 0; i < m_data.size(); ++i)
	{
		CModelTreeItem& it = m_data[i];

		if (m_props.find(it.type) != m_props.end())
		{
			CObjectValidator* val = m_props[it.type].val;
			if (val)
			{
				if (val->Validate(it.obj) == false)
				{
					uint id = val->GetWarningID();
					QString msg = QString("W%1 : %2").arg(id, 3, 10, QChar('0')).arg(val->GetErrorString());
					errs.push_back(msg);
				}
			}
		}
	}
	return errs;
}

void CModelTree::UpdateItem(QTreeWidgetItem* item)
{
	if (item == nullptr) return;

	QVariant data = item->data(0, Qt::UserRole);
	int n = data.toInt();

	FSObject* po = m_data[n].obj;
	if (po)
	{
		FSStepComponent* pc = dynamic_cast<FSStepComponent*>(po);
		if (pc)
		{
			QFont font = item->font(0);
			font.setItalic(pc->IsActive() == false);
			item->setFont(0, font);
		}

		Post::CGLPlot* plot = dynamic_cast<Post::CGLPlot*>(po);
		if (plot)
		{
			QFont font = item->font(0);
			font.setItalic(plot->IsActive() == false);
			item->setFont(0, font);
		}

		GDiscreteObject* pdo = dynamic_cast<GDiscreteObject*>(po);
		if (pdo)
		{
			QFont font = item->font(0);
			font.setItalic(pdo->IsActive() == false);
			item->setFont(0, font);
		}

		GPart* pg = dynamic_cast<GPart*>(po);
		if (pg)
		{
			QFont font = item->font(0);
			font.setItalic(pg->IsActive() == false);
			item->setFont(0, font);
		}
	}

	CObjectValidator* val = nullptr;
	if (m_props.find(m_data[n].type) != m_props.end())
	{
		val = m_props[m_data[n].type].val;
	}

	if (val)
	{
		if (val->Validate(po) == false)
		{
			if (dynamic_cast<GMaterial*>(po))
			{
				GMaterial* m = dynamic_cast<GMaterial*>(po);
				QIcon icon = CIconProvider::BuildPixMap(toQColor(m->Diffuse()), ::Shape::Circle, 24);
				item->setIcon(0, CIconProvider::CreateIcon(icon, Emblem::Caution));
			}
			else
			{
				if (m_data[n].szicon) item->setIcon(0, CIconProvider::GetIcon(m_data[n].szicon, Emblem::Caution));
				else item->setIcon(0, QIcon(":/icons/warning.png"));
			}

			item->setToolTip(0, QString("<font color=\"black\">") + val->GetErrorString());
			return;
		}
	}

	if (po && (po->GetInfo().empty() == false))
	{
		std::string s = po->GetInfo();
		if (s.size() > 40)
		{
			s.erase(s.begin() + 37, s.end());
			s.append("...");
		}
		if (m_data[n].szicon) item->setIcon(0, CIconProvider::GetIcon(m_data[n].szicon, "info"));
		else item->setIcon(0, QIcon(":/icons/info.png"));
		item->setToolTip(0, QString::fromStdString(s));
	}
	else
	{
		if (dynamic_cast<GMaterial*>(po))
		{
			GMaterial* m = dynamic_cast<GMaterial*>(po);
			item->setIcon(0, CIconProvider::BuildPixMap(toQColor(m->Diffuse()), ::Shape::Circle, 24));
		}
		else if (m_data[n].szicon)
		{
			item->setIcon(0, CIconProvider::GetIcon(m_data[n].szicon));
			item->setToolTip(0, QString());
		}
		else
		{
			item->setIcon(0, QIcon());
			item->setToolTip(0, QString());
		}
	}
}

void CModelTree::UpdateObject(FSObject* po)
{
	// make sure there is something to update
	if (po == nullptr) return;

	// Often, this will be the currently active object,
	// so check that first, before doing a potentially expensive search
	QTreeWidgetItem* current = currentItem();
	if (current)
	{
		QVariant data = current->data(0, Qt::UserRole);
		int n = data.toInt();
		if (m_data[n].obj == po)
		{
			UpdateItem(current);
			return;
		}
	}

	// Ok, it was not the current object, so let's do a more extensive search
	QTreeWidgetItemIterator it(this);
	while (*it)
	{
		QVariant data = (*it)->data(0, Qt::UserRole);
		int n = data.toInt();
		if (m_data[n].obj == po)
		{
			UpdateItem(*it);
			return;
		}
		++it;
	}
}

void CModelTree::ShowItem(QTreeWidgetItem* item)
{
	QTreeWidgetItem* parent = item->parent();
	while (parent)
	{
		if (parent->isExpanded() == false) parent->setExpanded(true);
		parent = parent->parent();
	}
}

void CModelTree::Select(FSObject* po)
{
	if (po == 0) { clearSelection(); return; }

	QTreeWidgetItemIterator it(this);
	while (*it)
	{
		QVariant data = (*it)->data(0, Qt::UserRole);
		int n = data.toInt();
		if (m_data[n].obj == po)
		{
			ShowItem(*it);
			this->setCurrentItem(*it);
			return;
		}
		++it;
	}

//	assert((false) || (m_nfilter != 0));
}

void CModelTree::Select(const std::vector<FSObject*>& objList)
{
	clearSelection();
	m_view->SetCurrentItem(-1);

	int N = (int)objList.size();
	if (N == 0) return;

	QTreeWidgetItemIterator it(this);
	while (*it)
	{
		QVariant data = (*it)->data(0, Qt::UserRole);
		int index = data.toInt();
		CModelTreeItem& item = m_data[index];

		for (int i=0; i<N; ++i)
		{
			if ((item.obj == objList[i]) && ((item.flag & OBJECT_NOT_EDITABLE) == 0))
			{
				ShowItem(*it);
				(*it)->setSelected(true);
				break;
			}
		}
		++it;
	}
}

void CModelTree::Build(CModelDocument* doc)
{
	// clear the tree
	clear();
	ClearData();

	if (doc == nullptr) return;

	BuildPropertyLists(doc);

	// get the model
	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	GModel& mdl = fem.GetModel();

	std::string modelName = doc->GetDocFileBase();
	if (modelName.empty()) modelName = "Model";

	std::string moduleName = prj.GetModuleName();

	if      (m_nfilter == ModelTreeFilter::FILTER_GEOMETRY ) modelName += " > Geometry";
	else if (m_nfilter == ModelTreeFilter::FILTER_MATERIALS) modelName += " > Materials";
	else if (m_nfilter == ModelTreeFilter::FILTER_PHYSICS  ) modelName += " > Physics";
	else if (m_nfilter == ModelTreeFilter::FILTER_STEPS    ) modelName += " > Steps";
	else if (m_nfilter == ModelTreeFilter::FILTER_JOBS     ) modelName += " > Jobs";
    else if (m_nfilter == ModelTreeFilter::FILTER_IMAGES     ) modelName += " > Images";

	QTreeWidgetItem* t1 = nullptr;

	if (m_nfilter == ModelTreeFilter::FILTER_NONE)
	{
		t1 = AddTreeItem(nullptr, "Geometry", 0, 0, &mdl, OBJECT_NOT_EDITABLE);
		t1->setExpanded(true);
		QFont f = t1->font(0);
		f.setBold(true);
		t1->setFont(0, f);
	}

	// add the objects
	QTreeWidgetItem* t2;
	if ((m_nfilter == ModelTreeFilter::FILTER_NONE) || (m_nfilter == ModelTreeFilter::FILTER_GEOMETRY))
	{
		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t2 = AddTreeItem(t1, "Objects", MT_OBJECT_LIST, mdl.Objects());
			UpdateObjects(t2, fem);
		}
		else UpdateObjects(t1, fem);
	}

	// add the groups
	if (m_nfilter == ModelTreeFilter::FILTER_NONE)
	{
		int nsel = mdl.CountNamedSelections();
		t2 = AddTreeItem(t1, "Named Selections", MT_NAMED_SELECTION, nsel);
		UpdateGroups(t2, fem);
	}

	if (m_nfilter == ModelTreeFilter::FILTER_NONE)
	{
		QString s = QString("Model [%1]").arg(QString::fromStdString(moduleName));
		t1 = AddTreeItem(nullptr, s, 0, 0, &mdl, OBJECT_NOT_EDITABLE);
		t1->setExpanded(true);
		QFont f = t1->font(0);
		f.setBold(true);
		t1->setFont(0, f);
	}

	// add data variables
	if (m_nfilter == ModelTreeFilter::FILTER_NONE)
	{
		t2 = AddTreeItem(t1, "Globals", MT_FEMODEL, 0, 0);
		UpdateModelData(t2, fem);
	}

	// add the materials
	if ((m_nfilter == ModelTreeFilter::FILTER_NONE)||(m_nfilter == ModelTreeFilter::FILTER_MATERIALS))
	{
		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t2 = AddTreeItem(t1, "Materials", MT_MATERIAL_LIST, fem.Materials());
			UpdateMaterials(t2, fem);
		}
		else UpdateMaterials(t1, fem);
	}

	if (m_nfilter == ModelTreeFilter::FILTER_NONE)
	{
		// count mesh data fields. 
		int n = fem.CountMeshDataFields();
		// Mesh data
		t2 = AddTreeItem(t1, "Mesh Data", MT_MESH_DATA_LIST, n);
		UpdateMeshData(t2, fem);
	}

	// add the boundary conditions
	if ((m_nfilter == ModelTreeFilter::FILTER_NONE) || (m_nfilter == ModelTreeFilter::FILTER_PHYSICS))
	{
		int nbc = 0;
		for (int i = 0; i < fem.Steps(); ++i) nbc += fem.GetStep(i)->BCs();

		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t2 = AddTreeItem(t1, "Boundary Conditions", MT_BC_LIST, nbc);
			UpdateBC(t2, fem, 0);
		}
		else if (nbc)
		{
			UpdateBC(t1, fem, 0);
		}
	}

	// add the boundary loads
	if ((m_nfilter == ModelTreeFilter::FILTER_NONE) || (m_nfilter == ModelTreeFilter::FILTER_PHYSICS))
	{
		int nload = 0;
		for (int i = 0; i < fem.Steps(); ++i) nload += fem.GetStep(i)->Loads();

		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t2 = AddTreeItem(t1, "Loads", MT_LOAD_LIST, nload);
			UpdateLoads(t2, fem, 0);
		}
		else if (nload > 0) UpdateLoads(t1, fem, 0);
	}

	if ((m_nfilter == ModelTreeFilter::FILTER_NONE) || (m_nfilter == ModelTreeFilter::FILTER_PHYSICS))
	{
		// add the initial conditions
		int nic = 0;
		for (int i = 0; i < fem.Steps(); ++i) nic += fem.GetStep(i)->ICs();

		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t2 = AddTreeItem(t1, "Initial Conditions", MT_IC_LIST, nic);
			UpdateICs(t2, fem, 0);
		}
		else if (nic) UpdateICs(t1, fem, 0);
	}

	if ((m_nfilter == ModelTreeFilter::FILTER_NONE) || (m_nfilter == ModelTreeFilter::FILTER_PHYSICS))
	{
		// add the interfaces
		int nint = 0;
		for (int i = 0; i < fem.Steps(); ++i) nint += fem.GetStep(i)->Interfaces();

		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t2 = AddTreeItem(t1, "Contact", MT_CONTACT_LIST, nint);
			UpdateContact(t2, fem, 0);
		}
		else if (nint) UpdateContact(t1, fem, 0);
	}

	if ((m_nfilter == ModelTreeFilter::FILTER_NONE) || (m_nfilter == ModelTreeFilter::FILTER_PHYSICS))
	{
		// add the nonlinear constraints
		int nlc = 0;
		for (int i = 0; i < fem.Steps(); ++i) nlc += fem.GetStep(i)->Constraints();

		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t2 = AddTreeItem(t1, "Constraints", MT_CONSTRAINT_LIST, nlc);
			UpdateConstraints(t2, fem, 0);
		}
		else if (nlc) UpdateConstraints(t1, fem, 0);
	}

	if ((m_nfilter == ModelTreeFilter::FILTER_NONE) || (m_nfilter == ModelTreeFilter::FILTER_PHYSICS))
	{
		// add the rigid components
		int nnlc = 0;
		for (int i = 0; i < fem.Steps(); ++i)
		{
			nnlc += fem.GetStep(i)->RigidBCs();
			nnlc += fem.GetStep(i)->RigidICs();
			nnlc += fem.GetStep(i)->RigidLoads();
			nnlc += fem.GetStep(i)->RigidConnectors();
		}

		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t2 = AddTreeItem(t1, "Rigid", MT_RIGID_COMPONENT_LIST, nnlc);
			UpdateRigid(t2, fem, 0);
		}
		else if (nnlc) UpdateRigid(t1, fem, 0);
	}

	// add the discrete objects
	if ((m_nfilter == ModelTreeFilter::FILTER_NONE) || (m_nfilter == ModelTreeFilter::FILTER_PHYSICS))
	{
		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t2 = AddTreeItem(t1, "Discrete", MT_DISCRETE_LIST, mdl.DiscreteObjects());
			UpdateDiscrete(t2, fem);
		}
		else if (mdl.DiscreteObjects()) UpdateDiscrete(t1, fem);
	}

	if (m_nfilter == ModelTreeFilter::FILTER_NONE)
	{
		// Mesh adaptors
		t2 = AddTreeItem(t1, "Mesh Adaptors", MT_MESH_ADAPTOR_LIST);
		UpdateMeshAdaptors(t2, fem, 0);
	}

	// add the steps
	if ((m_nfilter == ModelTreeFilter::FILTER_NONE) || (m_nfilter == ModelTreeFilter::FILTER_STEPS))
	{
		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t2 = AddTreeItem(t1, "Steps", MT_STEP_LIST, fem.Steps() - 1);
			UpdateSteps(t2, prj);
		}
		else if (fem.Steps()) UpdateSteps(t1, prj);
	}

	// add load controllers
	if (m_nfilter == ModelTreeFilter::FILTER_NONE)
	{
		t2 = AddTreeItem(t1, "Load Controllers", MT_LOAD_CONTROLLERS);
		UpdateLoadControllers(t2, fem);
	}

	// add the output
	if (m_nfilter == ModelTreeFilter::FILTER_NONE)
	{
		t2 = AddTreeItem(t1, "Output", MT_PROJECT_OUTPUT);
		UpdateOutput(t2, prj);
	}

	// add the jobs
	if ((m_nfilter == ModelTreeFilter::FILTER_NONE) || (m_nfilter == ModelTreeFilter::FILTER_JOBS))
	{
		if (m_nfilter == ModelTreeFilter::FILTER_NONE)
		{
			t1 = AddTreeItem(nullptr, "Jobs", MT_JOBLIST, doc->FEBioJobs(), nullptr, OBJECT_NOT_EDITABLE);
			t1->setExpanded(true);
			QFont f = t1->font(0);
			f.setBold(true);
			t1->setFont(0, f);

			UpdateJobs(t1, doc);
		}
		else
			UpdateJobs(t1, doc);
	}

	if (m_nfilter == ModelTreeFilter::FILTER_NONE || (m_nfilter == ModelTreeFilter::FILTER_IMAGES))
	{
		// add the image stacks
		if (doc->ImageModels())
		{
			QTreeWidgetItem* t1 = new QTreeWidgetItem(this);
			t1->setText(0, "3D Images");
			t1->setExpanded(true);
			t1->setData(0, Qt::UserRole, (int)m_data.size());

			QFont f = t1->font(0);
			f.setBold(true);
			t1->setFont(0, f);

			CModelTreeItem it = { 0, 0 };
			m_data.push_back(it);

			UpdateImages(t1, doc);
		}
	}

//	resizeColumnToContents(0);
}

void CModelTree::BuildPropertyLists(CModelDocument* doc)
{
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	GModel& mdl = fem.GetModel();

	m_props[MT_JOB                ] = { new CFEBioJobProps(m_view->GetMainWindow(), m_view), new CJobValidator()};
	m_props[MT_3DIMAGE            ] = { new CImageModelProperties(), new CImageModelValidator()};
	m_props[MT_IMGANALYSIS        ] = { new CFSObjectProps(), nullptr };
	m_props[MT_SOLUTE             ] = { new CFSObjectProps(), nullptr };
	m_props[MT_SBM                ] = { new CFSObjectProps(), nullptr };
	m_props[MT_DISCRETE_SET       ] = { new CDiscreteObjectProps(), new CDiscreteSetValidator()};
	m_props[MT_OBJECT             ] = { nullptr, new CGObjectValidator()};
	m_props[MT_PART               ] = { new CPartProperties(fem), new CPartValidator()};
	m_props[MT_PART_GROUP         ] = { nullptr, new CGroupValidator()};
	m_props[MT_FACE_GROUP         ] = { nullptr, new CGroupValidator()};
	m_props[MT_EDGE_GROUP         ] = { nullptr, new CGroupValidator()};
	m_props[MT_NODE_GROUP         ] = { nullptr, new CGroupValidator()};
	m_props[MT_FENODE_GROUP       ] = { nullptr, new CGroupValidator()};
	m_props[MT_FEFACE_GROUP       ] = { nullptr, new CGroupValidator()};
	m_props[MT_FEEDGE_GROUP       ] = { nullptr, new CGroupValidator()};
	m_props[MT_FEELEM_GROUP       ] = { nullptr, new CGroupValidator()};
	m_props[MT_FEPART_GROUP       ] = { nullptr, new CGroupValidator()};
	m_props[MT_FEMODEL            ] = { new FSGlobalsProps(&fem), nullptr};
	m_props[MT_MESH_DATA_GENERATOR] = { new CFSObjectProps(), nullptr};
	m_props[MT_LOAD               ] = { new CFSObjectProps(&fem), new CBCValidator()};
	m_props[MT_CONSTRAINT         ] = { new CFSObjectProps(&fem), new CBCValidator() };
	m_props[MT_RIGID_BC           ] = { new CFSObjectProps(&fem), nullptr };
	m_props[MT_RIGID_IC           ] = { new CFSObjectProps(&fem), nullptr };
	m_props[MT_RIGID_LOAD         ] = { new CFSObjectProps(&fem), nullptr };
	m_props[MT_RIGID_CONNECTOR    ] = { new CRigidConnectorSettings(&fem), nullptr };
	m_props[MT_LOAD_CONTROLLER    ] = { nullptr, new CLCValidator() };
	m_props[MT_PROJECT_OUTPUT_PLT ] = { new CPlotfileProperties(m_view, prj), nullptr };
	m_props[MT_PROJECT_OUTPUT_LOG ] = { new CLogfileProperties (m_view, prj), nullptr };
	m_props[MT_MATERIAL           ] = { new CMaterialProps(&fem), new CMaterialValidator(&fem) };
	m_props[MT_MATERIAL_REACTANTS ] = { new CReactionReactantProperties(&fem), nullptr };
	m_props[MT_MATERIAL_PRODUCTS  ] = { new CReactionProductProperties (&fem), nullptr };
	m_props[MT_CONTACT            ] = { new CFSObjectProps(&fem), new CContactValidator()};
	m_props[MT_IC                 ] = { new CFSObjectProps(&fem), new CBCValidator()};
	m_props[MT_BC                 ] = { new CFSObjectProps(&fem), new CBCValidator()};
	m_props[MT_STEP               ] = { new CStepSettings(prj), nullptr };
}

void CModelTree::UpdateJobs(QTreeWidgetItem* t1, CModelDocument* doc)
{
	for (int i=0; i<doc->FEBioJobs(); ++i)
	{
		CFEBioJob* job = doc->GetFEBioJob(i);
		QString name = QString::fromStdString(job->GetName());
		if (job->GetStatus() == CFEBioJob::RUNNING) name += " [RUNNING]";
		QTreeWidgetItem* t2 = AddTreeItem(t1, name, MT_JOB, 0, job, SHOW_PROPERTY_FORM);
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateImages(QTreeWidgetItem* t1, CModelDocument* doc)
{
	for (int i = 0; i < doc->ImageModels(); ++i)
	{
		CImageModel* img = doc->GetImageModel(i);
		QTreeWidgetItem* t2 = AddTreeItem(t1, QString::fromStdString(img->GetName()), MT_3DIMAGE, 0, img);

        for(int j = 0; j < img->ImageAnalyses(); j++)
        {
            AddTreeItem(t2, QString::fromStdString(img->GetImageAnalysis(j)->GetName()), MT_IMGANALYSIS, 0, img->GetImageAnalysis(j));
        }
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateModelData(QTreeWidgetItem* t1, FSModel& fem)
{
	int NSOL = fem.Solutes();
	if (NSOL > 0)
	{
		QTreeWidgetItem* t2 = AddTreeItem(t1, "Solutes", MT_SOLUTES_LIST, NSOL);
		for (int i=0; i<NSOL; ++i)
		{
			SoluteData& s = fem.GetSoluteData(i);
			AddTreeItem(t2, QString::fromStdString(s.GetName()), MT_SOLUTE, 0, &s);
		}	
	}

	int NSBM = fem.SBMs();
	if (NSBM > 0)
	{
		QTreeWidgetItem* t2 = AddTreeItem(t1, "Solid-bound Molecules", MT_SBM_LIST, NSBM);
		for (int i = 0; i<NSBM; ++i)
		{
			SoluteData& s = fem.GetSBMData(i);
			AddTreeItem(t2, QString::fromStdString(s.GetName()), MT_SBM, 0, &s);
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateDiscrete(QTreeWidgetItem* t1, FSModel& fem)
{
	GModel& model = fem.GetModel();
	for (int i = 0; i<model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);
		if (dynamic_cast<GDiscreteSpringSet*>(po))
		{
			GDiscreteSpringSet* pg = dynamic_cast<GDiscreteSpringSet*>(po);

			QTreeWidgetItem* t2 = nullptr;
			t2 = AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_DISCRETE_SET, pg->size(), pg);

			if (t2 && !pg->IsActive())
			{
				QFont font = t2->font(0);
				font.setItalic(true);
				t2->setFont(0, font);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateObjects(QTreeWidgetItem* t1, FSModel& fem)
{
	QTreeWidgetItem* t2, *t3, *t4;

	// max nr. of items in a branch
	const int MAX_BRANCH_ITEM = 10000;

	// get the model
	GModel& model = fem.GetModel();

	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);

		t2 = AddTreeItem(t1, QString::fromStdString(po->GetName()), MT_OBJECT, 0, po);

		if (po->IsVisible() == false) t2->setForeground(0, Qt::gray);

		t3 = AddTreeItem(t2, "Parts", MT_PART_LIST, po->Parts(), po);
		for (int j = 0; j<po->Parts(); ++j)
		{
			GPart* pg = po->Part(j);

			QString name = QString::fromStdString(pg->GetName());
			if      (pg->IsSolid()) name += " [solid]";
			else if (pg->IsShell()) name += " [shell]";
			else if (pg->IsBeam ()) name += " [beam]";
			else name += " []";

			t4 = AddTreeItem(t3, name, MT_PART, 0, pg, 1);

			if (pg->IsVisible() == false)
			{
				t4->setForeground(0, Qt::gray);
			}

			if (!pg->IsActive())
			{
				QFont font = t4->font(0);
				font.setItalic(true);
				t4->setFont(0, font);
			}
		}
		t3->setExpanded(false);

		t3 = AddTreeItem(t2, "Surfaces", MT_FACE_LIST, po->Faces(), po);
		int NF = po->Faces();
		if (NF > MAX_BRANCH_ITEM) NF = MAX_BRANCH_ITEM;
		for (int j = 0; j<NF; ++j)
		{
			GFace* pg = po->Face(j);
			t4 = AddTreeItem(t3, QString::fromStdString(pg->GetName()), MT_SURFACE, 0, pg);
			if (pg->IsVisible() == false) t4->setForeground(0, Qt::gray);
		}
		t3->setExpanded(false);

		t3 = AddTreeItem(t2, "Edges", MT_EDGE_LIST, po->Edges(), po);
		int NE = po->Edges();
		if (NE > MAX_BRANCH_ITEM) NE = MAX_BRANCH_ITEM;
		for (int j=0; j<NE; ++j)
		{
			GEdge* pg = po->Edge(j);
			t4 = AddTreeItem(t3, QString::fromStdString(pg->GetName()), MT_EDGE, 0, pg);
			if (pg->IsVisible() == false) t4->setForeground(0, Qt::gray);
		}
		t3->setExpanded(false);

		t3 = AddTreeItem(t2, "Nodes", MT_NODE_LIST, po->Nodes(), po, OBJECT_NOT_EDITABLE);
		int NN = po->Nodes();
		if (NN > MAX_BRANCH_ITEM) NN = MAX_BRANCH_ITEM;
		for (int j = 0; j<NN; ++j)
		{
			GNode* pg = po->Node(j);
			if ((pg->Type() == 0) || (pg->Type() == NODE_VERTEX))
			{
				t4 = AddTreeItem(t3, QString::fromStdString(pg->GetName()), MT_NODE, 0, pg);
				if (pg->IsVisible() == false) t4->setForeground(0, Qt::gray);
			}
		}
		t3->setExpanded(false);

		t2->setExpanded(false);
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateGroups(QTreeWidgetItem* t1, FSModel& fem)
{
	// get the model
	GModel& model = fem.GetModel();

	// add the groups
	int gparts = model.PartLists();
	for (int j = 0; j<gparts; ++j)
	{
		GPartList* pg = model.PartList(j);
		int n = pg->GetReferenceCount(); if (n < 2) n = 0;
		AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_PART_GROUP, n, pg, 0, "selectPart");
	}

	int gsurfs = model.FaceLists();
	for (int j = 0; j<gsurfs; ++j)
	{
		GFaceList* pg = model.FaceList(j);
		int n = pg->GetReferenceCount(); if (n < 2) n = 0;
		AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_FACE_GROUP, n, pg, 0, "selectSurface");
	}

	int gedges = model.EdgeLists();
	for (int j = 0; j<gedges; ++j)
	{
		GEdgeList* pg = model.EdgeList(j);
		int n = pg->GetReferenceCount(); if (n < 2) n = 0;
		AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_EDGE_GROUP, n, pg, 0, "selectCurves");
	}

	int gnodes = model.NodeLists();
	for (int j = 0; j<gnodes; ++j)
	{
		GNodeList* pg = model.NodeList(j);
		int n = pg->GetReferenceCount(); if (n < 2) n = 0;
		AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_NODE_GROUP, n, pg, 0, "selectNodes");
	}

	// add the mesh groups
	// a - node sets
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int nsets = po->FENodeSets();
			for (int j = 0; j<nsets; ++j)
			{
				FSNodeSet* pg = po->GetFENodeSet(j);
				int n = pg->GetReferenceCount(); if (n < 2) n = 0;
				AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_FENODE_GROUP, n, pg, 0, "selNode");
			}
		}
	}

	// b - surfaces
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int surfs = po->FESurfaces();
			for (int j = 0; j<surfs; ++j)
			{
				FSSurface* pg = po->GetFESurface(j);
				int n = pg->GetReferenceCount(); if (n < 2) n = 0;
				AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_FEFACE_GROUP, n, pg, 0, "selFace");
			}
		}
	}

	// c - edges
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int edges = po->FEEdgeSets();
			for (int j = 0; j<edges; ++j)
			{
				FSEdgeSet* pg = po->GetFEEdgeSet(j);
				int n = pg->GetReferenceCount(); if (n < 2) n = 0;
				AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_FEEDGE_GROUP, n, pg, 0, "selEdge");
			}
		}
	}

	// d - element sets
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int parts = po->FEElemSets();
			for (int j = 0; j<parts; ++j)
			{
				FSElemSet* pg = po->GetFEElemSet(j);
				int n = pg->GetReferenceCount(); if (n < 2) n = 0;
				AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_FEELEM_GROUP, n, pg, 0, "selElem");
			}
		}
	}

	// d - part sets
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int parts = po->FEPartSets();
			for (int j = 0; j < parts; ++j)
			{
				FSPartSet* pg = po->GetFEPartSet(j);
				int n = pg->GetReferenceCount(); if (n < 2) n = 0;
				AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_FEPART_GROUP, n, pg, 0, "selElem");
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateMeshData(QTreeWidgetItem* t1, FSModel& fem)
{
	GModel& mdl = fem.GetModel();
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		FSMesh* mesh = po->GetFEMesh();
		if (mesh)
		{
			for (int j = 0; j < mesh->MeshDataFields(); ++j)
			{
				FEMeshData& data = *mesh->GetMeshDataField(j);
				AddTreeItem(t1, QString::fromStdString(data.GetName()), MT_MESH_DATA, 0, &data);
			}
		}
	}

	for (int i = 0; i < fem.MeshDataGenerators(); ++i)
	{
		FSMeshDataGenerator* map = fem.GetMeshDataGenerator(i);
		AddTreeItem(t1, QString::fromStdString(map->GetName()), MT_MESH_DATA_GENERATOR, 0, map);
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateMeshAdaptors(QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep)
{
	QTreeWidgetItem* t2;

	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		if ((pstep == 0) || (ps == pstep))
		{
			for (int j = 0; j < ps->MeshAdaptors(); ++j)
			{
				FSMeshAdaptor* pma = ps->MeshAdaptor(j);
				assert(pma->GetStep() == ps->GetID());

				int flags = SHOW_PROPERTY_FORM;
				if (pstep == 0) flags |= DUPLICATE_ITEM;
				QString name = QString("%1 [%2]").arg(QString::fromStdString(pma->GetName())).arg(pma->GetTypeString());
				t2 = AddTreeItem(t1, name, MT_MESH_ADAPTOR, 0, pma, flags);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateBC(QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep)
{
	QTreeWidgetItem* t2;

	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		if ((pstep == 0) || (ps == pstep))
		{
			for (int j = 0; j<ps->BCs(); ++j)
			{
				FSBoundaryCondition* pbc = ps->BC(j);
				assert(pbc->GetStep() == ps->GetID());

				int flags = SHOW_PROPERTY_FORM;
				if (pstep == 0) flags |= DUPLICATE_ITEM;
				QString name = QString("%1 [%2]").arg(QString::fromStdString(pbc->GetName())).arg(pbc->GetTypeString());
				t2 = AddTreeItem(t1, name, MT_BC, 0, pbc, flags);
			}
		}
	}
}

//----------------------------------------------------------------------------
void CModelTree::UpdateLoads(QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep)
{
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		if ((pstep == 0) || (ps == pstep))
		{
			for (int j = 0; j<ps->Loads(); ++j)
			{
				FSLoad* pfc = ps->Load(j);
				assert(pfc->GetStep() == ps->GetID());

				int flags = SHOW_PROPERTY_FORM;
				if (pstep == 0) flags |= DUPLICATE_ITEM;
				QString name = QString("%1 [%2]").arg(QString::fromStdString(pfc->GetName())).arg(pfc->GetTypeString());
				QTreeWidgetItem* t2 = AddTreeItem(t1, name, MT_LOAD, 0, pfc, flags);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateICs(QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep)
{
	QTreeWidgetItem* t2;

	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		if ((pstep == 0) || (ps == pstep))
		{
			for (int j = 0; j<ps->ICs(); ++j)
			{
				FSInitialCondition* pic = ps->IC(j);
				assert(pic->GetStep() == i);

				int flags = SHOW_PROPERTY_FORM;
				if (pstep == 0) flags |= DUPLICATE_ITEM;
				QString name = QString("%1 [%2]").arg(QString::fromStdString(pic->GetName())).arg(pic->GetTypeString());
				t2 = AddTreeItem(t1, QString::fromStdString(pic->GetName()), MT_IC, 0, pic, flags);
			}
		}
	}
}

void setInactive(QTreeWidgetItem* ti)
{
	QFont f = ti->font(0);
	f.setItalic(true);
	ti->setFont(0, f);
}

void CModelTree::UpdateContact(QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep)
{
	for (int n = 0; n<fem.Steps(); ++n)
	{
		FSStep* ps = fem.GetStep(n);
		if ((pstep == 0) || (pstep == ps))
		{
			int flags = 0;
			if (pstep == 0) flags |= DUPLICATE_ITEM;

			for (int i = 0; i<ps->Interfaces(); ++i)
			{
				FSInterface* pi = ps->Interface(i);
				if (pi)
				{
					QString name = QString("%1 [%2]").arg(QString::fromStdString(pi->GetName())).arg(pi->GetTypeString());
					QTreeWidgetItem* t2 = AddTreeItem(t1, name, MT_CONTACT, 0, pi, flags);
					if (pi->IsActive() == false) setInactive(t2);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateConstraints(QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep)
{
	QTreeWidgetItem* t2;
	for (int n = 0; n<fem.Steps(); ++n)
	{
		FSStep* ps = fem.GetStep(n);
		if ((pstep == 0) || (pstep == ps))
		{
			int flags = 0;
			if (pstep == 0) flags |= DUPLICATE_ITEM;

			// add constraints
			for (int i = 0; i<ps->Constraints(); ++i)
			{
				FSModelConstraint* pc = ps->Constraint(i);
				if (pc)
				{
					QString name = QString("%1 [%2]").arg(QString::fromStdString(pc->GetName())).arg(pc->GetTypeString());
					t2 = AddTreeItem(t1, name, MT_CONSTRAINT, 0, pc, flags);
					if (pc->IsActive() == false) setInactive(t2);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateSteps(QTreeWidgetItem* t1, FSProject& prj)
{
	QTreeWidgetItem* t2, *t3;

	FSModel& fem = prj.GetFSModel();

	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);

		QString name = (i==0 ? QString::fromStdString(pstep->GetName()) : QString("%1 [%2]").arg(QString::fromStdString(pstep->GetName())).arg(pstep->GetTypeString()));

		// add control settings
		t2 = AddTreeItem(t1, name, MT_STEP, 0, pstep, 1);

		// add the boundary conditions
		t3 = AddTreeItem(t2, "Boundary Conditions", MT_BC_LIST, pstep->BCs());
		UpdateBC(t3, fem, pstep);

		// add the loads
		t3 = AddTreeItem(t2, "Loads", MT_LOAD_LIST, pstep->Loads());
		UpdateLoads(t3, fem, pstep);

		// add the initial conditions
		t3 = AddTreeItem(t2, "Initial Conditions", MT_IC_LIST, pstep->ICs());
		UpdateICs(t3, fem, pstep);

		// add the interfaces
		t3 = AddTreeItem(t2, "Contact", MT_CONTACT_LIST, pstep->Interfaces());
		UpdateContact(t3, fem, pstep);

		// add the nonlinear constraints
		t3 = AddTreeItem(t2, "Constraints", MT_CONSTRAINT_LIST, pstep->Constraints());
		UpdateConstraints(t3, fem, pstep);

		// add the constraints
		t3 = AddTreeItem(t2, "Rigid", MT_RIGID_COMPONENT_LIST);
		UpdateRigid(t3, fem, pstep);

		// add the mesh adaptors
		t3 = AddTreeItem(t2, "Mesh Adaptors", MT_MESH_ADAPTOR_LIST, pstep->MeshAdaptors());
		UpdateMeshAdaptors(t3, fem, pstep);
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateRigid(QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep)
{
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		if ((pstep == 0) || (ps == pstep))
		{
			// rigid BCs
			for (int j = 0; j<ps->RigidBCs(); ++j)
			{
				FSRigidBC* prc = ps->RigidBC(j);

				int flags = SHOW_PROPERTY_FORM;
				if (pstep) flags |= DUPLICATE_ITEM;
				QString name = QString("%1 [%2]").arg(QString::fromStdString(prc->GetName())).arg(prc->GetTypeString());
				AddTreeItem(t1, name, MT_RIGID_BC, 0, prc, flags);
			}

			// rigid ICs
			for (int j = 0; j < ps->RigidICs(); ++j)
			{
				FSRigidIC* prc = ps->RigidIC(j);
				int flags = SHOW_PROPERTY_FORM;
				if (pstep) flags |= DUPLICATE_ITEM;
				QString name = QString("%1 [%2]").arg(QString::fromStdString(prc->GetName())).arg(prc->GetTypeString());
				AddTreeItem(t1, name, MT_RIGID_IC, 0, prc, flags);
			}

			// rigid loads
			for (int j = 0; j < ps->RigidLoads(); ++j)
			{
				FSRigidLoad* prl = ps->RigidLoad(j);
				int flags = SHOW_PROPERTY_FORM;
				if (pstep) flags |= DUPLICATE_ITEM;
				QString name = QString("%1 [%2]").arg(QString::fromStdString(prl->GetName())).arg(prl->GetTypeString());
				AddTreeItem(t1, name, MT_RIGID_LOAD, 0, prl, flags);
			}

			// rigid connectors
			for (int j = 0; j < ps->RigidConnectors(); ++j)
			{
				FSRigidConnector* prc = ps->RigidConnector(j);
				int flags = SHOW_PROPERTY_FORM;
				if (pstep) flags |= DUPLICATE_ITEM;
				QString name = QString("%1 [%2]").arg(QString::fromStdString(prc->GetName())).arg(prc->GetTypeString());
				AddTreeItem(t1, name, MT_RIGID_CONNECTOR, 0, prc, flags);
			}
		}
	}

	if (t1)
	{
		int n = t1->childCount();
		if (n > 0)
		{
			t1->setText(0, QString("Rigid (%2)").arg(n));
		}
	}
}

void CModelTree::UpdateMaterials(QTreeWidgetItem* t1, FSModel& fem)
{
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* gm = fem.GetMaterial(i);
		FSMaterial* mat = gm->GetMaterialProperties();
		QString name = QString("%1 [%2]").arg(QString::fromStdString(gm->GetName())).arg(mat ? mat->GetTypeString(): "null");
		QTreeWidgetItem* t2 = AddTreeItem(t1, name, MT_MATERIAL, 0, gm);
		UpdateItem(t2);
	}
}

void CModelTree::UpdateLoadControllers(QTreeWidgetItem* t1, FSModel& fem)
{
	for (int i = 0; i < fem.LoadControllers(); ++i)
	{
		FSLoadController* plc = fem.GetLoadController(i);
		string name = plc->GetName();
		AddTreeItem(t1, QString::fromStdString(name), MT_LOAD_CONTROLLER, 0, plc);
	}

	int n = t1->childCount();
	if (n != 0) t1->setText(0, QString("Load Controllers (%1)").arg(n));
}

void CModelTree::UpdateOutput(QTreeWidgetItem* t1, FSProject& prj)
{
	AddTreeItem(t1, "plotfile", MT_PROJECT_OUTPUT_PLT, 0, nullptr, 1);
	AddTreeItem(t1, "logfile" , MT_PROJECT_OUTPUT_LOG, 0, nullptr, 1);
}
