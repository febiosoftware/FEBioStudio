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

	void Update();

	void Apply();

	//! Process Esc key event (return true if processed)
	bool OnEscapeEvent();
	bool OnDeleteEvent();

public slots:
	void on_buildTab_currentChanged(int index);

private:
	Ui::CBuildPanel*	ui;
};
