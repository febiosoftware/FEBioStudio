#pragma once
#include "CommandPanel.h"

namespace Ui {
	class CToolsPanel;
}

class CMainWindow;
class CAbstractTool;

class CToolsPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CToolsPanel(CMainWindow* wnd, QWidget* parent = 0);

	// update the tools panel
	void Update(bool breset = true) override;

private:
	void initTools();

	void hideEvent(QHideEvent* event);
	void showEvent(QShowEvent* event);

private slots:
	void on_buttons_buttonClicked(int id);

private:
	Ui::CToolsPanel*	ui;

	CAbstractTool*			m_activeTool;
	QList<CAbstractTool*>	tools;

	friend class Ui::CToolsPanel;
};
