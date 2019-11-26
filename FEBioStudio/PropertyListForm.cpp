#include "PropertyListForm.h"
#include "PropertyList.h"
#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QListWidget>
#include "CIntInput.h"
#include "GLHighlighter.h"
#include "ResourceEdit.h"
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include "CurvePicker.h"
#include "DataFieldSelector.h"
#include "PropertyListView.h"

//=================================================================================================

//-----------------------------------------------------------------------------
// constructor
CPropertyListForm::CPropertyListForm(QWidget* parent) : QWidget(parent)
{
	ui = new QVBoxLayout;
//	ui->setMargin(0);
	setLayout(ui);

	m_list = 0;
}

//-----------------------------------------------------------------------------
// clear the form

void clearLayout(QLayout* layout)
{
	// delete all children in the layout
    QLayoutItem* item;
    while (layout->count() && (item = layout->takeAt(0)))
    {
		if (QWidget* w = item->widget()) w->deleteLater();
		if (QLayout* l = item->layout()) clearLayout(l); 
		delete item;
    }
}

void CPropertyListForm::clear()
{
	// clear the layout
	clearLayout(ui);

	// reset list
	m_list = 0;

	m_widget.clear();
}

//-----------------------------------------------------------------------------
// attach a property list to this form
void CPropertyListForm::setPropertyList(CPropertyList* pl)
{
	// clear the current list
	clear();

	// set the new list
	m_list = pl;
	if (pl == 0) return;

	QGroupBox* pg = 0;
	QFormLayout* form = 0;

	// loop over all properties
	int nprops = pl->Properties();
	for (int i=0; i<nprops; ++i)
	{
		CProperty& pi = pl->Property(i);
		QVariant v = pl->GetPropertyValue(i);
		QWidget* pw = 0;
		QString label = pi.name;

		// see if we need to create a group
		if (pi.type == CProperty::Group)
		{
			pg = new QGroupBox(pi.name);
			ui->addWidget(pg);
			m_widget.push_back(pw);
			pg->setFlat(true);

			pg->setStyleSheet("QGroupBox { font-weight: bold; } ");

			pg->setAlignment(Qt::AlignCenter);

			// we must create a new form
			form = 0;
		}
		else
		{
			// make sure we have a form
			if (form == 0)
			{
				form = new QFormLayout;
				form->setLabelAlignment(Qt::AlignRight);
//				form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
				form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
				form->setVerticalSpacing(2);

				if (pg != 0)
				{
					pg->setLayout(form);
				}
				else
				{
					ui->addLayout(form);
				}
			}

			pw = createPropertyEditor(pi, v);
			if (pi.isEditable() == false) pw->setDisabled(true);

			// add the widget (if defined)
			if (pw) form->addRow(label, pw);

			// add the widget to the list.
			// Note that we always add the pw, even when it's zero. 
			// This is because the m_widget list must have the same size as the property list
			m_widget.push_back(pw);
		}
	}		
}

//-----------------------------------------------------------------------------
// create an editor for a property and set the initial value
QWidget* CPropertyListForm::createPropertyEditor(CProperty& pi, QVariant v)
{
	if (pi.flags & CProperty::Variable)
	{
		CEditVariableProperty* box = new CEditVariableProperty();
		box->setProperty(&pi, v);
		connect(box, SIGNAL(editTextChanged(const QString&)), this, SLOT(onDataChanged()));
		return box;
	}

	if (pi.isEditable() == false)
	{
		QLineEdit* edit = new QLineEdit;
		return edit;
	}

	switch (pi.type)
	{
	case CProperty::Int:
		{
			QSpinBox* spin = new QSpinBox;
			spin->setRange(pi.imin, pi.imax);
			spin->setValue(v.toInt());
			connect(spin, SIGNAL(valueChanged(int)), this, SLOT(onDataChanged()));
			return spin;
		}
		break;
	case CProperty::Float:
		{
/*			QDoubleSpinBox* spin = new QDoubleSpinBox;
			spin->setRange(pi.fmin, pi.fmax);
			spin->setSingleStep(pi.fstep);
			spin->setValue(v.toDouble());
			connect(spin, SIGNAL(valueChanged(double)), this, SLOT(onDataChanged()));
			pw = spin;
*/
			QLineEdit* edit = new QLineEdit;
			edit->setValidator(new QDoubleValidator);
			edit->setText(QString::number(v.toDouble(),'g', 3));
			connect(edit, SIGNAL(textChanged(const QString&)), this, SLOT(onDataChanged()));
			return edit;
		}
		break;
	case CProperty::Enum:
		{
			if (pi.values.isEmpty() == false)
			{
				if (pi.isEditable())
				{
					QComboBox* pc = new QComboBox;
					pc->setMinimumWidth(100);
					pc->addItems(pi.values);
					pc->setCurrentIndex(v.toInt());
					connect(pc, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataChanged()));
					return pc;
				}
				else
				{
					QLabel* l = new QLabel(pi.values.at(v.toInt()));
					return l;
				}
			}
			else
			{
				QLineEdit* pc = new QLineEdit;
				pc->setText("(undefined)");
				pc->setReadOnly(true);
				return pc;
			}
		}
		break;
	case CProperty::Bool:
		{
/*			QComboBox* pc = new QComboBox;
			pc->addItem("No");
			pc->addItem("Yes");
			pc->setCurrentIndex(v.toInt());
			connect(pc, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataChanged()));
			pw = pc;
*/
			QCheckBox* pc = new QCheckBox();
			pc->setChecked(v.toBool());
			connect(pc, SIGNAL(toggled(bool)), this, SLOT(onDataChanged()));
			return pc;
		}
		break;
	case CProperty::String:
		{
			QLineEdit* edit = new QLineEdit;
			edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			edit->setText(v.toString());
			connect(edit, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
			return edit;
		}
		break;
	case CProperty::Resource:
		{
			CResourceEdit* edit = new CResourceEdit;
			edit->setResourceName(v.toString());
			if (pi.values.isEmpty() == false) edit->setResourceFilter(pi.values);
			connect(edit, SIGNAL(resourceChanged()), this, SLOT(onDataChanged()));
			return edit;
		}
		break;
	case CProperty::Curve:
		{
			CCurvePicker* pick = new CCurvePicker;
			pick->setCurve(v.toString());
			connect(pick, SIGNAL(curveChanged()), this, SLOT(onDataChanged()));
			return pick;
		}
		break;
	case CProperty::CurveList:
		{
			CCurveListPicker* pick = new CCurveListPicker;
			pick->setCurves(v.toStringList());
			connect(pick, SIGNAL(curveChanged()), this, SLOT(onDataChanged()));
			return pick;
		}
		break;
	case CProperty::Color:
		{
			CColorButton* col = new CColorButton;
			col->setColor(v.value<QColor>());
			connect(col, SIGNAL(colorChanged(QColor)), this, SLOT(onDataChanged()));
			return col;
		}
		break;
	case CProperty::Action:
		{
			QPushButton* b = new QPushButton(pi.info);
			connect(b, SIGNAL(clicked(bool)), this, SLOT(onDataChanged()));
			return b;
		}
		break;
	case CProperty::Vec3:
		{
			QLineEdit* edit = new QLineEdit;
			edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			edit->setText(v.toString());
			connect(edit, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
			return edit;
		}
		break;
	case CProperty::Vec2i:
		{
			QLineEdit* edit = new QLineEdit;
			edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			edit->setText(v.toString());
			connect(edit, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
			return edit;
		}
		break;
	case CProperty::Mat3:
		{
			QLineEdit* edit = new QLineEdit;
			edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			edit->setText(v.toString());
			connect(edit, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
			return edit;
		}
		break;
	default:
		assert(false);
	}

	return 0;
}

//-----------------------------------------------------------------------------
// update the value of the editors
void CPropertyListForm::updateData()
{
	// make sure we have a list
	if (m_list == 0) return;

	// loop over all widgets
	int nprops = m_list->Properties();
	QList<QWidget*>::iterator it = m_widget.begin();
	for (int i=0; i<nprops; ++i, ++it)
	{
		QWidget* pw = *it;
		if (pw)
		{
			const CProperty& pi = m_list->Property(i);
			QVariant v = m_list->GetPropertyValue(i);

			if (pi.isEditable())
			{
				switch (pi.type)
				{
				case CProperty::Int:
					{
						QSpinBox* spin = qobject_cast<QSpinBox*>(pw);
						if (spin)
						{
							spin->setRange(pi.imin, pi.imax);
							spin->setValue(v.toInt());
						}
					}
					break;
				case CProperty::Float:
					{
/*						QDoubleSpinBox* spin = qobject_cast<QDoubleSpinBox*>(pw);
						if (spin)
						{
							spin->setRange(pi.fmin, pi.fmax);
							spin->setSingleStep(pi.fstep);
							spin->setValue(v.toDouble());
						}
*/
						QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
						if (edit) edit->setText(QString::number(v.toDouble()));
					}
					break;
				case CProperty::Enum:
					{
						QComboBox* pc = qobject_cast<QComboBox*>(pw);
						if (pc)
						{
							pc->clear();
							pc->addItems(pi.values);
							pc->setCurrentIndex(v.toInt());
						}
					}
					break;
				case CProperty::Bool:
					{
//						QComboBox* pc = qobject_cast<QComboBox*>(pw);
//						if (pc) pc->setCurrentIndex(v.toInt());
						QCheckBox* pc = qobject_cast<QCheckBox*>(pw);
						pc->setChecked(v.toBool());		
					}
					break;
				case CProperty::String:
					{
						QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
						if (edit) edit->setText(v.toString());
					}
					break;
				case CProperty::Resource:
					{
						CResourceEdit* edit = qobject_cast<CResourceEdit*>(pw);
						if (edit) edit->setResourceName(v.toString());
					}
					break;
				case CProperty::Curve:
					{
						CCurvePicker* pick = qobject_cast<CCurvePicker*>(pw);
						if (pick) pick->setCurve(v.toString());
					}
					break;
				case CProperty::CurveList:
					{
						CCurveListPicker* pick = qobject_cast<CCurveListPicker*>(pw);
						if (pick) pick->setCurves(v.toStringList());
					}
					break;
				}
			}
			else
			{
				QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
				if (edit)
				{
					if (pi.type == CProperty::Float)
					{
						edit->setText(QString::number(v.toDouble()));
					}
					else if (pi.type == CProperty::Int)
					{
						edit->setText(QString::number(v.toInt()));
					}
					else edit->setText(v.toString());
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// catch all function for when an editor inside the form is changed.
void CPropertyListForm::onDataChanged()
{
	// make sure we have a property list attached
	if (m_list==0) return;

	// get the sending widget
	QWidget* pw = qobject_cast<QWidget*>(sender());
	if (pw == 0) return;

	// Locate the sender widget and its associated property
	QList<QWidget*>::iterator it = m_widget.begin();
	for (int i=0; i<m_widget.size(); ++i, ++it)
	{
		if (*it==pw)
		{
			const CProperty& pi = m_list->Property(i);
			if (pi.isEditable())
			{
				switch (pi.type)
				{
				case CProperty::Int:
					{
						QSpinBox* spin = qobject_cast<QSpinBox*>(pw);
						if (spin) m_list->SetPropertyValue(i, spin->value());
					}
					break;
				case CProperty::Float:
					{
//						QDoubleSpinBox* spin = qobject_cast<QDoubleSpinBox*>(pw);
//						if (spin) m_list->SetPropertyValue(i, spin->value());
						QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
						if (edit) m_list->SetPropertyValue(i, edit->text().toDouble());

						CEditVariableProperty* var = qobject_cast<CEditVariableProperty*>(pw);
						if (var)
						{
							QString s = var->currentText();
							m_list->SetPropertyValue(i, s.toDouble());
						}
					}
					break;
				case CProperty::Enum:
					{
						QComboBox* pc = qobject_cast<QComboBox*>(pw);
						if (pc) m_list->SetPropertyValue(i, pc->currentIndex());
					}
					break;
				case CProperty::Bool:
					{
//						QComboBox* pc = qobject_cast<QComboBox*>(pw);
//						if (pc) m_list->SetPropertyValue(i, pc->currentIndex());
						QCheckBox* pc = qobject_cast<QCheckBox*>(pw);
						if (pc) m_list->SetPropertyValue(i, pc->isChecked());
					}
					break;
				case CProperty::String:
					{
						QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
						if (edit) m_list->SetPropertyValue(i, edit->text());
					}
					break;
				case CProperty::DataScalar:
					{
						CDataFieldSelector* pc = dynamic_cast<CDataFieldSelector*>(pw);
						if (pc) m_list->SetPropertyValue(i, pc->currentValue());
					}
					break;
				case CProperty::ColorMap:
					{
						CColorMapSelector* pc = dynamic_cast<CColorMapSelector*>(pw);
						if (pc) m_list->SetPropertyValue(i, pc->currentIndex());
					}
					break;
				case CProperty::DataVec3:
					{
						CDataFieldSelector* pc = dynamic_cast<CDataFieldSelector*>(pw);
						if (pc) m_list->SetPropertyValue(i, pc->currentValue());
					}
					break;
				case CProperty::DataMat3:
					{
						CDataFieldSelector* pc = dynamic_cast<CDataFieldSelector*>(pw);
						if (pc) m_list->SetPropertyValue(i, pc->currentValue());
					}
					break;
				case CProperty::Resource:
					{
						CResourceEdit* edit = qobject_cast<CResourceEdit*>(pw);
						if (edit) m_list->SetPropertyValue(i, edit->resourceName());
					}
					break;
				case CProperty::Curve:
					{
						CCurvePicker* pick = qobject_cast<CCurvePicker*>(pw);
						if (pick) m_list->SetPropertyValue(i, pick->curveName());
					}
					break;
				case CProperty::CurveList:
					{
						CCurveListPicker* pick = qobject_cast<CCurveListPicker*>(pw);
						if (pick) m_list->SetPropertyValue(i, pick->curveNames());
					}
					break;
				case CProperty::Action:
					{
						QPushButton* b = qobject_cast<QPushButton*>(pw);
						if (b) m_list->SetPropertyValue(i, true);
					}
					break;
				case CProperty::MathString:
					{
						CEditVariableProperty* e = qobject_cast<CEditVariableProperty*>(pw);
						if (e) m_list->SetPropertyValue(i, e->currentText());
					}
					break;
				case CProperty::Vec3:
					{
						QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
						if (edit) m_list->SetPropertyValue(i, edit->text());
					}
					break;
				case CProperty::Vec2i:
					{
						QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
						if (edit) m_list->SetPropertyValue(i, edit->text());
					}
					break;
				case CProperty::Mat3:
					{
						QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
						if (edit) m_list->SetPropertyValue(i, edit->text());
					}
					break;
				}
			}

			bool itemModified = m_list->IsModified();
			if (itemModified)
			{
				setPropertyList(m_list);
				m_list->SetModified(false);
			}

			// send a signal that the data was changed
			emit dataChanged(itemModified);
			return;
		}
	}
}
