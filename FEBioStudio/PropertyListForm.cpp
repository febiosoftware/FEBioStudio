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

#include "PropertyListForm.h"
#include "PropertyList.h"
#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QListWidget>
#include <QToolButton>
#include "InputWidgets.h"
#include "GLHighlighter.h"
#include "ResourceEdit.h"
#include "LinkPropertyEdit.h"
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QEvent>
#include "CurvePicker.h"
#include "DataFieldSelector.h"
#include "PropertyListView.h"
#include "units.h"

//=================================================================================================

//-----------------------------------------------------------------------------
// constructor
CPropertyListForm::CPropertyListForm(QWidget* parent) : QWidget(parent)
{
	ui = new QVBoxLayout;
//	ui->setContentsMargins(0,0,0,0);
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

	m_widgets.clear();
}

class MyCheckBox : public QCheckBox
{
public:
	MyCheckBox(QWidget* parent = nullptr) : QCheckBox(parent) { m_data = -1; };
public:
	int	m_data;
};

QString unitString(CProperty& p)
{
	QString s;
	if (p.param && p.param->GetUnit())
	{
		QString us = Units::GetUnitString(p.param->GetUnit());
		if (us.isEmpty() == false) s = QString(" (%1)").arg(us);
	}
	return s;
}

//================================================================================
CWrapperBox::CWrapperBox(const QString& name, QWidget* parent) : QFrame(parent)
{
	setFrameStyle(QFrame::StyledPanel);

	setSizePolicy(sizePolicy().horizontalPolicy(), QSizePolicy::Maximum);

	m_pc = new QComboBox;
	m_pc->installEventFilter(new CMouseWheelFilter);

	m_pc->setSizePolicy(QSizePolicy::Expanding, m_pc->sizePolicy().verticalPolicy());
	QHBoxLayout* h = new QHBoxLayout;
	h->setContentsMargins(1, 1, 1, 1);
	m_tb = new QToolButton;
	m_tb->setText(QChar(0x25BC));
	m_tb->setAutoRaise(true);
	h->addWidget(m_tb);
	h->addWidget(new QLabel(QString("<b>%1</b>").arg(name)));
	h->addWidget(m_pc);
	QVBoxLayout* vl = new QVBoxLayout;
	vl->setContentsMargins(0, 0, 0, 0);
	vl->addLayout(h);
	m_pg = new QGroupBox;
	vl->addWidget(m_pg);
	setLayout(vl);

	QObject::connect(m_tb, SIGNAL(clicked()), SLOT(OnExpandClicked()));
}

void CWrapperBox::addItems(const QStringList& items) { m_pc->addItems(items); }
void CWrapperBox::setCurrentIndex(int n) { m_pc->setCurrentIndex(n); }

void CWrapperBox::OnExpandClicked()
{
	if (m_pg->isVisible())
	{
		m_pg->hide();
		m_tb->setText(QChar(0x2BC8));
	}
	else
	{
		m_pg->show();
		m_tb->setText(QChar(0x2BC6));
	}
}

//-----------------------------------------------------------------------------
// get the property list
CPropertyList* CPropertyListForm::getPropertyList()
{
	return m_list;
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
        if(!pi.isVisible()) continue;

		QVariant v = pl->GetPropertyValue(i);
		QWidget* pw = 0;
		QString label = pi.name;

		label += unitString(pi);

//		if (label.isEmpty() == false) label += ":";

		// see if we need to create a group
		if (pi.type == CProperty::Group)
		{
			if (pi.values.isEmpty())
			{
				pg = new QGroupBox(pi.name);
				ui->addWidget(pg);
                m_widgets[pw] = i;
				pg->setFlat(true);

				pg->setStyleSheet("QGroupBox { font-weight: bold; } ");

				pg->setAlignment(Qt::AlignCenter);
			}
			else
			{
				CWrapperBox* pw = new CWrapperBox(pi.name);
				pw->addItems(pi.values);
				pg = pw->m_pg;
				ui->addWidget(pw);

				int index = v.toInt();
				pw->setCurrentIndex(index);

                m_widgets[pw->m_pc] = i;
				connect(pw->m_pc, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataChanged()));
			}

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
			if (pi.isEditable() == false)
			{
				QLineEdit* pe = dynamic_cast<QLineEdit*>(pw);
				if (pe) pe->setReadOnly(true);
				else pw->setDisabled(true);
			}

			if (pi.param && pi.param->IsCheckable() && pw)
			{
				QWidget* tmp = new QWidget;
				QHBoxLayout* l = new QHBoxLayout;
				MyCheckBox* c = new MyCheckBox;
				c->m_data = i;
				c->setChecked(pi.param->IsChecked());
				l->setContentsMargins(0,0,0,0);
				l->addWidget(c);
				l->addWidget(pw);
				tmp->setLayout(l);

				QObject::connect(c, SIGNAL(stateChanged(int)), this, SLOT(onCheckStateChanged(int)));

				// add the widget (if defined)
				if (pw) form->addRow(label, tmp);
			}
			else if (pi.param && pw)
			{
				if (pi.param->IsEditable())
				{
					QCheckBox* pc = dynamic_cast<QCheckBox*>(pw);
					if (pc)
					{
						pc->setText(label);
						label = "";
					}

					form->addRow(label, pw);
				}
				else
				{
					QLabel* l = new QLabel(label);
					l->setStyleSheet("QLabel {color: gray;}");
					form->addRow(l, pw);
				}
			}
			else
			{
				// add the widget (if defined)
				if (pw) form->addRow(label, pw);
			}

			// add the widget to the list.
			// Note that we always add the pw, even when it's zero. 
			// This is because the m_widget list must have the same size as the property list
            m_widgets[pw] = i;
		}
	}
//	ui->addStretch();
}

//-----------------------------------------------------------------------------
bool CMouseWheelFilter::eventFilter(QObject* po, QEvent* ev)
{
	if (ev->type() == QEvent::Wheel) {
		qDebug("Ate wheel event");
		return true;
	}
	else 
	{
		// standard event processing
		return QObject::eventFilter(po, ev);
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

	switch (pi.type)
	{
	case CProperty::Int:
		{
			if (pi.isEditable())
			{
				if (pi.brange)
				{
					QSpinBox* spin = new QSpinBox;
					spin->setRange(pi.imin, pi.imax);
					spin->setValue(v.toInt());
					spin->setSizePolicy(QSizePolicy::Expanding, sizePolicy().verticalPolicy());
					connect(spin, SIGNAL(valueChanged(int)), this, SLOT(onDataChanged()));
					return spin;
				}
				else
				{
					QLineEdit* edit = new QLineEdit;
					edit->setValidator(new QIntValidator);
					edit->setText(QString::number(v.toInt()));
					connect(edit, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
					return edit;
				}
			}
			else
			{
				QLineEdit* pw = new QLineEdit;
				pw->setReadOnly(true);
				pw->setText(QString::number(v.toInt()));
				return pw;
			}
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
			edit->setText(QString::number(v.toDouble()));
			connect(edit, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
			return edit;
		}
		break;
	case CProperty::Enum:
		{
			if (pi.isEditable())
			{
				QComboBox* pc = new QComboBox;
				pc->installEventFilter(new CMouseWheelFilter);
				pc->setMinimumWidth(100);
				pc->addItems(pi.values);
				pc->setCurrentIndex(v.toInt());
				pc->setSizePolicy(QSizePolicy::Expanding, sizePolicy().verticalPolicy());
				connect(pc, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataChanged()));
				return pc;
			}
			else
			{
				QLabel* l = new QLabel("(none)");
				if (pi.values.empty() == false){ l->setText(pi.values.at(v.toInt())); }
				return l;
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
	case CProperty::MathString:
		{
			if (pi.values.empty() == false)
			{
				QComboBox* box = new QComboBox();
				box->setSizePolicy(QSizePolicy::Expanding, sizePolicy().verticalPolicy());
				QStringList enumValues = pi.values;
				box->addItems(enumValues);
				QString s = v.toString();
				int n = box->findText(s);
				box->setCurrentIndex(n);
				QObject::connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataChanged()));
				return box;
			}
			else
			{
				QLineEdit* edit = new QLineEdit;
				edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
				edit->setText(v.toString());
				connect(edit, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
				return edit;
			}
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
	case CProperty::InternalLink:
		{
			QStringList fileNames = v.toStringList();
			if (fileNames.size() != 2)
			{
				fileNames.clear();
				fileNames.append("(none)");
				fileNames.append("(none)");
			}
			CLinkPropertyEdit* edit = new CLinkPropertyEdit(fileNames[0], fileNames[1], true);
			return edit;
		}
		break;
	case CProperty::ExternalLink:
		{
			QStringList fileNames = v.toStringList();
			if (fileNames.size() != 2)
			{
				fileNames.clear();
				fileNames.append("(none)");
				fileNames.append("(none)");
			}
			CLinkPropertyEdit* edit = new CLinkPropertyEdit(fileNames[0], fileNames[1]);
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
			QPushButton* b = new QPushButton(v.isValid() ? v.toString() : pi.info);
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
	case CProperty::Std_Vector_Int:
		{
			if (pi.values.empty())
			{
				QLineEdit* edit = new QLineEdit;
				edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
				edit->setText(v.toString());
				connect(edit, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
				return edit;
			}
			else
			{
				std::vector<int> l = StringToVectorInt(v.toString());
				QListWidget* pw = new QListWidget;
				for (int i = 0; i < pi.values.size(); ++i)
				{
					QListWidgetItem* it = new QListWidgetItem;
					it->setFlags(it->flags() | Qt::ItemFlag::ItemIsUserCheckable);

					if (std::find(l.begin(), l.end(), i) != l.end())
						it->setCheckState(Qt::Checked);
					else
						it->setCheckState(Qt::Unchecked);

					it->setText(pi.values[i]);
					pw->addItem(it);
				}
				connect(pw, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onDataChanged()));
				return pw;
			}
		}
		break;
	case CProperty::Std_Vector_Double:
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
	for (auto pair : m_widgets)
	{
		QWidget* pw = pair.first;
		if (pw)
		{
            int index = pair.second;

			const CProperty& pi = m_list->Property(index);
			QVariant v = m_list->GetPropertyValue(index);

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

						QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
						if (edit)
						{
							edit->setText(QString::number(v.toInt()));
						}

						CIntSlider* slider = dynamic_cast<CIntSlider*>(pw);
						if (slider) 
						{ 
							slider->setValue(v.toInt());
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
							pc->blockSignals(true);
							pc->clear();
							pc->addItems(pi.values);
							pc->setCurrentIndex(v.toInt());
							pc->blockSignals(false);
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
void CPropertyListForm::onCheckStateChanged(int m)
{
	// get the sending widget
	QWidget* pw = qobject_cast<QWidget*>(sender());
	if (pw == 0) return;

	MyCheckBox* pc = dynamic_cast<MyCheckBox*>(pw);
	if (pc)
	{
		int n = pc->m_data;
		assert(n >= 0);
		CProperty& p = m_list->Property(n);

		if (p.param && p.param->IsCheckable())
		{
			p.param->SetChecked(pc->isChecked());
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

    int propIndex = m_widgets.at(pw);

    const CProperty& pi = m_list->Property(propIndex);
    if (pi.isEditable())
    {
        switch (pi.type)
        {
        case CProperty::Int:
            {
                QSpinBox* spin = qobject_cast<QSpinBox*>(pw);
                if (spin) m_list->SetPropertyValue(propIndex, spin->value());
                else
                {
                    QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
                    if (edit) m_list->SetPropertyValue(propIndex, edit->text().toInt());
                }
            }
            break;
        case CProperty::Float:
            {
//						QDoubleSpinBox* spin = qobject_cast<QDoubleSpinBox*>(pw);
//						if (spin) m_list->SetPropertyValue(i, spin->value());
                QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
                if (edit) m_list->SetPropertyValue(propIndex, edit->text().toDouble());

                CEditVariableProperty* var = qobject_cast<CEditVariableProperty*>(pw);
                if (var)
                {
                    QString s = var->currentText();
                    m_list->SetPropertyValue(propIndex, s.toDouble());
                }
            }
            break;
        case CProperty::Enum:
        case CProperty::Group:
            {
                QComboBox* pc = qobject_cast<QComboBox*>(pw);
                if (pc) m_list->SetPropertyValue(propIndex, pc->currentIndex());
            }
            break;
        case CProperty::Bool:
            {
//						QComboBox* pc = qobject_cast<QComboBox*>(pw);
//						if (pc) m_list->SetPropertyValue(i, pc->currentIndex());
                QCheckBox* pc = qobject_cast<QCheckBox*>(pw);
                if (pc) m_list->SetPropertyValue(propIndex, pc->isChecked());
            }
            break;
        case CProperty::String:
            {
                QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
                if (edit) m_list->SetPropertyValue(propIndex, edit->text());
                else {
                    QComboBox* box = qobject_cast<QComboBox*>(pw);
                    if (box) m_list->SetPropertyValue(propIndex, box->currentText());
                }
            }
            break;
		case CProperty::Color:
		{
			CColorButton* cb = qobject_cast<CColorButton*>(pw);
			if (cb) m_list->SetPropertyValue(propIndex, cb->color());
		}
		break;
        case CProperty::DataScalar:
            {
                CDataFieldSelector* pc = dynamic_cast<CDataFieldSelector*>(pw);
                if (pc) m_list->SetPropertyValue(propIndex, pc->currentValue());
            }
            break;
        case CProperty::ColorMap:
            {
                CColorMapSelector* pc = dynamic_cast<CColorMapSelector*>(pw);
                if (pc) m_list->SetPropertyValue(propIndex, pc->currentIndex());
            }
            break;
        case CProperty::DataVec3:
            {
                CDataFieldSelector* pc = dynamic_cast<CDataFieldSelector*>(pw);
                if (pc) m_list->SetPropertyValue(propIndex, pc->currentValue());
            }
            break;
        case CProperty::DataMat3:
            {
                CDataFieldSelector* pc = dynamic_cast<CDataFieldSelector*>(pw);
                if (pc) m_list->SetPropertyValue(propIndex, pc->currentValue());
            }
            break;
        case CProperty::Resource:
            {
                CResourceEdit* edit = qobject_cast<CResourceEdit*>(pw);
                if (edit) m_list->SetPropertyValue(propIndex, edit->resourceName());
            }
            break;
        case CProperty::Curve:
            {
                CCurvePicker* pick = qobject_cast<CCurvePicker*>(pw);
                if (pick) m_list->SetPropertyValue(propIndex, pick->curveName());
            }
            break;
        case CProperty::CurveList:
            {
                CCurveListPicker* pick = qobject_cast<CCurveListPicker*>(pw);
                if (pick) m_list->SetPropertyValue(propIndex, pick->curveNames());
            }
            break;
        case CProperty::Action:
            {
                QPushButton* b = qobject_cast<QPushButton*>(pw);
                if (b) m_list->SetPropertyValue(propIndex, true);
            }
            break;
        case CProperty::MathString:
            {
                CEditVariableProperty* e = qobject_cast<CEditVariableProperty*>(pw);
                if (e) m_list->SetPropertyValue(propIndex, e->currentText());
                else
                {
                    QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
                    if (edit) m_list->SetPropertyValue(propIndex, edit->text());
                }
            }
            break;
        case CProperty::Vec3:
            {
                QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
                if (edit) m_list->SetPropertyValue(propIndex, edit->text());
            }
            break;
        case CProperty::Vec2i:
            {
                QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
                if (edit) m_list->SetPropertyValue(propIndex, edit->text());
            }
            break;
        case CProperty::Mat3:
            {
                QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
                if (edit) m_list->SetPropertyValue(propIndex, edit->text());
            }
            break;
        case CProperty::Std_Vector_Int:
            {
                QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
                if (edit) m_list->SetPropertyValue(propIndex, edit->text());

                QListWidget* plist = qobject_cast<QListWidget*>(pw);
                if (plist)
                {
                    QString s;
                    for (int i = 0; i < plist->count(); ++i)
                    {
                        QListWidgetItem* it = plist->item(i);
                        if (it->checkState() == Qt::Checked)
                        {
                            if (s.isEmpty() == false) s += ",";
                            s += QString::number(i);
                        }
                    }
                    m_list->SetPropertyValue(propIndex, s);
                }
            }
            break;
        case CProperty::Std_Vector_Double:
            {
                QLineEdit* edit = qobject_cast<QLineEdit*>(pw);
                if (edit) m_list->SetPropertyValue(propIndex, edit->text());
            }
            break;
        }
    }

    bool itemModified = (m_list ? m_list->IsModified() : false);
    if (itemModified)
    {
        setPropertyList(m_list);
        m_list->SetModified(false);
    }

    // send a signal that the data was changed
    emit dataChanged(itemModified, propIndex);
    return;
}
