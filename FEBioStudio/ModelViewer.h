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
	void Update() override;

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

	// set the current item
	void SetCurrentItem(int item);

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

	void RefreshProperties();

private slots:
	void on_modelTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev);
	void on_selectButton_clicked();
	void on_deleteButton_clicked();
	void on_searchButton_toggled(bool b);
	void on_syncButton_clicked();
	void on_props_nameChanged(const QString& txt);
	void on_props_selectionChanged();
	void on_props_dataChanged(bool b);

	// slots for model tree context menu actions
	void OnDeleteItem();
	void OnAddMaterial();
	void OnUnhideAllObjects();
	void OnUnhideAllParts();
	void OnAddBC();
	void OnAddSurfaceLoad();
	void OnAddBodyLoad();
	void OnAddInitialCondition();
	void OnAddContact();
	void OnAddConstraint();
	void OnAddRigidConstraint();
	void OnAddRigidConnector();
	void OnAddStep();
	void OnHideObject();
	void OnShowObject();
	void OnSelectObject();
	void OnSelectDiscreteObject();
	void OnDetachDiscreteObject();
	void OnHidePart();
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
	void OnCopyRigidConstraint();
	void OnCopyStep();
	void OnRerunJob();
	void OnEditOutput();
	void OnEditOutputLog();
	void OnRemoveEmptySelections();
	void OnRemoveAllSelections();
	void OnChangeMaterial();
	void OnMaterialHideParts();
	void OnMaterialShowParts();
	void OnMaterialHideOtherParts();
	void OnExportMaterials();
	void OnExportAllMaterials();
	void OnImportMaterials();
	void OnDeleteAllMaterials();
	void OnSwapMasterSlave();
	void OnGenerateMap();
	void OnDeleteAllBC();
	void OnDeleteAllLoads();
	void OnDeleteAllIC();
	void OnDeleteAllContact();
	void OnDeleteAllConstraints();
	void OnDeleteAllRigidConstraints();
	void OnDeleteAllRigidConnectors();
	void OnDeleteAllSteps();

signals:
	void currentObjectChanged(FSObject* po);

private:
	Ui::CModelViewer*		ui;
	FSObject*				m_currentObject;	// object whose properties are displayed
	std::vector<FSObject*>	m_selection;		// list of selected items
};
