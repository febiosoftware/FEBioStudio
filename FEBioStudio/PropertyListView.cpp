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
					if (n >= 0) return prop.values.at(n);
					else return "(none)";
				}
				return v;
			}
		}

		return QVariant();
	}

	Qt::ItemFlags flags(const QModelIndex& index) const
	{
		if (!index.isValid()) return 0;
		if (index.column() == 1)
		{
			if (m_list->Property(index.row()).isEditable())
				return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
		}
		return 0;
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

		QVariant data = index.data(Qt::EditRole);		
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
			QLineEdit* pc = new QLineEdit(parent);
			const CProperty& prop = model->getPropertyList().Property(index.row());
			pc->setValidator(new QDoubleValidator);
//			pc->setRange(prop.fmin, prop.fmax);
			pc->setText(QString::number(data.value<double>()));
//			pc->setAccelerated(true);
			m_view->connect(pc, SIGNAL(editingFinished()), m_view, SLOT(onDataChanged()));
			return pc;
		}
		else if (data.type() == QVariant::Int)
		{
			const CProperty& prop = model->getPropertyList().Property(index.row());
			if (prop.type == CProperty::Int)
			{
				QSpinBox* pc = new QSpinBox(parent);
				pc->setRange(prop.imin, prop.imax);
				pc->setValue(data.toInt());
				pc->setAccelerated(true);
				if (prop.bauto) pc->setSpecialValueText("auto");
				m_view->connect(pc, SIGNAL(valueChanged(int)), m_view, SLOT(onDataChanged()));
				return pc;
			}
			else if (prop.type == CProperty::Enum)
			{
				if (prop.values.isEmpty() == false)
				{
					QComboBox* pc = new QComboBox;
					pc->addItems(prop.values);
					pc->setCurrentIndex(data.toInt());
					m_view->connect(pc, SIGNAL(currentIndexChanged(int)), m_view, SLOT(onDataChanged()));
					return pc;
				}
			}
		}
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
	{
		if (!index.isValid()) return;

		QComboBox* box = qobject_cast<QComboBox*>(editor);
		if (box) { model->setData(index, box->currentIndex(), Qt::EditRole); return; }

		CColorButton* col = qobject_cast<CColorButton*>(editor);
		if (col) { model->setData(index, col->color(), Qt::EditRole); return; }

		QStyledItemDelegate::setModelData(editor, model, index);
	}
};

class Ui::CPropertyListView
{
public:
	CPropertyList*			m_list;
	QTableView*				m_prop;
	CPropertyListDelegate*	m_delegate;
	CPropertyListModel*		m_data;
//	QLabel*					m_info;

public:
	void setupUi(::CPropertyListView* parent)
	{
		QVBoxLayout* playout = new QVBoxLayout(parent);
		playout->setMargin(0);

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

		emit dataChanged();
	}
}
