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
#include "WindowPanel.h"
#include <vector>

//-----------------------------------------------------------------------------
class QTreeWidgetItem;
class CPostDocument;
class FSObject;

//-----------------------------------------------------------------------------
namespace Ui {
	class CPostModelPanel;
}

//-----------------------------------------------------------------------------
class CPostModelPanel : public CWindowPanel
{
	Q_OBJECT

public:
	CPostModelPanel(CMainWindow* pwnd, QWidget* parent = 0);

public:
	void Update(bool breset) override;

	// this is called when the view needs to be updated
	void UpdateView();

	void selectObject(FSObject* po);

	FSObject* selectedObject();

	void ShowContextMenu(QContextMenuEvent* ev);

private slots:
	void on_postModel_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev);
	void on_postModel_itemClicked(QTreeWidgetItem* item, int column);
	void on_postModel_itemDoubleClicked(QTreeWidgetItem* item, int column);
	void on_nameEdit_editingFinished();
	void on_deleteButton_clicked();
	void on_props_dataChanged();
	void OnSelectNodes();
	void OnSelectFaces();
	void OnSelectElements();
	void OnHideElements();
	void OnShowAllElements();
	void OnMoveUpInRenderingQueue();
	void OnMoveToGroup();
	void OnMoveDownInRenderingQueue();
	void OnExportImage();
	void OnExportMCSurface();
	void OnExportProbeData();
	void OnImportCurveProbePoints();
	void OnCurveProbePlotData();
	void OnCurveProbePlotTimeAveragedData();
	void OnExportMusclePathData();
	void OnSwapMusclePathEndPoints();
//	void on_autoUpdate_toggled(bool b);
//	void on_applyButton_clicked();

signals:
	void postObjectStateChanged();
	void postObjectPropsChanged(FSObject* po);

private:
	CPostDocument* GetActiveDocument();

	void BuildModelTree();

private:
	Ui::CPostModelPanel*				ui;
};
