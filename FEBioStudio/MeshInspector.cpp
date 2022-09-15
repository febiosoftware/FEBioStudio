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
#include "ui_meshinspector.h"
#include "MainWindow.h"
#include "GLView.h"
#include "Document.h"
#include <MeshTools/FEMeshValuator.h>
#include <PostLib/ColorMap.h>
#include "Commands.h"

CMeshInspector::CMeshInspector(CMainWindow* wnd) : m_wnd(wnd), QMainWindow(wnd), ui(new Ui::CMeshInspector)
{
	m_po = 0;
	ui->setupUi(this);
	ui->plot->setChartStyle(ChartStyle::BARCHART_PLOT);

	ui->m_map = Post::ColorMapManager::GetDefaultMap();
}

void CMeshInspector::Update()
{
	m_po = m_wnd->GetActiveObject();
	ui->setMesh(m_po);

	UpdateData(ui->var->currentIndex());
}

void CMeshInspector::showEvent(QShowEvent* ev)
{
	ui->col->blockSignals(true);
	ui->col->clear();
	for (int i = 0; i < Post::ColorMapManager::ColorMaps(); ++i)
	{
		string name = Post::ColorMapManager::GetColorMapName(i);
		ui->col->addItem(QString::fromStdString(name));
	}
	ui->col->setCurrentIndex(ui->m_map);
	ui->col->blockSignals(false);

	m_wnd->GetGLView()->SetColorMap(Post::ColorMapManager::GetColorMap(ui->m_map));
	m_wnd->GetGLView()->ShowMeshData(true);
	m_wnd->RedrawGL();
}

void CMeshInspector::hideEvent(QHideEvent* ev)
{
	m_wnd->GetGLView()->ShowMeshData(false);
	m_wnd->RedrawGL();
}

void CMeshInspector::on_var_currentIndexChanged(int n)
{
	if ((n == Ui::CMeshInspector::PRINC_CURVE_1) || (n == Ui::CMeshInspector::PRINC_CURVE_2)) ui->propsWidget->show();
	else ui->propsWidget->hide();

	UpdateData(n);
	m_wnd->GetGLView()->ShowMeshData(true); // this is called so the planecut gets updated
	m_wnd->RedrawGL();
}

void CMeshInspector::on_col_currentIndexChanged(int n)
{
	m_wnd->GetGLView()->SetColorMap(Post::ColorMapManager::GetColorMap(n));
	m_wnd->GetGLView()->ShowMeshData(true); // this is called so the planecut gets updated
	m_wnd->RedrawGL();
}

void CMeshInspector::UpdateData(int ndata)
{
	ui->plot->clear();

	// We added a separator between eval fields and data fields
	// so we need to subtract one if ndata is larger than the number of eval fields
	if (ndata > ui->MAX_EVAL_FIELDS) ndata--;

	FSMesh* pm = (m_po ? m_po->GetFEMesh() : 0);
	if (pm == 0) return;

	
	const int MAX_ELEM = 21;
	vector<bool> ET(MAX_ELEM, false);
	for (int i = 0; i < ui->table->rowCount(); ++i)
	{
		QTableWidgetItem* item = ui->table->item(i, 0);
		if (item->checkState() == Qt::Checked)
		{
			int e = item->data(Qt::UserRole).toInt(); assert((e >= 0) && (e < MAX_ELEM));
			ET[e] = true;
		}
	}

	FEMeshValuator eval(*pm);

	int curvatureLevels = ui->curvatureLevels->value();
	int curvatureMaxIters = ui->curvatureMaxIters->value();
	bool curvatureExtQuad = ui->curvatureExtQuad->isChecked();
	eval.SetCurvatureLevels(curvatureLevels);
	eval.SetCurvatureMaxIters(curvatureMaxIters);
	eval.SetCurvatureExtQuad(curvatureExtQuad);

	int NE = pm->Elements();
	vector<double> v; v.reserve(NE*FSElement::MAX_NODES);
	double vmax = -1e99, vmin = 1e99, vavg = 0;
	eval.Evaluate(ndata);
	Mesh_Data& data = pm->GetMeshData();
	if (data.IsValid())
	{
		for (int i = 0; i < NE; ++i)
		{
			FSElement& el = pm->Element(i);
			int ne = el.Nodes();
			if (ET[el.Type()])
			{
				if (data[i].tag)
				{
					int nerr;
					for (int j = 0; j < ne; ++j)
					{
						double vj = data.GetElementValue(i, j);
						vavg += vj;
						v.push_back(vj);
					}
				}
			}
			else
			{
				data.SetElementDataTag(i, 0);
			}
		}
		data.UpdateValueRange();
		data.GetValueRange(vmin, vmax);
	}
	int NC = v.size();
	if (NC > 0) vavg /= (double)NC; else { vmin = vmax = vavg = 0.0; }
	ui->stats->setRange(vmin, vmax, vavg);

	ui->sel->setRange(vmin, vmax);

	// determine number of bins
	// sqrt rule
//	int M = (int)ceil(sqrt((double)NC));

	// sturges rule
	int M = (int)ceil(log2(NC) + 1);

	// rice rule
//	int M = (int)ceil(2.0*pow(NC, 1.0/3.0));

	if (M < 1) M = 1;
	if (M > width() / 3) M = width() / 3;

	if (fabs(vmax - vmin) < 1e-5) vmax++;
	vector<double> bin; bin.assign(M, 0.0);
	double ymax = 0;
	for (int i = 0; i<NC; ++i)
	{
		int n = (int)(M*(v[i] - vmin) / (vmax - vmin));
		if (n < 0) n = 0;
		if (n >= M) n = M - 1;
		if (n >= 0) bin[n] += 1;
		if (bin[n] > ymax) ymax = bin[n];
	}

	if (ui->logScale->isChecked())
	{
		for (int i = 0; i < M; ++i)
		{
			if (bin[i] > 0) bin[i] = log10(bin[i]);
		}
	}
	else
	{
		for (int i = 0; i < M; ++i) bin[i] /= (double)NC;
	}

	CPlotData* pltData = new CPlotData;
	for (int i=0; i<M; ++i)
	{
		double v = vmin + i*(vmax - vmin)/(M-1);
		pltData->addPoint(v, bin[i]);
	}
	ui->plot->addPlotData(pltData);
	ui->plot->OnZoomToFit();
}

void CMeshInspector::on_select_clicked()
{
	CGLDocument* pdoc = m_wnd->GetGLDocument();
	if (pdoc == nullptr) return;

	GObject* po = pdoc->GetActiveObject();
	if (po == 0) return;

	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) return;

	double smin, smax;
	ui->sel->getRange(smin, smax);

	double eps = 0;
	if (smin != smax) eps = fabs((smax - smin) / 1e5);

	int etype = -1;
	QModelIndexList sel = ui->table->selectionModel()->selectedRows();
	if (sel.isEmpty() == false)
	{
		QModelIndex index = *sel.begin();
		etype = ui->table->item(index.row(), 0)->data(Qt::UserRole).toInt();
	}

	int NE = pm->Elements();
	vector<int> elem; elem.reserve(NE);
	Mesh_Data& data = pm->GetMeshData();
	for (int i = 0; i<NE; ++i)
	{
		FSElement& e = pm->Element(i);
		if ((etype == -1) || (e.Type() == etype))
		{
			if (data.GetElementDataTag(i) > 0)
			{
				double v = data.GetElementAverageValue(i);
				if ((v + eps >= smin) && (v - eps <= smax)) elem.push_back(i);
			}
		}
	}

	if (elem.empty() == false)
	{
		CCommand* pcmd = new CCmdSelectElements(pm, elem, false);
		pdoc->DoCommand(pcmd);
		m_wnd->Update(this);
		m_wnd->RedrawGL();
	}
}

void CMeshInspector::on_curvatureLevels_valueChanged(int n)
{
	int nvar = ui->var->currentIndex();
	if ((nvar == Ui::CMeshInspector::PRINC_CURVE_1) || (nvar == Ui::CMeshInspector::PRINC_CURVE_2))
	{
		UpdateData(nvar);
		m_wnd->GetGLView()->ShowMeshData(true); // this is called so the planecut gets updated
		m_wnd->RedrawGL();
	}
}

void CMeshInspector::on_curvatureMaxIters_valueChanged(int n)
{
	int nvar = ui->var->currentIndex();
	if ((nvar == Ui::CMeshInspector::PRINC_CURVE_1) || (nvar == Ui::CMeshInspector::PRINC_CURVE_2))
	{
		UpdateData(nvar);
		m_wnd->GetGLView()->ShowMeshData(true); // this is called so the planecut gets updated
		m_wnd->RedrawGL();
	}

}

void CMeshInspector::on_curvatureExtQuad_stateChanged(int n)
{
	int nvar = ui->var->currentIndex();
	if ((nvar == Ui::CMeshInspector::PRINC_CURVE_1) || (nvar == Ui::CMeshInspector::PRINC_CURVE_2))
	{
		UpdateData(nvar);
		m_wnd->GetGLView()->ShowMeshData(true); // this is called so the planecut gets updated
		m_wnd->RedrawGL();
	}
}

void CMeshInspector::on_table_cellChanged(int r, int c)
{
	if (c == 0) Update();
}

void CMeshInspector::on_logScale_clicked()
{
	Update();
}
