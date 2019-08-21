#pragma once
#include <QWidget>

namespace Ui {
	class CInfoPanel;
}

class FSObject;
class CMainWindow;

class CInfoPanel : public QWidget
{
	Q_OBJECT

public:
	CInfoPanel(CMainWindow* wnd, QWidget* parent = nullptr);

	void SetObject(FSObject* po);

public slots:
	void on_edit_textChanged();
	void on_infoSave_clicked(bool b);
	void on_infoClear_clicked(bool b);

private:
	Ui::CInfoPanel*	ui;
};
