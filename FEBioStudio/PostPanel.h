#pragma once
#include "CommandPanel.h"

namespace Ui {
	class CPostPanel;
}

class CPostPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CPostPanel(CMainWindow* wnd, QWidget* parent = 0);

	void Update();

	void Apply();

public slots:
	void on_postTab_currentChanged(int index);

private:
	Ui::CPostPanel*	ui;
};
