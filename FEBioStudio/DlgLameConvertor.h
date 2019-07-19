#pragma once
#include <QDialog>

namespace Ui {
	class CDlgLameConvertor;
}

class CDlgLameConvertor : public QDialog
{
	Q_OBJECT

public:
	CDlgLameConvertor(QWidget* parent);

private:
	void UpdateData(int fromIndex, int toIndex);

private slots:
	void fromChanged(int n);
	void toChanged(int n);
	void InputChanged();

private:
	Ui::CDlgLameConvertor*	ui;
};
