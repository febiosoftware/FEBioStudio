#pragma once
#include <QWidget>

class CPostDoc;

namespace Ui {
	class CPostPanel;
}

class CPostPanel : public QWidget
{
	Q_OBJECT

public:
	CPostPanel(QWidget* parent = 0);

	void SetPostDoc(CPostDoc* pd);

private slots:
	void on_slider_changed(int n);
	void on_data_changed(int n);

signals:
	void dataChanged();

private:
	Ui::CPostPanel*	ui;
};
