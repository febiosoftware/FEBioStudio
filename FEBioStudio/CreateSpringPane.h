#pragma once
#include "CreatePanel.h"
#include <vector>
using namespace std;

namespace Ui {
	class CCreateSpringPane;
}

//-----------------------------------------------------------------------------
// create pane for creating spring groups
//
class CCreateSpringPane : public CCreatePane
{
	Q_OBJECT

public:
	CCreateSpringPane(CCreatePanel* parent);

	FEObject* Create();

	void showEvent(QShowEvent* ev);

	void hideEvent(QHideEvent* ev);

private:
	bool getNodeSelection(vector<int>& node, int n);

	bool updateTempObject();

protected slots:
	void on_node1_addButtonClicked();
	void on_node1_subButtonClicked();
	void on_node1_selButtonClicked();
	void on_node1_delButtonClicked();

	void on_node2_addButtonClicked();
	void on_node2_subButtonClicked();
	void on_node2_selButtonClicked();
	void on_node2_delButtonClicked();

	void on_newSet_clicked();

	void on_method_currentIndexChanged(int n);

private:
	Ui::CCreateSpringPane*	ui;
};
