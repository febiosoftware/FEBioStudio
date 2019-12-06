#pragma once
#include "CommandPanel.h"

class CMainWindow;
class CAbstractTool;

namespace Ui {
	class CPostToolsPanel;
}

class CPostToolsPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CPostToolsPanel(CMainWindow* window, QWidget* parent = 0);

	void Update() override;

private:
	void initTools();

	void hideEvent(QHideEvent* event);
	void showEvent(QShowEvent* event);

private slots:
	void on_buttons_buttonClicked(int id);

private:
	Ui::CPostToolsPanel*	ui;
};
