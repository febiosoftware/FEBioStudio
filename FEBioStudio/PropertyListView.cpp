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

#include "PropertyListView.h"
#include <QBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QHeaderView>
#include <QLabel>
#include <QComboBox>
#include <QApplication>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QtCore/QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QtCore/QStringListModel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <FSCore/ParamBlock.h>
#include "DataFieldSelector.h"
#include <PostLib/FEMeshData.h>
#include <FSCore/ColorMapManager.h>
#include <CUILib/InputWidgets.h>
#include "DragBox.h"
#include "units.h"


//-----------------------------------------------------------------------------
CEditVariableProperty::CEditVariableProperty(QWidget* parent) : QComboBox(parent)
{
	addItem("<constant>");
	addItem("<math>");
	addItem("<map>");

	setEditable(true);
	setInsertPolicy(QComboBox::NoInsert);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_prop = nullptr;

	QObject::connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
	QObject::connect(this, SIGNAL(editTextChanged(const QString&)), this, SLOT(onEditTextChanged(const QString&)));
}

void CEditVariableProperty::setProperty(CProperty* p, QVariant data)
{
	m_prop = p;
	if (p == nullptr) return;

	blockSignals(true);
	if (p->type == CProperty::Float)
	{
		setCurrentIndex(0);
		setEditText(QString("%1").arg(data.toDouble()));
	}
	else if (p->type == CProperty::MathString)
	{
		setCurrentIndex(1);
		setEditText(data.toString());
	}
	else if (p->type == CProperty::String)
	{
		setCurrentIndex(2);
		setEditText(data.toString());
	}
	else
	{
		assert(false);
	}
	blockSignals(false);
}

void CEditVariableProperty::onCurrentIndexChanged(int index)
{
	switch (index)
	{
	case 0: m_prop->type = CProperty::Float; break;
	case 1: m_prop->type = CProperty::MathString; break;
	case 2: m_prop->type = CProperty::String; break;
	}

	if (m_prop->param)
	{
		Param* p = m_prop->param;
		if (index == 0) p->SetParamType(Param_FLOAT);
		if (index == 1) p->SetParamType(Param_MATH);
		if (index == 2) p->SetParamType(Param_STRING);
	}

	setEditText("0");

	emit typeChanged();
}

void CEditVariableProperty::onEditTextChanged(const QString& txt)
{
	if (txt.isEmpty()) return;
	if (m_prop->param == nullptr) return;

	Param* p = m_prop->param;
	if ((txt[0] == '=') && (p->GetParamType() != Param_MATH))
	{
		m_prop->type = CProperty::MathString;
		p->SetParamType(Param_MATH);
		blockSignals(true);
		setCurrentIndex(1);
		setEditText(txt);
		blockSignals(false);
		emit typeChanged();
	}
	else if ((txt[0] == '\"') && (p->GetParamType() != Param_STRING))
	{
		m_prop->type = CProperty::String;
		p->SetParamType(Param_STRING);
		blockSignals(true);
		setCurrentIndex(2);
		setEditText(txt);
		blockSignals(false);
		emit typeChanged();
	}
}

//-----------------------------------------------------------------------------
class CPropertyListModel : public QAbstractTableModel
{
public:
	CPropertyListModel(QWidget* parent) : QAbstractTableModel(parent) { m_list = 0; }

	void setPropertyList(CPropertyList* plist)
	{
		beginResetModel();
		m_list = plist;
		endResetModel();
	}

	int rowCount(const QModelIndex& parent) const 
	{
		if (m_list) return m_list->Properties();
		return 0;
	}

	int columnCount(const QModelIndex& parent) const { return 2; }

	QVariant headerData(int section, Qt::Orientation orient, int role) const
	{
		if ((orient == Qt::Horizontal)&&(role == Qt::DisplayRole))
		{
			switch (section)
			{
			case 0: return QString("Property"); break;
			case 1: return QString("Value"); break;
			}
		}
		return QAbstractTableModel::headerData(section, orient, role);
	}

	QVariant data(const QModelIndex& index, int role) const
	{
		if ((m_list==0)||(!index.isValid())) return QVariant();

//		if (role == Qt::TextColorRole) return QColor(Qt::color0);

		const CProperty& prop = m_list->Property(index.row());

		if (role == Qt::ToolTipRole)
		{
			QString tip = (tr("<font color=\"black\"><p><b>%1</b></p><p>%2</p></font>").arg(prop.name).arg(prop.info));
			return tip;
		}
		if (index.column() == 0)
		{
			if ((role == Qt::DisplayRole)||(role==Qt::EditRole)) return prop.name;
		}
		else if (index.column() == 1)
		{
			QVariant v = m_list->GetPropertyValue(index.row());
			if (prop.type == CProperty::Int)
			{
				if (role == Qt::DisplayRole)
				{
					if (prop.bauto)
					{
						int n = v.toInt();
						if (n == 0) return QVariant(QString("auto"));
					}
				}
			}
			if (prop.type == CProperty::ColorMap)
			{
				if (role == Qt::DisplayRole)
				{
					int n = v.toInt();
					std::string mapName = ColorMapManager::GetColorMapName(n);
					return QString::fromStdString(mapName);
				}
				else if (role == Qt::EditRole) return v;
			}
			if (prop.type == CProperty::DataScalar)
			{
				if (role == Qt::DisplayRole)
				{
					Post::FEPostModel* fem = Post::FEPostModel::GetInstance(); assert(fem);
					std::string s;
					if (fem)
					{
						s = fem->GetDataManager()->getDataString(v.toInt(), Post::TENSOR_SCALAR);
					}
					if (s.empty()) s = "(select)";
					return QVariant(s.c_str());
				}
				else if (role == Qt::EditRole) return v;
			}
			if (prop.type == CProperty::DataVec3)
			{
				if (role == Qt::DisplayRole)
				{
					Post::FEPostModel* fem = Post::FEPostModel::GetInstance(); assert(fem);
					std::string s;
					if (fem)
					{
						s = fem->GetDataManager()->getDataString(v.toInt(), Post::TENSOR_VECTOR);
					}
					if (s.empty()) s = "(select)";
					return QVariant(s.c_str());
				}
				else if (role == Qt::EditRole) return v;
			}
			if (prop.type == CProperty::DataMat3)
			{
				if (role == Qt::DisplayRole)
				{
					Post::FEPostModel* fem = Post::FEPostModel::GetInstance(); assert(fem);
					std::string s;
					if (fem)
					{
						s = fem->GetDataManager()->getDataString(v.toInt(), Post::TENSOR_TENSOR2);
					}
					if (s.empty()) s = "(select)";
					return QVariant(s.c_str());
				}
				else if (role == Qt::EditRole) return v;
			}
			if (role == Qt::EditRole)
			{
				if ((prop.type == CProperty::Enum)&&(prop.values.isEmpty()==false))
				{
					return prop.values;
				}
				return v;
			}
			if (v.type() == QVariant::Color)
			{
				if (role == Qt::BackgroundRole) return v;
			}
			else if (role == Qt::DisplayRole)
			{
				if (v.type() == QVariant::Bool)
				{
					bool b = v.value<bool>();
					v = (b ? "Yes" : "No");
					return v;
				}
				else if ((prop.type == CProperty::Enum)&&(prop.values.isEmpty()==false))
				{
					int n = v.value<int>();
					if ((n >= 0) && (n < prop.values.count())) return prop.values.at(n);
					else return "(none)";
				}
				else if (prop.type == CProperty::Float)
				{
					Param* p = prop.param;
					if (p && p->GetUnit())
					{
						QString unitString = Units::GetUnitString(p->GetUnit());
						if (unitString.isEmpty() == false)
							v = v.toString() + QString(" %1").arg(unitString);
					}
				}
				return v;
			}
		}

		return QVariant();
	}

	Qt::ItemFlags flags(const QModelIndex& index) const
	{
		if (!index.isValid()) return Qt::NoItemFlags; // does not allow return 0;
		if (index.column() == 1)
		{
			if (m_list->Property(index.row()).isEditable())
				return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
		}
		return Qt::NoItemFlags; // does not allow return 0;
		//		return QAbstractTableModel::flags(index);
	}

	bool setData(const QModelIndex& index, const QVariant& value, int role)
	{
		if (index.isValid() && (role == Qt::EditRole))
		{
			m_list->SetPropertyValue(index.row(), value);
			return true;
		}
		return false;
	}

	const CPropertyList& getPropertyList() const { return *m_list; }
	
private:
	CPropertyList*	m_list;
};

class CPropertyListDelegate : public QStyledItemDelegate
{
private:
	CPropertyListView*	m_view;

public:
	explicit CPropertyListDelegate(CPropertyListView* view) : QStyledItemDelegate(view), m_view(view) {}

	void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const
	{
		if (index.column() == 0)
		{
			QStyleOptionViewItem opt = option;
			initStyleOption(&opt, index);

			opt.font.setBold(true);
	        QStyledItemDelegate::paint(painter, opt, index);
		}

		if (index.column() == 1)
		{
	        QStyledItemDelegate::paint(painter, option, index);

			// Fill the background before calling the base class paint
			// otherwise selected cells would have a white background
/*			QVariant background = index.data(Qt::BackgroundRole);
			if (background.canConvert<QBrush>())
				painter->fillRect(option.rect, background.value<QBrush>());

		    // To draw a border on selected cells
	        if(option.state & QStyle::State_Selected)
			{
				painter->save();
				QPen pen(Qt::black, 2, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
				int w = pen.width()/2;
				painter->setPen(pen);
				painter->drawRect(option.rect.adjusted(w,w,-w,-w));
				painter->restore();
			}
*/		}
    }

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		const CPropertyListModel* model = dynamic_cast<const CPropertyListModel*>(index.model());
		if ((model == 0)||(index.column()==0)) return QStyledItemDelegate::createEditor(parent, option, index);

		int nrow = index.row();
		CPropertyList* propList = m_view->GetPropertyList();
		CProperty* prop = nullptr;
		if (propList)
		{
			if ((nrow >= 0) && (nrow < propList->Properties()))
			{
				prop = &propList->Property(nrow);
			}
		}

		QVariant data = index.data(Qt::EditRole);
		if (prop && (prop->flags & CProperty::Variable))
		{
			CEditVariableProperty* box = new CEditVariableProperty(parent);
			box->setProperty(prop, data);
			return box;
		}

		if (data.type() == QVariant::Bool)
		{
			QComboBox* box = new QComboBox(parent);
			box->addItem("No");
			box->addItem("Yes");
			bool b = data.value<bool>();
			box->setCurrentIndex(b ? 1 : 0);
			m_view->connect(box, SIGNAL(currentIndexChanged(int)), m_view, SLOT(onDataChanged()));
			return box;
		}
		else if (data.type() == QVariant::StringList)
		{
			QComboBox* pc = new QComboBox(parent);
			QStringListModel* pdata = new QStringListModel(pc);
			pdata->setStringList(data.value<QStringList>());
			pc->setModel(pdata);
			pc->setCurrentText(index.data(Qt::DisplayRole).toString());
			m_view->connect(pc, SIGNAL(currentIndexChanged(int)), m_view, SLOT(onDataChanged()));
			return pc;
		}
		else if (data.type() == QVariant::Color)
		{
			CColorButton* pc = new CColorButton(parent);
			pc->setColor(data.value<QColor>());
			m_view->connect(pc, SIGNAL(colorChanged(QColor)), m_view, SLOT(onDataChanged()));
			return pc;
		}
		else if ((data.type() == QVariant::Double)||(data.type() == QMetaType::Float))
		{
			QWidget* pw = 0;
			const CProperty& prop = model->getPropertyList().Property(index.row());
			if (prop.brange)
			{
				CDragBox* pc = new CDragBox(parent);
				pc->setSingleStep(prop.fstep);
				pc->setRange(prop.fmin, prop.fmax);
				pc->textFromValue(data.value<double>());
				pc->setAccelerated(true);
				m_view->connect(pc, SIGNAL(valueChanged(double)), m_view, SLOT(onDataChanged()));
				pw = pc;
			}
			else
			{
				QDoubleValidator* val = new QDoubleValidator;
				val->setDecimals(6);
				QLineEdit* pe = new QLineEdit(parent); pe->setValidator(val);
				m_view->connect(pe, SIGNAL(textEdited(const QString&)), m_view, SLOT(onDataChanged()));
				pw = pe;
			}
			return pw;
		}
		else if (data.type() == QVariant::Int)
		{
			const CProperty& prop = model->getPropertyList().Property(index.row());
			if (prop.type == CProperty::ColorMap)
			{
				CColorMapSelector* pc = new CColorMapSelector(parent);
				pc->setCurrentIndex(data.toInt());
				m_view->connect(pc, SIGNAL(currentIndexChanged(int)), m_view, SLOT(onDataChanged()));
				return pc;
			}
			else if (prop.type == CProperty::DataScalar)
			{
				CDataFieldSelector* pc = new CDataFieldSelector(parent);
				Post::FEPostModel* fem = Post::FEPostModel::GetInstance(); assert(fem);
				if (fem) pc->BuildMenu(fem, Post::TENSOR_SCALAR);
				int nfield = data.toInt();
				pc->setCurrentValue(nfield);
				m_view->connect(pc, SIGNAL(currentValueChanged(int)), m_view, SLOT(onDataChanged()));
				return pc;
			}
			else if (prop.type == CProperty::DataVec3)
			{
				CDataFieldSelector* pc = new CDataFieldSelector(parent);
				Post::FEPostModel* fem = Post::FEPostModel::GetInstance(); assert(fem);
				if (fem) pc->BuildMenu(fem, Post::TENSOR_VECTOR);
				int nfield = data.toInt();
				pc->setCurrentValue(nfield);
				m_view->connect(pc, SIGNAL(currentValueChanged(int)), m_view, SLOT(onDataChanged()));
				return pc;
			}
			else if (prop.type == CProperty::DataMat3)
			{
				CDataFieldSelector* pc = new CDataFieldSelector(parent);
				Post::FEPostModel* fem = Post::FEPostModel::GetInstance(); assert(fem);
				if (fem) pc->BuildMenu(fem, Post::TENSOR_TENSOR2);
				int nfield = data.toInt();
				pc->setCurrentValue(nfield);
				m_view->connect(pc, SIGNAL(currentValueChanged(int)), m_view, SLOT(onDataChanged()));
				return pc;
			}
			else if (prop.type == CProperty::Int)
			{
				if(prop.brange)
				{
					CIntSlider* pc = new CIntSlider(parent);
					pc->setRange(prop.imin, prop.imax);
					pc->setValue(data.toInt());
					m_view->connect(pc, SIGNAL(valueChanged(int)), m_view, SLOT(onDataChanged()));
					return pc;
				}
				else
				{
					QSpinBox* pc = new QSpinBox(parent);
					pc->setRange(prop.imin, prop.imax);
					pc->setValue(data.toInt());
					pc->setAccelerated(true);
					if (prop.bauto) pc->setSpecialValueText("auto");
					m_view->connect(pc, SIGNAL(valueChanged(int)), m_view, SLOT(onDataChanged()));
					return pc;
				}
			}
			else if (prop.type == CProperty::Enum)
			{
				if (prop.values.isEmpty() == false)
				{
					QComboBox* pc = new QComboBox(parent);
					pc->addItems(prop.values);
					pc->setCurrentIndex(data.toInt());
					m_view->connect(pc, SIGNAL(currentIndexChanged(int)), m_view, SLOT(onDataChanged()));
					return pc;
				}
			}
		}
		else if (data.type() == QVariant::String)
		{
			const CProperty& prop = model->getPropertyList().Property(index.row());
			if ((prop.type == CProperty::Std_Vector_Double) ||
				(prop.type == CProperty::Vec2d) ||
				(prop.type == CProperty::Vec3) ||
				(prop.type == CProperty::Mat3s))
			{
				QLineEdit* w = new QLineEdit(parent);
				QObject::connect(w, SIGNAL(editingFinished()), m_view, SLOT(onDataChanged()));
				return w;
			}
		}
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
	{
		if (!index.isValid()) return;

		CDataFieldSelector* sel = dynamic_cast<CDataFieldSelector*>(editor);
		if (sel) {
			int nfield = sel->currentValue();
			model->setData(index, nfield, Qt::EditRole); return;
		}

		CEditVariableProperty* edit = dynamic_cast<CEditVariableProperty*>(editor);

		QComboBox* box = qobject_cast<QComboBox*>(editor);
		if (box && (edit == nullptr)) { model->setData(index, box->currentIndex(), Qt::EditRole); return; }

		CColorButton* col = qobject_cast<CColorButton*>(editor);
		if (col) { model->setData(index, col->color(), Qt::EditRole); return; }

		CIntSlider* slider = dynamic_cast<CIntSlider*>(editor);
		if (slider) { model->setData(index, slider->getValue()); return; }

		QStyledItemDelegate::setModelData(editor, model, index);
	}
};

class Ui::CPropertyListView
{
public:
	CPropertyList*			m_list = nullptr;
	QTableView*				m_prop;
	CPropertyListDelegate*	m_delegate;
	CPropertyListModel*		m_data;
//	QLabel*					m_info;

public:
	void setupUi(::CPropertyListView* parent)
	{
		QVBoxLayout* playout = new QVBoxLayout(parent);
		playout->setContentsMargins(0,0,0,0);

		m_prop = new QTableView;
		m_prop->setObjectName(QStringLiteral("modelProps"));
		m_prop->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_prop->setSelectionMode(QAbstractItemView::SingleSelection);
		m_prop->horizontalHeader()->setStretchLastSection(true);
		m_prop->horizontalHeader()->setResizeContentsPrecision(1000);
//		m_prop->horizontalHeader()->hide();
		m_prop->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//		m_prop->verticalHeader()->setDefaultSectionSize(24);
		m_prop->verticalHeader()->hide();
		m_prop->setEditTriggers(QAbstractItemView::AllEditTriggers);
		parent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

		m_delegate = new CPropertyListDelegate(parent);
		m_prop->setItemDelegate(m_delegate);

		m_data = new CPropertyListModel(m_prop);
		m_prop->setModel(m_data);

/*		m_info = new QLabel;
		m_info->setFrameStyle(QFrame::Panel);
		m_info->setMinimumHeight(50);
*/		
		playout->addWidget(m_prop);
//		playout->addWidget(m_info);

		QMetaObject::connectSlotsByName(parent);
	}
};

//-----------------------------------------------------------------------------
CPropertyListView::CPropertyListView(QWidget* parent) : QWidget(parent), ui(new Ui::CPropertyListView)
{
	ui->setupUi(this);
}

CPropertyList* CPropertyListView::GetPropertyList()
{
	return ui->m_list;
}

//-----------------------------------------------------------------------------
void CPropertyListView::Update(CPropertyList* plist)
{
	ui->m_list = plist;
	ui->m_data->setPropertyList(plist);

	// we make persistent editors for color properties
	if (plist)
	{
		for (int i=0; i<plist->Properties(); ++i)
		{
			const CProperty& p = plist->Property(i);
			if (p.type == CProperty::Color)
			{
				ui->m_prop->openPersistentEditor(ui->m_data->index(i, 1));
			}
		}
	}

	FitGeometry();
	updateGeometry();
}

//-----------------------------------------------------------------------------
void CPropertyListView::FitGeometry()
{
	ui->m_prop->setColumnWidth(0, ui->m_prop->width() / 2);
}

//-----------------------------------------------------------------------------
void CPropertyListView::Refresh()
{
	if (ui->m_list)
		ui->m_data->setPropertyList(ui->m_list);
}

//-----------------------------------------------------------------------------
void CPropertyListView::on_modelProps_clicked(const QModelIndex& index)
{
/*	if (ui->m_list)
	{
		if (index.isValid())
		{
			const CPropertyList::CProperty& pi = ui->m_list->Property(index.row());
			ui->m_info->setText(tr("<p><b>%1</b></p><p>%2</p>").arg(pi.m_name).arg(pi.m_info));
			return;
		}
	}
	ui->m_info->clear();
*/
}

//-----------------------------------------------------------------------------
void CPropertyListView::onDataChanged()
{
	QWidget* pw = qobject_cast<QWidget*>(sender());
	if (pw)
	{
		QModelIndex index = ui->m_prop->currentIndex();
		ui->m_delegate->setModelData(pw, ui->m_data, index);

		// If the list was modified, we need to rebuild the view
		if (ui->m_list->IsModified())
		{
			Update(ui->m_list);
			ui->m_list->SetModified(false);
		}

		emit dataChanged(index.row());
	}
}
