#pragma once
#include <QDialog>

namespace Ui {
	class CDlgCheck;
}

class CDlgCheck : public QDialog
{
public:
	CDlgCheck(QWidget* parent);

	void SetWarnings(const QStringList& errList);

private:
	Ui::CDlgCheck*	ui;
};
