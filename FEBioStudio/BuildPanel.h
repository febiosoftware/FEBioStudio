#pragma once
#include "CommandPanel.h"

namespace Ui {
	class CBuildPanel;
}

class CCreatePanel;

class CBuildPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CBuildPanel(CMainWindow* wnd, QWidget* parent = 0);

	CCreatePanel* CreatePanel();

	void Update(bool breset) override;

	void Apply();

	//! Process Esc key event (return true if processed)
	bool OnEscapeEvent();
	bool OnDeleteEvent();

	bool IsEditPanelVisible();
	bool IsMeshPanelVisible();

public slots:
	void on_buildTab_currentChanged(int index);

private:
	Ui::CBuildPanel*	ui;
};
