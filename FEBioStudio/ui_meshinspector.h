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

#pragma once
#include "MeshInspector.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QPushButton>
#include "PlotWidget.h"
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSSurfaceMesh.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshTools/FEMeshValuator.h>

class CMeshInfo : public QGroupBox
{
public:
	CMeshInfo(QWidget* parent = 0) : QGroupBox("Mesh Info", parent)
	{
		QFormLayout* infoLayout = new QFormLayout;
		infoLayout->addRow("Name:", name = new QLabel(""));
		infoLayout->addRow("Nodes:", nodes = new QLabel(""));
		infoLayout->addRow("Faces:", faces = new QLabel(""));
		infoLayout->addRow("Elements:", elems = new QLabel(""));
		setLayout(infoLayout);
	}

	void setMesh(GObject* po)
	{
		FSMesh* pm = (po ? po->GetFEMesh() : 0);
		if (pm)
		{
			name->setText(QString::fromStdString(po->GetName()));
			nodes->setText(QString::number(pm->Nodes()));
			faces->setText(QString::number(pm->Faces()));
			elems->setText(QString::number(pm->Elements()));
		}
		else
		{
			if (po) name->setText(QString::fromStdString(po->GetName()));
			else name->setText("---");

			nodes->setText("---");
			faces->setText("---");
			elems->setText("---");

			if (dynamic_cast<GSurfaceMeshObject*>(po))
			{
				GSurfaceMeshObject* pso = dynamic_cast<GSurfaceMeshObject*>(po);
				FSSurfaceMesh* psm = pso->GetSurfaceMesh();
				if (psm)
				{
					nodes->setText(QString::number(psm->Nodes()));
					faces->setText(QString::number(psm->Faces()));
					elems->setText("0");
				}
			}
		}
	}

private:
	QLabel*	name;
	QLabel*	nodes;
	QLabel*	faces;
	QLabel*	elems;
};

class CStatsInfo : public QGroupBox
{
public:
	CStatsInfo(QWidget* parent = 0) : QGroupBox("Statistics", parent)
	{
		QFormLayout* statsLayout = new QFormLayout;
		statsLayout->addRow("Min", min = new QLabel);
		statsLayout->addRow("Max", max = new QLabel);
		statsLayout->addRow("Avg", avg = new QLabel);
		setLayout(statsLayout);
	}

	void setRange(double fmin, double fmax, double favg)
	{
		min->setText(QString::number(fmin));
		max->setText(QString::number(fmax));
		avg->setText(QString::number(favg));
	}

private:
	QLabel*	min;
	QLabel*	max;
	QLabel*	avg;
};

class CSelectionInfo : public QGroupBox
{
public:
	CSelectionInfo(QWidget* parent = 0) : QGroupBox("Selection", parent)
	{
		QFormLayout* formLayout = new QFormLayout;
		formLayout->addRow("min:", min = new QLineEdit); min->setValidator(new QDoubleValidator);
		formLayout->addRow("max:", max = new QLineEdit); max->setValidator(new QDoubleValidator);

		QHBoxLayout* selLayout = new QHBoxLayout;
		selLayout->addLayout(formLayout);
		selButton = new QPushButton("Select");
		selButton->setObjectName("select");
		selLayout->addWidget(selButton);

		setLayout(selLayout);
	}

	void setRange(double fmin, double fmax)
	{
		min->setText(QString::number(fmin));
		max->setText(QString::number(fmax));
	}

	void getRange(double& fmin, double& fmax)
	{
		fmin = min->text().toDouble();
		fmax = max->text().toDouble();
	}

public:
	QLineEdit*	min;
	QLineEdit*	max;
	QPushButton* selButton;
};

class Ui::CMeshInspector
{
public:
	CMeshInfo*		info;
	QTableWidget*	table;
	CPlotWidget*	plot;
	QComboBox*		var;
	QComboBox*		col;
	CStatsInfo*		stats;
	CSelectionInfo*	sel;
	QCheckBox*		logScale;

	QWidget* propsWidget;
	QSpinBox* curvatureLevels;
	QSpinBox* curvatureMaxIters;
	QCheckBox* curvatureExtQuad;

	GObject*		m_po;
	FSMeshBase*		m_pm;

	int		m_map;

public:
	void setupUi(QMainWindow* wnd)
	{
		m_po = nullptr;
		m_pm = nullptr;

		m_map = -1;

		info = new CMeshInfo;

		table = new QTableWidget;
		table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		table->setObjectName("table");

		table->setColumnCount(2);
		table->setSelectionBehavior(QAbstractItemView::SelectRows);
		table->setSelectionMode(QAbstractItemView::SingleSelection);
		table->horizontalHeader()->setStretchLastSection(true);
		table->setHorizontalHeaderLabels(QStringList()<<"Type"<<"Count");
		table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
		table->verticalHeader()->setDefaultSectionSize(24);
		table->verticalHeader()->hide();
		table->setEditTriggers(QAbstractItemView::NoEditTriggers);

		var = new QComboBox;
		var->setObjectName("var");

		propsWidget = new QWidget;
		QFormLayout* propsForm = new QFormLayout;
		propsForm->addRow("Smoothness", curvatureLevels = new QSpinBox); curvatureLevels->setObjectName("curvatureLevels"); curvatureLevels->setRange(1, 10); curvatureLevels->setValue(1);
		propsForm->addRow("Max. Iterations", curvatureMaxIters = new QSpinBox); curvatureMaxIters->setObjectName("curvatureMaxIters"); curvatureMaxIters->setRange(1, 50); curvatureMaxIters->setValue(10);
		propsForm->addRow("Use Extended Quadric", curvatureExtQuad = new QCheckBox); curvatureExtQuad->setObjectName("curvatureExtQuad");
		propsWidget->setLayout(propsForm);
		propsWidget->hide();

		col = new QComboBox;
		col->setObjectName("col");

		logScale = new QCheckBox;
		logScale->setObjectName("logScale");

		QFormLayout* varForm = new QFormLayout;
		varForm->addRow("Variable:", var);
		varForm->addRow("Parameters:", propsWidget);
		varForm->addRow("Color map", col);
		varForm->addRow("Logarithmic scale", logScale);

		QHBoxLayout* infoLayout = new QHBoxLayout;
		infoLayout->addWidget(info);
		infoLayout->addWidget(table);

		plot = new CPlotWidget;
		plot->showLegend(false);
		plot->setViewLocked(true);

		stats = new CStatsInfo;

		sel = new CSelectionInfo;

		QHBoxLayout* statsLayout = new QHBoxLayout;
		statsLayout->addWidget(stats);
		statsLayout->addWidget(sel);

		QWidget* w = new QWidget;
		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(infoLayout);
		mainLayout->addLayout(varForm);
		mainLayout->addWidget(plot);
		mainLayout->addLayout(statsLayout);
		w->setLayout(mainLayout);

		wnd->setCentralWidget(w);

		var->setFocus();

		QMetaObject::connectSlotsByName(wnd);
	}

	void setMesh(GObject* po)
	{
		info->setMesh(po);
		if (po == 0)
		{
			table->setRowCount(0);
			var->clear();
			m_pm = nullptr;
			return;
		}

		FSMesh* pm = po->GetFEMesh();
		if (pm) setFEMesh(pm);
		else {
			GSurfaceMeshObject* pso = dynamic_cast<GSurfaceMeshObject*>(po);
			if (pso)
			{
				setSurfaceMesh(pso->GetSurfaceMesh());
			}
		}
	}

	void setFEMesh(FSMesh* pm)
	{
		const int MAX_ELEM = 21;
		static int ET[] = {
			FE_HEX8, FE_TET4, FE_PENTA6, FE_QUAD4, FE_TRI3, FE_BEAM2, FE_HEX20, FE_QUAD8, FE_BEAM3, FE_TET10, FE_TRI6, FE_TET15, FE_HEX27, FE_TRI7, FE_QUAD9, FE_TET20, FE_TRI10, FE_PYRA5, FE_PENTA15, FE_TET5, FE_PYRA13, 0 };

		static const char* EN[] = {
			"HEX8", "TET4", "PENTA6", "QUAD4", "TRI3", "BEAM2", "HEX20", "QUAD8", "LINE3", "TET10", "TRI6", "TET15", "HEX27", "TRI7", "QUAD9", "TET20", "TRI10", "PYRA5", "PENTA15", "TET5", "PYRA13", "(unknown)" };

		if (pm == 0)
		{
			table->setRowCount(0);
			m_pm = nullptr;
			return;
		}

		// We get ever when the selection has changed, but we don't
		// want to update when the mesh hasn't changed.
		if (m_pm == pm) return;
		m_pm = pm;

		// build the element table
		int n[MAX_ELEM + 1] = { 0 };
		int NE = pm->Elements();
		for (int i = 0; i<NE; ++i)
		{
			FSElement& e = pm->Element(i);
			switch (e.Type())
			{
			case FE_HEX8   : n[0]++; break;
			case FE_TET4   : n[1]++; break;
			case FE_PENTA6 : n[2]++; break;
			case FE_QUAD4  : n[3]++; break;
			case FE_TRI3   : n[4]++; break;
			case FE_BEAM2  : n[5]++; break;
			case FE_HEX20  : n[6]++; break;
			case FE_QUAD8  : n[7]++; break;
			case FE_BEAM3  : n[8]++; break;
			case FE_TET10  : n[9]++; break;
			case FE_TRI6   : n[10]++; break;
			case FE_TET15  : n[11]++; break;
			case FE_HEX27  : n[12]++; break;
			case FE_TRI7   : n[13]++; break;
			case FE_QUAD9  : n[14]++; break;
			case FE_TET20  : n[15]++; break;
			case FE_TRI10  : n[16]++; break;
			case FE_PYRA5  : n[17]++; break;
			case FE_PENTA15: n[18]++; break;
			case FE_TET5   : n[19]++; break;
			case FE_PYRA13 : n[20]++; break;
			default:
				assert(false);
				n[MAX_ELEM]++; break;
			}
		}

		int m = 0;
		for (int i = 0; i<MAX_ELEM + 1; ++i) if (n[i] != 0) m++;

		// fill the rows
		table->blockSignals(true);
		table->setRowCount(m); m = 0;
		for (int i = 0; i<MAX_ELEM + 1; ++i)
		{
			if (n[i] != 0)
			{
				QTableWidgetItem* item = new QTableWidgetItem(EN[i]);
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
				item->setCheckState(Qt::Checked);
				item->setData(Qt::UserRole, ET[i]);
				table->setItem(m, 0, item);
				item = new QTableWidgetItem(QString::number(n[i]));
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
				table->setItem(m, 1, item);
				m++;
			}
		}
		table->blockSignals(false);

		// build variables list
		std::vector< std::string> names = FEMeshValuator::GetDataFieldNames();
		QStringList items;
		for (std::string& s : names) items << QString::fromStdString(s);
		var->clear();
		var->addItems(items);

		// Add data fields
		if (pm && pm->MeshDataFields())
		{
			var->insertSeparator(var->count());

			for (int i = 0; i < pm->MeshDataFields(); ++i)
			{
				FSMeshData& di = *pm->GetMeshDataField(i);
				var->addItem(QString::fromStdString(di.GetName()));
			}
		}
	}

	void setSurfaceMesh(FSSurfaceMesh* pm)
	{
		if (pm == 0)
		{
			table->setRowCount(0);
			m_pm = nullptr;
			return;
		}

		// NOTE That the order matches FEFaceType
		const int MAX_TYPE = 8;
		static int FT[] = {
			0, FE_FACE_TRI3, FE_FACE_QUAD4, FE_FACE_TRI6, FE_FACE_TRI7, FE_FACE_QUAD8, FE_FACE_QUAD9, FE_FACE_TRI10 };

		static const char* FN[] = {
			"(unknown)", "TRI3", "QUAD4", "TRI6", "TRI7", "QUAD8", "QUAD9", "TRI10" };

		std::vector< std::string> names = FESurfaceMeshValuator::GetDataFieldNames();
		QStringList items;
		for (std::string& s : names) items << QString::fromStdString(s);
		var->clear();
		var->addItems(items);

		// We get ever when the selection has changed, but we don't
		// want to update when the mesh hasn't changed.
		if (m_pm == pm) return;
		m_pm = pm;

		int n[MAX_TYPE] = { 0 };
		int NF = pm->Faces();
		for (int i = 0; i < NF; ++i)
		{
			FSFace& f = pm->Face(i);
			switch (f.Type())
			{
			case FE_FACE_TRI3 : n[1]++; break;
			case FE_FACE_QUAD4: n[2]++; break;
			case FE_FACE_TRI6 : n[3]++; break;
			case FE_FACE_TRI7 : n[4]++; break;
			case FE_FACE_QUAD8: n[5]++; break;
			case FE_FACE_QUAD9: n[6]++; break;
			case FE_FACE_TRI10: n[7]++; break;
			default:
				assert(false);
				n[0]++; break;
			}
		}

		int m = 0;
		for (int i = 0; i < MAX_TYPE; ++i) if (n[i] != 0) m++;

		// fill the rows
		table->blockSignals(true);
		table->setRowCount(m); m = 0;
		for (int i = 0; i < MAX_TYPE; ++i)
		{
			if (n[i] != 0)
			{
				QTableWidgetItem* item = new QTableWidgetItem(FN[i]);
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
				item->setCheckState(Qt::Checked);
				item->setData(Qt::UserRole, FT[i]);
				table->setItem(m, 0, item);
				item = new QTableWidgetItem(QString::number(n[i]));
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
				table->setItem(m, 1, item);
				m++;
			}
		}
		table->blockSignals(false);
	}
};
