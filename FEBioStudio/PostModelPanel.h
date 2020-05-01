#pragma once
#include "CommandPanel.h"
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
class CPostModelPanel : public CCommandPanel
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

private slots:
	void on_postModel_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev);
	void on_postModel_itemDoubleClicked(QTreeWidgetItem* item, int column);
	void on_nameEdit_editingFinished();
	void on_deleteButton_clicked();
	void on_props_dataChanged();
	void on_enabled_stateChanged(int nstate);
//	void on_autoUpdate_toggled(bool b);
//	void on_applyButton_clicked();

signals:
	void postObjectStateChanged();
	void postObjectPropsChanged(FSObject* po);

private:
	CPostDocument* GetActiveDocument();

private:
	Ui::CPostModelPanel*				ui;
	std::vector<FSObject*>	m_obj;
};
