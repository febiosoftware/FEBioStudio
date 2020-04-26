#pragma once
#include "CommandPanel.h"
#include <vector>

//-----------------------------------------------------------------------------
class QTreeWidgetItem;
class CPostDoc;

namespace Post {
	class CGLObject;
}

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

	void selectObject(Post::CGLObject* po);

	Post::CGLObject* selectedObject();

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
	void postObjectPropsChanged(Post::CGLObject* po);

private:
	CPostDoc* GetActiveDocument();

private:
	Ui::CPostModelPanel*				ui;
	std::vector<Post::CGLObject*>	m_obj;
};
