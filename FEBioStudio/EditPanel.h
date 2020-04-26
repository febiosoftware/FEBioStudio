#pragma once
#include "CommandPanel.h"

class CMainWindow;

namespace Ui {
	class CEditPanel;
}

class CEditPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CEditPanel(CMainWindow* wnd, QWidget* parent = 0);

	// update mesh panel
	void Update(bool breset = true) override;

	void Apply() override;

private slots:
	void on_apply_clicked(bool b);
	void on_menu_triggered(QAction* pa);
	void on_buttons_buttonSelected(int n);

private:
	Ui::CEditPanel*	ui;
};
