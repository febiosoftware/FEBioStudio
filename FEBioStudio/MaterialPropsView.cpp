#include "stdafx.h"
#include "MaterialPropsView.h"
#include <MeshTools/GMaterial.h>
#include <FEMLib/FEMaterial.h>
#include <QPainter>
#include <QLineEdit>
#include <QComboBox>
#include "units.h"
#include "PropertyList.h"
#include <MeshTools/FEModel.h>


QStringList GetEnumValues(FEModel* fem, const char* ch)
{
	QStringList ops;
	char sz[256] = { 0 };
	if (ch[0] == '$')
	{
		if (fem)
		{
			fem->GetVariableNames(ch, sz);
			ch = sz;
		}
		else ch = 0;
	}

	while (ch && (*ch))
	{
		ops << QString(ch);
		ch = ch + strlen(ch) + 1;
	}

	return ops;
}

class CMaterialPropsModel : public QAbstractItemModel
{
public:
	class Item
	{
	public:
		CMaterialPropsModel*	m_model;

		FEMaterial*		m_pm;			// material pointer
		int				m_paramId;	// index of parameter (or -1 if this is property)
		int				m_propId;		// index of property (or -1 if this is parameter)
		int				m_matIndex;	// index into property's material array

		int		m_nrow;	// row index into parent's children array

	public:
		Item*			m_parent;		// pointer to parent
		vector<Item*>	m_children;	// list of children

	public:
		Item() { m_model = nullptr; m_pm = nullptr; m_parent = nullptr; m_paramId = -1; m_propId = -1; m_matIndex = 0; m_nrow = -1; }
		Item(FEMaterial* pm, int paramId = -1, int propId = -1, int matIndex = 0, int nrow = -1) {
			m_model = nullptr;
			m_pm = pm; m_paramId = paramId; m_propId = propId; m_matIndex = matIndex; m_nrow = nrow;
		}
		~Item() { for (int i = 0; i < m_children.size(); ++i) delete m_children[i]; m_children.clear(); }

		bool isParameter() const { return (m_paramId >= 0); }
		bool isProperty() const { return (m_propId >= 0); }

		Param* parameter() { return (m_paramId >= 0 ? m_pm->GetParamPtr(m_paramId) : nullptr); }

		void addChild(FEMaterial* pm, int paramId, int propId, int matIndex)
		{
			Item* item = new Item(pm, paramId, propId, matIndex, (int)m_children.size());
			item->m_model = m_model; assert(m_model);
			item->m_parent = this;
			m_children.push_back(item);

			if (propId >= 0)
			{
				assert(matIndex >= 0);
				FEMaterialProperty& p = pm->GetProperty(propId);
				FEMaterial* pm = p.GetMaterial(matIndex);
				if (pm) item->addChildren(pm);
			}
		}

		void addParameters(FEMaterial* pm)
		{
			pm->UpdateData(false);
			for (int i = 0; i < pm->Parameters(); ++i)
			{
				Param& p = pm->GetParam(i);
				if ((p.IsVisible() || p.IsEditable()) && (p.IsPersistent() || (m_pm == nullptr)))
					addChild(pm, i, -1, 0);
			}
		}

		void addChildren(FEMaterial* pm)
		{
			addParameters(pm);

			if (dynamic_cast<FETransverselyIsotropic*>(pm))
			{
				FETransverselyIsotropic* tiso = dynamic_cast<FETransverselyIsotropic*>(pm);
				addParameters(&tiso->GetFiberMaterial()->m_fiber);
			}
			else if (pm->HasMaterialAxes())
			{
				addParameters(pm->m_axes);
			}

			for (int i = 0; i < pm->Properties(); ++i)
			{
				FEMaterialProperty& p = pm->GetProperty(i);
				int nc = p.Size();
				for (int j = 0; j < nc; ++j) 
					if (p.GetMaterial(j)) addChild(pm, -1, i, j);
			}
		}

		FEModel* GetFEModel();

		QVariant data(int column, int role)
		{
			if (m_paramId >= 0)
			{
				Param& p = m_pm->GetParam(m_paramId);
				if (column == 0) return p.GetLongName();

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
						if (p.GetEnumNames())
						{
							return GetFEModel()->GetEnumValue(p.GetEnumNames(), n);
						}
						return n;
					}
					break;
					case Param_VEC3D: return Vec3dToString(p.val<vec3d>()); break;
					default:
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
						return p.val<int>(); break;
					case Param_VEC3D: return Vec3dToString(p.val<vec3d>()); break;
					default:
						return "in progress";
					}
				}
			}
			else if (m_propId >= 0)
			{
				FEMaterialProperty& p = m_pm->GetProperty(m_propId);
				if (column == 0)
				{
					QString s = QString::fromStdString(p.GetName());
					if (p.maxSize() != 1)
					{
						s += QString(" - %1").arg(m_matIndex + 1);
					}
					return s;
				}
				else
				{
					FEMaterial* pm = p.GetMaterial(m_matIndex);
					return pm->TypeStr();
				}
			}
			else return "No data";
		}

		bool setData(int column, const QVariant& value)
		{
			if (column != 1) return false;

			if (m_paramId >= 0)
			{
				Param& p = m_pm->GetParam(m_paramId);
				switch (p.GetParamType())
				{
				case Param_FLOAT: p.SetFloatValue(value.toDouble()); break;
				case Param_CHOICE:
				case Param_INT: 
				{
					int n = value.toInt();
					p.SetIntValue(n);
				}
				break;
				case Param_VEC3D: p.SetVec3dValue(StringToVec3d(value.toString())); break;
				}

				return m_pm->UpdateData(true);
			}
			return false;
		}

		Item* parent() { return m_parent; }

		Item* child(int n) { return ((n >= 0) && (n < children()) ? m_children[n] : nullptr); }

		int row() const { return m_nrow; }

		int children() const { return (int)m_children.size(); }
	};

public:
	explicit CMaterialPropsModel(QObject* parent = nullptr) : QAbstractItemModel(parent)
	{
		m_root = nullptr;
		m_valid = true;
	}

	~CMaterialPropsModel() { delete m_root; }

	void SetMaterial(GMaterial* mat)
	{
		m_valid = true;
		beginResetModel();
		delete m_root;
		m_root = nullptr;
		m_mat = mat;
		if (mat)
		{
			m_root = new Item();
			m_root->m_model = this;
			m_root->addChildren(m_mat->GetMaterialProperties());
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
		if ((role == Qt::FontRole) && item->isProperty())
		{
			QFont font;
			font.setBold(true);
			return font;
		}

		if ((role != Qt::DisplayRole)&& (role != Qt::EditRole)) return QVariant();

		return item->data(index.column(), role);
	}

	Qt::ItemFlags flags(const QModelIndex& index) const
	{
		if (!index.isValid()) return Qt::ItemIsEnabled;
		if (index.column() == 1)
		{
			Item* item = static_cast<Item*>(index.internalPointer());
			if (item->isParameter())
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
		SetMaterial(m_mat);
	}

private:
	GMaterial*		m_mat;
	Item*			m_root;
	bool		m_valid;
};

FEModel* CMaterialPropsModel::Item::GetFEModel()
{
	return m_model->m_mat->GetModel();
}

//=================================================================================================
CMaterialPropsDelegate::CMaterialPropsDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

QWidget* CMaterialPropsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.isValid())
	{
		CMaterialPropsModel::Item* item = static_cast<CMaterialPropsModel::Item*>(index.internalPointer());

		if (item->isParameter())
		{
			Param* p = item->parameter();
			if (p->GetParamType() == Param_FLOAT)
			{
				QLineEdit* pw = new QLineEdit(parent);
				pw->setValidator(new QDoubleValidator);
				return pw;
			}
			if ((p->GetParamType() == Param_INT) || (p->GetParamType() == Param_CHOICE))
			{
				if (p->GetEnumNames())
				{
					QComboBox* box = new QComboBox(parent);
					QStringList enumValues = GetEnumValues(item->GetFEModel(), p->GetEnumNames());
					box->addItems(enumValues);
					QObject::connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(OnEditorSignal()));
					return box;
				}
			}
		}
	}
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void CMaterialPropsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if (!index.isValid()) return;
	if (dynamic_cast<QComboBox*>(editor))
	{
		QComboBox* pw = dynamic_cast<QComboBox*>(editor);
		CMaterialPropsModel::Item* item = static_cast<CMaterialPropsModel::Item*>(index.internalPointer());
		QVariant v = item->data(1, Qt::EditRole);
		pw->setCurrentIndex(v.toInt());
	}
	else QStyledItemDelegate::setEditorData(editor, index);
}

void CMaterialPropsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	if (!index.isValid()) return;
	if (dynamic_cast<QComboBox*>(editor))
	{
		QComboBox* pw = dynamic_cast<QComboBox*>(editor);
		model->setData(index, pw->currentIndex());
	}
	else QStyledItemDelegate::setModelData(editor, model, index);
}

void CMaterialPropsDelegate::OnEditorSignal()
{
	QWidget* sender = dynamic_cast<QWidget*>(QObject::sender());
	emit commitData(sender);
}

//=================================================================================================
CMaterialPropsView::CMaterialPropsView(QWidget* parent) : QTreeView(parent)
{
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setUniformRowHeights(true);
	setEditTriggers(QAbstractItemView::AllEditTriggers);

	setItemDelegate(new CMaterialPropsDelegate);
	m_model = new CMaterialPropsModel;
	setModel(m_model);

	QObject::connect(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)), this, SLOT(onModelDataChanged()));
}

void CMaterialPropsView::SetMaterial(GMaterial* mat)
{
	m_model->SetMaterial(mat);
	expandAll();
	setColumnWidth(0, width() / 2);
}

void CMaterialPropsView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
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

void CMaterialPropsView::onModelDataChanged()
{
	if (m_model->IsValid() == false)
	{
		m_model->ResetModel();
		expandAll();
	}
}
