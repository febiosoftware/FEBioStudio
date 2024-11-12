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
	QPushButton*	m_add;
	QTableWidget*	m_table;
	QLineEdit*		m_val;
    QPushButton*    m_clear;

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

void CSurfaceMeshSizeTool::OnAddClicked()
{
	double v = ui->m_val->text().toDouble();

    GOCCObject* po = dynamic_cast<GOCCObject*>(GetMainWindow()->GetActiveObject());
	if (m_po == nullptr) m_po = po;
	if ((m_po == po) && (m_po != nullptr))
	{
		CModelDocument* doc = GetMainWindow()->GetModelDocument();
        GFaceSelection* sel = dynamic_cast<GFaceSelection*>(doc->GetCurrentSelection());
        if (sel)
		{
			FEItemListBuilder* items = sel->CreateItemList();
			if (items)
			{
				m_data.push_back(items);

                GFaceSelection::Iterator it(sel);
                for (int i=0; i<sel->Size(); ++i, ++it) {
                    GFace* gf = it;
                    if (gf) {
                        GBaseObject* gbo = gf->Object();
                        if (dynamic_cast<GOCCObject*>(gbo)) {
                            // exclude surfaces that have been selected previously
                            bool selprev = false;
                            for (int j=0; j<m_po->m_nface.size(); ++j) {
                                if (gf->GetID() == m_po->m_nface[j]) {
                                    selprev = true;
                                    FSLogger::Write("Face %d has been assigned a mesh size already.\n", j);
                                }
                            }
                            if (!selprev) {
                                int n = ui->m_table->rowCount();
                                ui->m_table->insertRow(n);
                                
                                QTableWidgetItem* it0 = new QTableWidgetItem;
                                it0->setText((QString::number(gf->GetID())).arg(n));
                                it0->setFlags(Qt::ItemIsSelectable);
                                ui->m_table->setItem(n, 0, it0);
                                m_po->m_nface.push_back(gf->GetID());
                                
                                QTableWidgetItem* it1 = new QTableWidgetItem;
                                it1->setText(QString::number(v));
                                ui->m_table->setItem(n, 1, it1);
                                m_po->m_msize.push_back(v);
                            }
                        }
                    }
                }
			}
		}		
	}
    else {
        FSLogger::Write("Select an OCC (e.g., STEP) object.\n");
    }
}

void CSurfaceMeshSizeTool::OnClearClicked()
{
    Clear();
}

void CSurfaceMeshSizeTool::OnTableEdited()
{
    QTableWidgetItem* item = ui->m_table->currentItem();
    if (item == nullptr) return;
    int row = item->row();
    double msize = item->text().toDouble();
    m_po->m_msize[row] = msize;
}

void CSurfaceMeshSizeTool::Activate()
{
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if (doc)
	{
        GModel& mdl = *doc->GetGModel();
        if (mdl.Objects() == 0) return;
        GOCCObject* pocc = nullptr;
        for (int n = 0; n < mdl.Objects(); ++n)
        {
            GObject* po = mdl.Object(n);
            pocc = dynamic_cast<GOCCObject*>(po);
            if (pocc) break;
        }
        // check if any of the objects in this model are of type GOCCObject
        if (pocc == nullptr) {
            FSLogger::Write("This model does not include any OCC (e.g., STEP) object.\n");
            Clear();
            return;
        }
        else if (mdl.Objects() > 1) {
            FSLogger::Write("This tool works only if there is a single OCC (e.g., STEP) object in the model.\n");
            Clear();
            return;
        }
        pocc->Select();
#ifdef HAS_NETGEN
        double mnedg = 0.0;
        double mxedg = 0.0;
        TopoDS_Shape& occ = pocc->GetShape();
        TopExp_Explorer anExp (occ, TopAbs_EDGE);
        for (; anExp.More(); anExp.Next()) {
            const TopoDS_Edge& anEdge = TopoDS::Edge (anExp.Current());
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
	}
    else
        Clear();
#endif

	CAbstractTool::Activate();
}

void CSurfaceMeshSizeTool::Clear()
{
    if (m_po) {
        m_po->m_nface.clear();
        m_po->m_msize.clear();
    }
	m_po = nullptr;
	for (int i = 0; i < m_data.size(); ++i) delete m_data[i];
	m_data.clear();
	ui->m_table->clear();
	ui->m_table->setRowCount(0);
}

