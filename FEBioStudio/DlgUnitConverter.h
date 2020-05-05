#pragma once
#include <QDialog>

namespace Ui {
	class CDlgUnitConverter;
}

class CDlgUnitConverter : public QDialog
{
	Q_OBJECT

public:
	CDlgUnitConverter(QWidget* parent);

public slots:
	void on_quantity_changed();
	void on_from_changed();
	void on_to_changed();
	void on_value_changed();

private:
	void update();

private:
	Ui::CDlgUnitConverter*	ui;
};
