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
#include <QMainWindow>
#include <vector>

namespace Ui {
	class CMeshInspector;
}

class CMainWindow;
class FSMesh;
class FSSurfaceMesh;
class GObject;

class CMeshInspector : public QMainWindow
{
	Q_OBJECT

public:
	CMeshInspector(CMainWindow* wnd);

	void Update(bool reset);

	void showEvent(QShowEvent* ev) override;
	void hideEvent(QHideEvent* ev) override;

private slots:
	void on_var_currentIndexChanged(int n);
	void on_col_currentIndexChanged(int n);
	void on_select_clicked();
	void on_curvatureLevels_valueChanged(int n);
	void on_curvatureMaxIters_valueChanged(int n);
	void on_curvatureExtQuad_stateChanged(int n);
	void on_table_cellChanged(int r, int c);
	void on_logScale_clicked();

private:
	void UpdateData(int ndata);
	void UpdateFEMeshData(FSMesh* pm, int ndata);
	void UpdateSurfaceMeshData(FSSurfaceMesh* pm, int ndata);

	void UpdateUI();

private:
	Ui::CMeshInspector*	ui;
	CMainWindow*	m_wnd;
};
