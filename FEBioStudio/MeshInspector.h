#pragma once
#include <QMainWindow>
#include <vector>

namespace Ui {
	class CMeshInspector;
}

class CMainWindow;
class FEMesh;
class GObject;

class CMeshInspector : public QMainWindow
{
	Q_OBJECT

public:
	CMeshInspector(CMainWindow* wnd);

	void Update();

	void showEvent(QShowEvent* ev) override;
	void hideEvent(QHideEvent* ev) override;

private slots:
	void on_var_currentIndexChanged(int n);
	void on_select_clicked();

private:
	void UpdateData(int ndata);

private:
	Ui::CMeshInspector*	ui;
	CMainWindow*	m_wnd;
	GObject*		m_po;
};
