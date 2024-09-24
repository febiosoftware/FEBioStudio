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
#include <QApplication>
#include <QLabel>
#include <QToolButton>
#include "EditVariableParam.h"
#include "units.h"
#include "PropertyList.h"
#include "PlotWidget.h"
#include "IconProvider.h"
#include <FEMLib/FSModel.h>
#include <FEMLib/FEBase.h>
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioInterface.h>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QMessageBox>
#include <QCheckBox>
#include <FSCore/FSCore.h>
#include <FEMLib/FEMaterial.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include "SelectionBox.h"
#include "DlgAddPhysicsItem.h"
using namespace std;

// in MaterialPropsView.cpp
QStringList GetEnumValues(FSModel* fem, const char* ch);

//=================================================================================
CPropertySelector::CPropertySelector(FSProperty* pp, FSCoreBase* pc, int index, FSModel* fem, QWidget* parent) : QComboBox(parent)
{
	m_fem = fem;
	m_pc = pc;
	m_pp = pp;
	m_index = index;

	if (pc) addItem(pc->GetTypeString(), pc->GetClassID());
	else addItem("(none)", -1);
	addItem("<select...>", -3);
	addItem("<copy  ...>", -4);
	addItem("<remove...>", -2);
	if (pc == nullptr) setCurrentIndex(-1);

	QObject::connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectionChanged(int)));
}

void CPropertySelector::onSelectionChanged(int n)
{
	int m = currentData().toInt();
	if (m == -2)
	{
		QString title = QString("Remove %1").arg(QString::fromStdString(m_pp->GetLongName()));
		if (QMessageBox::question(this, title, "Are you sure you want to remove this property?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
		{
			emit currentDataChanged(n);
		}
		else setCurrentIndex(0);
	}
	else if (m == -3)
	{
		int superID = m_pp->GetSuperClassID();
		int baseID = m_pp->GetPropertyType();
		QString title = QString("Add %1").arg(QString::fromStdString(m_pp->GetLongName()));
		CDlgAddPhysicsItem dlg(title, superID, baseID, nullptr, true, false, this);
		dlg.ShowNameAndCategoryFields(false);
		if (dlg.exec())
		{
			int n = dlg.GetClassID();
			this->setItemData(0, n);
			this->setItemText(0, FEBio::GetClassInfo(n).sztype);
			setCurrentIndex(0);

			emit currentDataChanged(n);
		}
	}
	else if (m == -4)
	{
		int superID = m_pp->GetSuperClassID();
		int baseID = m_pp->GetPropertyType();
		QString title = QString("Copy %1").arg(QString::fromStdString(m_pp->GetLongName()));
		FSModelComponent* src = dynamic_cast<FSModelComponent*>(m_pp->GetParent());
		CDlgCopyPhysicsItem dlg(title, superID, baseID, src, m_fem, this);
		if (dlg.exec())
		{
			FSModelComponent* pc = dlg.GetSelectedComponent();
			if (pc)
			{
				FSModelComponent* newpc = FEBio::CloneModelComponent(pc, m_fem);
				if ((m_index < 0) || (m_pp->Size() == 0)) m_pp->AddComponent(newpc);
				else m_pp->SetComponent(newpc, m_index);
				int n = newpc->GetClassID();
				this->setItemData(0, n);
				this->setItemText(0, newpc->GetTypeString());
				setCurrentIndex(0);
				emit currentDataChanged(n);
			}
		}
	}
}

//=================================================================================
CMeshItemPropertySelector::CMeshItemPropertySelector(GModel& m, FSProperty* pp, DOMAIN_TYPE domainType, QWidget* parent) : QComboBox(parent), m_mdl(m)
{
	m_domainType = domainType;

	FSMeshSelection* pms = dynamic_cast<FSMeshSelection*>(pp->GetComponent()); assert(pms);
	if (pms)
	{
		FEItemListBuilder* pg = pms->GetItemList();
		m_itemList = m.AllNamedSelections(m_domainType);
		int n = -1;
		for (int i=0; i< m_itemList.size(); ++i)
		{
			FEItemListBuilder* pi = m_itemList[i];
			addItem(QString::fromStdString(pi->GetName()));
			if (pi == pg) n = i;
		}
		setCurrentIndex(n);
		m_pp = pp;
		QObject::connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectionChanged(int)));
	}
	else m_pp = nullptr;
}

void CMeshItemPropertySelector::onSelectionChanged(int n)
{
	FSMeshSelection* pms = dynamic_cast<FSMeshSelection*>(m_pp->GetComponent()); assert(pms);
	if (pms) pms->SetItemList(n >= 0 ? m_itemList[n] : nullptr);
	emit currentDataChanged(n);
}

//=================================================================================
class FEClassPropsModel : public QAbstractItemModel
{
public:
	class Item
	{
	public:
		enum Flags {
			Item_Bold = 1,
		};

	public:
		FEClassPropsModel*	m_model;

		FSCoreBase*		m_pc;			// pointer to class
		int				m_paramId;		// index of parameter (or -1 if this is property)
		int				m_propId;		// index of property (or -1 if this is parameter)
		int				m_index;		// index of parameter (for array parameters) or property in property's class array

		int		m_nrow;	// row index into parent's children array

		int		m_flag;	// used to decided if the item will show up as bold or not. Default: parameters = no, properties = yes. 

	public:
		Item*			m_parent;		// pointer to parent
		std::vector<Item*>	m_children;		// list of children

	public:
		Item() { m_model = nullptr; m_pc = nullptr; m_parent = nullptr; m_paramId = -1; m_propId = -1; m_index = -1; m_nrow = -1; m_flag = 0;  }
		Item(FSCoreBase* pc, int paramId = -1, int propId = -1, int index = -1, int nrow = -1) {
			m_model = nullptr;
			m_pc = pc; m_paramId = paramId; m_propId = propId; m_index = index; m_nrow = nrow;
			m_flag = (paramId == -1 ? 1 : 0);
			m_parent = nullptr;
		}
		~Item() { for (int i = 0; i < m_children.size(); ++i) delete m_children[i]; m_children.clear(); }

		bool isParameter() const { return (m_paramId >= 0); }
		bool isProperty() const { return (m_propId >= 0); }
		bool isParamGroup() const { return (m_paramId < 0) && (m_propId < 0) && (m_index >= 0); }

		int flag() const { return m_flag; }

		Param* parameter() { return (m_paramId >= 0 ? m_pc->GetParamPtr(m_paramId) : nullptr); }

		FSModel* GetFSModel();

		QVariant data(int column, int role)
		{
			if (m_paramId >= 0)
			{
				Param& p = m_pc->GetParam(m_paramId);
				if (column == 0)
				{
					if (role == Qt::DisplayRole)
					{
						QString name;
						if (m_index == -1)
						{
							string sname = p.GetLongName();
//							string sname = FSCore::beautify_string(p.GetLongName());
							name = QString::fromStdString(sname);
						}
						else
						{
							if ((p.GetParamType() == Param_STD_VECTOR_INT) && (p.GetEnumNames()))
							{
								FSModel* fem = m_model->m_fem;
								QStringList keys = GetEnumValues(fem, p.GetEnumNames());
								name = keys.at(m_index);
							}
							else 
								name = QString("[%1]").arg(m_index);
						}

						return name;
					}
					else if (role == Qt::ToolTipRole)
					{
						if (m_index == -1)
						{
							QString toolTip = QString("<p><b>parameter:</b> <code>%1</code></p>").arg(p.GetShortName());
							if (p.IsVolatile())
							{
								if (p.GetLoadCurveID() > 0)
								{
									FSModel* fem = GetFSModel();
									FSLoadController* plc = fem->GetLoadControllerFromID(p.GetLoadCurveID());
									if (plc)
									{
										toolTip += QString("<p><b>load controller: </b>%1</p>").arg(QString::fromStdString(plc->GetName()));
									}
								}
								else toolTip += QString("<p><b>load controller: </b>(none)</p>");
							}

							int rng = p.GetRangeType();
							double vmin, vmax;
							p.GetRange(vmin, vmax);
							switch (rng)
							{
							case 0: break;
							case 1: toolTip += QString("<p><b>range:</b> (%1, inf)</p>").arg(vmin); break;
							case 2: toolTip += QString("<p><b>range:</b> [%1, inf)</p>").arg(vmin); break;
							case 3: toolTip += QString("<p><b>range:</b> (-inf, %1)</p>").arg(vmin); break;
							case 4: toolTip += QString("<p><b>range:</b> (-inf, %1]</p>").arg(vmin); break;
							case 5: toolTip += QString("<p><b>range:</b> (%1, %2)</p>").arg(vmin).arg(vmax); break;
							case 6: toolTip += QString("<p><b>range:</b> [%1, %2]</p>").arg(vmin).arg(vmax); break;
							case 7: toolTip += QString("<p><b>range:</b> (%1, %2]</p>").arg(vmin).arg(vmax); break;
							case 8: toolTip += QString("<p><b>range:</b> [%1, %2)</p>").arg(vmin).arg(vmax); break;
							case 9: toolTip += QString("<p><b>range:</b> value != %1</p>").arg(vmin); break;
							}
							
							return toolTip;
						}
						return QVariant();
					}
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
						if (p.GetEnumNames() && GetFSModel())
						{
							const char* sz = GetFSModel()->GetEnumKey(p);
							if (sz == nullptr) sz = "(select)";
							return sz;
						}
						int n = p.val<int>();
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
					case Param_VEC2D: return Vec2dToString(p.val<vec2d>()); break;
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
						if ((m_index == -1) || (p.GetEnumNames() == nullptr))
						{
							QString s;
							for (int i = 0; i < v.size(); ++i)
							{
								s += QString::number(v[i]);
								if (i < v.size() - 1) s += QString(",");
							}
							return s;
						}
						else if ((m_index != -1) && (p.GetEnumNames()))
						{
							bool bfound = false;
							for (int i = 0; i < v.size(); ++i)
							{
								if (v[i] == m_index)
								{
									bfound = true;
									break;
								}
							}
							return (bfound ? "Yes" : "No");
						}
					}
					break;
					case Param_STD_VECTOR_DOUBLE:
					{
						QString s;
						std::vector<double> v = p.val<std::vector<double> >();
						if (m_index == -1) s = QString::number(v.size());
						else
						{
							assert((m_index >= 0) && (m_index < v.size()));
							s = QString::number(v[m_index]);
						}
						return s;
					}
					break;
					case Param_ARRAY_INT:
					{
						std::vector<int> v = p.val<std::vector<int> >();
						QString s = VectorIntToString(v);
						return s;
					}
					break;
					case Param_ARRAY_DOUBLE:
					{
						std::vector<double> v = p.val<std::vector<double> >();
						QString s = VectorDoubleToString(v);
						return s;
					}
					break;
					case Param_STD_VECTOR_VEC2D:
					{
						std::vector<vec2d> v = p.GetVectorVec2dValue();
						QString s;
						if (m_index == -1) s = QString::number(v.size());
						else
						{
							assert((m_index >= 0) && (m_index < v.size()));
							vec2d& r = v[m_index];
							s = Vec2dToString(r);
						}
						return s;
					}
					break;
					default:
						assert(false);
						return "in progress";
					}
				}
				else if (role == Qt::EditRole)
				{
					switch (p.GetParamType())
					{
					case Param_FLOAT: return p.val<double>(); break;
					case Param_INT: 
					case Param_CHOICE:
					{ 
						int n = -1;
						if (p.GetEnumNames()) n = GetFSModel()->GetEnumIndex(p);
						else n = p.val<int>();
						return n;
					}
					break;
					case Param_VEC3D: return Vec3dToString(p.val<vec3d>()); break;
					case Param_BOOL: return (p.val<bool>() ? 1 : 0); break;
					case Param_VEC2I:return Vec2iToString(p.val<vec2i>()); break;
					case Param_VEC2D:return Vec2dToString(p.val<vec2d>()); break;
					case Param_MAT3D: return Mat3dToString(p.val<mat3d>()); break;
					case Param_MAT3DS: return Mat3dsToString(p.val<mat3ds>()); break;
					case Param_MATH: return QString::fromStdString(p.GetMathString()); break;
					case Param_STRING: return QString::fromStdString(p.GetStringValue()); break;
					case Param_STD_VECTOR_INT: return -1; break;
					case Param_STD_VECTOR_DOUBLE:
					{
						std::vector<double> v = p.GetVectorDoubleValue();
						if (m_index == -1) return (int)v.size();
						else return v[m_index];
					}
					break;
					case Param_STD_VECTOR_VEC2D:
					{
						std::vector<vec2d> v = p.GetVectorVec2dValue();
						if (m_index == -1) return (int) v.size();
						else return Vec2dToString(v[m_index]);
					}
					break;
					case Param_ARRAY_INT:
					{
						std::vector<int> v = p.GetArrayIntValue();
						return VectorIntToString(v);
					}
					break;
					case Param_ARRAY_DOUBLE:
					{
						std::vector<double> v = p.GetArrayDoubleValue();
						return VectorDoubleToString(v);
					}
					break;
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
					if (role == Qt::ToolTipRole)
					{
						if ((p.maxSize()==1) || (m_index < 0))
						{
							QString s = QString("<b>property:</b> <code>%1</code>").arg(QString::fromStdString(p.GetName()));
							return s;
						}
					}
					else
					{
//						string sname = FSCore::beautify_string(p.GetLongName().c_str());
                        string sname = p.GetLongName().c_str();
						QString s = QString::fromStdString(sname);
						if (p.maxSize() != 1)
						{
							if (m_index >= 0)
							{
								s += QString(" - %1").arg(m_index + 1);
								FSCoreBase* pc = m_pc->GetProperty(m_propId, m_index);
								if (pc && (pc->GetName().empty() == false))
								{
									QString name = QString::fromStdString(pc->GetName());
									s += QString(" [%1]").arg(name);
								}
							}
						}
						return s;
					}
				}
				else
				{
					FSProperty& prop = m_pc->GetProperty(m_propId);
					if (prop.GetSuperClassID() == FESURFACE_ID)
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
					else if (prop.GetSuperClassID() == FEEDGE_ID)
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
						if (m_index < 0)
						{
							int n = prop.Size();
							return QString("[%1]").arg(n);
						}
						else
						{

							FSCoreBase* pc = (m_index >= 0 ? m_pc->GetProperty(m_propId, m_index) : nullptr);
							if (pc == nullptr)
							{
								bool required = (p.GetFlags() & FSProperty::REQUIRED);
								return QString(required ? "(select)" : "(none)");
							}
							else return pc->GetTypeString();
						}
					}
				}
			}
			else if (m_index >= 0)
			{
				if (column == 0)
				{
					ParamBlock& PB = m_pc->GetParamBlock();
					return PB.GetParameterGroupName(m_index);
				}
			}
			return QVariant();
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
					if (f != p.GetFloatValue()) {
						p.SetFloatValue(f);
						p.SetModified(true);
					}
				}
				break;
				case Param_CHOICE:
				case Param_INT: 
				{
					int n = value.toInt();
					if (p.GetEnumNames() && GetFSModel())
					{
						int m = GetFSModel()->GetEnumIndex(p);
						if (m != n)
						{
							bool b = GetFSModel()->SetEnumIndex(p, n);
							assert(b);
							p.SetModified(true);
						}
					}
					else
					{
						if (n != p.GetIntValue()) {
							p.SetIntValue(n);
							p.SetModified(true);
						}
					}
				}
				break;
				case Param_VEC3D: {
					vec3d v = StringToVec3d(value.toString());
					if ((v == p.GetVec3dValue()) == false) {
						p.SetVec3dValue(v);
						p.SetModified(true);
					}
				}break;
				case Param_VEC2I: {
					vec2i v = StringToVec2i(value.toString());
					vec2i s = p.GetVec2iValue();
					if ((v.x != s.x) || (v.y != s.y)) {
						p.SetVec2iValue(v);
						p.SetModified(true);
					}
				}break;
				case Param_VEC2D: {
					vec2d v = StringToVec2d(value.toString());
					vec2d s = p.GetVec2dValue();
					if ((v.x() != s.x()) || (v.y() != s.y())) {
						p.SetVec2dValue(v);
						p.SetModified(true);
					}
				}break;
				case Param_MAT3D: {
					mat3d m = StringToMat3d(value.toString());
					p.SetMat3dValue(m);
					p.SetModified(true);
				}
				break;
				case Param_MAT3DS: {
					mat3ds m = StringToMat3ds(value.toString());
					p.SetMat3dsValue(m);
					p.SetModified(true);
				}
				break;
				case Param_BOOL:
				{
					bool b = (value.toInt() != 0);
					if (p.GetBoolValue() != b) {
						p.SetBoolValue(b);
						p.SetModified(true);

						if (p.GetFlags() & FS_PARAM_WATCH) return true;
					}
				}
				break;
				case Param_MATH:
				{
					string s = value.toString().toStdString();
					if ((s.empty() == false) && (s[0] == '=')) s.erase(s.begin());

					if (p.GetMathString() != s) {
						p.SetMathString(s);
						p.SetModified(true);
					}
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

					if (p.GetStringValue() != s) {
						p.SetStringValue(s);
						p.SetModified(true);
					}
				}
				break;
				case Param_STD_VECTOR_DOUBLE:
				{
					std::vector<double> v = p.GetVectorDoubleValue();
					if (m_index == -1)
					{
						int newSize = value.toInt();
						v.resize(newSize);
						p.SetVectorDoubleValue(v);
						return true;
					}
					else
					{
						v[m_index] = value.toDouble();
						p.SetVectorDoubleValue(v);
					}
					p.SetModified(true);
				}
				break;
				case Param_STD_VECTOR_VEC2D:
				{
					std::vector<vec2d> v = p.GetVectorVec2dValue();
					if (m_index == -1)
					{
						int newSize = value.toInt();
						v.resize(newSize);
						p.SetVectorVec2dValue(v);
						return true;
					}
					else
					{
						vec2d r = StringToVec2d(value.toString());
						v[m_index] = r;
						p.SetVectorVec2dValue(v);
					}
					p.SetModified(true);
				}
				break;
				case Param_ARRAY_INT:
				{
					QString s = value.toString();
					std::vector<int> v = StringToVectorInt(s);
					p.SetArrayIntValue(v);
					p.SetModified(true);
				}
				break;				
				case Param_ARRAY_DOUBLE:
				{
					QString s = value.toString();
					std::vector<double> v = StringToVectorDouble(s);
					p.SetArrayDoubleValue(v);
					p.SetModified(true);
				}
				break;
				default:
					assert(false);
				}

				return m_pc->UpdateData(true);
			}
			else if (isProperty())
			{
				int classId = value.toInt();

				FSProperty& prop = m_pc->GetProperty(m_propId);

				if (classId == -2)
				{
					FSCoreBase* pc = m_pc->GetProperty(m_propId, m_index);
					if (pc) prop.RemoveComponent(pc);
				}
				else
				{
					if (m_index < 0)
					{
						int newSize = value.toInt();
						if (newSize != prop.Size())
						{
							prop.SetSize(newSize);
							return true;
						}
					}
					else
					{
						// check if this is a different type
						if (m_index >= 0)
						{
							FSCoreBase* oldprop = m_pc->GetProperty(m_propId, m_index);

							if (oldprop && (oldprop->GetClassID() == classId))
							{
								// the type has not changed, so don't replace the property
								return true;
							}
						}

						FSCoreBase* pc = nullptr;
						if (classId > 0)
						{
							pc = FEBio::CreateClass(classId, GetFSModel(), prop.GetFlags());
						}

						if (pc)
						{
							if (m_index >= 0)
								prop.SetComponent(pc, m_index);
							else
							{
								prop.AddComponent(pc);
							}
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
	enum ModelDisplayMode { DefaultMode, ParamsOnlyMode };

public:
	explicit FEClassPropsModel(QObject* parent = nullptr) : QAbstractItemModel(parent)
	{
		m_root = nullptr;
		m_valid = true;
		m_mode = DefaultMode;
	}

	~FEClassPropsModel() { delete m_root; }

	void SetDisplayMode(ModelDisplayMode mode)
	{
		m_mode = mode;
		SetClass(m_pc, m_fem);
	}

	void SetFilter(const QString& flt)
	{
		m_filter = flt;
		if (m_filter.isEmpty()) SetDisplayMode(DefaultMode);
		else SetDisplayMode(ParamsOnlyMode);
	}

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
			addChildren(m_root, pc);
		}
		endResetModel();
	}

	void addChildren(Item* parent, FSCoreBase* pc)
	{
		addParameters(parent, pc);

		for (int i = 0; i < pc->Properties(); ++i)
		{
			FSProperty& p = pc->GetProperty(i);

			if (p.maxSize() == FSProperty::NO_FIXED_SIZE)
			{
				if (m_mode == DefaultMode)
				{
					Item* item = new Item(pc, -1, i, -1, (int)parent->m_children.size());
					item->m_model = parent->m_model; assert(parent->m_model);
					item->m_parent = parent;
					parent->m_children.push_back(item);

					int nc = p.Size();
					for (int j = 0; j < nc; ++j) addChild(item, pc, -1, i, j);
				}
				else
				{
					int nc = p.Size();
					for (int j = 0; j < nc; ++j) addChild(parent, pc, -1, i, j);
				}
			}
			else {
				int nc = p.Size();
				for (int j = 0; j < nc; ++j) addChild(parent, pc, -1, i, j);

//				if ((p.maxSize() == FSProperty::NO_FIXED_SIZE) && ((p.GetFlags() & FSProperty::NON_EXTENDABLE) == 0))
//				{
//					addChild(parent, pc, -1, i, -1);
//				}
			}
		}		
	}

	void addParameters(Item* parent, FSCoreBase* pc)
	{
		pc->UpdateData(false);
		int currentGroup = -1;
		ParamBlock& PB = pc->GetParamBlock();
		int NPG = PB.ParameterGroups();
		Item* item = parent;
		for (int n = -1; n < NPG; ++n)
		{
			if (n != -1)
			{
				item = addChild(parent, pc, -1, -1, n);
			}
			for (int i = 0; i < pc->Parameters(); ++i)
			{
				Param& p = pc->GetParam(i);
				if (p.GetParameterGroup() == n)
				{
					if (p.IsVisible())
					{
						if (p.IsWatched())
						{
							if (p.GetWatchFlag()) addChild(item, pc, i, -1, -1);
						}
						else if (p.IsEditable() && (p.IsPersistent() || (m_pc == nullptr)))
							addChild(item, pc, i, -1, -1);
						else if (p.IsPersistent() == false)
						{
							addChild(item, pc, i, -1, -1);
						}
					}
				}
			}
		}
	}

	Item* addChild(Item* parent, FSCoreBase* pc, int paramId, int propId, int index)
	{
		// check for a special case first
		if (pc && (paramId >= 0) && (index == -1))
		{
			Param& p = pc->GetParam(paramId);
			if ((p.GetParamType() == Param_STD_VECTOR_INT) && (p.GetEnumNames()))
			{
				FSModel* fem = parent->m_model->m_fem;
				QStringList keys = GetEnumValues(fem, p.GetEnumNames());
				for (int i = 0; i < keys.size(); ++i)
				{
					addItem(parent, pc, paramId, -1, i);
				}
				return nullptr;
			}
		}

		Item* item = nullptr;
		if ((m_mode == DefaultMode) || (paramId >= 0))
		{
			item = addItem(parent, pc, paramId, propId, index);
		}
		else item = parent;

		if (pc && (paramId >= 0) && (index == -1))
		{
			Param& p = pc->GetParam(paramId);
			if (p.GetParamType() == Param_STD_VECTOR_DOUBLE)
			{
				std::vector<double> v = p.GetVectorDoubleValue();
				for (int i = 0; i < v.size(); ++i)
				{
					addChild(item, pc, paramId, -1, i);
				}
			}
			else if (p.GetParamType() == Param_STD_VECTOR_VEC2D)
			{
				std::vector<vec2d> v = p.GetVectorVec2dValue();
				for (int i = 0; i < v.size(); ++i)
				{
					addChild(item, pc, paramId, -1, i);
				}
			}
		}
		else if (propId >= 0)
		{
			FSCoreBase* pci = pc->GetProperty(propId, index);
			if (pci)
			{
				// don't add children of classes that have custom widgets
				FSFunction1D* pf = dynamic_cast<FSFunction1D*>(pci);
				if (pf && (pf->IsType("point") || pf->IsType("math")))
					return item;

				// don't add mesh selection 
				FSMeshSelection* pms = dynamic_cast<FSMeshSelection*>(pci);
				if (pms) return item;

				addChildren(item, pci);
			}
		}
		return item;
	}

	Item* addItem(Item* parent, FSCoreBase* pc, int paramId, int propId, int index)
	{
		if ((m_mode == ParamsOnlyMode) && (m_filter.isEmpty() == false))
		{
			if (paramId >= 0)
			{
				Param& p = pc->GetParam(paramId);
				QString name = p.GetLongName();

				if (name.contains(m_filter, Qt::CaseInsensitive) == false)
				{
					return parent;
				}
			}
			else return parent;
		}

		Item* item = new Item(pc, paramId, propId, index, (int)parent->m_children.size());
		item->m_model = parent->m_model; assert(parent->m_model);
		item->m_parent = parent;
		parent->m_children.push_back(item);
		return item;
	}

	bool isProperty(const QModelIndex& index)
	{
		if (index.isValid() == false) return false;
		Item* item = static_cast<Item*>(index.internalPointer());
		return item->isProperty();
	}

	bool isParameter(const QModelIndex& index)
	{
		if (index.isValid() == false) return false;
		Item* item = static_cast<Item*>(index.internalPointer());
		return item->isParameter();
	}

	Param* GetParameter(const QModelIndex& index)
	{
		if (index.isValid() == false) return nullptr;
		Item* item = static_cast<Item*>(index.internalPointer());
		return (item->isParameter() ? item->parameter() : nullptr);
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
		if ((role == Qt::FontRole))
		{
			QFont font;

			if (item->m_flag & Item::Item_Bold) font.setBold(true);

			if ((index.column() == 1) && item->isParameter())
			{
				Param* p = item->parameter();
				if (p && p->IsModified()) font.setBold(true);
			}

			return font;
		}

	/*	if ((role == Qt::BackgroundRole) && (index.column() == 1) && item->isParameter())
		{
			Param* p = item->parameter();
			if (p && p->IsModified())
			{
				QPalette palette = qApp->palette();
				QColor tc = palette.color(QPalette::WindowText);

				QLinearGradient gradient(0, 0, 100, 0);
				if (tc.red() == 0)
				{
					gradient.setColorAt(0, QColor::fromRgb(220, 255, 255, 0));
					gradient.setColorAt(0.5, QColor::fromRgb(220, 255, 255, 0));
					gradient.setColorAt(1, QColor::fromRgb(220, 255, 255, 255));
				}
				else
				{
					gradient.setColorAt(0, QColor::fromRgb(32, 64, 72, 0));
					gradient.setColorAt(0.5, QColor::fromRgb(32, 64, 72, 0));
					gradient.setColorAt(1, QColor::fromRgb(32, 64, 72, 255));
				}
				return QBrush(gradient);
			}
		}
		*/

		if ((index.column() == 0) && (role == Qt::DecorationRole))
		{
			QColor c;
			Shape s = Shape::Circle;
			bool bicon = false;
			if (item->isParameter()) 
			{ 
				if (item->m_index == -1)
				{
					Param* p = item->parameter();
					if (p)
					{
						if (p->GetLoadCurveID() > 0)
						{
							assert(p->IsVolatile());
							return CIconProvider::GetIcon("curve");
						}
					}
				}
			}
			else
			{
				bicon = true;
				if (item->isProperty()) { c = QColor::fromRgb(255, 0, 0); s = Shape::Square; }
				if (item->isParamGroup()) { c = QColor::fromRgb(200, 0, 200); s = Shape::Square; }
			}

			if (bicon)
				return CIconProvider::BuildPixMap(c, s);
			else
			{
				QPixmap pix(12, 12);
				pix.fill(Qt::transparent);
				return pix;
			}
		}

		if ((role != Qt::DisplayRole)&& (role != Qt::EditRole) && (role != Qt::ToolTipRole)) return QVariant();

		return item->data(index.column(), role);
	}

	Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		if (!index.isValid()) return Qt::ItemIsEnabled;
		if (index.column() == 1)
		{
			Item* item = static_cast<Item*>(index.internalPointer());
			if (item->isParameter())
			{
				Param* p = item->parameter();
				if (p && (p->GetParamType() == Param_BOOL))
					return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
				else
					return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
			}

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
		if ((parent == nullptr) || (parent == m_root)) return QModelIndex();
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
	void SetValid(bool b) { m_valid = b; }

	void ResetModel()
	{
		SetClass(m_pc, m_fem);
	}

private:
	FSModel*		m_fem;
	FSCoreBase*		m_pc;
	Item*			m_root;
	bool		m_valid;
	int			m_mode;
	QString		m_filter;
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
/*				FSModel* fem = item->GetFSModel();
				if (fem)
				{
					int n = fem->MeshDataGenerators();
					for (int i = 0; i < n; ++i)
					{
						FSMeshDataGenerator* m = fem->GetMeshDataGenerator(i);
						pw->addItem(QString::fromStdString(m->GetName()));
					}

					GModel& gm = fem->GetModel();
					for (int i = 0; i < gm.Objects(); ++i)
					{
						GObject* po = gm.Object(i);
						FSMesh* pm = po->GetFEMesh();
						int n = pm->MeshDataFields();
						for (int j = 0; j < n; ++j)
						{
							FEMeshData* md = pm->GetMeshDataField(j);
							pw->addItem(QString::fromStdString(md->GetName()));
						}
					}
				}
*/
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
			case Param_VEC2D:
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

						int n = item->GetFSModel()->GetEnumIndex(*p);
						box->setCurrentIndex(n);
						QObject::connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(OnEditorSignal()));
						return box;
					}
				}
				break;
			case Param_BOOL:
				{
					QCheckBox* pw = new QCheckBox(parent);
					bool b = p->GetBoolValue();
					pw->setChecked(b);
					QObject::connect(pw, SIGNAL(toggled(bool)), this, SLOT(OnEditorSignal()));
					return pw;
//					return nullptr;
				}
				break;
			case Param_STRING:
				{
					if (p->GetEnumNames())
					{
						QComboBox* box = new QComboBox(parent);
						QStringList enumValues = GetEnumValues(item->GetFSModel(), p->GetEnumNames());
						box->addItems(enumValues);
						string s = p->val<std::string>();
						int n = box->findText(QString::fromStdString(s));
						box->setCurrentIndex(n);
						QObject::connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(OnEditorSignal()));
						return box;
					}
					else {
						QLineEdit* pw = new QLineEdit(parent);
						return pw;
					}
				}
				break;
			case Param_STD_VECTOR_INT:
				{
					if (p->GetEnumNames())
					{
						int index = item->m_index;

						if (index == -1)
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
						else
						{
							std::vector<int> v = p->val<std::vector<int> >();
							bool bfound = false;
							for (int i = 0; i < v.size(); ++i)
							{
								if (v[i] == index)
								{
									bfound = true;
									break;
								}
							}

							QComboBox* box = new QComboBox(parent);
							box->addItems(QStringList() << "No" << "Yes");
							box->setCurrentIndex(bfound ? 1 : 0);
							return box;
						}
					}
				}
				break;
			case Param_STD_VECTOR_DOUBLE:
				{
					std::vector<double> v = p->GetVectorDoubleValue();
					if (item->m_index == -1)
					{
						QSpinBox* pw = new QSpinBox(parent);
						pw->setMinimum(0);
						pw->setValue(v.size());
						return pw;
					}
					else
					{
						QLineEdit* pw = new QLineEdit(parent);
						return pw;
					}
				}
				break;
			case Param_STD_VECTOR_VEC2D:
				{
					std::vector<vec2d> v = p->GetVectorVec2dValue();
					if (item->m_index == -1)
					{
						QSpinBox* pw = new QSpinBox(parent);
						pw->setMinimum(0);
						pw->setValue(v.size());
						return pw;
					}
					else
					{
						QLineEdit* pw = new QLineEdit(parent);
						return pw;
					}
				}
				break;
			case Param_ARRAY_INT   : return new QLineEdit(parent); break;
			case Param_ARRAY_DOUBLE: return new QLineEdit(parent); break;
			}
		}
		else if (item->isProperty())
		{
			FSCoreBase* pcb = item->m_pc;
			FSProperty& prop = pcb->GetProperty(item->m_propId);

			if (item->m_index < 0)
			{
				QSpinBox* ps = new QSpinBox(parent);
				ps->setRange(0, 100);
				ps->setValue(prop.Size());

				return ps;
			}
			else
			{
				FSCoreBase* pcbi = pcb->GetProperty(item->m_propId, item->m_index);

				int nclass = prop.GetSuperClassID();

				if (nclass == FESURFACE_ID)
				{
					GModel& m = item->GetFSModel()->GetModel();
					CMeshItemPropertySelector* pc = new CMeshItemPropertySelector(m, &prop, DOMAIN_TYPE::DOMAIN_SURFACE, parent);
					QObject::connect(pc, SIGNAL(currentDataChanged(int)), this, SLOT(OnEditorSignal()));
					return pc;
				}
				else if (nclass == FEEDGE_ID)
				{
					GModel& m = item->GetFSModel()->GetModel();
					CMeshItemPropertySelector* pc = new CMeshItemPropertySelector(m, &prop, DOMAIN_TYPE::DOMAIN_EDGE, parent);
					QObject::connect(pc, SIGNAL(currentDataChanged(int)), this, SLOT(OnEditorSignal()));
					return pc;
				}
				else
				{
					CPropertySelector* pc = new CPropertySelector(&prop, pcbi, item->m_index, item->GetFSModel(), parent);
					QObject::connect(pc, SIGNAL(currentDataChanged(int)), this, SLOT(OnEditorSignal()));
					return pc;
				}
			}
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
	if (dynamic_cast<CPropertySelector*>(editor))
	{
		CPropertySelector* ps = dynamic_cast<CPropertySelector*>(editor);
		FEClassPropsModel::Item* item = static_cast<FEClassPropsModel::Item*>(index.internalPointer());
		if (item->isProperty())
		{
			int n = ps->currentData().toInt();
			model->setData(index, n);
			return;
		}
	}
	else if (dynamic_cast<QComboBox*>(editor))
	{
		QComboBox* pw = dynamic_cast<QComboBox*>(editor);
		FEClassPropsModel::Item* item = static_cast<FEClassPropsModel::Item*>(index.internalPointer());
		if (item->isParameter())
		{
			Param* p = item->parameter();
			if (p && (p->GetParamType() == Param_STD_VECTOR_INT))
			{
				int index = item->m_index;
				if (index == -1)
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
				}
				else
				{
					std::vector<int> v = p->val<std::vector<int> >();
					bool b = (pw->currentIndex() == 1);
					if (b)
					{
						// add index
						int m = 0;
						for (int i = 0; i < v.size(); ++i, ++m)
						{
							if (v[i] == index) return;
							if (v[i] > index) {
								m = i;
								break;
							}
						}
						v.insert(v.begin() + m, index);
					}
					else
					{
						// remove index
						for (int i = 0; i < v.size(); ++i)
						{
							if (v[i] == index)
							{
								v.erase(v.begin() + i);
								break;
							}
						}
					}
					p->val<std::vector<int> >() = v;
				}
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

void FEClassPropsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (!index.isValid()) return;
	if (index.column() == 1)
	{
		FEClassPropsModel::Item* item = static_cast<FEClassPropsModel::Item*>(index.internalPointer());
		if (item && item->isParameter())
		{
			Param* p = item->parameter();
			if (p && (p->GetParamType() == Param_BOOL))
			{
				QStyleOptionButton cbOpt;
				cbOpt.rect = option.rect;
				cbOpt.state = option.state;
				bool b = p->GetBoolValue();
				if (b) cbOpt.state |= QStyle::State_On;
				else cbOpt.state |= QStyle::State_Off;

				QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
				QApplication::style()->drawControl(QStyle::CE_CheckBox, &cbOpt, painter);
				return;
			}
		}
	}

	QStyledItemDelegate::paint(painter, option, index);
}

bool FEClassPropsDelegate::editorEvent(QEvent* event,
	QAbstractItemModel* model,
	const QStyleOptionViewItem& option,
	const QModelIndex& index)
{
/*	if (event->type() == QEvent::MouseButtonPress)
	{
		if (index.column() == 1)
		{
			FEClassPropsModel::Item* item = static_cast<FEClassPropsModel::Item*>(index.internalPointer());
			if (item && item->isParameter())
			{
				Param* p = item->parameter();
				if (p && (p->GetParamType() == Param_BOOL))
				{
					bool b = p->GetBoolValue();
					p->SetBoolValue(b ? false : true);
					p->SetModified(true);

					if (p->GetFlags() & FS_PARAM_WATCH)
					{
						FEClassPropsModel* mdl = dynamic_cast<FEClassPropsModel*>(model);
						mdl->SetValid(false);
//						emit mdl->dataChanged(index, index);
					}
					return true;
				}
			}
		}
	}
*/
	return QStyledItemDelegate::editorEvent(event, model, option, index);
}

//=================================================================================================
FEClassPropsView::FEClassPropsView(QWidget* parent) : QTreeView(parent)
{
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setUniformRowHeights(true);
	setEditTriggers(QAbstractItemView::AllEditTriggers);

	setItemDelegate(new FEClassPropsDelegate(this));
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

FSProperty* FEClassPropsView::getSelectedProperty()
{
	QModelIndex index = currentIndex();
	if (index.isValid() == false) return nullptr;
	return getProperty(index);
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

void FEClassPropsView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QTreeView::drawRow(painter, option, index);

	if (index.isValid())
	{
		if (m_model->isParameter(index))
		{
			Param* p = m_model->GetParameter(index); assert(p);
			if (p)
			{
				if (!p->IsValueValid())
				{
					QRect rt = option.rect;
					rt.setLeft(rt.right() - 5);
					painter->fillRect(rt, Qt::red);
				}
				else if (p->IsModified())
				{
					QRect rt = option.rect;
					rt.setLeft(rt.right() - 5);
					painter->fillRect(rt, Qt::darkCyan);
				}
			}
		}
	}
}

void FEClassPropsView::onModelDataChanged()
{
	if (m_model->IsValid() == false)
	{
		m_model->ResetModel();
		expandAll();
	}

	QModelIndex index = currentIndex();
	FEClassPropsModel::Item* item = static_cast<FEClassPropsModel::Item*>(index.internalPointer());
	if (item && item->isParameter())
	{
		emit paramChanged(item->m_pc, item->parameter());
	}
}

void FEClassPropsView::setFilter(const QString& flt)
{
	m_model->SetFilter(flt);
}

//=============================================================================
FEClassPropsWidget::FEClassPropsWidget(QWidget* parent) : QWidget(parent)
{
	QVBoxLayout* l = new QVBoxLayout;
	l->setContentsMargins(0, 2, 0, 0);

	m_flt = new QLineEdit;
	QToolButton* tb = new QToolButton();
	tb->setIcon(CIconProvider::GetIcon("clear"));
	tb->setToolTip("Clear filter");
	QHBoxLayout* h = new QHBoxLayout;
	h->addWidget(new QLabel("Filter:"));
	h->addWidget(m_flt);
	h->addWidget(tb);
	l->addLayout(h);

	l->addWidget(m_view = new FEClassPropsView);

	setLayout(l);

	QObject::connect(m_flt, SIGNAL(textChanged(const QString&)), m_view, SLOT(setFilter(const QString&)));
	QObject::connect(tb, SIGNAL(clicked(bool)), m_flt, SLOT(clear()));
	QObject::connect(m_view, SIGNAL(clicked(const QModelIndex&)), this, SLOT(on_clicked(const QModelIndex&)));
	QObject::connect(m_view, SIGNAL(paramChanged(FSCoreBase*, Param*)), this, SLOT(on_paramChanged(FSCoreBase*, Param*)));
}

void FEClassPropsWidget::on_clicked(const QModelIndex& index)
{
	emit clicked(index);
}

void FEClassPropsWidget::SetFEClass(FSCoreBase* pc, FSModel* fem)
{
	m_view->SetFEClass(nullptr, fem);
	m_flt->setText("");
	m_view->SetFEClass(pc, fem);
}

void FEClassPropsWidget::on_paramChanged(FSCoreBase* pc, Param* p)
{
	emit paramChanged(pc, p);
}

FSProperty* FEClassPropsWidget::getProperty(const QModelIndex& index)
{
	return m_view->getProperty(index);
}

FSProperty* FEClassPropsWidget::getSelectedProperty()
{
	return m_view->getSelectedProperty();
}

//=============================================================================
class FEClassEditUI
{
public:
	FEClassPropsWidget* feprops;
	QStackedWidget* stack;

	CCurveEditWidget* plt;
	CMathEditWidget* math;

	FSMeshSelection* m_pms;

public:
	void setup(CMainWindow* wnd, QWidget* w)
	{
		m_pms = nullptr;

		feprops = new FEClassPropsWidget;

		stack = new QStackedWidget;
		stack->addWidget(plt  = new CCurveEditWidget);
		stack->addWidget(math = new CMathEditWidget);

		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0, 0, 0, 0);
		l->addWidget(feprops, 1);
		l->addWidget(stack, 2);

		stack->hide();

		w->setLayout(l);

		QObject::connect(feprops, SIGNAL(clicked(const QModelIndex&)), w, SLOT(onItemClicked(const QModelIndex&)));
		QObject::connect(feprops, SIGNAL(paramChanged(FSCoreBase*, Param*)), w, SLOT(on_paramChanged(FSCoreBase*, Param*)));
		QObject::connect(plt, SIGNAL(dataChanged()), w, SLOT(onPlotChanged()));
		QObject::connect(math, SIGNAL(mathChanged(QString)), w, SLOT(onMathChanged(QString)));
	}

	void SetFunction1D(FSFunction1D* pf)
	{
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
				math->ClearVariables();

				// we need to figure out what variable is used as the ordinate
				MSimpleExpression tmp;
				if (tmp.Create(p->GetStringValue(), true))
				{
					for (int i = 0; i < tmp.Variables(); ++i)
					{
						MVariable* vi = tmp.Variable(i);
						if (strstr(vi->Name().c_str(), "fem.") == nullptr)
						{
							// let's assume this is ordinate name
							math->SetOrdinate(QString::fromStdString(vi->Name()));
						}
					}
				}

				FSModel* fem = pf->GetFSModel();
				if (fem)
				{
					for (int i = 0; i < fem->Parameters(); ++i)
					{
						Param& pi = fem->GetParam(i);
						if (pi.GetFlags() & FS_PARAM_USER)
						{
							QString n = QString("fem.%1").arg(pi.GetShortName());
							math->SetVariable(n, pi.GetFloatValue());
						}
					}
				}

				math->SetMath(QString::fromStdString(p->GetStringValue()));
				stack->setCurrentIndex(1);
				stack->show();
			}
			else stack->hide();
		}
	}
};

FEClassEdit::FEClassEdit(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new FEClassEditUI)
{
	ui->setup(wnd, this);
}

void FEClassEdit::SetFEClass(FSCoreBase* pc, FSModel* fem)
{
//	ui->m_pf = nullptr;
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
	else ui->SetFunction1D(nullptr);
}

void FEClassEdit::onMathChanged(QString s)
{
	FSProperty* prop = ui->feprops->getSelectedProperty();
	if (prop && (prop->GetSuperClassID() == FEFUNCTION1D_ID))
	{
		FSFunction1D* pf = dynamic_cast<FSFunction1D*>(prop->GetComponent(0));
		if (pf && pf->IsType("math"))
		{
			Param* p = pf->GetParam("math"); assert(p);
			if(p) p->SetStringValue(s.toStdString());
			return;
		}
	}

	// Whatever is selected is not a math function, so let's just hide the widget
	ui->SetFunction1D(nullptr);
}

void FEClassEdit::onPlotChanged()
{
	FSProperty* prop = ui->feprops->getSelectedProperty();
	if (prop && (prop->GetSuperClassID() == FEFUNCTION1D_ID))
	{
		FSFunction1D* pf = dynamic_cast<FSFunction1D*>(prop->GetComponent(0));
		if (pf && pf->IsType("point"))
		{
			pf->UpdateData(true);
			return;
		}
	}

	// Whatever is selected is not a load curve, so let's just hide the stack
	ui->SetFunction1D(nullptr);
}

void FEClassEdit::on_paramChanged(FSCoreBase* pc, Param* p)
{
	emit paramChanged(pc, p);
}
