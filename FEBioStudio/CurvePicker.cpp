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
#include "CurvePicker.h"
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QBoxLayout>
#include "GLHighlighter.h"
#include "SelectionBox.h"
#include "FEBioStudio.h"
#include "ModelDocument.h"
#include "Commands.h"
#include <GeomLib/GModel.h>

class Ui::CCurvePicker
{
public:
	QLineEdit*		m_name;
	QPushButton*	m_pick;

public:
	void setup(QWidget* w)
	{
		QHBoxLayout* l = new QHBoxLayout;
		l->setContentsMargins(0,0,0,0);
		l->setSpacing(0);
		l->addWidget(m_name = new QLineEdit);
		l->addWidget(m_pick = new QPushButton("..."));
		w->setLayout(l);

		m_pick->setFixedWidth(20);
		m_pick->setCheckable(true);

		QObject::connect(m_name, SIGNAL(editingFinished()), w, SLOT(nameChanged()));
		QObject::connect(m_pick, SIGNAL(toggled(bool)), w, SLOT(buttonToggled(bool)));
	}
};

//-----------------------------------------------------------------------------
CCurvePicker::CCurvePicker(QWidget* parent) : QWidget(parent), ui(new Ui::CCurvePicker)
{
	ui->setup(this);
}

//-----------------------------------------------------------------------------
void CCurvePicker::buttonToggled(bool bchecked)
{
	if (bchecked)
	{
		GLHighlighter::setTracking(true);
		QObject::connect(GLHighlighter::Instance(), SIGNAL(itemPicked(GItem*)), this, SLOT(itemPicked(GItem*)));
	}
	else
	{
		GLHighlighter::setTracking(false);
		GLHighlighter::Instance()->disconnect(this);
	}
}

//-----------------------------------------------------------------------------
void CCurvePicker::itemPicked(GItem* pick)
{
	if (pick)
	{
		const string& name = pick->GetName();
		ui->m_name->setText(QString::fromStdString(name));
		ui->m_pick->setChecked(false);
		emit curveChanged();
	}
}

//-----------------------------------------------------------------------------
void CCurvePicker::setCurve(const QString& curveName)
{
	ui->m_name->setText(curveName);
}

//-----------------------------------------------------------------------------
QString CCurvePicker::curveName() const
{
	return ui->m_name->text();
}

//-----------------------------------------------------------------------------
void CCurvePicker::nameChanged()
{
	emit curveChanged();
}

//=================================================================================================

class Ui::CCurveListPicker
{
public:
	::CSelectionBox*	m_list;

public:
	void setup(QWidget* w)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0,0,0,0);
		l->setSpacing(0);
		l->addWidget(m_list = new ::CSelectionBox);

		m_list->showNameType(false);

		w->setLayout(l);

		QObject::connect(m_list, SIGNAL(addButtonClicked()), w, SLOT(onAddButtonClicked()));
		QObject::connect(m_list, SIGNAL(subButtonClicked()), w, SLOT(onSubButtonClicked()));
		QObject::connect(m_list, SIGNAL(delButtonClicked()), w, SLOT(onDelButtonClicked()));
		QObject::connect(m_list, SIGNAL(selButtonClicked()), w, SLOT(onSelButtonClicked()));
	}
};


//=============================================================================

CCurveListPicker::CCurveListPicker(QWidget* parent) : QWidget(parent), ui(new Ui::CCurveListPicker)
{
	ui->setup(this);
}

CCurveListPicker::~CCurveListPicker()
{
}

//-----------------------------------------------------------------------------
void CCurveListPicker::onAddButtonClicked()
{
	// get the document
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(FBS::getActiveDocument());

	// get the current selection and make sure it's not empty
	FESelection* ps = pdoc->GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return;

	// make sure this is an edge selection
	GEdgeSelection* sel = dynamic_cast<GEdgeSelection*>(ps);
	if (sel == 0) return;

	GEdgeSelection::Iterator it(sel);
	int N = sel->Size();
	for (int i=0; i<N; ++i, ++it)
	{
		GEdge* edge = it;
		ui->m_list->addData(QString::fromStdString(edge->GetName()), edge->GetID());
	}

	emit curveChanged();
}

//-----------------------------------------------------------------------------
void CCurveListPicker::onSubButtonClicked()
{
	// get the document
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(FBS::getActiveDocument());

	// get the current selection and make sure it's not empty
	FESelection* ps = pdoc->GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return;

	// make sure this is an edge selection
	GEdgeSelection* sel = dynamic_cast<GEdgeSelection*>(ps);
	if (sel == 0) return;

	GEdgeSelection::Iterator it(sel);
	int N = sel->Size();
	for (int i = 0; i<N; ++i, ++it)
	{
		GEdge* edge = it;
		ui->m_list->removeData(edge->GetID());
	}

	emit curveChanged();
}

//-----------------------------------------------------------------------------
void CCurveListPicker::onDelButtonClicked()
{
	ui->m_list->removeSelectedItems();
	emit curveChanged();
}

//-----------------------------------------------------------------------------
void CCurveListPicker::onSelButtonClicked()
{
	// get the document
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(FBS::getActiveDocument());

	// get the current selection and make sure it's not empty
	FESelection* ps = pdoc->GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return;

	// make sure this is an edge selection
	GEdgeSelection* sel = dynamic_cast<GEdgeSelection*>(ps);
	if (sel == 0) return;

	std::vector<int> edges;

	GEdgeSelection::Iterator it(sel);
	int N = sel->Size();
	for (int i = 0; i<N; ++i, ++it)
	{
		GEdge* edge = it;
		edges.push_back(edge->GetID());
	}

	if (edges.empty() == false)
	{
		pdoc->DoCommand(new CCmdSelectEdge(pdoc->GetGModel(), edges, false));
	}
}

//-----------------------------------------------------------------------------
void CCurveListPicker::setCurves(const QStringList& curves)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(FBS::getActiveDocument());
	GModel& m = *doc->GetGModel();

	ui->m_list->clearData();
	for (int i=0; i<curves.size(); ++i)
	{
		QString name = curves[i];
		std::string s = name.toStdString();
		GEdge* edge = m.FindEdgeFromName(s); assert(edge);
		if (edge)
		{
			ui->m_list->addData(name, edge->GetID());
		}
	}
}

//-----------------------------------------------------------------------------
QStringList CCurveListPicker::curveNames() const
{
	QStringList names;
	ui->m_list->getAllNames(names);
	return names;
}
