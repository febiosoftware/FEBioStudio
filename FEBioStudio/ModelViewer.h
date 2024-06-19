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
#include "CommandPanel.h"
#include <vector>

class QTreeWidgetItem;
class FSObject;
class FEItemListBuilder;
struct CModelTreeItem;

namespace Ui {
	class CModelViewer;
}

class CModelViewer : public CCommandPanel
{
	Q_OBJECT

public:
	CModelViewer(CMainWindow* wnd, QWidget* parent = 0);

	// update the model tree and props window
	void Update(bool breset = true) override;

	// clear the model viewer
	void Clear();

	// get the currently selected object
	FSObject* GetCurrentObject();

	// update an object
	void UpdateObject(FSObject* po);

	// select an object
	void Select(FSObject* po);

	// select a list of objects
	void SelectObjects(const std::vector<FSObject*>& objList);

	// select an item list
	void SelectItemList(FEItemListBuilder *pitem, bool badd = false);

	// assign the current selection to the currently selected item in the tree
	void AssignCurrentSelection();

	// set the current item
	void SetCurrentItem(int item);
	void SetCurrentItem(CModelTreeItem& item);

	// show the context menu
	void ShowContextMenu(CModelTreeItem* data, QPoint pt);

	// clear selection
	void ClearSelection();

	// set the current selection
	void SetSelection(FSObject* sel);
	void SetSelection(std::vector<FSObject*>& sel);

	// get selected items
	void UpdateSelection();

	// show the model viewer
	void Show();

	// Does the model viewer's tree have the focus
	bool IsFocus();

	bool OnDeleteEvent() override;

	void blockUpdate(bool block);

	void UpdateCurrentItem();

	void SetFilter(int index);

	void IncWarningCount();

	bool IsHighlightSelectionEnabled() const;

public slots:
	void on_modelTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev);
	void on_modelSearch_itemChanged();
	void on_modelTree_itemDoubleClicked(QTreeWidgetItem* item, int column);
	void on_selectButton_clicked();
	void on_deleteButton_clicked();
	void on_searchButton_toggled(bool b);
	void on_syncButton_clicked();
	void on_refreshButton_clicked();
	void on_highlightButton_toggled(bool);
	void on_props_nameChanged(const QString& txt);
	void on_props_selectionChanged();
	void on_props_dataChanged(bool b);
	void on_props_modelChanged();
	void on_filter_currentIndexChanged(int n);
	void on_warnings_clicked();

public slots:
	// slots for model tree context menu actions
	void OnDeleteItem();
	void OnUnhideAllObjects();
	void OnCreateNewMeshLayer();
	void OnDeleteMeshLayer();
	void OnUnhideAllParts();
	void OnDeleteNamedSelection();
	void OnExportFESurface();
	void OnHideObject();
	void OnShowObject();
	void OnSelectObject();
	void OnDeleteAllDiscete();
	void OnShowAllDiscrete();
	void OnHideAllDiscrete();
	void OnSelectDiscreteObject();
	void OnHideDiscreteObject();
	void OnShowDiscreteObject();
	void OnDetachDiscreteObject();
	void OnChangeDiscreteType();
	void OnHidePart();
	void OnSelectPartElements();
	void OnSelectSurfaceFaces();
	void OnShowPart();
	void OnSelectPart();
	void OnSelectSurface();
	void OnSelectCurve();
	void OnSelectNode();
	void OnCopyMaterial();
	void OnCopyInterface();
	void OnCopyBC();
	void OnCopyIC();
	void OnCopyRigidConnector();
	void OnCopyLoad();
	void OnCopyConstraint();
	void OnCopyRigidBC();
	void OnCopyRigidIC();
	void OnCopyRigidLoad();
	void OnCopyStep();
	void OnStepMoveUp();
	void OnStepMoveDown();
	void OnRerunJob();
	void OnOpenJob();
	void OnEditOutput();
	void OnEditOutputLog();
	void OnRemoveEmptySelections();
	void OnRemoveUnusedSelections();
	void OnRemoveUnusedLoadControllers();
	void OnRemoveAllSelections();
	void OnDeleteAllMeshAdaptors();
	void OnChangeMaterial();
	void OnMaterialHideParts();
	void OnMaterialShowParts();
	void OnMaterialHideOtherParts();
	void OnExportMaterials();
	void OnExportAllMaterials();
	void OnImportMaterials(QAction*);
	void OnDeleteAllMaterials();
	void OnSwapContactSurfaces();
	void OnReplaceContactInterface();
	void OnDeleteAllBC();
	void OnDeleteAllLoads();
	void OnDeleteAllIC();
	void OnDeleteAllContact();
	void OnDeleteAllConstraints();
	void OnDeleteAllRigidComponents();
	void OnDeleteAllSteps();
	void OnDeleteAllJobs();
	void OnEditMeshData();
    void OnFindImage();
    void OnExportRawImage();
    void OnExportTIFF();
    void OnExportNRRD();

    void OnAddFiberODFAnalysis();

signals:
	void currentObjectChanged(FSObject* po);

private:
	Ui::CModelViewer*		ui;
	FSObject*				m_currentObject;	// object whose properties are displayed
	std::vector<FSObject*>	m_selection;		// list of selected items
};
