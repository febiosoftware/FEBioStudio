#pragma once
#include <QWidget>

namespace Ui {
	class CInfoPanel;
}

class FSObject;

class CInfoPanel : public QWidget
{
	Q_OBJECT

public:
	CInfoPanel(QWidget* parent = nullptr);

	void SetObject(FSObject* po);

public slots:
	void on_edit_textChanged();

private:
	Ui::CInfoPanel*	ui;
};
