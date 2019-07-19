#pragma once
#include <QWidget>

namespace Ui {
	class CMeshSelectionBox;
}

//-------------------------------------------------------------------
class CMainWindow;

//-------------------------------------------------------------------
// Tool for selecting items of a mesh
class CMeshSelectionBox : public QWidget
{
	Q_OBJECT

public:
	enum 
	{
		ELEM_BUTTON,
		FACE_BUTTON,
		EDGE_BUTTIN,
		NODE_BUTTON
	};

public:
	CMeshSelectionBox(CMainWindow* wnd, QWidget* parent = 0);

	double maxAngle();

	void setMaxAngle(double w);

	void checkMaxAngle(bool b);

	void setSurfaceSelectionMode(bool b);

	void showEvent(QShowEvent* ev) override;

	void hideEvent(QHideEvent* ev) override;

	void setItemMode(int mode);

	void setSelection(int n);

private slots:
	void onButtonClicked(int id);
	void on_hideSelection_triggered(bool b);
	void on_unhideAll_triggered(bool b);
	void on_growSelection_triggered(bool b);
	void on_shrinkSelection_triggered(bool b);
	void on_selectByID_triggered(bool b);
	void on_selectByValue_triggered(bool b);
	void on_selectConnected_clicked(bool b);
	void on_maxAngleCheck_toggled(bool b);
	void on_maxAngle_editingFinished();
	void on_respectPartitions_toggled(bool b);
	void on_ignoreInterior_toggled(bool b);
	void on_selectAndHide_toggled(bool b);
	void on_ignoreBackfacing_toggled(bool b);
	void on_shortestPath_toggled(bool b);

signals:
	void itemModeChanged(int newMode);

private:
	Ui::CMeshSelectionBox*	ui;
};
