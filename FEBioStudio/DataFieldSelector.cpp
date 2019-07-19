#include "DataFieldSelector.h"
#include <PostViewLib/FEModel.h>
#include <PostViewLib/constants.h>
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


CModelDataSelector::CModelDataSelector(FEModel* fem, Data_Tensor_Type ntype, bool btvec)
{
	m_fem = fem;
	m_class = ntype;
	m_bvec = btvec;
	m_fem->AddDependant(this);
}

CModelDataSelector::~CModelDataSelector()
{
	if (m_fem) m_fem->RemoveDependant(this);
}

void CModelDataSelector::Update(FEModel* pfem)
{
	// TODO: update the menu
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
		FEDataField& d = *(*pd);
		int dataClass = d.DataClass();
		int dataComponents = d.components(m_class);
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

				for (int n = 0; n<dataComponents; ++n)
				{
					int nfield = BUILD_FIELD(dataClass, i, n);
					std::string s = d.componentName(n, m_class);

					QAction* pa = sub->addAction(QString::fromStdString(s));
					pa->setData(QVariant(nfield));

					if (d.Type() == DATA_ARRAY_VEC3F)
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
//	m_currentValue = -1;
	UpdateMenu();
}

void CDataSelectorButton::UpdateMenu()
{
	// get the current field
	// we'll use it to restore the current selected option
	int noldField = m_currentValue;
	m_currentValue = -1;
	setText("");

	// clear the menu
	m_menu->clear();

	// build a new menu
	if (m_src) m_src->BuildMenu(m_menu);

	// try to set the old field
	if (noldField != m_currentValue) setCurrentValue(noldField);
}

int CDataSelectorButton::currentValue() const
{
	return m_currentValue;
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

void CDataFieldSelector::BuildMenu(FEModel* fem, Data_Tensor_Type ntype, bool btvec)
{
	SetDataSelector(new CModelDataSelector(fem, ntype, btvec));
}
