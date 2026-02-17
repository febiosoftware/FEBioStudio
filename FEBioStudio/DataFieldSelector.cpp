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
#include "DataFieldSelector.h"
#include <PostLib/constants.h>
#include <FSCore/ColorMapManager.h>
#include <QPainter>
using namespace Post;

CDataSelector::CDataSelector()
{

}

CDataSelector::~CDataSelector()
{

}

CGenericDataSelector::CGenericDataSelector()
{

}

void CGenericDataSelector::AddOption(const QString& opt)
{
	m_ops.append(opt);
}

void CGenericDataSelector::BuildMenu(QMenu* menu)
{
	for (int i = 0; i < m_ops.count(); ++i)
	{
		QAction* pa = menu->addAction(m_ops.at(i));
		pa->setData(i);
	}
}

CTimeStepSelector::CTimeStepSelector()
{

}

void CTimeStepSelector::BuildMenu(QMenu* menu)
{
	QAction* timeAction = menu->addAction("Time"); timeAction->setData(0);
	QAction* stepAction = menu->addAction("Steps"); stepAction->setData(1);
}

CMusclePathDataSelector::CMusclePathDataSelector() {}
void CMusclePathDataSelector::BuildMenu(QMenu* menu)
{
	QAction* lengthAction = menu->addAction("Length"       ); lengthAction->setData(1);

	QMenu* startPt = new QMenu("Start Point");
	QAction* startXAction = startPt->addAction("Start Point X"); startXAction->setData(2);
	QAction* startYAction = startPt->addAction("Start Point Y"); startYAction->setData(3);
	QAction* startZAction = startPt->addAction("Start Point Z"); startZAction->setData(4);
	menu->addMenu(startPt);

	QMenu* endPt = new QMenu("End Point");
	QAction* endXAction   = endPt->addAction("End Point X"  ); endXAction  ->setData(5);
	QAction* endYAction   = endPt->addAction("End Point Y"  ); endYAction  ->setData(6);
	QAction* endZAction   = endPt->addAction("End Point Z"  ); endZAction  ->setData(7);
	menu->addMenu(endPt);

	QMenu* depPt = new QMenu("Departure Point");
	QAction* depXAction = depPt->addAction("Departure Point X"); depXAction->setData(8);
	QAction* depYAction = depPt->addAction("Departure Point Y"); depYAction->setData(9);
	QAction* depZAction = depPt->addAction("Departure Point Z"); depZAction->setData(10);
	menu->addMenu(depPt);

	QMenu* tng = new QMenu("Departure Tangent");
	QAction* tngXAction = tng->addAction("Departure Tangent X"); tngXAction->setData(11);
	QAction* tngYAction = tng->addAction("Departure Tangent Y"); tngYAction->setData(12);
	QAction* tngZAction = tng->addAction("Departure Tangent Z"); tngZAction->setData(13);
	menu->addMenu(tng);
}

//=============================================================================
CProbeDataSelector::CProbeDataSelector(){}
void CProbeDataSelector::BuildMenu(QMenu* menu)
{
	QMenu* pos = new QMenu("Position");
	QAction* posXAction = pos->addAction("X-Position"); posXAction->setData(1);
	QAction* posYAction = pos->addAction("Y-Position"); posYAction->setData(2);
	QAction* posZAction = pos->addAction("Z-Position"); posZAction->setData(3);
	menu->addMenu(pos);
}

//=============================================================================
CRulerDataSelector::CRulerDataSelector() {}
void CRulerDataSelector::BuildMenu(QMenu* menu)
{
	QMenu* pos = new QMenu("Relative position");
	QAction* posXAction = pos->addAction("X-Relative position"); posXAction->setData(1);
	QAction* posYAction = pos->addAction("Y-Relative position"); posYAction->setData(2);
	QAction* posZAction = pos->addAction("Z-Relative position"); posZAction->setData(3);
	menu->addMenu(pos);

	menu->addAction("Distance")->setData(4);
}

//=============================================================================
CModelDataSelector::CModelDataSelector(FEPostModel* fem, Data_Tensor_Type ntype, bool explicitOnly)
{
	m_fem = fem;
	m_class = ntype;
	m_explicitOnly = explicitOnly;
	m_fem->AddDependant(this);
}

CModelDataSelector::~CModelDataSelector()
{
	if (m_fem) m_fem->RemoveDependant(this);
}

void CModelDataSelector::Update(FEPostModel* pfem)
{
	if (pfem == nullptr) m_fem = nullptr;

	emit dataChanged();
}

void CModelDataSelector::BuildMenu(QMenu* menu)
{
	// get the datamanager
	if (m_fem == 0) return;
	FEDataManager& dm = *m_fem->GetDataManager();

	// loop over all data fields
	int N = dm.DataFields();
	FEDataFieldPtr pd = dm.FirstDataField();
	for (int i = 0; i<N; ++i, ++pd)
	{
		ModelDataField& d = *(*pd);
		int dataClass = d.DataClass();
		int dataComponents = d.components(m_class);
		int dataType = d.Type();

		bool ok = true;
		if (m_explicitOnly)
		{
			if (d.Flags() == IMPLICIT_DATA) ok = false;
			else
			{
				switch (m_class)
				{
				case TENSOR_SCALAR: if ((dataType != DATA_SCALAR) && (dataType != DATA_ARRAY)) ok = false; break;
				case TENSOR_VECTOR: if ((dataType != DATA_VEC3) && (dataType != DATA_ARRAY_VEC3)) ok = false; break;
				case TENSOR_TENSOR2: if ((dataType != DATA_MAT3) && (dataType != DATA_MAT3S) && (dataType != DATA_MAT3SD)) ok = false; break;
				default:
					ok = false;
					assert(false);
					break;
				}
			}
		}

		if ((dataClass != OBJECT_DATA) && (dataComponents > 0) && ok)
		{
			if ((dataComponents == 1) && (d.Type() != DATA_ARRAY))
			{
				int nfield = BUILD_FIELD(dataClass, i, 0);

				QAction* pa = menu->addAction(QString::fromStdString(d.GetName()));
				pa->setData(QVariant(nfield));
			}
			else
			{
				QMenu* sub = new QMenu(QString::fromStdString(d.GetName()), menu);
				menu->addMenu(sub);

				for (int n = 0; n<dataComponents; ++n)
				{
					int nfield = BUILD_FIELD(dataClass, i, n);
					std::string s = d.componentName(n, m_class);

					QAction* pa = sub->addAction(QString::fromStdString(s));
					pa->setData(QVariant(nfield));

					if (d.Type() == DATA_ARRAY_VEC3)
					{
						if ((n > 0) && ((n + 1) % 4 == 0))
						{
							sub->addSeparator();
						}
					}
				}
			}
		}
	}
}

CPlotObjectDataSelector::CPlotObjectDataSelector(Post::FEPostModel::PlotObject* po)
{
	m_po = po;
}

void CPlotObjectDataSelector::BuildMenu(QMenu* menu)
{
	for (int i = 0; i < (int) m_po->DataCount(); ++i)
	{
		PlotObjectData& d = *m_po->GetData(i);
		int dataClass = d.DataClass();
		int dataComponents = d.components(TENSOR_SCALAR);
		if (dataComponents > 0)
		{
			if ((dataComponents == 1) && (d.Type() != DATA_ARRAY))
			{
				int nfield = BUILD_FIELD(dataClass, i, 0);

				QAction* pa = menu->addAction(QString::fromStdString(d.GetName()));
				pa->setData(QVariant(nfield));
			}
			else
			{
				QMenu* sub = new QMenu(QString::fromStdString(d.GetName()), menu);
				menu->addMenu(sub);

				for (int n = 0; n < dataComponents; ++n)
				{
					int nfield = BUILD_FIELD(dataClass, i, n);
					std::string s = d.componentName(n, TENSOR_SCALAR);

					QAction* pa = sub->addAction(QString::fromStdString(s));
					pa->setData(QVariant(nfield));

					if (d.Type() == DATA_ARRAY_VEC3)
					{
						if ((n > 0) && ((n + 1) % 4 == 0))
						{
							sub->addSeparator();
						}
					}
				}
			}
		}
	}
}

CPlotGlobalDataSelector::CPlotGlobalDataSelector(Post::FEDataFieldPtr pdf) : m_pdf(pdf)
{

}

void CPlotGlobalDataSelector::BuildMenu(QMenu* menu)
{
	switch ((*m_pdf)->Type())
	{
	case DATA_SCALAR: {
		QAction* pa = menu->addAction("value");
		pa->setData(1);
	}
		break;
	case DATA_VEC3:
		break;
	case DATA_MAT3S:
		break;
	case DATA_MAT3SD:
		break;
	case DATA_TENS4S:
		break;
	case DATA_MAT3:
		break;
	case DATA_ARRAY:
		{
			std::vector<std::string> names = (*m_pdf)->GetArrayNames();
			for (int i = 0; i < names.size(); ++i)
			{
				QAction* pa = menu->addAction(QString::fromStdString(names[i]));
				pa->setData(i + 1);
			}
		}
		break;
	case DATA_ARRAY_VEC3:
		break;
	default:
		break;
	}
}

CDataSelectorButton::CDataSelectorButton(QWidget* parent) : QPushButton(parent)
{
	m_src = nullptr;
	m_menu = new QMenu(this);
	m_currentValue = -1;

	setStyleSheet("QPushButton { text-align : left; }");

	setMenu(m_menu);

	QObject::connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(onAction(QAction*)));
}

CDataSelectorButton::~CDataSelectorButton()
{
	delete m_src;
}

// set the data selector
void CDataSelectorButton::SetDataSelector(CDataSelector* dataSelector)
{
	if (m_src) delete m_src;
	m_src = dataSelector;

	m_currentValue = -1;
	setText("");

	// clear the menu
	m_menu->clear();

	// build a new menu
	if (m_src) m_src->BuildMenu(m_menu);

	QObject::connect(dataSelector, SIGNAL(dataChanged()), this, SLOT(onDataChanged()));
}

int CDataSelectorButton::currentValue() const
{
	return m_currentValue;
}

void CDataSelectorButton::onDataChanged()
{
	// clear the menu
	m_menu->clear();

	// build a new menu
	if (m_src) m_src->BuildMenu(m_menu);
}

QAction* findAction(QMenu* menu, int searchValue)
{
	QList<QAction*> actions = menu->actions();
	foreach(QAction* action, menu->actions())
	{
		if (action->isSeparator())
		{
			// separator, nothing to do
		}
		else if (action->menu())
		{
			// call findAction on the submenu
			QAction* pa = findAction(action->menu(), searchValue);
			if (pa) return pa;
		}
		else
		{
			int value = action->data().toInt();
			if (value == searchValue) return action;
		}
	}
	return nullptr;
}

QAction* findAction(QMenu* menu, const char* szname)
{
	QList<QAction*> actions = menu->actions();
	foreach(QAction* action, menu->actions())
	{
		if (action->isSeparator())
		{
			// separator, nothing to do
		}
		else if (action->menu())
		{
			// call findAction on the submenu
			QAction* pa = findAction(action->menu(), szname);
			if (pa) return pa;
		}
		else
		{
			QString actionText = action->text();
			if (actionText == szname) return action;
		}
	}
	return nullptr;
}


void CDataSelectorButton::setCurrentValue(int newField)
{
	// make sure there is something to change
	if (newField == m_currentValue) return;

	// find the item
	QAction* pa = findAction(m_menu, newField);
	if (pa)
	{
		m_currentValue = newField;
		setText(pa->text());
	}
	else
	{
		m_currentValue = -1;
		setText("");
	}

	emit currentValueChanged(m_currentValue);
}

// set the current value based on string
bool CDataSelectorButton::setCurrentValue(const char* sz)
{
	bool bsuccess = false;
	QAction* pa = findAction(m_menu, sz);
	if (pa)
	{
		bsuccess = true;
		int value = pa->data().toInt();
		if (value == m_currentValue) return true;

		m_currentValue = value;
		setText(pa->text());
	}
	else
	{
		m_currentValue = -1;
		setText("");
	}

	emit currentValueChanged(m_currentValue);

	return bsuccess;
}

void CDataSelectorButton::onAction(QAction* pa)
{
	setText(pa->text());
	m_currentValue = pa->data().toInt();

	emit currentValueChanged(m_currentValue);
}

//=============================================================================
CDataFieldSelector::CDataFieldSelector(QWidget* parent) : CDataSelectorButton(parent)
{
}

void CDataFieldSelector::BuildMenu(FEPostModel* fem, Data_Tensor_Type ntype, bool explicitOnly)
{
	SetDataSelector(new CModelDataSelector(fem, ntype, explicitOnly));
}

//=============================================================================
CColorMapSelector::CColorMapSelector(QWidget* parent) : QComboBox(parent)
{
	QStringList cols;
	for (int i = 0; i<ColorMapManager::ColorMaps(); ++i)
	{
		string name = ColorMapManager::GetColorMapName(i);
		cols << name.c_str();
	}

	addItems(cols);
}
