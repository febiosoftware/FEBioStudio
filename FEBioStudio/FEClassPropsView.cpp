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
#include "FEClassPropsView.h"
#include <QPainter>
#include <QLineEdit>
#include <QComboBox>
#include <QListView>
#include <QBoxLayout>
#include <QStackedWidget>
#include "EditVariableParam.h"
#include "units.h"
#include "PropertyList.h"
#include "PlotWidget.h"
#include <MeshTools/FEModel.h>
#include <FEBioLink/FEBioInterface.h>
#include <FEMLib/FEBase.h>
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioInterface.h>
#include <QStandardItemModel>
#include "SelectionBox.h"
using namespace std;

// in MaterialPropsView.cpp
QStringList GetEnumValues(FSModel* fem, const char* ch);

QPixmap BuildPixMap(QColor c, int shape = 0, int size = 12)
{
	if (size < 8) size = 8;

	QColor c2 = c;
	QColor c1 = c2.lighter();
	QColor c3 = c2.darker();

	QRadialGradient g(QPointF(size/3, size/3), size/2);
	g.setColorAt(0.0, c1);
	g.setColorAt(0.2, c2);
	g.setColorAt(1.0, c3);

	QPixmap pix(size, size);
//	pix.setDevicePixelRatio(m_list->devicePixelRatio());
	pix.fill(Qt::transparent);
	QPainter p(&pix);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::PenStyle::NoPen);
	p.setBrush(QBrush(g));
	if (shape == 0)
		p.drawEllipse(2, 2, size - 4, size - 4);
	else
		p.drawRect(2, 2, size - 4, size - 4);

	p.end();

	return pix;
}

class FEClassPropsModel : public QAbstractItemModel
{
public:
	class Item
	{
	public:
		enum Flags {
			Item_Bold = 1,
			Item_Indented = 2
		};

	public:
		FEClassPropsModel*	m_model;

		FSCoreBase*		m_pc;			// pointer to class
		int				m_paramId;		// index of parameter (or -1 if this is property)
		int				m_propId;		// index of property (or -1 if this is parameter)
		int				m_propIndex;	// index into property's class array

		int		m_nrow;	// row index into parent's children array

		int		m_flag;	// used to decided if the item will show up as bold or not. Default: parameters = no, properties = yes. 

	public:
		Item*			m_parent;		// pointer to parent
		std::vector<Item*>	m_children;		// list of children

	public:
		Item() { m_model = nullptr; m_pc = nullptr; m_parent = nullptr; m_paramId = -1; m_propId = -1; m_propIndex = 0; m_nrow = -1; m_flag = 0;  }
		Item(FSCoreBase* pc, int paramId = -1, int propId = -1, int propIndex = 0, int nrow = -1) {
			m_model = nullptr;
			m_pc = pc; m_paramId = paramId; m_propId = propId; m_propIndex = propIndex; m_nrow = nrow;
			m_flag = (propId != -1 ? 1 : 0);
		}
		~Item() { for (int i = 0; i < m_children.size(); ++i) delete m_children[i]; m_children.clear(); }

		bool isParameter() const { return (m_paramId >= 0); }
		bool isProperty() const { return (m_propId >= 0); }

		int flag() const { return m_flag; }

		Param* parameter() { return (m_paramId >= 0 ? m_pc->GetParamPtr(m_paramId) : nullptr); }

		Item* addChild(FSCoreBase* pc, int paramId, int propId, int propIndex)
		{
			Item* item = new Item(pc, paramId, propId, propIndex, (int)m_children.size());
			item->m_model = m_model; assert(m_model);
			item->m_parent = this;
			m_children.push_back(item);

			if (propId >= 0)
			{
				FSCoreBase* pci = pc->GetProperty(propId, propIndex);
				if (pci)
				{
					// don't add children of classes that have custom widgets
					FSFunction1D* pf = dynamic_cast<FSFunction1D*>(pci);
					if (pf && (pf->IsType("point") || pf->IsType("math")))
						return item;

					// don't add mesh selection 
					FSMeshSelection* pms = dynamic_cast<FSMeshSelection*>(pci);
					if (pms) return item;

					item->addChildren(pci);
				}
			}
			return item;
		}

		void addParameters(FSCoreBase* pc)
		{
			pc->UpdateData(false);
			for (int i = 0; i < pc->Parameters(); ++i)
			{
				Param& p = pc->GetParam(i);
				if (p.IsVisible())
				{
					if (p.IsEditable() && (p.IsPersistent() || (m_pc == nullptr)))
						addChild(pc, i, -1, 0);
					else if (p.IsPersistent() == false)
					{
						addChild(pc, i, -1, 0);
					}
				}
			}
		}

		void addChildren(FSCoreBase* pc)
		{
			addParameters(pc);

			for (int i = 0; i < pc->Properties(); ++i)
			{
				FSProperty& p = pc->GetProperty(i);
				int nc = p.Size();
				for (int j = 0; j < nc; ++j) addChild(pc, -1, i, j);

				if ((p.maxSize() == FSProperty::NO_FIXED_SIZE) && ((p.GetFlags() & FSProperty::NON_EXTENDABLE) == 0))
				{
					addChild(pc, -1, i, -1);
				}
			}
		}

		FSModel* GetFSModel();

		QVariant data(int column, int role)
		{
			if (m_paramId >= 0)
			{
				Param& p = m_pc->GetParam(m_paramId);
				if (column == 0)
				{
					QString name(p.GetLongName());
					if (m_flag & Item_Indented)
					{
						name = "  " + name;
					}
					return name;
				}

				if (role == Qt::DisplayRole)
				{
					switch (p.GetParamType())
					{
					case Param_FLOAT:
					{
						QString v = QString::number(p.val<double>());
						const char* szunit = p.GetUnit();
						if (szunit)
						{
							QString unitString = Units::GetUnitString(szunit);
							if (unitString.isEmpty() == false)
								v += QString(" %1").arg(unitString);
						}
						return v;
					}
					break;
					case Param_CHOICE:
					case Param_INT:
					{
						int n = p.val<int>();
						if (p.GetEnumNames() && GetFSModel())
						{
							const char* sz = GetFSModel()->GetEnumValue(p.GetEnumNames(), n);
							if (sz == nullptr) sz = "please select";
							return sz;
						}
						return n;
					}
					break;
					case Param_VEC3D:
					{
						QString v = Vec3dToString(p.val<vec3d>());
						const char* szunit = p.GetUnit();
						if (szunit)
						{
							QString unitString = Units::GetUnitString(szunit);
							if (unitString.isEmpty() == false)
								v += QString(" %1").arg(unitString);
						}
						return v;
					}
					break;
					case Param_BOOL:
					{
						bool b = p.val<bool>();
						return (b ? "Yes" : "No");
					}
					break;
					case Param_VEC2I: return Vec2iToString(p.val<vec2i>()); break;
					case Param_MAT3D:
					{
						QString v = Mat3dToString(p.val<mat3d>());
						const char* szunit = p.GetUnit();
						if (szunit)
						{
							QString unitString = Units::GetUnitString(szunit);
							if (unitString.isEmpty() == false)
								v += QString(" %1").arg(unitString);
						}
						return v;
					}
					break;
					case Param_MAT3DS:
					{
						QString v = Mat3dsToString(p.val<mat3ds>());
						const char* szunit = p.GetUnit();
						if (szunit)
						{
							QString unitString = Units::GetUnitString(szunit);
							if (unitString.isEmpty() == false)
								v += QString(" %1").arg(unitString);
						}
						return v;
					}
					break;
					case Param_MATH:
					{
						string s = p.GetMathString();
						QString v = QString("= ") + QString::fromStdString(s);
						const char* szunit = p.GetUnit();
						if (szunit)
						{
							QString unitString = Units::GetUnitString(szunit);
							if (unitString.isEmpty() == false)
								v += QString(" %1").arg(unitString);
						}
						return v;
					}
					break;
					case Param_STRING:
					{
						string s = p.GetStringValue();
						QString v = QString("\"") + QString::fromStdString(s) + QString("\"");
						const char* szunit = p.GetUnit();
						if (szunit)
						{
							QString unitString = Units::GetUnitString(szunit);
							if (unitString.isEmpty() == false)
								v += QString(" %1").arg(unitString);
						}
						return v;
					}
					break;
					case Param_STD_VECTOR_INT:
					{
						std::vector<int> v = p.val<std::vector<int> >();
						QString s;
						for (int i = 0; i < v.size(); ++i)
						{
							s += QString::number(v[i]);
							if (i < v.size() - 1) s += QString(",");
						}
						return s;
					}
					break;
					case Param_STD_VECTOR_DOUBLE:
					{
						std::vector<double> v = p.val<std::vector<double> >();
						QString s;
						if (v.empty()) s = "(none)";
						else
						{
							for (int i = 0; i < v.size(); ++i)
							{
								s += QString::number(v[i]);
								if (i < v.size() - 1) s += QString(",");
							}
						}
						return s;
					}
					break;
					case Param_STD_VECTOR_VEC2D:
						return "in progress";
					default:
						assert(false);
						return "in progress";
					}
				}
				else
				{
					switch (p.GetParamType())
					{
					case Param_FLOAT: return p.val<double>(); break;
					case Param_INT: 
					case Param_CHOICE:
					{ 
						int n = p.val<int>();
						return n;
					}
					break;
					case Param_VEC3D: return Vec3dToString(p.val<vec3d>()); break;
					case Param_BOOL: return (p.val<bool>() ? 1 : 0); break;
					case Param_VEC2I:return Vec2iToString(p.val<vec2i>()); break;
					case Param_MAT3D: return Mat3dToString(p.val<mat3d>()); break;
					case Param_MAT3DS: return Mat3dsToString(p.val<mat3ds>()); break;
					case Param_MATH: return QString::fromStdString(p.GetMathString()); break;
					case Param_STRING: return QString::fromStdString(p.GetStringValue()); break;
					case Param_STD_VECTOR_INT: return -1; break;
					default:
//						assert(false);
						return "in progress";
					}
				}
			}
			else if (m_propId >= 0)
			{
				FSProperty& p = m_pc->GetProperty(m_propId);
				if (column == 0)
				{
					QString s = QString::fromStdString(p.GetName());
					if (p.maxSize() != 1)
					{
						if (m_propIndex >= 0)
						{
							s += QString(" - %1").arg(m_propIndex + 1);
							FSCoreBase* pc = m_pc->GetProperty(m_propId, m_propIndex);
							if (pc && (pc->GetName().empty() == false))
							{
								QString name = QString::fromStdString(pc->GetName());
								s += QString(" [%1]").arg(name);
							}
						}
						else 
							s = QString("<add %1>").arg(s);
					}
					return s;
				}
				else
				{
					FSProperty& prop = m_pc->GetProperty(m_propId);
					if (prop.GetSuperClassID() == FEDOMAIN_ID)
					{
						FSMeshSelection* pms = dynamic_cast<FSMeshSelection*>(prop.GetComponent()); assert(pms);
						if (pms)
						{
							FEItemListBuilder* pi = pms->GetItemList();
							if (pi == nullptr) return QString("(empty)");
							string s = pi->GetName();
							if (s.empty()) return QString("(unnamed)");
							else return QString::fromStdString(s);
						}
					}
					else
					{
						FSCoreBase* pc = (m_propIndex >= 0 ? m_pc->GetProperty(m_propId, m_propIndex) : nullptr);
						if (pc == nullptr)
						{
							bool required = (p.GetFlags() & FSProperty::REQUIRED);
							return QString(required ? "(select)" : "(none)");
						}
						else return pc->GetTypeString();
					}
				}
			}
			else return "No data";
		}

		bool setData(int column, const QVariant& value)
		{
			if (column != 1) return false;

			if (m_paramId >= 0)
			{
				Param& p = m_pc->GetParam(m_paramId);
				switch (p.GetParamType())
				{
				case Param_FLOAT: 
				{
					double f = value.toDouble();
					p.SetFloatValue(f);
				}
				break;
				case Param_CHOICE:
				case Param_INT: 
				{
					int n = value.toInt();
					p.SetIntValue(n);
				}
				break;
				case Param_VEC3D: p.SetVec3dValue(StringToVec3d(value.toString())); break;
				case Param_VEC2I: p.SetVec2iValue(StringToVec2i(value.toString())); break;
				case Param_MAT3D: p.SetMat3dValue(StringToMat3d(value.toString())); break;
				case Param_MAT3DS: p.SetMat3dsValue(StringToMat3ds(value.toString())); break;
				case Param_BOOL:
				{
					int n = value.toInt();
					p.SetBoolValue(n != 0);
				}
				break;
				case Param_MATH:
				{
					string s = value.toString().toStdString();
					if ((s.empty() == false) && (s[0] == '=')) s.erase(s.begin());
					p.SetMathString(s);
				}
				break;
				case Param_STRING:
				{
					string s = value.toString().toStdString();
					if ((s.empty() == false) && (s[0] == '\"'))
					{
						s.erase(s.begin());
						size_t n = s.rfind('\"');
						if (n != string::npos) s.erase(s.begin() + n);
					}
					p.SetStringValue(s);
				}
				break;
//				default:
//					assert(false);
				}

				return m_pc->UpdateData(true);
			}
			else if (isProperty())
			{
				int classId = value.toInt();

				FSProperty& prop = m_pc->GetProperty(m_propId);

				if (classId == -2)
				{
					FSCoreBase* pc = m_pc->GetProperty(m_propId, m_propIndex);
					if (pc) prop.RemoveComponent(pc);
				}
				else
				{
					// check if this is a different type
					if (m_propIndex >= 0)
					{
						FSCoreBase* oldprop = m_pc->GetProperty(m_propId, m_propIndex);

						if (oldprop && (oldprop->GetClassID() == classId))
						{
							// the type has not changed, so don't replace the property
							return false;
						}
					}

					FSCoreBase* pc = nullptr;
					if (classId > 0)
					{
						pc = FEBio::CreateClass(classId, GetFSModel());
					}

					if (pc)
					{
						if (m_propIndex >= 0)
							m_pc->GetProperty(m_propId).SetComponent(pc, m_propIndex);
						else
						{
							m_pc->GetProperty(m_propId).AddComponent(pc);
						}
					}
				}
				return true;
			}
			return false;
		}

		Item* parent() { return m_parent; }

		Item* child(int n) { return ((n >= 0) && (n < children()) ? m_children[n] : nullptr); }

		int row() const { return m_nrow; }

		int children() const { return (int)m_children.size(); }
	};

public:
	explicit FEClassPropsModel(QObject* parent = nullptr) : QAbstractItemModel(parent)
	{
		m_root = nullptr;
		m_valid = true;
	}

	~FEClassPropsModel() { delete m_root; }

	void SetClass(FSCoreBase* pc, FSModel* fem)
	{
		m_valid = true;
		beginResetModel();
		delete m_root;
		m_root = nullptr;
		m_pc = pc;
		m_fem = fem;
		if (pc)
		{
			m_root = new Item();
			m_root->m_model = this;
			m_root->addChildren(pc);
		}
		endResetModel();
	}

	bool isProperty(const QModelIndex& index)
	{
		if (index.isValid() == false) return false;
		Item* item = static_cast<Item*>(index.internalPointer());
		return item->isProperty();
	}

	QVariant data(const QModelIndex& index, int role) const override
	{
		if (!index.isValid()) return QVariant();

		if (role == Qt::SizeHintRole)
		{
			QFont font("Times", 12);
			QFontInfo fi(font);
			return QSize(10, 3*fi.pixelSize()/2);
		}

		Item* item = static_cast<Item*>(index.internalPointer());

/*		if (role == Qt::BackgroundRole)
		{
			// This color has to match the color in CMaterialPropsView::drawBranches
			if (item->isProperty()) return QColor(Qt::darkGray);
		}
*/
		if ((role == Qt::FontRole) && (item->m_flag & Item::Item_Bold))
		{
			QFont font;
			font.setBold(true);
			return font;
		}

		if ((index.column() == 0) && (role == Qt::DecorationRole))
		{
			QColor c;
			int s = 0;
			if (item->isParameter()) 
			{ 
				Param* p = item->parameter();
				if (p)
				{
					if (p->GetLoadCurveID() > 0) c = QColor::fromRgb(0, 255, 0);
					else c = QColor::fromRgb(0, 128, 0);
				}
				else c = QColor::fromRgb(0, 0, 0); 
				s = 0; 
			}
			if (item->isProperty()) { c = QColor::fromRgb(255, 0, 0); s = 1; }

			return BuildPixMap(c, s, 12);
		}

		if ((role != Qt::DisplayRole)&& (role != Qt::EditRole)) return QVariant();

		return item->data(index.column(), role);
	}

	Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		if (!index.isValid()) return Qt::ItemIsEnabled;
		if (index.column() == 1)
		{
			Item* item = static_cast<Item*>(index.internalPointer());
			if (item->isParameter())
				return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

			if (item->isProperty())
				return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
		}
		return QAbstractItemModel::flags(index);
	}

	bool setData(const QModelIndex& index, const QVariant& value, int role) override
	{
		if (index.isValid() && (role == Qt::EditRole))
		{
			Item* item = static_cast<Item*>(index.internalPointer());
			if (item->setData(index.column(), value))
			{
				m_valid = false;
			}
			emit dataChanged(index, index);
			return true;
		}
		return false;
	}

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
			return (section == 0? "Property" : "Value");
		return QAbstractItemModel::headerData(section, orientation, role);
	}

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
	{
		if (!hasIndex(row, column, parent)) return QModelIndex();
		Item* parentItem = nullptr;
		if (!parent.isValid()) parentItem = m_root;
		else parentItem = static_cast<Item*>(parent.internalPointer());
		Item* childItem = parentItem->child(row);
		if (childItem) return createIndex(row, column, childItem);
		return QModelIndex();
	}

	QModelIndex parent(const QModelIndex &index) const override
	{
		if (!index.isValid()) return QModelIndex();
		Item* item = static_cast<Item*>(index.internalPointer());
		Item* parent = item->parent();
		if (parent == m_root) return QModelIndex();
		return createIndex(parent->row(), 0, parent);
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const override
	{
		if (parent.column() > 0) return 0;
		Item* parentItem = nullptr;
		if (!parent.isValid()) parentItem = m_root;
		else parentItem = static_cast<Item*>(parent.internalPointer());

		return (parentItem ? parentItem->children() : 0);
	}

	int columnCount(const QModelIndex &parent = QModelIndex()) const override
	{
		return 2;
	}

	bool IsValid() const { return m_valid; }

	void ResetModel()
	{
		SetClass(m_pc, m_fem);
	}

private:
	FSModel*		m_fem;
	FSCoreBase*		m_pc;
	Item*			m_root;
	bool		m_valid;
};

FSModel* FEClassPropsModel::Item::GetFSModel()
{
	return m_model->m_fem;
}

//=================================================================================================
FEClassPropsDelegate::FEClassPropsDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

QWidget* FEClassPropsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.isValid())
	{
		FEClassPropsModel::Item* item = static_cast<FEClassPropsModel::Item*>(index.internalPointer());

		if (item->isParameter())
		{
			Param* p = item->parameter();

			// check for variable parameters first
			if (p->IsVariable())
			{
				CEditVariableParam* pw = new CEditVariableParam(parent);
				pw->setParam(p);
				return pw;
			}

			switch (p->GetParamType())
			{
			case Param_FLOAT:
				{
					QLineEdit* pw = new QLineEdit(parent);
					pw->setValidator(new QDoubleValidator);
					return pw;
				}
				break;
			case Param_VEC2I:
				{
					QLineEdit* pw = new QLineEdit(parent);
					return pw;
				}
				break;
			case Param_VEC3D:
				{
					QLineEdit* pw = new QLineEdit(parent);
					return pw;
				}
				break;
			case Param_MAT3D:
				{
					QLineEdit* pw = new QLineEdit(parent);
					return pw;
				}
				break;
			case Param_MAT3DS:
				{
					QLineEdit* pw = new QLineEdit(parent);
					return pw;
				}
				break;
			case Param_INT:
			case Param_CHOICE:
				{
					if (p->GetEnumNames())
					{
						QComboBox* box = new QComboBox(parent);
						QStringList enumValues = GetEnumValues(item->GetFSModel(), p->GetEnumNames());
						box->addItems(enumValues);
						int n = p->val<int>();
						box->setCurrentIndex(n);
						QObject::connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(OnEditorSignal()));
						return box;
					}
				}
				break;
			case Param_BOOL:
				{
					QComboBox* pw = new QComboBox(parent);
					pw->addItems(QStringList() << "No" << "Yes");
					bool b = p->GetBoolValue();
					pw->setCurrentIndex(b ? 1 : 0);
					QObject::connect(pw, SIGNAL(currentIndexChanged(int)), this, SLOT(OnEditorSignal()));
					return pw;
				}
				break;
			case Param_STRING:
				{
					QLineEdit* pw = new QLineEdit(parent);
					return pw;
				}
				break;
			case Param_STD_VECTOR_INT:
				{
					if (p->GetEnumNames())
					{
						QStringList enumValues = GetEnumValues(item->GetFSModel(), p->GetEnumNames());

						int n = enumValues.size();
						QStandardItemModel* mdl = new QStandardItemModel(n, 1);
						for (int i = 0; i < n; ++i)
						{
							QStandardItem* item = new QStandardItem(enumValues.at(i));
							item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
							item->setData(Qt::Unchecked, Qt::CheckStateRole);
							mdl->setItem(i, item);
						}

						std::vector<int> v = p->val<std::vector<int> >();
						for (int i = 0; i < v.size(); ++i)
						{
							QStandardItem* it = mdl->item(v[i]);
							it->setData(Qt::Checked, Qt::CheckStateRole);
						}

						QComboBox* box = new QComboBox(parent);
						box->setModel(mdl);
						return box;
					}
				}
				break;
			}
		}
		else if (item->isProperty())
		{
			FSCoreBase* pcb = item->m_pc;
			FSProperty& prop = pcb->GetProperty(item->m_propId);
			FSCoreBase* pcbi = pcb->GetProperty(item->m_propId, item->m_propIndex);

			QComboBox* pc = new QComboBox(parent);

			// fill the combo box
			int nclass = prop.GetSuperClassID();
			vector<FEBio::FEBioClassInfo> classInfo = FEBio::FindAllActiveClasses(nclass, prop.GetBaseClassID(), true);
			pc->clear();
			int classes = classInfo.size();
			for (int i = 0; i < classes; ++i)
			{
				FEBio::FEBioClassInfo& ci = classInfo[i];
				pc->addItem(ci.sztype, ci.classId);
			}
			pc->model()->sort(0);

			// add a remove option
			pc->insertSeparator(pc->count());
			pc->addItem("(remove)", -2);

			// see if the current option is in the list and selected it if so
			int n = (pcbi ? pc->findText(pcbi->GetTypeString()) : -1);
			pc->setCurrentIndex(n);

			QObject::connect(pc, SIGNAL(currentIndexChanged(int)), this, SLOT(OnEditorSignal()));

			return pc;
		}
	}
	QWidget* pw = QStyledItemDelegate::createEditor(parent, option, index);
	pw->setSizePolicy(QSizePolicy::Expanding, pw->sizePolicy().verticalPolicy());
	return pw;
}

void FEClassPropsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if (!index.isValid()) return;
/*	if (dynamic_cast<QComboBox*>(editor))
	{
		QComboBox* pw = dynamic_cast<QComboBox*>(editor);
		FEClassPropsModel::Item* item = static_cast<FEClassPropsModel::Item*>(index.internalPointer());
		if (item->isParameter())
		{
			// We only want to do this for enum parameters
			Param* p = item->parameter();
			if (p && (p->GetEnumNames()))
			{
				QVariant v = item->data(1, Qt::EditRole);
				pw->setCurrentIndex(v.toInt());
			}
		}
	}
	else */ QStyledItemDelegate::setEditorData(editor, index);
}

void FEClassPropsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	if (!index.isValid()) return;
	if (dynamic_cast<QComboBox*>(editor))
	{
		QComboBox* pw = dynamic_cast<QComboBox*>(editor);
		FEClassPropsModel::Item* item = static_cast<FEClassPropsModel::Item*>(index.internalPointer());
		if (item->isParameter())
		{
			Param* p = item->parameter();
			if (p && (p->GetParamType() == Param_STD_VECTOR_INT))
			{
				QStandardItemModel* m = dynamic_cast<QStandardItemModel*>(pw->model());
				std::vector<int> v;
				for (int i = 0; i < m->rowCount(); ++i)
				{
					QStandardItem* it = m->item(i);
					if (it->checkState() == Qt::Checked)
					{
						v.push_back(i);
					}
				}
				p->val<std::vector<int> >() = v;
				return;
			}
			else if (p && (p->GetEnumNames() || (p->GetParamType() == Param_BOOL)))
			{
				model->setData(index, pw->currentIndex());
				return;
			}
		}
		else if (item->isProperty())
		{
			int matId = pw->currentData(Qt::UserRole).toInt();
			model->setData(index, matId);
			return;
		}
	}
	QStyledItemDelegate::setModelData(editor, model, index);
}

void FEClassPropsDelegate::OnEditorSignal()
{
	QWidget* sender = dynamic_cast<QWidget*>(QObject::sender());
	emit commitData(sender);
}

//=================================================================================================
FEClassPropsView::FEClassPropsView(QWidget* parent) : QTreeView(parent)
{
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setUniformRowHeights(true);
	setEditTriggers(QAbstractItemView::AllEditTriggers);

	setItemDelegate(new FEClassPropsDelegate);
	m_model = new FEClassPropsModel;
	setModel(m_model);

	QObject::connect(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)), this, SLOT(onModelDataChanged()));
}

void FEClassPropsView::SetFEClass(FSCoreBase* pc, FSModel* fem)
{
	m_model->SetClass(pc, fem);
	expandAll();
	setColumnWidth(0, width() / 2);
}

Param* FEClassPropsView::getParam(const QModelIndex& index)
{
	FEClassPropsModel::Item* item = static_cast<FEClassPropsModel::Item*>(index.internalPointer());
	if (item->isParameter())
	{
		Param* p = item->parameter(); assert(p);
		return p;
	}
	return nullptr;
}

FSProperty* FEClassPropsView::getProperty(const QModelIndex& index)
{
	FEClassPropsModel::Item* item = static_cast<FEClassPropsModel::Item*>(index.internalPointer());
	if (item->isProperty())
	{
		FSProperty* p = &item->m_pc->GetProperty(item->m_propId); assert(p);
		return p;
	}
	return nullptr;
}

void FEClassPropsView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
/*	if (index.isValid())
	{
		if (m_model->isProperty(index))
		{
			painter->fillRect(rect, Qt::darkGray);
		}
	}
*/	QTreeView::drawBranches(painter, rect, index);
}

void FEClassPropsView::onModelDataChanged()
{
	if (m_model->IsValid() == false)
	{
		m_model->ResetModel();
		expandAll();
	}
}

//=============================================================================
class FEClassEditUI
{
public:
	FEClassPropsView* feprops;
	QStackedWidget* stack;

	CCurveEditWidget* plt;
	CMathEditWidget* math;
	CMeshSelectionBox* sel;

	FSFunction1D* m_pf;
	FSMeshSelection* m_pms;

public:
	void setup(QWidget* w)
	{
		m_pf = nullptr;
		m_pms = nullptr;

		feprops = new FEClassPropsView;

		stack = new QStackedWidget;
		stack->addWidget(plt  = new CCurveEditWidget);
		stack->addWidget(math = new CMathEditWidget);
		stack->addWidget(sel = new CMeshSelectionBox);

		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0, 0, 0, 0);
		l->addWidget(feprops, 1);
		l->addWidget(stack, 2);

		stack->hide();

		w->setLayout(l);

		QObject::connect(feprops, SIGNAL(clicked(const QModelIndex&)), w, SLOT(onItemClicked(const QModelIndex&)));
		QObject::connect(plt, SIGNAL(dataChanged()), w, SLOT(onPlotChanged()));
		QObject::connect(math, SIGNAL(mathChanged(QString)), w, SLOT(onMathChanged(QString)));
	}

	void SetFunction1D(FSFunction1D* pf)
	{
		m_pf = pf;
		if (pf == nullptr) stack->hide();
		else
		{
			if (pf && pf->IsType("point"))
			{
				stack->setCurrentIndex(0);
				LoadCurve* pc = pf->CreateLoadCurve();
				plt->SetLoadCurve(pc);
				stack->show();
			}
			else if (pf && pf->IsType("math"))
			{
				Param* p = pf->GetParam("math");
				math->SetMath(QString::fromStdString(p->GetStringValue()));
				stack->setCurrentIndex(1);
				stack->show();
			}
			else stack->hide();
		}
	}

	void SetMeshSelection(FSMeshSelection* pms)
	{
		m_pms = pms;
		if (pms == nullptr) stack->hide();
		else
		{
			sel->SetSelection(pms);
			stack->setCurrentIndex(2);
			stack->show();
		}
	}
};

FEClassEdit::FEClassEdit(QWidget* parent) : QWidget(parent), ui(new FEClassEditUI)
{
	ui->setup(this);
}

void FEClassEdit::SetFEClass(FSCoreBase* pc, FSModel* fem)
{
	ui->m_pf = nullptr;
	ui->stack->hide();
	ui->feprops->SetFEClass(pc, fem);
}

void FEClassEdit::onItemClicked(const QModelIndex& i)
{
	if (i.column() != 0) return;
	FSProperty* p = ui->feprops->getProperty(i);
	if (p == nullptr) { ui->SetFunction1D(nullptr); return; }

	if (p->GetSuperClassID() == FEFUNCTION1D_ID)
	{
		FSFunction1D* pf = dynamic_cast<FSFunction1D*>(p->GetComponent());
		ui->SetFunction1D(pf);
	}
	else if (p->GetSuperClassID() == FEDOMAIN_ID)
	{
		FSMeshSelection* pms = dynamic_cast<FSMeshSelection*>(p->GetComponent());
		ui->SetMeshSelection(pms); return;
	}
	else ui->SetFunction1D(nullptr);
}

void FEClassEdit::onMathChanged(QString s)
{
	if (ui->m_pf && ui->m_pf->IsType("math"))
	{
		Param* p = ui->m_pf->GetParam("math");
		p->SetStringValue(s.toStdString());
	}
}

void FEClassEdit::onPlotChanged()
{
	if (ui->m_pf && ui->m_pf->IsType("point"))
	{
		ui->m_pf->UpdateData(true);
	}
}
