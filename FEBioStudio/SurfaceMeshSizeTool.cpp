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
#include "SurfaceMeshSizeTool.h"
#include "ModelDocument.h"
#include <MeshTools/LaplaceSolver.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GGroup.h>
#include <GeomLib/GOCCObject.h>
#include <MeshLib/FENodeData.h>
#include <MeshLib/FEElementData.h>
#include <QLineEdit>
#include <QBoxLayout>
#include <QFormLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QValidator>
#include <QMessageBox>
#include <QLabel>
#include <QHeaderView>
#include <QComboBox>
#include "MainWindow.h"
#include <FSCore/FSLogger.h>
#include <MeshTools/NetGenMesher.h>

// NOTE: Can't build with Netgen in debug config, so just turning it off for now. 
#if defined(WIN32) && defined(_DEBUG)
#undef HAS_NETGEN
#endif

#ifdef HAS_NETGEN

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>

#define OCCGEOMETRY

//using namespace std;

namespace nglib {
#include <nglib.h>
#include <nglib_occ.h>
}

#define NO_PARALLEL_THREADS
#include <occgeom.hpp>

#endif

class UISurfaceMeshSizeTool : public QWidget
{
public:
	QPushButton* m_add;
	QTableWidget* m_table;
	QLineEdit* m_val;
	QPushButton* m_clear;

public:
	UISurfaceMeshSizeTool(CSurfaceMeshSizeTool* w)
	{
		QVBoxLayout* l = new QVBoxLayout;

		m_add = new QPushButton("Add");
		m_val = new QLineEdit; m_val->setValidator(new QDoubleValidator);
		m_val->setText("0");

		QHBoxLayout* h2 = new QHBoxLayout;
		h2->addWidget(new QLabel("Mesh size"));
		h2->addWidget(m_val);
		h2->addWidget(m_add);

		l->addLayout(h2);
		l->addWidget(m_table = new QTableWidget);
		m_table->setColumnCount(2);
		m_table->horizontalHeader()->setStretchLastSection(true);
		m_table->setHorizontalHeaderLabels(QStringList() << "surface" << "mesh size");
		l->addStretch();

		QHBoxLayout* h3 = new QHBoxLayout;
		m_clear = new QPushButton("Clear");
		h3->addWidget(m_clear);
		l->addLayout(h3);
		l->addStretch();

		setLayout(l);

		QObject::connect(m_add, SIGNAL(clicked()), w, SLOT(OnAddClicked()));
		QObject::connect(m_clear, SIGNAL(clicked()), w, SLOT(OnClearClicked()));
		QObject::connect(m_table, SIGNAL(itemChanged(QTableWidgetItem*)), w, SLOT(OnTableEdited()));
	}
};

CSurfaceMeshSizeTool::CSurfaceMeshSizeTool(CMainWindow* wnd) : CAbstractTool(wnd, "Surface Mesh Size")
{
	m_po = nullptr;
	ui = nullptr;
}

QWidget* CSurfaceMeshSizeTool::createUi()
{
	if (ui == nullptr) ui = new UISurfaceMeshSizeTool(this);
	return ui;
}

void CSurfaceMeshSizeTool::addRow(const QString& name, double v)
{
	int n = ui->m_table->rowCount();
	ui->m_table->setRowCount(n + 1);
	QTableWidgetItem* it0 = new QTableWidgetItem;
	it0->setText(name);
	it0->setFlags(Qt::ItemIsSelectable);
	ui->m_table->setItem(n, 0, it0);

	QTableWidgetItem* it1 = new QTableWidgetItem;
	it1->setText(QString::number(v));
	ui->m_table->setItem(n, 1, it1);
}

void CSurfaceMeshSizeTool::OnAddClicked()
{
	if (m_po == nullptr) return;
	NetGenMesher* ngen = dynamic_cast<NetGenMesher*>(m_po->GetFEMesher()); assert(ngen);
	if (ngen == nullptr) return;

	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if ((doc == nullptr) || !doc->IsValid()) return;

	GFaceSelection* sel = dynamic_cast<GFaceSelection*>(doc->GetCurrentSelection());
	if ((sel == nullptr) || (sel->Count() == 0)) return;

	double v = ui->m_val->text().toDouble();

	GFaceSelection::Iterator it(sel);
	for (int i = 0; i < sel->Size(); ++i, ++it) 
	{
		GFace* gf = it;
		if (gf && (m_po == gf->Object())) 
		{
			ngen->SetMeshSize(gf->GetLocalID(), v);
		}
	}
	SetObject(m_po);
}

void CSurfaceMeshSizeTool::OnClearClicked()
{
	Clear();
}

void CSurfaceMeshSizeTool::OnTableEdited()
{
	if (m_po == nullptr) return;

	QTableWidgetItem* item = ui->m_table->currentItem();
	if (item == nullptr) return;
	int row = item->row();
	double msize = item->text().toDouble();
	NetGenMesher* ngen = dynamic_cast<NetGenMesher*>(m_po->GetFEMesher());
	if (ngen)
	{
		ngen->SetMeshSizeFromIndex(row, msize);
	}
}

void CSurfaceMeshSizeTool::Update()
{
	GOCCObject* po = dynamic_cast<GOCCObject*>(GetActiveObject());
	if (po == nullptr)
	{
		m_po = nullptr;
		Clear();
		ui->setDisabled(true);
	}
	else if (po != m_po)
	{
		SetObject(po);
		ui->setEnabled(true);
	}
	CAbstractTool::Update();
}

void CSurfaceMeshSizeTool::SetObject(GOCCObject* po)
{
	if (po && (po != m_po))
	{
		// print some stats
#ifdef HAS_NETGEN
		double mnedg = 0.0;
		double mxedg = 0.0;
		TopoDS_Shape& occ = po->GetShape();
		TopExp_Explorer anExp(occ, TopAbs_EDGE);
		for (; anExp.More(); anExp.Next()) {
			const TopoDS_Edge& anEdge = TopoDS::Edge(anExp.Current());
			GProp_GProps props;
			BRepGProp::LinearProperties(anEdge, props);
			double length = props.Mass();
			if (mnedg == 0) mnedg = length;
			else {
				mnedg = fmin(mnedg, length);
				mxedg = fmax(mxedg, length);
			}
		}
		FSLogger::Write("Minimum edge length in this object is %g.\n", mnedg);
		FSLogger::Write("Maximum edge length in this object is %g.\n", mxedg);
#endif
	}

	m_po = nullptr;
	Clear();
	m_po = po;
	if (po == nullptr) return;

	// fill the table
	NetGenMesher* ngen = dynamic_cast<NetGenMesher*>(po->GetFEMesher());
	if (ngen)
	{
		for (int i = 0; i < ngen->GetMeshSizes(); ++i)
		{
			const NetGenMesher::MeshSize& ms = ngen->GetMeshSize(i);
			GFace* pf = m_po->Face(ms.faceId); assert(pf);
			if (pf) addRow(QString::fromStdString(pf->GetName()), ms.meshSize);
		}
	}
}

void CSurfaceMeshSizeTool::Activate()
{
	assert(m_po == nullptr);
	GOCCObject* po = dynamic_cast<GOCCObject*>(GetActiveObject());
	if (po)
	{
		SetObject(po);
		ui->setEnabled(true);
	}
	else
	{
		ui->setDisabled(true);
	}

	CAbstractTool::Activate();
}

void CSurfaceMeshSizeTool::Deactivate()
{
	m_po = nullptr;
	Clear();
	CAbstractTool::Deactivate();
}

void CSurfaceMeshSizeTool::Clear()
{
	if (m_po) {
		NetGenMesher* ngen = dynamic_cast<NetGenMesher*>(m_po->GetFEMesher());
		if (ngen) ngen->ClearMeshSizes();
	}
	ui->m_table->clear();
	ui->m_table->setRowCount(0);
}
